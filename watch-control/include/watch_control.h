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
	WATCH_POLICY_HINT_EXPAND = 1,
} watch_policy_size_hint;

#define WATCH_SMART_SIGNAL_ADDED "watch,added"
#define WATCH_SMART_SIGNAL_REMOVED "watch,removed"

extern int watch_manager_init(Evas_Object *win);
extern int watch_manager_fini();
extern int watch_manager_get_app_control(const char *app_id, app_control_h *app_control);
extern int watch_manager_send_terminate_request(Evas_Object *watch);
extern int watch_policy_set_size_hint(watch_policy_size_hint hint);

#ifdef __cplusplus
}
#endif

#endif
