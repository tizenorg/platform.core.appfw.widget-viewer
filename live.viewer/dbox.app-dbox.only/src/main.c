#include <Elementary.h>

#include <app.h>
#include <dlog.h>
#include <vconf.h>

#include <dynamicbox.h>
#include <dynamicbox_errno.h>
#include <dynamicbox_service.h>

#include <dynamicbox_provider_app.h>

struct dynamicbox_data {
	Evas_Object *win;
	Evas_Object *bg;
};

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

	dynamicbox_provider_app_set_data(id, dbox_data);
	evas_object_resize(dbox_data->win, w, h);
	evas_object_show(dbox_data->win);

	dbox_data->bg = elm_bg_add(dbox_data->win);
   	elm_win_resize_object_add(dbox_data->win, dbox_data->bg);
	evas_object_resize(dbox_data->bg, w, h);
	elm_bg_color_set(dbox_data->bg, 255, 0, 0);
	evas_object_show(dbox_data->bg);

	return DBOX_STATUS_ERROR_NONE;
}

static int dbox_resize(const char *id, int w, int h, void *data)
{
	struct dynamicbox_data *dbox_data;

	dbox_data = dynamicbox_provider_app_data(id);
	if (!dbox_data) {
		return DBOX_STATUS_ERROR_FAULT;
	}

	evas_object_resize(dbox_data->win, w, h);

	/**
	 * @TODO:
	 */
	return 0;
}

static int dbox_destroy(const char *id, int reason, void *data)
{
	struct dynamicbox_data *dbox_data;

	dbox_data = dynamicbox_provider_app_data(id);
	if (!dbox_data) {
		return DBOX_STATUS_ERROR_NOT_EXIST;
	}

	evas_object_del(dbox_data->win);
	free(dbox_data);

	dynamicbox_provider_app_set_data(id, NULL);
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
		return DBOX_STATUS_ERROR_NOT_EXIST;
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
	return DBOX_STATUS_ERROR_NOT_IMPLEMENTED;
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
	return DBOX_STATUS_ERROR_NOT_IMPLEMENTED;
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
	return DBOX_STATUS_ERROR_NOT_IMPLEMENTED;
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
	return DBOX_STATUS_ERROR_NOT_IMPLEMENTED;
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
	/**
	 * @TODO:
	 */
}

static void app_pause(void *data)
{
	/**
	 * @TODO:
	 */
}

static void app_resume(void *data)
{
	/**
	 * @TODO:
	 */
}

static int update_all_cb(struct dynamicbox_data *dbox_data, void *data)
{
	int r = 0;
	int g = 0;
	int b = 0;

	if (!strcasecmp(data, "red")) {
		r = 255;
	} else if (!strcasecmp(data, "blue")) {
		b = 255;
	} else if (!strcasecmp(data, "green")) {
		g = 255;
	}

	if (dbox_data && dbox_data->bg) {
		elm_bg_color_set(dbox_data->bg, r, g, b);
	}

	return 0;
}

static void _app_control(app_control_h service, void *data)
{
	int ret;
	char *value = NULL;

	ret = app_control_get_extra_data(service, "color", &value);
	if (ret == APP_CONTROL_ERROR_NONE && value) {
		Eina_List *list;
		struct dynamicbox_data *dbox_data;

		LOGD("Color: [%s]\n", value);
		list = dynamicbox_provider_app_data_list();
		EINA_LIST_FREE(list, dbox_data) {
			update_all_cb(dbox_data, value);
		}
		free(value);
	} else {
		if (dynamicbox_provider_app_init(service, data) < 0) {
			LOGE("Failed to initialize the provider_app\n");
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

int main(int argc, char *argv[])
{
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
		.clicked = NULL,

		.pause = dbox_pause,
		.resume = dbox_resume,

		.connected = NULL,
		.disconnected = NULL,

		.data = NULL,
	};

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
