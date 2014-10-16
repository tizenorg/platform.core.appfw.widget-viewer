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

#ifndef __DEF_TEST_PROGRESSBAR_H_
#define __DEF_TEST_PROGRESSBAR_H_

#include <Elementary.h>
#include <Evas.h>

typedef Evas_Object*(*PROGRESSBAR_CREATE)(Evas_Object *parent);

Ecore_Timer *pb_timer_1;
Ecore_Timer *pb_timer_2;
Ecore_Timer *pb_timer_3;

void progressbar_cb(void *data, Evas_Object *obj, void *event_info);
void progressbar_add(char *style_name, char *container_name,
                     Evas_Object *layout, Ecore_Timer *timer);
void progressbar_pulse_add(char *style_name, char *container_name,
                           Evas_Object *layout);
void progressbars_create(void *data, Evas_Object *obj, void *event_info,
                         char *page_name, PROGRESSBAR_CREATE _progressbar_create);
Evas_Object *scroller_create(Evas_Object *parent);
Eina_Bool fn_pb_timer(void *data);

#endif /* __DEF_TEST_PROGRESSBAR_H_ */
