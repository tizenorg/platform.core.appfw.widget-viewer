#include <Evas.h>
#include <Pepper_Efl.h>
#include <glib.h>
#include <dlog.h>
#include <unistd.h>
#include <sys/types.h>
#include <aul.h>

#ifndef _E
#define _E LOGE
#endif

#ifndef _D
#define _D LOGD
#endif

#undef LOG_TAG
#define LOG_TAG "WIDGET_TOOLKIT"

#define API __attribute__((visibility("default")))

struct compositor_handler {
	char *app_id;
	void (*cb)(const char *, Evas_Object *, void *);
	void *data;
	Evas_Object *evas_obj;
};

const char *__compositor_name = NULL;
GHashTable *__appid_tbl = NULL;

static void __obj_added_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *added = (Evas_Object *)event_info;
	const char *app_id;
	struct compositor_handler *handler;

	_E("object added");

	if (!added) {
		_E("invalid parameter");
		return;
	}

	app_id = pepper_efl_object_app_id_get(added);
	if (!app_id) {
		_E("can't get app_id of (%d)", pepper_efl_object_pid_get(added));
		return;
	}

	handler = g_hash_table_lookup(__appid_tbl, app_id);
	if (!handler) {
		_E("can't find compositor handler for %s", app_id);
		/* workaround */
		char buf[255] = {0,};
		aul_app_get_appid_bypid(pepper_efl_object_pid_get(added), buf, sizeof(buf));
		handler = g_hash_table_lookup(__appid_tbl, buf);
		if (!handler)
			return;
	}

	handler->evas_obj = added;

	if (handler->cb)
		handler->cb(handler->app_id, added, handler->data);
}

static void __obj_deleted_cb(void *data, Evas_Object *obj, void *event_info)
{
	_E("object removed");
	/* TODO */
}

static void __handler_free(gpointer val)
{
	struct compositor_handler *handler = (struct compositor_handler *)val;

	if (handler) {
		if (handler->app_id)
			g_free(handler->app_id);

		g_free(handler);
	}
}

static void __set_runtime_dir(void)
{
	char buf[256];

	snprintf(buf, sizeof(buf) - 1, "/run/user/%d", getuid());
	if (setenv("XDG_RUNTIME_DIR", buf, 0) < 0) {
		_E("Unable to set XDB_RUNTIME_DIR: %s (%s)\n", buf, strerror(errno));
	}
}

API const char *_compositor_init(Evas_Object *win)
{
	static const char *compositor_name = NULL;
	char app_id[255];

	if (__compositor_name)
		return __compositor_name;

	if (aul_app_get_appid_bypid(getpid(), app_id, sizeof(app_id)) != AUL_R_OK) {
		_E("failed to get appid of pid: %d", getpid());
		return NULL;
	}

	__set_runtime_dir();

	compositor_name = pepper_efl_compositor_create(win, app_id);
	if (compositor_name == NULL) {
		_E("failed to create wayland display");
		return NULL;
	}

	if (setenv("WAYLAND_DISPLAY", compositor_name, 1) < 0) {
		_E("failed to set WAYLAND_DISPLAY: %s\n", strerror(errno));
	}

	evas_object_smart_callback_add(win, PEPPER_EFL_OBJ_ADD, __obj_added_cb, NULL);
	evas_object_smart_callback_add(win, PEPPER_EFL_OBJ_DEL, __obj_deleted_cb, NULL);

	__appid_tbl = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, __handler_free);
	if (!__appid_tbl)
		_E("failed to create table");

	__compositor_name = compositor_name;

	return __compositor_name;
}

API void _compositor_fini()
{
	if (__compositor_name)
		pepper_efl_compositor_destroy(__compositor_name);

	if (__appid_tbl)
		g_hash_table_destroy(__appid_tbl);
}

API int _compositor_set_handler(const char *app_id, void (*cb)(const char *app_id, Evas_Object *obj, void *data), void *data)
{
	struct compositor_handler *handler;
	if (!app_id || !cb) {
		_E("inavlid parameter");
		return -1;
	}

	handler = (struct  compositor_handler *)g_malloc0(sizeof(struct compositor_handler));
	if (!handler) {
		_E("out of memory");
		return -1;
	}

	handler->app_id = g_strdup(app_id);
	handler->cb = cb;
	handler->data = data;

	g_hash_table_insert(__appid_tbl, handler->app_id, handler);

	return 0;
}

API int _compositor_unset_handler(const char *app_id)
{
	g_hash_table_remove(__appid_tbl, app_id);

	return 0;
}

API const char *_compositor_get_title(Evas_Object *obj)
{
	return pepper_efl_object_title_get(obj);
}

API const char *_compositor_get_app_id(Evas_Object *obj)
{
	return pepper_efl_object_app_id_get(obj);
}

API int _compositor_get_pid(Evas_Object *obj)
{
	return pepper_efl_object_pid_get(obj);
}

