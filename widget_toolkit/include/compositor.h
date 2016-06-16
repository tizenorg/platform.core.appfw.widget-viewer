/*
 * Samsung API
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

#ifndef __COMPOSITOR_H__
#define __COMPOSITOR_H__

#include <Evas.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	VISIBILITY_TYPE_UNOBSCURED = 0,
	VISIBILITY_TYPE_PARTIALLY_OBSCURED,
	VISIBILITY_TYPE_FULLY_OBSCURED,
} visibility_type;

typedef void (*_compositor_handler_cb)(const char *app_id, const char *event, Evas_Object *obj, void *data);

const char *_compositor_init(Evas_Object *win);
void _compositor_fini();
int _compositor_set_handler(const char *app_id, _compositor_handler_cb cb, void *data);
int _compositor_unser_handler(const char *app_id);
const char *_compositor_get_title(Evas_Object *obj);
const char *_compositor_get_app_id(Evas_Object *obj);
int _compositor_get_pid(Evas_Object *obj);
int _compositor_set_visibility(Evas_Object *obj, visibility_type type);
int _compositor_freeze_visibility(Evas_Object *obj, visibility_type type);
int _compositor_thaw_visibility(Evas_Object *obj);
int _compositor_stop_visibility_notify();
int _compositor_start_visibility_notify();

#ifdef __cplusplus
}
#endif

#endif
