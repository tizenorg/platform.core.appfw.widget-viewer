#include <stdio.h>
#include <errno.h>
#include <stdlib.h> /* malloc */
#include <string.h> /* strdup */
#include <libgen.h>

#include <gio/gio.h>

#include <aul.h>
#include <dlog.h>

#include "debug.h"
#include "fb_file.h"
#include "livebox.h"
#include "livebox0.h"
#include "dlist.h"
#include "util.h"
#include "dbus.h"

#define EAPI __attribute__((visibility("default")))

#if defined(FLOG)
FILE *__file_log_fp;
#endif

struct info {
	struct dlist *livebox_list;
	struct dlist *event_list;
	struct dlist *fault_list;
} s_info = {
	.livebox_list = NULL,
	.event_list = NULL,
	.fault_list = NULL,
};

struct event_info {
	int (*handler)(struct livebox *handler, const char *event, void *data);
	void *user_data;
};

struct fault_info {
	int (*handler)(const char *event, const char *pkgname, const char *filename, const char *func, void *data);
	void *user_data;
};

EAPI int livebox_init(void)
{
#if defined(FLOG)
	char filename[BUFSIZ];
	snprintf(filename, sizeof(filename), "/tmp/%d.box.log", getpid());
	__file_log_fp = fopen(filename, "w+t");
	if (!__file_log_fp)
		__file_log_fp = fdopen(1, "w+t");
#endif

	dbus_init();
	return 0;
}

EAPI int livebox_fini(void)
{
	dbus_fini();
	return 0;
}

static void event_ret_cb(struct livebox *handler, int ret, void *data)
{
	if (ret < 0) {
		lb_invoke_event_handler(handler, "event,ignored");
	} else {
	}
}

static void ret_cb(struct livebox *handler, int ret, void *data)
{
	if (ret < 0) {
		lb_invoke_event_handler(handler, "lb,deleted");
		livebox_del(handler, 0);
	} else {
	}
}

EAPI struct livebox *livebox_add(const char *pkgname, const char *content, const char *cluster, const char *category)
{
	struct livebox *handler;
	GVariant *param;
	int ret;

	if (!pkgname || !cluster || !category)
		return NULL;

	handler = calloc(1, sizeof(*handler));
	if (!handler) {
		ErrPrint("Error: %s\n", strerror(errno));
		return NULL;
	}

	handler->pkgname = strdup(pkgname);
	if (!handler->pkgname) {
		ErrPrint("Error: %s\n", strerror(errno));
		free(handler);
		return NULL;
	}

	if (content) {
		handler->content = strdup(content);
		if (!handler->content) {
			ErrPrint("Error: %s\n", strerror(errno));
			free(handler->pkgname);
			free(handler);
			return NULL;
		}
	}

	handler->cluster = strdup(cluster);
	if (!handler->cluster) {
		ErrPrint("Error: %s\n", strerror(errno));
		free(handler->content);
		free(handler->pkgname);
		free(handler);
		return NULL;
	}

	handler->category = strdup(category);
	if (!handler->category) {
		ErrPrint("Error: %s\n", strerror(errno));
		free(handler->cluster);
		free(handler->content);
		free(handler->pkgname);
		free(handler);
		return NULL;
	}

	/* Data provider will set this */
	handler->data_type = FILEDATA;

	/* Cluster infomration is not determined yet */
	handler->nr_of_sizes = 0x01;

	handler->timestamp = util_get_timestamp();

	s_info.livebox_list = dlist_append(s_info.livebox_list, handler);

	param = g_variant_new("(dssss)", handler->timestamp, pkgname, content, cluster, category);
	if (!param) {
		free(handler->category);
		free(handler->cluster);
		free(handler->content);
		free(handler->pkgname);
		free(handler);
		return NULL;
	}

	ret = dbus_push_command(handler, "new", param, ret_cb, handler);
	if (ret < 0) {
		free(handler->category);
		free(handler->cluster);
		free(handler->content);
		free(handler->pkgname);
		free(handler);
		g_variant_unref(param);
		return NULL;
	}

	return handler;
}

EAPI int livebox_del(struct livebox *handler, int server)
{
	if (!handler)
		return -EINVAL;

	if (server) {
		GVariant *param;
		int ret;

		param = g_variant_new("(ss)", handler->pkgname, handler->filename);
		if (!param)
			return -EFAULT;

		ret = dbus_push_command(handler, "delete", param, ret_cb, handler);
		if (ret < 0) {
			g_variant_unref(param);
			return ret;
		}

		return 0;
	}

	dlist_remove_data(s_info.livebox_list, handler);

	free(handler->cluster);
	free(handler->category);
	free(handler->filename);
	free(handler->pkgname);

	if (handler->lb_fb) {
		fb_destroy_buffer(handler->lb_fb);
		fb_destroy(handler->lb_fb);
		handler->lb_fb = NULL;
	}

	if (handler->pd_fb) {
		fb_destroy_buffer(handler->pd_fb);
		fb_destroy(handler->pd_fb);
		handler->pd_fb = NULL;
	}

	return 0;
}

EAPI int livebox_fault_handler_set(int (*cb)(const char *, const char *, const char *, const char *, void *), void *data)
{
	struct fault_info *info;

	if (!cb)
		return -EINVAL;

	info = malloc(sizeof(*info));
	if (!info)
		return -ENOMEM;

	info->handler = cb;
	info->user_data = data;

	s_info.fault_list = dlist_append(s_info.fault_list, info);
	return 0;
}

EAPI void *livebox_fault_handler_unset(int (*cb)(const char *, const char *, const char *, const char *, void *))
{
	struct fault_info *info;
	struct dlist *l;

	dlist_foreach(s_info.fault_list, l, info) {
		if (info->handler == cb) {
			void *data;
			s_info.fault_list = dlist_remove(s_info.fault_list, l);
			data = info->user_data;
			free(info);

			return data;
		}
	}

	return NULL;
}

EAPI int livebox_event_handler_set(int (*cb)(struct livebox *, const char *, void *), void *data)
{
	struct event_info *info;

	DbgPrint("event callback adding\n");
	if (!cb)
		return -EINVAL;

	DbgPrint("event callback cb found, adding\n");
	info = malloc(sizeof(*info));
	if (!info)
		return -ENOMEM;

	info->handler = cb;
	info->user_data = data;

	s_info.event_list = dlist_append(s_info.event_list, info);
	DbgPrint("event callback added\n");
	return 0;
}

EAPI void *livebox_event_handler_unset(int (*cb)(struct livebox *, const char *, void *))
{
	struct event_info *info;
	struct dlist *l;

	dlist_foreach(s_info.event_list, l, info) {
		if (info->handler == cb) {
			void *data;

			s_info.event_list = dlist_remove(s_info.event_list, l);
			data = info->user_data;
			free(info);

			return data;
		}
	}

	return NULL;
}

EAPI int livebox_resize(struct livebox *handler, int w, int h)
{
	GVariant *param;
	int ret;

	if (!handler)
		return -EINVAL;

	param = g_variant_new("(ssii)", handler->pkgname, handler->filename, w, h);
	if (!param)
		return -EFAULT;

	ret = dbus_push_command(handler, "resize", param, event_ret_cb, handler);
	if (ret < 0)
		g_variant_unref(param);

	return ret;
}

EAPI int livebox_click(struct livebox *handler, double x, double y)
{
	GVariant *param;
	double timestamp;
	int ret;

	if (!handler)
		return -EINVAL;

	if (handler->auto_launch)
		if (aul_launch_app(handler->pkgname, NULL) < 0)
			ErrPrint("Failed to launch app %s\n", handler->pkgname);

	timestamp = util_get_timestamp();
	param = g_variant_new("(sssddd)", handler->pkgname, handler->filename, "clicked", timestamp, x, y);
	if (!param)
		return -EFAULT;

	ret = dbus_push_command(handler, "clicked", param, event_ret_cb, handler);
	if (ret < 0)
		g_variant_unref(param);

	return ret;
}

static inline int send_mouse_event(struct livebox *handler, const char *event, double x, double y)
{
	GVariant *param;
	double timestamp;
	int ret;

	timestamp = util_get_timestamp();
	param = g_variant_new("(ssiiddd)", handler->pkgname, handler->filename,
						handler->pd_w, handler->pd_h,
						timestamp, x, y);
	if (!param)
		return -EFAULT;

	ret = dbus_push_command(handler, event, param, event_ret_cb, handler);
	if (ret < 0)
		g_variant_unref(param);

	return ret;
}

EAPI int livebox_has_pd(struct livebox *handler)
{
	if (!handler)
		return -EINVAL;

	return !!handler->pd_fb;
}

EAPI int livebox_pd_is_created(struct livebox *handler)
{
	if (!handler || !handler->pd_fb)
		return -EINVAL;

	return fb_is_created(handler->pd_fb);
}

static void pd_created_cb(struct livebox *handler, int ret, void *data)
{
	if (ret == 0) {
		fb_create_buffer(data);
		lb_invoke_event_handler(handler, "pd,created");
	} else {
		lb_invoke_event_handler(handler, "pd,create,failed");
	}
}

static void activated_cb(struct livebox *handler, int ret, void *data)
{
	char *pkgname = data;

	if (ret == 0)
		lb_invoke_fault_handler("activated", pkgname, NULL, NULL);
	else if (ret == -EINVAL)
		lb_invoke_fault_handler("invalid,request", pkgname, NULL, NULL);
	else
		lb_invoke_fault_handler("activation,failed", pkgname, NULL, NULL);

	free(pkgname);
}

EAPI int livebox_create_pd(struct livebox *handler)
{
	GVariant *param;
	int ret;

	if (!handler || !handler->pd_fb)
		return -EINVAL;

	if (fb_is_created(handler->pd_fb) == 1)
		return 0;

	param = g_variant_new("(ss)", handler->pkgname, handler->filename);
	if (!param)
		return -EFAULT;

	ret = dbus_push_command(handler, "create_pd", param, pd_created_cb, handler->pd_fb);
	if (ret < 0)
		g_variant_unref(param);

	return ret;
}

EAPI int livebox_activate(const char *pkgname)
{
	GVariant *param;
	int ret;

	if (!pkgname)
		return -EINVAL;

	param = g_variant_new("(s)", pkgname);
	if (!param)
		return -EFAULT;

	ret = dbus_push_command(NULL, "activate_package", param, activated_cb, strdup(pkgname));
	if (ret < 0)
		g_variant_unref(param);

	return ret;
}

static void pd_destroy_cb(struct livebox *handler, int ret, void *data)
{
	DbgPrint("destroy_pd returns %d\n", ret);
	fb_destroy_buffer(data);
	lb_invoke_event_handler(handler, "pd,deleted");
}

EAPI int livebox_destroy_pd(struct livebox *handler)
{
	GVariant *param;
	int ret;

	if (!handler || !handler->pd_fb)
		return -EINVAL;

	if (fb_is_created(handler->pd_fb) != 1)
		return -EINVAL;

	param = g_variant_new("(ss)", handler->pkgname, handler->filename);
	if (!param)
		return -EFAULT;

	ret = dbus_push_command(handler, "destroy_pd", param, pd_destroy_cb, handler->pd_fb);
	if (ret < 0)
		g_variant_unref(param);

	return ret;
}

EAPI int livebox_pd_mouse_down(struct livebox *handler, double x, double y)
{
	if (!handler || !handler->pd_fb)
		return -EINVAL;

	return send_mouse_event(handler, "pd_mouse_down", x, y);
}

EAPI int livebox_pd_mouse_up(struct livebox *handler, double x, double y)
{
	if (!handler || !handler->pd_fb)
		return -EINVAL;

	return send_mouse_event(handler, "pd_mouse_up", x, y);
}

EAPI int livebox_pd_mouse_move(struct livebox *handler, double x, double y)
{
	if (!handler || !handler->pd_fb)
		return -EINVAL;

	return send_mouse_event(handler, "pd_mouse_move", x, y);
}

EAPI int livebox_livebox_mouse_down(struct livebox *handler, double x, double y)
{
	if (!handler || !handler->lb_fb)
		return -EINVAL;

	return send_mouse_event(handler, "lb_mouse_down", x, y);
}

EAPI int livebox_livebox_mouse_up(struct livebox *handler, double x, double y)
{
	if (!handler || !handler->lb_fb)
		return -EINVAL;

	return send_mouse_event(handler, "lb_mouse_up", x, y);
}

EAPI int livebox_livebox_mouse_move(struct livebox *handler, double x, double y)
{
	if (!handler || !handler->lb_fb)
		return -EINVAL;

	return send_mouse_event(handler, "lb_mouse_move", x, y);
}

EAPI const char *livebox_filename(struct livebox *handler)
{
	if (!handler)
		return NULL;

	return handler->filename;
}

EAPI int livebox_get_pdsize(struct livebox *handler, int *w, int *h)
{
	int _w;
	int _h;

	if (!handler)
		return -EINVAL;

	if (!w)
		w = &_w;
	if (!h)
		h = &_h;

	*w = handler->pd_w;
	*h = handler->pd_h;
	return 0;
}

EAPI int livebox_get_size(struct livebox *handler, int *w, int *h)
{
	int _w;
	int _h;

	if (!handler)
		return -EINVAL;

	if (!w)
		w = &_w;
	if (!h)
		h = &_h;

	*w = handler->lb_w;
	*h = handler->lb_h;
	return 0;
}

EAPI int livebox_set_group(struct livebox *handler, const char *cluster, const char *category)
{
	GVariant *param;
	int ret;

	if (!handler || !cluster || !category)
		return -EINVAL;

	param = g_variant_new("(ssss)", handler->pkgname, handler->filename, cluster, category);
	if (!param)
		return -EFAULT;

	ret = dbus_push_command(handler, "change_group", param, event_ret_cb, handler);
	if (ret < 0)
		g_variant_unref(param);

	return ret;
}

EAPI int livebox_get_group(struct livebox *handler, char ** const cluster, char ** const category)
{
	if (!handler || !cluster || !category)
		return -EINVAL;

	*cluster = handler->cluster;
	*category = handler->category;
	return 0;
}

EAPI int livebox_get_supported_sizes(struct livebox *handler, int *cnt, int *w, int *h)
{
	register int i;
	register int j;

	if (!handler || !cnt)
		return -EINVAL;

	for (j = i = 0; i < NR_OF_SIZE_LIST; i++) {
		if (handler->size_list & (0x01 << i)) {
			if (j == *cnt)
				break;

			if (w)
				w[j] = SIZE_LIST[i].w;
			if (h)
				h[j] = SIZE_LIST[i].h;
			j++;
		}
	}

	*cnt = j;
	return 0;
}

EAPI const char *livebox_pkgname(struct livebox *handler)
{
	if (!handler)
		return NULL;

	return handler->pkgname;
}

EAPI double livebox_priority(struct livebox *handler)
{
	if (!handler)
		return 0.0f;

	return handler->priority;
}

EAPI int livebox_delete_cluster(const char *cluster)
{
	return -ENOSYS;
}

EAPI int livebox_delete_category(const char *cluster, const char *category)
{
	return -ENOSYS;
}

EAPI int livebox_is_file(struct livebox *handler)
{
	if (!handler)
		return -EINVAL;

	return handler->data_type == FILEDATA;
}

EAPI int livebox_is_text(struct livebox *handler)
{
	if (!handler)
		return -EINVAL;

	return handler->text_lb;
}

EAPI int livebox_pd_is_text(struct livebox *handler)
{
	if (!handler)
		return -EINVAL;

	return handler->text_pd;
}

EAPI int livebox_pd_set_text_handler(struct livebox *handler, struct livebox_script_operators *ops)
{
	if (!handler)
		return -EINVAL;

	memcpy(&handler->pd_ops, ops, sizeof(*ops));
	return 0;
}

EAPI int livebox_set_text_handler(struct livebox *handler, struct livebox_script_operators *ops)
{
	if (!handler)
		return -EINVAL;

	memcpy(&handler->ops, ops, sizeof(*ops));
	return 0;
}

EAPI void *livebox_fb(struct livebox *handler)
{
	if (!handler)
		return NULL;

	if (handler->data_type == FBDATA)
		return fb_buffer(handler->lb_fb);

	return NULL;
}

EAPI void *livebox_pdfb(struct livebox *handler)
{
	if (!handler)
		return NULL;

	return fb_buffer(handler->pd_fb);
}

EAPI int livebox_pdfb_bufsz(struct livebox *handler)
{
	if (!handler)
		return -EINVAL;

	return fb_size(handler->pd_fb);
}

EAPI int livebox_lbfb_bufsz(struct livebox *handler)
{
	if (!handler)
		return -EINVAL;

	return fb_size(handler->lb_fb);
}

EAPI int livebox_is_user(struct livebox *handler)
{
	if (!handler)
		return -EINVAL;

	return handler->is_user;
}

static void pinup_done_cb(struct livebox *handler, int ret, void *data)
{
	if (ret != 0) {
		ErrPrint("Pinup is not changed: %s\n", strerror(ret));
		lb_invoke_event_handler(handler, "pinup,failed");
	} else {
		handler->is_pinned_up = (int)data;
		lb_invoke_event_handler(handler, "pinup,changed");
	}
}

EAPI int livebox_set_pinup(struct livebox *handler, int flag)
{
	GVariant *param;
	int ret;

	if (!handler)
		return -EINVAL;

	if (handler->is_pinned_up == flag)
		return 0;

	param = g_variant_new("(ssi)", handler->pkgname, handler->filename, flag);
	if (!param)
		return -EFAULT;

	ret = dbus_push_command(handler, "pinup_changed", param, pinup_done_cb, (void *)flag);
	if (ret < 0)
		g_variant_unref(param);

	return ret;
}

EAPI int livebox_pinup(struct livebox *handler)
{
	return handler->is_pinned_up;
}

EAPI int livebox_has_pinup(struct livebox *handler)
{
	return handler->pinup_supported;
}

EAPI int livebox_set_data(struct livebox *handler, void *data)
{
	if (!handler)
		return -EINVAL;

	DbgPrint("%p carry data %p\n", handler, data);
	handler->data = data;
	return 0;
}

EAPI void *livebox_get_data(struct livebox *handler)
{
	if (!handler)
		return NULL;

	DbgPrint("Get carried data of %p\n", handler);
	return handler->data;
}

EAPI int livebox_is_exists(const char *pkgname)
{
	GVariant *param;
	int ret;

	param = g_variant_new("(s)", pkgname);
	if (!param)
		return -EFAULT;

	ret = dbus_sync_command("livebox_is_exists", param);
	return (ret == 0) ? 1 : ret;
}

EAPI int livebox_text_emit_signal(struct livebox *handler, const char *emission, const char *source)
{
	GVariant *param;
	int ret;

	if (!handler->text_lb && !handler->text_pd)
		return -EINVAL;

	if (!emission)
		emission = "";

	if (!source)
		source = "";

	param = g_variant_new("(ssss)", handler->pkgname, handler->filename, emission, source);
	ret = dbus_push_command(handler, "text_signal", param, event_ret_cb, handler);
	if (ret < 0) {
		g_variant_unref(param);
		return -EIO;
	}

	return 0;
}

int lb_set_group(struct livebox *handler, const char *cluster, const char *category)
{
	void *pc;
	void *ps;

	pc = strdup(cluster);
	if (!pc)
		return -ENOMEM;

	ps = strdup(category);
	if (!ps) {
		free(pc);
		return -ENOMEM;
	}

	if (handler->cluster)
		free(handler->cluster);

	if (handler->category)
		free(handler->category);

	handler->cluster = pc;
	handler->category = ps;

	return 0;
}

int lb_set_size(struct livebox *handler, int w, int h)
{
	handler->lb_w = w;
	handler->lb_h = h;
	return 0;
}

int lb_set_pdsize(struct livebox *handler, int w, int h)
{
	handler->pd_w = w;
	handler->pd_h = h;
	return 0;
}

int lb_invoke_fault_handler(const char *event, const char *pkgname, const char *file, const char *func)
{
	struct dlist *l;
	struct dlist *n;
	struct fault_info *info;

	dlist_foreach_safe(s_info.fault_list, l, n, info) {
		if (info->handler(event, pkgname, file, func, info->user_data) == EXIT_FAILURE)
			s_info.fault_list = dlist_remove(s_info.fault_list, l);
	}

	return 0;
}

int lb_invoke_event_handler(struct livebox *handler, const char *event)
{
	struct dlist *l;
	struct dlist *n;
	struct event_info *info;

	dlist_foreach_safe(s_info.event_list, l, n, info) {
		if (info->handler(handler, event, info->user_data) == EXIT_FAILURE)
			s_info.event_list = dlist_remove(s_info.event_list, l);
	}

	return 0;
}

struct livebox *lb_find_livebox(const char *pkgname, const char *filename)
{
	struct dlist *l;
	struct livebox *handler;

	dlist_foreach(s_info.livebox_list, l, handler) {
		if (!handler->filename)
			continue;

		if (!strcmp(handler->pkgname, pkgname) && !strcmp(handler->filename, filename))
			return handler;
	}

	return NULL;
}

struct livebox *lb_find_livebox_by_timestamp(double timestamp)
{
	struct dlist *l;
	struct livebox *handler;

	dlist_foreach(s_info.livebox_list, l, handler) {
		if (handler->timestamp == timestamp)
			return handler;
	}

	return NULL;
}

struct livebox *lb_new_livebox(const char *pkgname, const char *filename)
{
	struct livebox *handler;

	handler = calloc(1, sizeof(*handler));
	if (!handler) {
		ErrPrint("Failed to create a new livebox\n");
		return NULL;
	}

	handler->pkgname = strdup(pkgname);
	if (!handler->pkgname) {
		ErrPrint("%s\n", strerror(errno));
		free(handler);
		return NULL;
	}

	handler->filename = strdup(filename);
	if (!handler->filename) {
		ErrPrint("%s\n", strerror(errno));
		free(handler->pkgname);
		free(handler);
		return NULL;
	}

	s_info.livebox_list = dlist_append(s_info.livebox_list, handler);
	return handler;
}

int lb_set_content(struct livebox *handler, const char *content)
{
	if (handler->content) {
		free(handler->content);
		handler->content = NULL;
	}

	if (content) {
		handler->content = strdup(content);
		if (!handler->content)
			return -ENOMEM;
	}

	return 0;
}

void lb_set_size_list(struct livebox *handler, int size_list)
{
	handler->size_list = size_list;
}

int lb_set_auto_launch(struct livebox *handler, int auto_launch)
{
	handler->auto_launch = auto_launch;
	return 0;
}

void lb_set_priority(struct livebox *handler, double priority)
{
	handler->priority = priority;
}

void lb_set_filename(struct livebox *handler, const char *filename)
{
	if (handler->filename)
		free(handler->filename);

	handler->filename = strdup(filename);
	if (!handler->filename)
		ErrPrint("Error: %s\n", strerror(errno));
}

void lb_update_lb_fb(struct livebox *handler, int w, int h)
{
	int ow;
	int oh;
	char *filename;
	const char *tmp;

	if (!handler)
		return;

	if (!handler->lb_fb)
		return;

	fb_get_size(handler->lb_fb, &ow, &oh);
	if (ow == w && oh == h) {
		DbgPrint("Buffer size is not changed\n");
		return;
	}

	tmp = fb_filename(handler->lb_fb);
	if (!tmp) {
		ErrPrint("Filename for LB fb is not exists\n");
		return;
	}

	filename = strdup(tmp);
	if (!filename) {
		ErrPrint("Error: %s\n", strerror(errno));
		return;
	}

	fb_destroy_buffer(handler->lb_fb);
	fb_destroy(handler->lb_fb);

	handler->lb_fb = fb_create(filename, w, h);
	free(filename);
	if (!handler->lb_fb) {
		ErrPrint("Faield to create a FB\n");
		return;
	}

	if (fb_create_buffer(handler->lb_fb) < 0) {
		ErrPrint("Failed to create a FB\n");
		fb_destroy(handler->lb_fb);
		handler->lb_fb = NULL;
		return;
	}
}

void lb_update_pd_fb(struct livebox *handler, int w, int h)
{
	int ow;
	int oh;
	char *filename;
	const char *tmp;
	int ret;

	if (!handler)
		return;

	if (!handler->pd_fb)
		return;

	fb_get_size(handler->pd_fb, &ow, &oh);
	if (ow == w && oh == h) {
		DbgPrint("Buffer size is not changed\n");
		return;
	}

	tmp = fb_filename(handler->pd_fb);
	if (!tmp) {
		ErrPrint("PD fb has no file\n");
		return;
	}

	filename = strdup(tmp);
	if (!filename) {
		ErrPrint("Error: %s\n", strerror(errno));
		return;
	}

	fb_destroy_buffer(handler->pd_fb);
	fb_destroy(handler->pd_fb);

	handler->pd_fb = fb_create(filename, w, h);
	free(filename);
	if (!handler->pd_fb) {
		ErrPrint("Failed to create a FB\n");
		return;
	}

	ret = fb_create_buffer(handler->pd_fb);
	if (ret < 0) {
		ErrPrint("Error: %s\n", strerror(ret));
		fb_destroy(handler->pd_fb);
		handler->pd_fb = NULL;
	}
}

void lb_set_lb_fb(struct livebox *handler, const char *filename, int w, int h)
{
	if (!handler)
		return;

	if (handler->lb_fb) {
		fb_destroy_buffer(handler->lb_fb);
		fb_destroy(handler->lb_fb);
		handler->lb_fb = NULL;
	}

	if (!filename || filename[0] == '\0')
		return;

	handler->lb_fb = fb_create(filename, w, h);
	if (!handler->lb_fb) {
		ErrPrint("Faield to create a FB\n");
		return;
	}

	if (fb_create_buffer(handler->lb_fb) < 0) {
		fb_destroy(handler->lb_fb);
		handler->lb_fb = NULL;
		ErrPrint("Failed to create frame buffer\n");
		return;
	}

	handler->data_type = FBDATA;
}

void lb_set_pd_fb(struct livebox *handler, const char *filename, int w, int h)
{
	if (!handler)
		return;

	if (handler->pd_fb) {
		fb_destroy_buffer(handler->pd_fb);
		fb_destroy(handler->pd_fb);
		handler->pd_fb = NULL;
	}

	if (!filename || filename[0] == '\0')
		return;

	handler->pd_fb = fb_create(filename, w, h);
	if (!handler->pd_fb) {
		ErrPrint("Failed to create a FB\n");
		return;
	}
}

struct fb_info *lb_get_lb_fb(struct livebox *handler)
{
	return handler->lb_fb;
}

struct fb_info *lb_get_pd_fb(struct livebox *handler)
{
	return handler->pd_fb;
}

void lb_set_user(struct livebox *handler, int user)
{
	handler->is_user = user;
}

void lb_set_pinup(struct livebox *handler, int pinup_supported)
{
	handler->pinup_supported = pinup_supported;
}

void lb_set_text_lb(struct livebox *handler, int flag)
{
	handler->text_lb = flag;
}

void lb_set_text_pd(struct livebox *handler, int flag)
{
	handler->text_pd = flag;
}

int lb_text_lb(struct livebox *handler)
{
	return handler->text_lb;
}

int lb_text_pd(struct livebox *handler)
{
	return handler->text_pd;
}

/* End of a file */
