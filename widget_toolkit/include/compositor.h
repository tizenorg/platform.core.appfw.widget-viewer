#ifndef __COMPOSITOR_H__
#define __COMPOSITOR_H__

#include <Evas.h>

const char *_compositor_init(Evas_Object *win);
void _compositor_fini();
int _compositor_set_handler(const char *app_id, void (*cb)(const char *app_id, Evas_Object *obj, void *data), void *data);
int _compositor_unser_handler(const char *app_id);
const char *_compositor_get_title(Evas_Object *obj);
const char *_compositor_get_app_id(Evas_Object *obj);
int _compositor_get_pid(Evas_Object *obj);

#endif
