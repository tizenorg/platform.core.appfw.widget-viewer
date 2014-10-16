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
#include "progressbar.h"
#include "progressbar_custom.h"

#define PROGRESS_INCREASE_STEP 0.01
#define TIMER_LAP_TIME 0.5

static Evas_Object *in_layout = NULL;
static Evas_Object *progressbar = NULL;

static struct _menu_item menu_its[] = {
   { "Default Style", progressbar_default_cb},
   { "Custom Style", progressbar_custom_cb},
   { NULL, NULL }
};

void
progressbar_add(char *style_name, char *container_name, Evas_Object *layout, Ecore_Timer *timer)
{
   progressbar = elm_progressbar_add(layout);

   elm_object_style_set(progressbar, style_name);
   elm_object_text_set(progressbar, "Label");
   elm_progressbar_horizontal_set(progressbar, EINA_TRUE);
   elm_progressbar_value_set(progressbar, 0.0);
   elm_object_part_content_set(layout, container_name, progressbar);

   evas_object_size_hint_align_set(progressbar, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(progressbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

   if (timer) ecore_timer_del(timer);
   timer = ecore_timer_add(TIMER_LAP_TIME, fn_pb_timer, progressbar);

   evas_object_show(progressbar);
}

void
progressbar_pulse_add(char *style_name, char *container_name, Evas_Object *layout)
{
   progressbar = elm_progressbar_add(layout);

   elm_object_style_set(progressbar, style_name);
   elm_progressbar_pulse(progressbar, EINA_TRUE);
   elm_object_part_content_set(layout, container_name, progressbar);

   evas_object_size_hint_align_set(progressbar, EVAS_HINT_FILL, 0.5);
   evas_object_size_hint_weight_set(progressbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

   evas_object_show(progressbar);
}

static void
_list_clicked(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Object_Item *it = (Elm_Object_Item *) elm_list_selected_item_get(obj);
   if (it == NULL)
     {
        fprintf(stderr, "list item is NULL\n");
        return;
     }

   elm_list_item_selected_set(it, EINA_FALSE);
}

Eina_Bool
fn_pb_timer(void *data)
{
   if(!progressbar) return EINA_FALSE;


   Evas_Object *progressbar = (Evas_Object*) data;
   double value = elm_progressbar_value_get(progressbar);

   if (value == 1.0) value = 0.0;
   value += PROGRESS_INCREASE_STEP;

   elm_progressbar_value_set(progressbar, value);

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_pop_cb(void *data, Elm_Object_Item *it)
{
   if (pb_timer_1) ecore_timer_del(pb_timer_1);
   if (pb_timer_2) ecore_timer_del(pb_timer_2);
   if (pb_timer_3) ecore_timer_del(pb_timer_3);

   if (in_layout) in_layout = NULL;
   if (progressbar) progressbar = NULL;

   elm_theme_extension_del(NULL, ELM_DEMO_EDJ);

   return EINA_TRUE;
}

static Evas_Object*
_list_winset_create(struct appdata *ad)
{
   Evas_Object *li;
   if (ad == NULL) return NULL;

   li = elm_list_add(ad->nf);
   elm_list_mode_set(li, ELM_LIST_COMPRESS);
   evas_object_smart_callback_add(li, "selected", _list_clicked, NULL);

   int idx = 0;
   while (menu_its[idx].name != NULL)
     {
        elm_list_item_append(li, menu_its[idx].name, NULL, NULL, menu_its[idx].func, ad);
        ++idx;
     }

   elm_list_go(li);

   return li;
}

void
progressbar_cb(void *data, Evas_Object *obj, void *event_info)
{
   struct appdata *ad = data;
   if (ad == NULL) return;
   Evas_Object *list = _list_winset_create(ad);

   elm_naviframe_item_push(ad->nf, _("ProgressBar"), NULL, NULL, list, NULL);
}

Evas_Object *scroller_create(Evas_Object *parent)
{
   Evas_Object* scroller = elm_scroller_add(parent);

   elm_scroller_bounce_set(scroller, EINA_FALSE, EINA_TRUE);
   elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);

   evas_object_show(scroller);

   return scroller;
}

void
progressbars_create(void *data, Evas_Object *obj, void *event_info,
                    char *page_name, PROGRESSBAR_CREATE _progressbar_create)
{
   Evas_Object *scroller_main, *layout_inner;
   Elm_Object_Item *it;
   struct appdata *ad = data;

   if (ad == NULL) return;

   elm_theme_extension_add(NULL, ELM_DEMO_EDJ);

   scroller_main = scroller_create(ad->nf);

   it = elm_naviframe_item_push(ad->nf, _(page_name), NULL, NULL, scroller_main, NULL);
   elm_naviframe_item_pop_cb_set(it, _pop_cb, NULL);

   layout_inner = _progressbar_create(ad->nf);
   elm_object_content_set(scroller_main, layout_inner);
}

