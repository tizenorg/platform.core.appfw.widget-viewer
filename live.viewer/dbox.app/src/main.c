#include <Elementary.h>

#include <Ecore_X.h>

#include <app.h>
#include <dlog.h>
#include <vconf.h>

#include <dynamicbox.h>
#include <dynamicbox_errno.h>
#include <dynamicbox_service.h>
#include <dynamicbox_provider_app.h>

#include "debug.h"

#include "entry.h"
#include "util.h"
#include "window.h"

struct dynamicbox_data {
	Evas_Object *win;
	Evas_Object *bg;
	Evas_Object *conformant;
	char *content;
};

extern int htt_main(Evas_Object *win);

static int dbox_create(const char *id, const char *content, int w, int h, void *data)
{
	struct dynamicbox_data *dbox_data;
	Evas_Object *tmp_parent;

	dbox_data = calloc(1, sizeof(*dbox_data));
	if (!dbox_data) {
		return DBOX_STATUS_ERROR_OUT_OF_MEMORY;
	}

	tmp_parent = dynamicbox_get_evas_object(id, 0);
	if (!tmp_parent) {
		free(dbox_data);
		return DBOX_STATUS_ERROR_FAULT;
	}

	dbox_data->win = elm_win_add(tmp_parent, "DBox Window", ELM_WIN_DYNAMIC_BOX);
	evas_object_del(tmp_parent);
	if (!dbox_data->win) {
		free(dbox_data);
		return DBOX_STATUS_ERROR_FAULT;
	}

	dbox_data->content = strdup(content);
	if (!dbox_data->content) {
		evas_object_del(dbox_data->win);
		dbox_data->win = NULL;
		free(dbox_data);
		return DBOX_STATUS_ERROR_OUT_OF_MEMORY;
	}

	evas_object_resize(dbox_data->win, w, h);
	evas_object_show(dbox_data->win);

	dbox_data->bg = elm_bg_add(dbox_data->win);
	if (dbox_data->bg) {
		evas_object_resize(dbox_data->bg, w, h);
		//elm_win_resize_object_add(dbox_data->win, dbox_data->bg);
		elm_bg_color_set(dbox_data->bg, 255, 0, 0);
		evas_object_show(dbox_data->bg);
	}

	htt_main(dbox_data->win);

	DbgPrint("Resized to %dx%d\n", w, h);

	dynamicbox_provider_app_set_data(id, dbox_data);
	return DBOX_STATUS_ERROR_NONE;
}

static int dbox_resize(const char *id, int w, int h, void *data)
{
	struct dynamicbox_data *dbox_data;

	dbox_data = dynamicbox_provider_app_data(id);
	if (!dbox_data) {
		return DBOX_STATUS_ERROR_INVALID_PARAMETER;
	}

	evas_object_resize(dbox_data->win, w, h);
	evas_object_resize(dbox_data->bg, w, h);
	evas_object_resize(dbox_data->conformant, w, h);

	DbgPrint("Resized to %dx%d\n", w, h);

	return 0;
}

static int dbox_destroy(const char *id, int reason, void *data)
{
	struct dynamicbox_data *dbox_data;

	dbox_data = dynamicbox_provider_app_data(id);
	if (!dbox_data) {
		return DBOX_STATUS_ERROR_INVALID_PARAMETER;
	}
	dynamicbox_provider_app_set_data(id, NULL);

	evas_object_del(dbox_data->win);
	free(dbox_data->content);
	free(dbox_data);

	/**
	 * @TODO:
	 */
	return 0;
}

static int gbar_create(const char *id, int w, int h, double x, double y, void *data)
{
	struct dynamicbox_data *dbox_data;

	dbox_data = dynamicbox_provider_app_data(id);
	if (!dbox_data) {
		return DBOX_STATUS_ERROR_INVALID_PARAMETER;
	}

	/**
	 * @TODO:
	 */
	return DBOX_STATUS_ERROR_NOT_IMPLEMENTED;
}

static int gbar_resize_move(const char *id, int w, int h, double x, double y, void *data)
{
	struct dynamicbox_data *dbox_data;

	dbox_data = dynamicbox_provider_app_data(id);
	if (!dbox_data) {
		return DBOX_STATUS_ERROR_INVALID_PARAMETER;
	}
	/**
	 * @TODO:
	 */
	return DBOX_STATUS_ERROR_NOT_IMPLEMENTED;
}

static int gbar_destroy(const char *id, int reason, void *data)
{
	struct dynamicbox_data *dbox_data;

	dbox_data = dynamicbox_provider_app_data(id);
	if (!dbox_data) {
		return DBOX_STATUS_ERROR_INVALID_PARAMETER;
	}
	/**
	 * @TODO:
	 */
	return DBOX_STATUS_ERROR_NOT_IMPLEMENTED;
}

static int dbox_update(const char *id, const char *content, int force, void *data)
{
	struct dynamicbox_data *dbox_data;

	dbox_data = dynamicbox_provider_app_data(id);
	if (!dbox_data) {
		return DBOX_STATUS_ERROR_INVALID_PARAMETER;
	}

	/**
	 * @TODO:
	 */
	return 0;
}


static int dbox_clicked(const char *id, const char *event, double x, double y, double timestamp, void *data) /**< If viewer calls dynamicbox_click function, this event callback will be called */
{
	struct dynamicbox_data *dbox_data;

	dbox_data = dynamicbox_provider_app_data(id);
	if (!dbox_data) {
		return DBOX_STATUS_ERROR_INVALID_PARAMETER;
	}

	/**
	 * @TODO:
	 */
	return 0;
}

static int dbox_script_event(const char *id, const char *emission, const char *source, dynamicbox_event_info_t info, void *data)
{
	struct dynamicbox_data *dbox_data;

	dbox_data = dynamicbox_provider_app_data(id);
	if (!dbox_data) {
		return DBOX_STATUS_ERROR_INVALID_PARAMETER;
	}

	/**
	 * @TODO:
	 */
	return 0;
}

static int dbox_pause(const char *id, void *data)
{
	struct dynamicbox_data *dbox_data;

	dbox_data = dynamicbox_provider_app_data(id);
	if (!dbox_data) {
		return DBOX_STATUS_ERROR_INVALID_PARAMETER;
	}

	/**
	 * @TODO:
	 */
	return 0;
}

static int dbox_resume(const char *id, void *data)
{
	struct dynamicbox_data *dbox_data;

	dbox_data = dynamicbox_provider_app_data(id);
	if (!dbox_data) {
		return DBOX_STATUS_ERROR_INVALID_PARAMETER;
	}

	/**
	 * @TODO:
	 */
	return 0;
}

static bool app_create(void *data)
{
	/**
	 * @TODO:
	 */
	return true;
}

static void app_terminate(void *data)
{
	if (window_list_destroy() != 0) {
	}
	/**
	 * @TODO:
	 */
}

static void app_pause(void *data)
{
	/**
	 * @TODO:
	 */
	DbgPrint("Paused\n");
}

static void app_resume(void *data)
{
	/**
	 * @TODO:
	 */
	DbgPrint("Resumed\n");
}

static void _app_control(app_control_h service, void *data)
{
	char *op = NULL;

	app_control_get_operation(service, &op);
	if (op && strcmp(op, "http://tizen.org/appcontrol/operation/main") == 0) {
		if (dynamicbox_provider_app_init(service, data) == 0) {
			DbgPrint("initiated");
		} else {
			// Do others
		}
	}
	/**
	 * @TODO:
	 */
}

static void app_language_changed(void *data)
{
	/**
	 * @TODO:
	 */
}

static void provider_connected(void *data)
{
	dynamicbox_provider_app_manual_control(1);
}

int main(int argc, char *argv[])
{
	/**
	 * \note
	 * To allocate this table in the stack.
	 * Access this from callbacks.
	 */
	struct dynamicbox_provider_event_callback table = {
		.dbox = {
			.create = dbox_create,
			.resize = dbox_resize,
			.destroy = dbox_destroy,
		},

		.gbar = {
			.create = gbar_create,
			.resize_move = gbar_resize_move,
			.destroy = gbar_destroy,
		},

		.update = dbox_update,
		.script_event = dbox_script_event,
		.clicked = dbox_clicked,

		.pause = dbox_pause,
		.resume = dbox_resume,

		.connected = provider_connected,
		.disconnected = NULL,

		.data = NULL,
	};

	setenv("ELM_ENGINE", "gl", 1);

	app_event_callback_s app_callback = {
		.create = app_create,
		.terminate = app_terminate,
		.pause = app_pause,
		.resume = app_resume,
		.app_control = _app_control,
		.language_changed = app_language_changed,
		.low_memory = NULL,
		.low_battery = NULL,
		.device_orientation = NULL,
		.region_format_changed = NULL,
	};

	return app_efl_main(&argc, &argv, &app_callback, &table);
}

/* End of a file */
