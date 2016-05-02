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

#include <aul_svc.h>
#include <bundle_internal.h>
#include <app_control_internal.h>
#include <compositor.h>
#include <dlog.h>
#include "watch_control.h"
#include <aul.h>

#define API __attribute__((visibility("default")))

#undef LOG_TAG
#define LOG_TAG "WATCH_CONTROL"

#ifdef LOGE
#define _E LOGE
#endif

#ifdef LOGD
#define _D LOGD
#endif

static int __watch_viewer_initialized = 0;
static int __watch_size_policy = WATCH_POLICY_HINT_EXPAND;

static int __default_width;
static int __default_height;

static Evas_Object *__win;

static void __win_resized(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Object *win = obj;
	Evas_Coord x, y, w, h;

	if (!win)
		return;

	evas_object_geometry_get(win, &x, &y, &w, &h);
	__default_width = w;
	__default_height = h;
}

static int __watch_viewer_init(Evas_Object *win)
{
	if (__watch_viewer_initialized)
		return 0;

	if (win == NULL) {
		_E("invalid arguments");
		return -1;
	}

	__win = win;
	_compositor_init(win);

	__win_resized(NULL, NULL, win, NULL); /* init */
	evas_object_event_callback_add(win, EVAS_CALLBACK_RESIZE, __win_resized, NULL);

	__watch_viewer_initialized = 1;

	return 0;
}

static void __watch_viewer_fini()
{
	if (__win)
		evas_object_event_callback_del(__win, EVAS_CALLBACK_RESIZE, __win_resized);

	_compositor_fini();
}

API int watch_manager_init(Evas_Object *win)
{
	__watch_viewer_init(win);

	return 0;
}

static int __watch_screen_get_width()
{
	if (__watch_size_policy == WATCH_POLICY_HINT_EXPAND)
		return __default_width;

	return 0;
}

static int __watch_screen_get_height()
{
	if (__watch_size_policy == WATCH_POLICY_HINT_EXPAND)
		return __default_height;

	return 0;
}

static void __pepper_cb(const char *app_id, const char *event, Evas_Object *obj, void *data)
{
	int w, h, x, y;
	Evas_Object *surface = obj;
	evas_object_geometry_get(surface, &x, &y, &w, &h);

	if (!event) {
		_E("invalid param");
		return;
	}

	if (strcmp(event, "added") == 0) {
		_E("obj added");
		_E("w: %d, h: %d, x: %d y: %d", w, h, x, y);
		evas_object_smart_callback_call(__win, WATCH_SMART_SIGNAL_ADDED, surface);
	} else if (strcmp(event, "removed") == 0) {
		_E("obj removed");
		evas_object_smart_callback_call(__win, WATCH_SMART_SIGNAL_REMOVED, surface);
	}
}

API int watch_manager_get_app_control(const char *app_id, app_control_h *app_control)
{
	char buf[10];
	bundle *b = NULL;
	app_control_create(app_control);
	app_control_set_app_id(*app_control, app_id);

	snprintf(buf, sizeof(buf), "%d", __watch_screen_get_width());
	app_control_add_extra_data(*app_control, "WATCH_WIDTH", buf);
	snprintf(buf, sizeof(buf), "%d", __watch_screen_get_height());
	app_control_add_extra_data(*app_control, "WATCH_HEIGHT", buf);

	app_control_set_operation(*app_control, APP_CONTROL_OPERATION_MAIN);

	_compositor_set_handler(app_id, __pepper_cb, NULL);

	app_control_to_bundle(*app_control, &b);
	if (b) {
		bundle_add_str(b, AUL_K_WAYLAND_DISPLAY, getenv("WAYLAND_DISPLAY"));
		bundle_add_str(b, "WAYLAND_DISPLAY", getenv("WAYLAND_DISPLAY"));
		bundle_add_str(b, AUL_K_WAYLAND_WORKING_DIR, getenv("XDG_RUNTIME_DIR"));
		bundle_add_str(b, "XDG_RUNTIME_DIR", getenv("XDG_RUNTIME_DIR"));
		aul_svc_set_loader_id(b, 1);
	}

	return 0;
}

API int watch_manager_fini()
{
	__watch_viewer_fini();

	return 0;
}

API int watch_policy_set_size_hint(watch_policy_size_hint hint)
{
	__watch_size_policy = hint;

	return 0;
}

