/*
 * Copyright 2013  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.1 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://floralicense.org/license/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#include <dlog.h>
#include <dynamicbox_errno.h> /* For error code */

#include "debug.h"
#include "util.h"

int errno;
#if defined(_USE_ECORE_TIME_GET)
static struct {
	clockid_t type;
} s_info = {
	.type = CLOCK_MONOTONIC,
};
#endif

int util_check_extension(const char *filename, const char *check_ptr)
{
	int name_len;

	name_len = strlen(filename);
	while (--name_len >= 0 && *check_ptr) {
		if (filename[name_len] != *check_ptr) {
			return DBOX_STATUS_ERROR_INVALID_PARAMETER;
		}

		check_ptr ++;
	}

	return 0;
}

double util_timestamp(void)
{
#if defined(_USE_ECORE_TIME_GET)
	struct timespec ts;

	do {
		if (clock_gettime(s_info.type, &ts) == 0) {
			return ts.tv_sec + ts.tv_nsec / 1000000000.0f;
		}

		ErrPrint("%d: %s\n", s_info.type, strerror(errno));
		if (s_info.type == CLOCK_MONOTONIC) {
			s_info.type = CLOCK_REALTIME;
		} else if (s_info.type == CLOCK_REALTIME) {
			struct timeval tv;
			if (gettimeofday(&tv, NULL) < 0) {
				ErrPrint("gettimeofday: %s\n", strerror(errno));
				break;
			}

			return tv.tv_sec + tv.tv_usec / 1000000.0f;
		}
	} while (1);

	return 0.0f;
#else
	struct timeval tv;

	if (gettimeofday(&tv, NULL) < 0) {
		ErrPrint("gettimeofday: %s\n", strerror(errno));
		tv.tv_sec = 0;
		tv.tv_usec = 0;
	}

	return (double)tv.tv_sec + (double)tv.tv_usec / 1000000.0f;
#endif
}

const char *util_basename(const char *name)
{
	int length;
	length = name ? strlen(name) : 0;
	if (!length) {
		return ".";
	}

	while (--length > 0 && name[length] != '/');

	return length <= 0 ? name : name + length + (name[length] == '/');
}

static inline int check_native_livebox(const char *pkgname)
{
	int len;
	char *path;

	len = strlen(pkgname) * 2;
	len += strlen("/opt/usr/live/%s/libexec/liblive-%s.so");

	path = malloc(len + 1);
	if (!path) {
		ErrPrint("Heap: %s\n", strerror(errno));
		return DBOX_STATUS_ERROR_OUT_OF_MEMORY;
	}

	snprintf(path, len, "/opt/usr/live/%s/libexec/liblive-%s.so", pkgname, pkgname);
	if (access(path, F_OK | R_OK) != 0) {
		ErrPrint("%s is not a valid package\n", pkgname);
		free(path);
		return DBOX_STATUS_ERROR_INVALID_PARAMETER;
	}

	free(path);
	return 0;
}

static inline int check_web_livebox(const char *pkgname)
{
	int len;
	char *path;

	len = strlen(pkgname) * 2;
	len += strlen("/opt/usr/apps/%s/res/wgt/livebox/index.html");

	path = malloc(len + 1);
	if (!path) {
		ErrPrint("Heap: %s\n", strerror(errno));
		return DBOX_STATUS_ERROR_OUT_OF_MEMORY;
	}

	snprintf(path, len, "/opt/usr/apps/%s/res/wgt/livebox/index.html", pkgname);
	if (access(path, F_OK | R_OK) != 0) {
		ErrPrint("%s is not a valid package\n", pkgname);
		free(path);
		return DBOX_STATUS_ERROR_INVALID_PARAMETER;
	}

	free(path);
	return 0;
}

int util_validate_livebox_package(const char *pkgname)
{
	if (!pkgname) {
		ErrPrint("Invalid argument\n");
		return DBOX_STATUS_ERROR_INVALID_PARAMETER;
	}

	if (!check_native_livebox(pkgname) || !check_web_livebox(pkgname)) {
		return 0;
	}

	return DBOX_STATUS_ERROR_INVALID_PARAMETER;
}

const char *util_uri_to_path(const char *uri)
{
	int len;

	len = strlen(SCHEMA_FILE);
	if (strncasecmp(uri, SCHEMA_FILE, len)) {
		return NULL;
	}

	return uri + len;
}

int util_unlink(const char *filename)
{
	char *descfile;
	int desclen;
	int ret;

	if (!filename) {
		return DBOX_STATUS_ERROR_INVALID_PARAMETER;
	}

	desclen = strlen(filename) + 6; /* .desc */
	descfile = malloc(desclen);
	if (!descfile) {
		ErrPrint("Heap: %s\n", strerror(errno));
		return DBOX_STATUS_ERROR_OUT_OF_MEMORY;
	}

	ret = snprintf(descfile, desclen, "%s.desc", filename);
	if (ret < 0) {
		ErrPrint("Error: %s\n", strerror(errno));
		free(descfile);
		return DBOX_STATUS_ERROR_FAULT;
	}

	(void)unlink(descfile);
	free(descfile);
	(void)unlink(filename);

	return DBOX_STATUS_ERROR_NONE;
}

/* End of a file */
