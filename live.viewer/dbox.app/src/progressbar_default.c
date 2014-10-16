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
_default_progressbar_list_create(Evas_Object *parent)
{
   Evas_Object *in_layout = elm_layout_add(parent);
   elm_layout_file_set(in_layout, ELM_DEMO_EDJ, "progessbar");
   evas_object_size_hint_weight_set(in_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

   // list_progress style
   progressbar_add("list_progress", "list_progress", in_layout, pb_timer_1);

   // list_title_progress style
   progressbar_add("list_title_progress", "progress_list_text", in_layout, pb_timer_2);

  // pending_list style
   progressbar_pulse_add("pending_list", "pending_list", in_layout);

   // Progress circle style
   progressbar_add("progress_circle", "progress_circle", in_layout, pb_timer_3);

   // list_process style
   progressbar_pulse_add("process_Xlarge", "process_Xlarge", in_layout);

   // process_medium style
   progressbar_pulse_add("process_large", "process_large", in_layout);

   // list process small style
   progressbar_pulse_add("process_medium", "process_medium", in_layout);

   // list_process black style
   progressbar_pulse_add("process_small", "process_small", in_layout);

   return in_layout;
}

void
progressbar_default_cb(void *data, Evas_Object *obj, void *event_info)
{
   progressbars_create(data, obj, event_info, "Default Style", _default_progressbar_list_create);
}
