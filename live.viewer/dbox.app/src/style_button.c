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

#include "elmdemo_test.h"
#include "elmdemo_util.h"
#include "style_button.h"
#define PKG_DATA_DIR "/usr/share/elementary"

/*********************************************************
 Add Style
 ********************************************************/
static Evas_Object *_create_scroller(Evas_Object * parent);

static Evas_Object *_create_buttons(Evas_Object * parent);

static Evas_Object *
_create_buttons(Evas_Object * parent)
{
   Evas_Object *default_button;

   Evas_Object *new_style_button;

   Evas_Object *new_style_button_2;

   Evas_Object *layout;

   Evas_Object *ic1, *ic2;

   layout = elm_layout_add(parent);
   elm_layout_file_set(layout, ELM_DEMO_EDJ, "elmdemo-test/style");
   evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

   ic1 = elm_image_add(layout);
   elm_image_file_set(ic1, ICON_DIR "/logo_small.png", NULL);
   elm_image_resizable_set(ic1, 1, 1);

   /* default style */
   default_button = elm_button_add(layout);
   elm_object_content_set(default_button, ic1);
   elm_object_text_set(default_button, _("default style"));
   elm_object_part_content_set(layout, "button1", default_button);
   elm_access_info_set(default_button, ELM_ACCESS_INFO, "The label is changed");

   ic2 = elm_image_add(layout);
   elm_image_file_set(ic2, ICON_DIR "/logo_small.png", NULL);
   elm_image_resizable_set(ic2, 1, 1);

   /* new style */
   elm_theme_extension_add(NULL, ELM_DEMO_EDJ);

   new_style_button = elm_button_add(layout);
   // set new style to the button object
   elm_object_style_set(new_style_button, "elm_demo_tizen/text_with_icon");
   elm_object_content_set(new_style_button, ic1);
   elm_object_text_set(new_style_button, _("new style new style new style"));
   elm_object_part_content_set(layout, "button2", new_style_button);
   elm_access_info_set(new_style_button, ELM_ACCESS_INFO, "The button has label new style");

   new_style_button_2 = elm_button_add(layout);
   elm_object_style_set(new_style_button_2, "elm_demo_tizen/anim_up_down");
   elm_object_content_set(new_style_button_2, ic2);
   elm_object_text_set(new_style_button_2, _("new style_2"));
   elm_object_part_content_set(layout, "button3", new_style_button_2);
   elm_access_info_set(new_style_button_2, ELM_ACCESS_INFO, "The button has label new style_2");

   return layout;
}

static Evas_Object *
_create_scroller(Evas_Object * parent)
{
   Evas_Object *scroller = elm_scroller_add(parent);

   elm_scroller_bounce_set(scroller, EINA_FALSE, EINA_TRUE);
   elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
   evas_object_show(scroller);

   return scroller;
}

void
style_button_cb(void *data, Evas_Object * obj, void *event_info)
{
   Evas_Object *scroller, *layout_inner;

   struct appdata *ad = (struct appdata *)data;

   if (ad == NULL) return;

   scroller = _create_scroller(ad->nf);
   elm_naviframe_item_push(ad->nf, _("Custom Style"), NULL, NULL, scroller, NULL);

   layout_inner = _create_buttons(ad->nf);
   elm_object_content_set(scroller, layout_inner);
}
