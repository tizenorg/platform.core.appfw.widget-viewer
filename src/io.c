#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sqlite3.h>

#include <dlog.h>
#include <db-util.h>

#include "dlist.h"
#include "util.h"
#include "debug.h"
#include "io.h"
#include "livebox.h"

static struct {
	sqlite3 *handle;
	const char *dbfile;
} s_info = {
	.handle = NULL,
	.dbfile = "/opt/dbspace/.livebox.db",
};

int io_init(void)
{
	int ret;

	if (s_info.handle)
		return -EALREADY;

	ret = db_util_open(s_info.dbfile, &s_info.handle, DB_UTIL_REGISTER_HOOK_METHOD);
	if (ret != SQLITE_OK) {
		ErrPrint("Failed to open a DB\n");
		return -EIO;
	}

	return 0;
}

int io_fini(void)
{
	if (!s_info.handle)
		return -EINVAL;

	db_util_close(s_info.handle);
	s_info.handle = NULL;
	return 0;
}

char *io_lb_pkgname(const char *appid)
{
	sqlite3_stmt *stmt;
	char *tmp;
	int ret;
	char *pkgname;

	if (!s_info.handle) {
		ErrPrint("IO is not initialized\n");
		return NULL;
	}

	pkgname = NULL;
	ret = sqlite3_prepare_v2(s_info.handle, "SELECT pkgid FROM pkgmap WHERE (appid = ? AND prime = 1) OR pkgid = ?", -1, &stmt, NULL);
	if (ret != SQLITE_OK) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		return NULL;
	}

	ret = sqlite3_bind_text(stmt, 1, appid, -1, NULL);
	if (ret != SQLITE_OK) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		goto out;
	}

	ret = sqlite3_bind_text(stmt, 2, appid, -1, NULL);
	if (ret != SQLITE_OK) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		goto out;
	}

	ret = sqlite3_step(stmt);
	if (ret != SQLITE_ROW) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		goto out;
	}

	tmp = (char *)sqlite3_column_text(stmt, 0);
	if (tmp && strlen(tmp)) {
		pkgname = strdup(tmp);
		if (!pkgname)
			ErrPrint("Heap: %s\n", strerror(errno));
	}

out:
	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);
	return pkgname;
}

char *io_app_pkgname(const char *lbpkg)
{
	sqlite3_stmt *stmt;
	char *tmp;
	int ret;
	char *pkgname;

	if (!s_info.handle) {
		ErrPrint("IO is not initialized\n");
		return NULL;
	}

	pkgname = NULL;
	ret = sqlite3_prepare_v2(s_info.handle, "SELECT appid FROM pkgmap WHERE appid = ? OR (pkgid = ? AND prime = 1)", -1, &stmt, NULL);
	if (ret != SQLITE_OK) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		return NULL;
	}

	ret = sqlite3_bind_text(stmt, 1, lbpkg, -1, NULL);
	if (ret != SQLITE_OK) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		goto out;
	}

	ret = sqlite3_bind_text(stmt, 2, lbpkg, -1, NULL);
	if (ret != SQLITE_OK) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		goto out;
	}

	ret = sqlite3_step(stmt);
	if (ret != SQLITE_ROW) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		goto out;
	}

	tmp = (char *)sqlite3_column_text(stmt, 0);
	if (tmp && strlen(tmp)) {
		pkgname = strdup(tmp);
		if (!pkgname)
			ErrPrint("Heap: %s\n", strerror(errno));
	}

out:
	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);
	return pkgname;
}

int io_enumerate_cluster_list(int (*cb)(const char *cluster, void *data), void *data)
{
	sqlite3_stmt *stmt;
	const char *cluster;
	int cnt;
	int ret;

	if (!s_info.handle)
		return -EIO;

	cnt = 0;
	ret = sqlite3_prepare_v2(s_info.handle, "SELECT DISTINCT cluster FROM groupinfo", -1, &stmt, NULL);
	if (ret != SQLITE_OK) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));	
		return -EIO;
	}

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		cluster = (const char *)sqlite3_column_text(stmt, 0);
		if (!cluster || !strlen(cluster))
			continue;

		if (cb(cluster, data) < 0)
			break;

		cnt++;
	}

	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);
	return cnt;
}

int io_enumerate_category_list(const char *cluster, int (*cb)(const char *cluster, const char *category, void *data), void *data)
{
	sqlite3_stmt *stmt;
	const char *category;
	int cnt;
	int ret;

	if (!s_info.handle)
		return -EIO;

	cnt = 0;
	ret = sqlite3_prepare_v2(s_info.handle, "SELECT DISTINCT category FROM groupinfo WHERE cluster = ?", -1, &stmt, NULL);
	if (ret != SQLITE_OK) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		return -EIO;
	}

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		category = (const char *)sqlite3_column_text(stmt, 0);
		if (!category || !strlen(category))
			continue;

		if (cb(cluster, category, data) < 0)
			break;

		cnt++;
	}

	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);
	return cnt;
}

int io_get_supported_sizes(const char *pkgid, int *cnt, int *w, int *h)
{
	sqlite3_stmt *stmt;
	int size;
	int ret;
	int idx;

	if (!s_info.handle) {
		ErrPrint("IO is not initialized\n");
		return -EINVAL;
	}

	ret = sqlite3_prepare_v2(s_info.handle, "SELECT size_type FROM box_size WHERE pkgid = ? ORDER BY size_type ASC", -1, &stmt, NULL);
	if (ret != SQLITE_OK) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		ret = -EIO;
		goto out;
	}

	ret = sqlite3_bind_text(stmt, 1, pkgid, -1, NULL);
	if (ret != SQLITE_OK) {
		ErrPrint("Error: %s\n", sqlite3_errmsg(s_info.handle));
		sqlite3_reset(stmt);
		sqlite3_finalize(stmt);
		ret = -EIO;
		goto out;
	}

	ret = 0;
	while (sqlite3_step(stmt) == SQLITE_ROW && ret < *cnt) {
		size = sqlite3_column_int(stmt, 0);
		switch (size) {
		case 0x01:
			idx = 0;
			break;
		case 0x02:
			idx = 1;
			break;
		case 0x04:
			idx = 2;
			break;
		case 0x08:
			idx = 3;
			break;
		case 0x10:
			idx = 4;
			break;
		case 0x20:
			idx = 5;
			break;
		default:
			ErrPrint("Invalid size type: %d\n", size);
			continue;
		}

		if (w)
			w[ret] = SIZE_LIST[idx].w;
		if (h)
			h[ret] = SIZE_LIST[idx].h;

		ret++;
	}

	*cnt = ret;
	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);
	ret = 0;
out:
	return ret;
}

/* End of a file */
