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

#include "elmdemo_util.h"
#include "elmdemo_test.h"
#include "progressbar_custom.h"
#include "progressbar.h"

static Evas_Object*
_custom_progressbar_list_create(Evas_Object *parent)
{
   Evas_Object *in_layout = elm_layout_add(parent);
   elm_layout_file_set(in_layout, ELM_DEMO_EDJ, "pingpongs");
   evas_object_size_hint_weight_set(in_layout, EVAS_HINT_EXPAND,
                                    EVAS_HINT_EXPAND);

   // pingpong_bounces progress mode
   progressbar_add("pingpong_bounces", "pingpong_bounces_progress",
                   in_layout, pb_timer_1);

   // pingpong progress mode
   progressbar_add("pingpong", "pingpong_progress",
                   in_layout, pb_timer_2);

   // pingpong_bounces pulse mode
   progressbar_pulse_add("pingpong_bounces", "pingpong_bounces_pulse", in_layout);

   return in_layout;
}

void
progressbar_custom_cb(void *data, Evas_Object *obj, void *event_info)
{
   progressbars_create(data, obj, event_info, "Custom Style",
                       _custom_progressbar_list_create);
}
