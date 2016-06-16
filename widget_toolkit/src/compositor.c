/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Flora License, Version 1.1 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://floralicense.org/license/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <Evas.h>
#include <Pepper_Efl.h>
#include <glib.h>
#include <dlog.h>
#include <unistd.h>
#include <sys/types.h>
#include <aul.h>
#include "compositor.h"
#include <Ecore_Wayland.h>
#include <Elementary.h>

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
	_compositor_handler_cb cb;
	void *data;
	Evas_Object *evas_obj;
	int freeze;
};

const char *__compositor_name = NULL;
GHashTable *__appid_tbl = NULL;
GHashTable *__evas_tbl = NULL;
Ecore_Wl_Window *__window = NULL;
unsigned int __window_id = 0;
Ecore_Event_Handler *__visibility_listener = NULL;

static void __obj_added_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *added = (Evas_Object *)event_info;
	const char *app_id;
	struct compositor_handler *handler;

	_D("object added");

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
		int pid;

		pid = pepper_efl_object_pid_get(added);
		aul_app_get_appid_bypid(pid, buf, sizeof(buf));
		_E("found appid:%s with pid:%d", buf, pid);
		handler = g_hash_table_lookup(__appid_tbl, buf);
		if (!handler)
			return;
	}

	handler->evas_obj = added;
	g_hash_table_insert(__evas_tbl, handler->evas_obj, handler);

	if (handler->cb)
		handler->cb(handler->app_id, "added", added, handler->data);
}

static void __obj_deleted_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *removed = (Evas_Object *)event_info;
	const char *app_id;
	struct compositor_handler *handler;

	_D("object removed");

	if (!removed) {
		_E("invalid parameter");
		return;
	}

	app_id = pepper_efl_object_app_id_get((Evas_Object *)event_info);

	if (app_id) {
		_D("get object:%s", app_id);
		handler = g_hash_table_lookup(__appid_tbl, app_id);
	} else {
		handler = g_hash_table_lookup(__evas_tbl, removed);
		if (handler) {
			_D("get object:%s", handler->app_id);
			g_hash_table_remove(__evas_tbl, removed);
			handler->evas_obj = NULL;
		}
	}

	if (!handler) {
		_E("can't find compositor handler for %s", app_id);
		/* workaround */
		char buf[255] = {0,};
		aul_app_get_appid_bypid(pepper_efl_object_pid_get(removed), buf, sizeof(buf));
		handler = g_hash_table_lookup(__appid_tbl, buf);
		if (!handler)
			return;
	}

	if (handler->cb)
		handler->cb(handler->app_id, "removed", removed, handler->data);
}

static void __handler_free(gpointer val)
{
	struct compositor_handler *handler = (struct compositor_handler *)val;

	if (handler) {
		if (handler->evas_obj)
			g_hash_table_remove(__evas_tbl, handler->evas_obj);

		if (handler->app_id)
			g_free(handler->app_id);

		g_free(handler);
	}
}

static void __set_runtime_dir(void)
{
	char buf[256];

	snprintf(buf, sizeof(buf) - 1, "/run/user/%d", getuid());
	if (setenv("XDG_RUNTIME_DIR", buf, 0) < 0)
		_E("Unable to set XDB_RUNTIME_DIR: %s (%s)\n", buf, strerror(errno));
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

	__window = elm_win_wl_window_get(win);
	__window_id = ecore_wl_window_id_get(__window);

	__set_runtime_dir();

	compositor_name = pepper_efl_compositor_create(win, app_id);
	if (compositor_name == NULL) {
		_E("failed to create wayland display");
		return NULL;
	}

	if (setenv("WAYLAND_DISPLAY", compositor_name, 1) < 0)
		_E("failed to set WAYLAND_DISPLAY: %s\n", strerror(errno));

	evas_object_smart_callback_add(win, PEPPER_EFL_OBJ_ADD, __obj_added_cb, NULL);
	evas_object_smart_callback_add(win, PEPPER_EFL_OBJ_DEL, __obj_deleted_cb, NULL);

	__appid_tbl = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, __handler_free);
	if (!__appid_tbl)
		_E("failed to create table");

	__evas_tbl = g_hash_table_new(g_direct_hash, g_direct_equal);
	if (!__evas_tbl)
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

	if (__evas_tbl)
		g_hash_table_destroy(__evas_tbl);
}

API int _compositor_set_handler(const char *app_id, _compositor_handler_cb cb, void *data)
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
	handler->freeze = 0;

	g_hash_table_remove(__appid_tbl, app_id);
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

API int _compositor_set_visibility(Evas_Object *obj, visibility_type type)
{
	int obscured;
	int ret;

	switch (type) {
	case VISIBILITY_TYPE_UNOBSCURED:
		obscured = PEPPER_EFL_VISIBILITY_TYPE_UNOBSCURED;
		break;
	case VISIBILITY_TYPE_PARTIALLY_OBSCURED:
		obscured = PEPPER_EFL_VISIBILITY_TYPE_PARTIALLY_OBSCURED;
		break;
	case VISIBILITY_TYPE_FULLY_OBSCURED:
		obscured = PEPPER_EFL_VISIBILITY_TYPE_FULLY_OBSCURED;
		break;
	default:
		return -1;
	}

	ret = pepper_efl_object_visibility_set(obj, obscured);

	if (ret) /* EINA_TRUE on success */
		return 0;

	return -1;
}

API int _compositor_freeze_visibility(Evas_Object *obj, visibility_type type)
{
	struct compositor_handler *handler;

	handler = (struct compositor_handler *)g_hash_table_lookup(__evas_tbl, obj);

	if (!handler) {
		_E("obj not found");
		return -1;
	}

	handler->freeze = 1;

	return _compositor_set_visibility(obj, type);
}

API int _compositor_thaw_visibility(Evas_Object *obj)
{
	struct compositor_handler *handler;

	handler = (struct compositor_handler *)g_hash_table_lookup(__evas_tbl, obj);

	if (!handler) {
		_E("obj not found");
		return -1;
	}

	handler->freeze = 0;

	return 0;
}

static void __send_visibility(gpointer key, gpointer value, gpointer user_data)
{
	struct compositor_handler *handler = (struct compositor_handler *)value;
	Evas_Object *evas_obj = (Evas_Object *)key;
	unsigned int event = GPOINTER_TO_INT(user_data);
	int ret;
	Pepper_Efl_Visibility_Type type;

	if (handler->freeze)
		return;

	if (event)
		type = PEPPER_EFL_VISIBILITY_TYPE_FULLY_OBSCURED;
	else
		type = PEPPER_EFL_VISIBILITY_TYPE_UNOBSCURED;

	ret = pepper_efl_object_visibility_set(evas_obj, type);
	if (!ret) {
		_E("failed to set pepper efl object visibility set %p to %d",
			evas_obj, type);
	}
}

static Eina_Bool __visibility_cb(void *data, int type, void *event)
{
	Ecore_Wl_Event_Window_Visibility_Change *ev = event;

	_D("visibility change: %d %d", (unsigned int)ev->win,
					(unsigned int)ev->fully_obscured);

	if (ev->win != __window_id || !__evas_tbl)
		return ECORE_CALLBACK_RENEW;

	g_hash_table_foreach(__evas_tbl, __send_visibility,
					GINT_TO_POINTER(ev->fully_obscured));

	return ECORE_CALLBACK_RENEW;
}

API int _compositor_start_visibility_notify()
{
	if (__visibility_listener)
		return 0;

	__visibility_listener = ecore_event_handler_add(
		ECORE_WL_EVENT_WINDOW_VISIBILITY_CHANGE, __visibility_cb, NULL);

	return 0;
}

API int _compositor_stop_visibility_notify()
{
	if (!__visibility_listener)
		return 0;

	ecore_event_handler_del(__visibility_listener);
	__visibility_listener = NULL;

	return 0;
}
