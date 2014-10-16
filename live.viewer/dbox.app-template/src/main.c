#include <Elementary.h>

#include <app.h>
#include <dlog.h>
#include <vconf.h>

#include <dynamicbox_errno.h>
#include <dynamicbox_service.h>
#include <dynamicbox_provider.h>
#include <dynamicbox_provider_app.h>

static int dbox_create(const char *id, const char *content, int w, int h, void *data)
{
	/*!
	 * \TODO:
	 */
	return 0;
}

static int dbox_resize(const char *id, int w, int h, void *data)
{
	/*!
	 * \TODO:
	 */
	return 0;
}

static int dbox_destroy(const char *id, int reason, void *data)
{
	/*!
	 * \TODO:
	 */
	return 0;
}

static int gbar_create(const char *id, int w, int h, double x, double y, void *data)
{
	/*!
	 * \TODO:
	 */
	return 0;
}

static int gbar_resize_move(const char *id, int w, int h, double x, double y, void *data)
{
	/*!
	 * \TODO:
	 */
	return 0;
}

static int gbar_destroy(const char *id, int reason, void *data)
{
	/*!
	 * \TODO:
	 */
	return 0;
}

static int dbox_update(const char *id, const char *content, int force, void *data)
{
	/*!
	 * \TODO:
	 */
	return 0;
}

static int dbox_script_event(const char *id, const char *emission, const char *source, dynamicbox_event_info_t info, void *data)
{
	/*!
	 * \TODO:
	 */
	return 0;
}

static int dbox_pause(const char *id, void *data)
{
	/*!
	 * \TODO:
	 */
	return 0;
}

static int dbox_resume(const char *id, void *data)
{
	/*!
	 * \TODO:
	 */
	return 0;
}

static bool app_create(void *data)
{
	/*!
	 * \TODO:
	 */
	return true;
}

static void app_terminate(void *data)
{
	/*!
	 * \TODO:
	 */
}

static void app_pause(void *data)
{
	/*!
	 * \TODO:
	 */
}

static void app_resume(void *data)
{
	/*!
	 * \TODO:
	 */
}

static void app_service(app_control_h service, void *data)
{
	if (dynamicbox_provider_app_init(service, data) < 0) {
		LOGE("Failed to initialize the provider_app\n");
	}
	/*!
	 * \TODO:
	 */
}

static void app_language_changed(void *data)
{
	/*!
	 * \TODO:
	 */
}

int main(int argc, char *argv[])
{
	app_event_callback_s app_callback = {
		.create = app_create,
		.terminate = app_terminate,
		.pause = app_pause,
		.resume = app_resume,
		.app_control = app_service,
		.language_changed = app_language_changed,
		.low_memory = NULL,
		.low_battery = NULL,
		.device_orientation = NULL,
		.region_format_changed = NULL,
	};

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

		.pause = dbox_pause,
		.resume = dbox_resume,
		.data = NULL,
	};

	return app_efl_main(&argc, &argv, &app_callback, &table);
}

/* End of a file */
