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

#ifndef __WATCH_MANAGER_H__
#define __WATCH_MANAGER_H__

#include <tizen_type.h>
#include <Evas.h>
#include <app.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	WATCH_OBJ_ADD = 1,
	WATCH_OBJ_DEL = 2,
} watch_control_event;

typedef enum {
	WATCH_POLICY_HINT_EXPAND = 1,
} watch_policy_size_hint;

struct watch_control_s;
typedef struct watch_control_s *watch_control_h;

typedef int (*watch_control_callback)(watch_control_h watch_control, void *data);

extern int watch_manager_init(Evas_Object *win);
extern int watch_manager_fini();
extern int watch_manager_add_handler(watch_control_event e, watch_control_callback cb, void *data);
extern int watch_manager_get_app_control(const char *app_id, app_control_h *app_control);
extern int watch_policy_set_size_hint(watch_policy_size_hint hint);
extern Evas_Object *watch_control_get_evas_object(watch_control_h watch_control);

#ifdef __cplusplus
}
#endif

#endif
