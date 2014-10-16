#include <Elementary.h>
#include "fake-tizen.h"


void appcore_set_event_callback(int event, FakeCb fake_cb, void *data)
{
	return;
}

int appcore_measure_time()
{
	return 0;
}

void appcore_measure_start()
{
	return;
}

int  appcore_measure_time_from(const char *str)
{
	return 0;
}

void appcore_set_rotation_cb(int (*_rotation_cb)(enum appcore_rm mode, void *data), void *data)
{
	return;
}

void appcore_set_i18n(const char *pkg, const char* locale)
{
	return;
}

int appcore_efl_main(const char *pkg, int *argc, char **argv, struct appcore_ops *ops)
{
	elm_init(*argc, argv);
	ops->create(ops->data);
	elm_run();
	return 0;
}

char *vconf_get_str(const char *in_key)
{
	return NULL;
}

void vconf_get_int(const char *key, int *value)
{
	*value = 1;
}

int vconf_ignore_key_changed(int size, void (*func)(void))
{
	return -1;
}

int vconf_notify_key_changed(int size, void (*func)(void))
{
	return -1;
}
