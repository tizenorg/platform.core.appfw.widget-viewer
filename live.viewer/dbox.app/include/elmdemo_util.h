/*
 * Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

#ifndef __ELMDEMO_UTIL_H__
#define __ELMDEMO_UTIL_H_

#include <Elementary.h>
#include <syslog.h>
#include <stdio.h>
#include <efl_assist.h>

#ifdef DESKTOP
	#include "fake-tizen.h"
#else
	#include <vconf.h>
#endif

#ifdef _cplusplus
extern "C" {
#endif

#ifndef S_
#  define S_(str) dgettext("sys_string", str)
#endif

#define UNCLIPPED_CONTENT "unclipped"
#define BOTTOM_PADDING_CONTENT_SIGNAL "elm,layout,content,bottom_padding"
#define DEFAULT_CONTENT_SIGNAL "elm,layout,content,default"
#define BOTTOM_PADDING_CONTENT_SOURCE "layout"

void change_config_owner();
void set_portrait_mode(Eina_Bool on);
Eina_Bool is_portrait_mode();
int get_rotation_degree();
void set_rotation_degree(int degree);
void set_rotation_mode(Eina_Bool rot_mode);
Eina_Bool get_rotation_mode();


#ifdef _cplusplus
}
#endif

#endif // end of ELMDEMO_UTIL_H
