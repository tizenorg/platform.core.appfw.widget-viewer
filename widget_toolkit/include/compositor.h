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

#ifndef EXPORT_API
#define EXPORT_API
#endif // EXPORT_API


EXPORT_API const char *_compositor_init(Evas_Object *win);
EXPORT_API void _compositor_fini();
EXPORT_API int _compositor_set_handler(const char *app_id, void (*cb)(const char *app_id, Evas_Object *obj, void *data), void *data);
EXPORT_API int _compositor_unser_handler(const char *app_id);
EXPORT_API const char *_compositor_get_title(Evas_Object *obj);
EXPORT_API const char *_compositor_get_app_id(Evas_Object *obj);
EXPORT_API int _compositor_get_pid(Evas_Object *obj);

#ifdef __cplusplus
}
#endif

#endif
