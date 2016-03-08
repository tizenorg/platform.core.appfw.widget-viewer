#include <aul_svc.h>
#include <bundle_internal.h>
#include <app_control_internal.h>
#include <compositor.h>
#include <dlog.h>
#include "watch_control.h"

#define API __attribute__((visibility("default")))

#undef LOG_TAG
#define LOG_TAG "WATCH_CONTROL"

#ifdef LOGE
#define _E LOGE
#endif

#ifdef LOGD
#define _D LOGD
#endif

struct watch_control_s {
	Evas_Object *evas_obj;
};

static int __watch_viewer_initialized = 0;

static watch_control_h __default_control = NULL;
static watch_control_callback __default_handler = NULL;
static void *__default_data = NULL;
static int __watch_size_policy = WATCH_POLICY_HINT_EXPAND;

static int __watch_viewer_init(Evas_Object *win)
{
	if (__watch_viewer_initialized)
		return 0;

	if (win == NULL) {
		_E("invalid arguments");
		return -1;
	}

	_compositor_init(win);

	__watch_viewer_initialized = 1;

	return 0;
}

static void __watch_viewer_fini()
{
	_compositor_fini();
}

API int watch_manager_init(Evas_Object *win)
{
	__watch_viewer_init(win);

	return 0;
}

API Evas_Object *watch_control_get_evas_object(watch_control_h control)
{
	return control->evas_obj;
}

API int watch_manager_add_handler(watch_control_event e, watch_control_callback cb, void *data)
{
	switch (e) {
	case WATCH_OBJ_ADD:
		__default_handler = cb;
		__default_data = data;
		break;
	case WATCH_OBJ_DEL:
		/* TODO */
		break;
	}

	return 0;
}

static int __watch_screen_get_width()
{
	if (__watch_size_policy == WATCH_POLICY_HINT_EXPAND)
		return 360; /* TODO get width from win */

	return 0;
}

static int __watch_screen_get_height()
{
	if (__watch_size_policy == WATCH_POLICY_HINT_EXPAND)
		return 360; /* TODO get width from win */

	return 0;
}

static void __obj_added_cb(const char *app_id, Evas_Object *obj, void *data)
{
	int w, h, x, y;
	Evas_Object *surface = obj;
	evas_object_geometry_get(surface, &x, &y, &w, &h);

	_E("obj added");
	_E("w: %d, h: %d, x: %d y: %d", w, h, x, y);
	if (__default_control && __default_handler) {
		__default_control->evas_obj = surface;
		__default_handler(__default_control, __default_data);
		_E("default handler called");
	}
}

API int watch_manager_get_app_control(const char *app_id, app_control_h *app_control)
{
	char buf[10];
	bundle *b = NULL;
	app_control_create(app_control);
	app_control_set_app_id(*app_control, app_id);

	__default_control = (watch_control_h)malloc(sizeof(struct watch_control_s));

	snprintf(buf, sizeof(buf), "%d", __watch_screen_get_width());
	app_control_add_extra_data(*app_control, "WATCH_WIDTH", buf);
	snprintf(buf, sizeof(buf), "%d", __watch_screen_get_height());
	app_control_add_extra_data(*app_control, "WATCH_HEIGHT", buf);

	app_control_add_extra_data(*app_control, "WAYLAND_DISPLAY", getenv("WAYLAND_DISPLAY"));
	app_control_add_extra_data(*app_control, "XDG_RUNTIME_DIR", getenv("XDG_RUNTIME_DIR"));

	app_control_set_operation(*app_control, APP_CONTROL_OPERATION_MAIN);

	_compositor_set_handler(app_id, __obj_added_cb, NULL);

	app_control_to_bundle(*app_control, &b);
	aul_svc_set_loader_id(b, 1);

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

