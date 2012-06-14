#include <appcore-efl.h>
#include <Elementary.h>
#include <Ecore_X.h>
#include <unistd.h>

#include <dlog.h>


static struct info {
	Evas_Object *win;
	char *edje;
	char *group;
	int w;
	int h;

	Evas_Object *bg;
	Evas_Object *pd;
} s_info = {
	.win = NULL,
	.edje = NULL,
	.group = NULL,
	.w = 0,
	.h = 0,

	.bg = NULL,
	.pd = NULL,
};

static void win_del(void *data, Evas_Object *obj, void *event)
{
	elm_exit();
}

static int app_create(void *data)
{
	Evas_Coord w, h;
	s_info.win = elm_win_add(NULL, "test", ELM_WIN_BASIC);
	if (!s_info.win)
		return -1;

	elm_win_title_set(s_info.win, "test");
	elm_win_borderless_set(s_info.win, EINA_TRUE);
	evas_object_smart_callback_add(s_info.win, "delete,request", win_del, NULL);
	ecore_x_window_size_get(ecore_x_window_root_first_get(), &w, &h);
	evas_object_resize(s_info.win, w, h);
	evas_object_show(s_info.win);
	elm_win_indicator_mode_set(s_info.win, EINA_FALSE);

	s_info.bg = evas_object_rectangle_add(evas_object_evas_get(s_info.win));
	evas_object_resize(s_info.bg, 100, 100);
	evas_object_color_set(s_info.bg, 100, 100, 100, 255);
	evas_object_move(s_info.bg, 0, 100);
	evas_object_show(s_info.bg);

	return 0;
}

static int app_terminate(void *data)
{
	if (s_info.win)
		evas_object_del(s_info.win);

	free(s_info.edje);
	free(s_info.group);
	return 0;
}

static int app_pause(void *data)
{
	return 0;
}

static int app_resume(void *data)
{
	return 0;
}

static int app_reset(bundle *b,void *data)
{
	LOGD("RESET\n");
	if (!s_info.pd) {
		const char *edje;
		const char *group;
		const char *width;
		const char *height;

		edje = (char *)bundle_get_val(b, "edje");
		group = (char *)bundle_get_val(b, "group");
		width = (char *)bundle_get_val(b, "width");
		height = (char *)bundle_get_val(b, "height");

		LOGD("EDJE: %s, GROUP: %s, %sx%s\n", edje, group, width, height);

		s_info.edje = strdup(edje);
		s_info.group = strdup(group);

		s_info.w = atoi(width);
		s_info.h = atoi(height);
		LOGD("WxH: %dx%d\n", s_info.w, s_info.h);

		s_info.pd = edje_object_add(evas_object_evas_get(s_info.win));
		LOGD("Load: %s (%s)\n", s_info.edje, s_info.group);
		edje_object_file_set(s_info.pd, s_info.edje, s_info.group);
		LOGD("Reisze to %dx%d\n", s_info.w, s_info.h);
		evas_object_resize(s_info.pd, s_info.w, s_info.h);
		evas_object_move(s_info.pd, 0, 200);
		evas_object_show(s_info.pd);
	} else {
		const char *source;
		const char *signal;

		source = (char *)bundle_get_val(b, "source");
		signal = (char *)bundle_get_val(b, "signal");

		LOGD("Source: %s, signal: %s\n", source, signal);

		edje_object_signal_emit(s_info.pd, signal, source);
	}
	return 0;
}

int main(int argc, char *argv[])
{
	struct appcore_ops ops = {
		.create = app_create,
		.terminate = app_terminate,
		.pause = app_pause,
		.resume = app_resume,
		.reset = app_reset,
	};

	ops.data = NULL;
	return appcore_efl_main(argv[0], &argc, &argv, &ops);
}

/* End of a file */
