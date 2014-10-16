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
#include "config_scrolling.h"
#include <math.h>

#ifndef __UNUSED__
#define __UNUSED__ __attribute__((unused))
#endif

/*********************************************************
  config scrolling
 ********************************************************/

#define NUM_OF_ITEMS 44

static char *slider_itemlist[] = {
     "Thumb scroll threshold",
     "Thumb scroll hold threshold",
     "Thumb scroll momentum threshold",
     "Thumb scroll friction",
     "Thumb scroll min friction",
     "Thumb scroll friction standard",
     "Thumb scroll border friction",
     "Thumb scroll sensitivity friction",
     "Thumb scroll acceleration threshold",
     "Thumb scroll acceleration time limit",
     "Thumb scroll acceleration weight",
     NULL
};

static char *guide_itemlist[] = {
     "This is the minimum pixels for moving the scroller",
     "This is the minimum pixels for moving the scroller on hold",
     "This is the minimum speed for auto scrolling",
     "Scroll animation time",
     "Scroll animation minimum time",
     "Standard velocity of the scroller",
     "This is the lag between actual mouse cursor dragging movement and scroller's view movement",
     "Sensitivity of toucing to scroll",
     "The minimum speed of mouse cursor movement which will accelerate",
     "The time limit for accelerating velocity",
     "The weight for acceleration",
     NULL
};
static void
_thumbscroll_threshold_change(void *data __UNUSED__,
                              Evas_Object *obj,
                              void *event_info __UNUSED__)
{
   double tst = elm_config_scroll_thumbscroll_threshold_get();
   double val = elm_slider_value_get(obj);
   unsigned int v;
   if (tst == val) return;
   v = floor(val + 0.5);
   elm_config_scroll_thumbscroll_threshold_set(v);
   elm_config_all_flush();
   elm_config_save();
   change_config_owner();
}

static void
_thumbscroll_hold_threshold_change(void *data __UNUSED__,
                                   Evas_Object *obj,
                                   void *event_info __UNUSED__)
{
   double tst = elm_config_scroll_thumbscroll_hold_threshold_get();
   double val = elm_slider_value_get(obj);
   unsigned int v;
   if (tst == val) return;
   v = floor(val + 0.5);
   elm_config_scroll_thumbscroll_hold_threshold_set(v);
   elm_config_all_flush();
   elm_config_save();
   change_config_owner();
}

static void
_thumbscroll_momentum_threshold_change(void *data __UNUSED__,
                                       Evas_Object *obj,
                                       void *event_info __UNUSED__)
{
   double tsmt = elm_config_scroll_thumbscroll_momentum_threshold_get();
   double val = elm_slider_value_get(obj);

   if (tsmt == val) return;
   elm_config_scroll_thumbscroll_momentum_threshold_set(val);
   elm_config_all_flush();
   elm_config_save();
   change_config_owner();
}

static void
_thumbscroll_friction_change(void *data __UNUSED__,
                             Evas_Object *obj,
                             void *event_info __UNUSED__)
{
   double tsf = elm_config_scroll_thumbscroll_friction_get();
   double val = elm_slider_value_get(obj);

   if (tsf == val) return;
   elm_config_scroll_thumbscroll_friction_set(val);
   elm_config_all_flush();
   elm_config_save();
   change_config_owner();
}

static void
_thumbscroll_min_friction_change(void *data __UNUSED__,
                                 Evas_Object *obj,
                                 void *event_info __UNUSED__)
{
   double tsmf = elm_config_scroll_thumbscroll_min_friction_get();
   double val = elm_slider_value_get(obj);

   if (tsmf == val) return;
   elm_config_scroll_thumbscroll_min_friction_set(val);
   elm_config_all_flush();
   elm_config_save();
   change_config_owner();
}

static void
_thumbscroll_friction_standard_change(void *data __UNUSED__,
                                      Evas_Object *obj,
                                      void *event_info __UNUSED__)
{
   double tsfs = elm_config_scroll_thumbscroll_friction_standard_get();
   double val = elm_slider_value_get(obj);

   if (tsfs == val) return;
   elm_config_scroll_thumbscroll_friction_standard_set(val);
   elm_config_all_flush();
   elm_config_save();
   change_config_owner();
}

static void
_thumbscroll_border_friction_change(void *data __UNUSED__,
                                    Evas_Object *obj,
                                    void *event_info __UNUSED__)
{
   double tsbf = elm_config_scroll_thumbscroll_border_friction_get();
   double val = elm_slider_value_get(obj);

   if (tsbf == val) return;
   elm_config_scroll_thumbscroll_border_friction_set(val);
   elm_config_all_flush();
   elm_config_save();
   change_config_owner();
}
static void
_thumbscroll_sensitivity_friction_change(void *data __UNUSED__,
                                         Evas_Object *obj,
                                         void *event_info __UNUSED__)
{
   double tssf = elm_config_scroll_thumbscroll_sensitivity_friction_get();
   double val = elm_slider_value_get(obj);

   if (tssf == val) return;
   elm_config_scroll_thumbscroll_sensitivity_friction_set(val);
   elm_config_all_flush();
   elm_config_save();
   change_config_owner();
}

static void
_thumbscroll_acceleration_threshold_change(void *data __UNUSED__,
                                         Evas_Object *obj,
                                         void *event_info __UNUSED__)
{
   double tssf = elm_config_scroll_thumbscroll_acceleration_threshold_get();
   double val = elm_slider_value_get(obj);

   if (tssf == val) return;
   elm_config_scroll_thumbscroll_acceleration_threshold_set(val);
   elm_config_all_flush();
   elm_config_save();
   change_config_owner();
}

static void
_thumbscroll_acceleration_time_limit_change(void *data __UNUSED__,
                                         Evas_Object *obj,
                                         void *event_info __UNUSED__)
{
   double tssf = elm_config_scroll_thumbscroll_acceleration_time_limit_get();
   double val = elm_slider_value_get(obj);

   if (tssf == val) return;
   elm_config_scroll_thumbscroll_acceleration_time_limit_set(val);
   elm_config_all_flush();
   elm_config_save();
   change_config_owner();
}

static void
_thumbscroll_acceleration_weight_change(void *data __UNUSED__,
                                         Evas_Object *obj,
                                         void *event_info __UNUSED__)
{
   double tssf = elm_config_scroll_thumbscroll_acceleration_weight_get();
   double val = elm_slider_value_get(obj);

   if (tssf == val) return;
   elm_config_scroll_thumbscroll_acceleration_weight_set(val);
   elm_config_all_flush();
   elm_config_save();
   change_config_owner();
}

static void _reset_button_cb(void *data __UNUSED__,
                             Evas_Object *obj __UNUSED__,
                             void *event_info __UNUSED__)
{
   const char *pdir;
   pdir = elm_config_profile_dir_get("mobile", EINA_TRUE);
   if(!pdir)
     return;

   ecore_file_recursive_rm(pdir);
   elm_config_profile_dir_free(pdir);

   elm_config_reload();
   elm_config_all_flush();
   elm_config_save();
   change_config_owner();
}

static char *_gl_text_get(void *data, Evas_Object *obj, const char *part)
{
   int index = ((int)(long)data) / 4;
   return strdup(slider_itemlist[index]);
}

static char *_gl_guide_text_get(void *data, Evas_Object *obj, const char *part)
{
   int index = ((int)(long)data) / 4;
   return strdup(guide_itemlist[index]);
}

static void _parse_negative_zero(char *str)
{
   char *ptr, *str_temp;

   ptr = strstr(str, "-0");
   if (ptr) {
        str_temp = ptr++;
        while (*ptr++) {
             if (*ptr == '0' || *ptr == '.')
               continue;
             else if (*ptr >='1' && *ptr <='9')
               break;
             else {
                  while (*str_temp) {
                       *str_temp = *(str_temp+1);
                       str_temp++;
                  }
                  break;
             }
        }
   }
}

static char* _indicator_format(double val)
{
   char *indicator = malloc(sizeof(char) * 64);
   if (!indicator) return NULL;
   snprintf(indicator, 64, "%1.0f", val);

   // Check and remove -0 state
   _parse_negative_zero(indicator);

   return indicator;
}

static char* _indicator_format_point(double val)
{
   char *indicator = malloc(sizeof(char) * 64);
   if (!indicator) return NULL;
   snprintf(indicator, 64, "%1.1f", val);

   // Check and remove -0 state
   _parse_negative_zero(indicator);

   return indicator;
}

static void _indicator_free(char *str)
{
   free(str);
}

static Evas_Object *_gl_content_get(void *data, Evas_Object *obj, const char *part)
{
   Evas_Object *slider;
   int index = ((int)(long)data);

   slider = elm_slider_add(obj);
   elm_slider_indicator_show_set(slider, EINA_TRUE);
   evas_object_size_hint_weight_set(slider, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(slider, EVAS_HINT_FILL, 0.5);
   elm_slider_indicator_format_set(slider, "%1.0f");

   if (index == 1) {
        elm_slider_indicator_format_function_set(slider, _indicator_format, _indicator_free);
        elm_slider_min_max_set(slider, 8, 500);
        elm_slider_value_set(slider, elm_config_scroll_thumbscroll_threshold_get());
        evas_object_smart_callback_add(slider, "slider,drag,stop", _thumbscroll_threshold_change,
                                       NULL);
   } else if (index == 5) {
        elm_slider_indicator_format_function_set(slider, _indicator_format, _indicator_free);
        elm_slider_min_max_set(slider, 4, 500);
        elm_slider_value_set(slider, elm_config_scroll_thumbscroll_hold_threshold_get());
        evas_object_smart_callback_add(slider, "slider,drag,stop", _thumbscroll_hold_threshold_change,
                                       NULL);
   } else if (index == 9) {
        elm_slider_indicator_format_function_set(slider, _indicator_format_point, _indicator_free);
        elm_slider_min_max_set(slider, 10.0, 200.0);
        elm_slider_value_set(slider, elm_config_scroll_thumbscroll_momentum_threshold_get());
        evas_object_smart_callback_add(slider, "slider,drag,stop", _thumbscroll_momentum_threshold_change,
                                       NULL);
   } else if (index == 13) {
        elm_slider_indicator_format_function_set(slider, _indicator_format_point, _indicator_free);
        elm_slider_min_max_set(slider, 0.1, 15.0);
        elm_slider_value_set(slider, elm_config_scroll_thumbscroll_friction_get());
        evas_object_smart_callback_add(slider, "slider,drag,stop", _thumbscroll_friction_change,
                                       NULL);
   } else if (index == 17) {
        elm_slider_indicator_format_function_set(slider, _indicator_format_point, _indicator_free);
        elm_slider_min_max_set(slider, 0.1, 15.0);
        elm_slider_value_set(slider, elm_config_scroll_thumbscroll_min_friction_get());
        evas_object_smart_callback_add(slider, "slider,drag,stop", _thumbscroll_min_friction_change,
                                       NULL);
   } else if (index == 21) {
        elm_slider_indicator_format_function_set(slider, _indicator_format_point, _indicator_free);
        elm_slider_min_max_set(slider, 10.0, 5000.0);
        elm_slider_value_set(slider, elm_config_scroll_thumbscroll_friction_standard_get());
        evas_object_smart_callback_add(slider, "slider,drag,stop", _thumbscroll_friction_standard_change,
                                       NULL);
   } else if (index == 25) {
        elm_slider_indicator_format_function_set(slider, _indicator_format_point, _indicator_free);
        elm_slider_min_max_set(slider, 0.0, 1.0);
        elm_slider_value_set(slider, elm_config_scroll_thumbscroll_border_friction_get());
        evas_object_smart_callback_add(slider, "slider,drag,stop", _thumbscroll_border_friction_change,
                                       NULL);
   } else if (index == 29) {
        elm_slider_indicator_format_function_set(slider, _indicator_format_point, _indicator_free);
        elm_slider_min_max_set(slider, 0.1, 1.0);
        elm_slider_value_set(slider, elm_config_scroll_thumbscroll_sensitivity_friction_get());
        evas_object_smart_callback_add(slider, "slider,drag,stop", _thumbscroll_sensitivity_friction_change,
                                       NULL);
   } else if (index == 33) {
        elm_slider_indicator_format_function_set(slider, _indicator_format_point, _indicator_free);
        elm_slider_min_max_set(slider, 10.0, 5000.0);
        elm_slider_value_set(slider, elm_config_scroll_thumbscroll_acceleration_threshold_get());
        evas_object_smart_callback_add(slider, "slider,drag,stop", _thumbscroll_acceleration_threshold_change,
                                       NULL);
   } else if (index == 37) {
        elm_slider_indicator_format_function_set(slider, _indicator_format_point, _indicator_free);
        elm_slider_min_max_set(slider, 0.0, 15.0);
        elm_slider_value_set(slider, elm_config_scroll_thumbscroll_acceleration_time_limit_get());
        evas_object_smart_callback_add(slider, "slider,drag,stop", _thumbscroll_acceleration_time_limit_change,
                                       NULL);
   } else if (index == 41) {
        elm_slider_indicator_format_function_set(slider, _indicator_format_point, _indicator_free);
        elm_slider_min_max_set(slider, 1.0, 10.0);
        elm_slider_value_set(slider, elm_config_scroll_thumbscroll_acceleration_weight_get());
        evas_object_smart_callback_add(slider, "slider,drag,stop", _thumbscroll_acceleration_weight_change,
                                       NULL);
   }
   return slider;
}

static Evas_Object* _create_genlist(Evas_Object* parent)
{
   Evas_Object *genlist;
   Elm_Object_Item *gl_item = NULL;
   Elm_Genlist_Item_Class *itc_name, *itc_slider, *itc_guide, *itc_sep;
   int index;

   genlist = elm_genlist_add(parent);
   elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);

   itc_name = elm_genlist_item_class_new();
   itc_slider = elm_genlist_item_class_new();
   itc_guide = elm_genlist_item_class_new();
   itc_sep = elm_genlist_item_class_new();

   itc_name->item_style = "dialogue/grouptitle";
   itc_name->func.text_get = _gl_text_get;
   itc_name->func.content_get = NULL;
   itc_name->func.state_get = NULL;
   itc_name->func.del = NULL;

   itc_slider->item_style = "dialogue/1icon";
   itc_slider->func.text_get = NULL;
   itc_slider->func.content_get = _gl_content_get;
   itc_slider->func.state_get = NULL;
   itc_slider->func.del = NULL;

   itc_guide->item_style = "multiline/1text";
   itc_guide->func.text_get = _gl_guide_text_get;
   itc_guide->func.content_get = NULL;
   itc_guide->func.state_get = NULL;
   itc_guide->func.del = NULL;

   itc_sep->item_style = "dialogue/separator";
   itc_sep->func.text_get = NULL;
   itc_sep->func.content_get = NULL;
   itc_sep->func.state_get = NULL;
   itc_sep->func.del = NULL;

   evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, 0.0);
   evas_object_show(genlist);

   for (index = 0; index < NUM_OF_ITEMS; index++) {
        switch(index % 4) {
           case 0:
              gl_item = elm_genlist_item_append(genlist, itc_name, (void *) index, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
              elm_genlist_item_select_mode_set(gl_item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
              break;
           case 1:
              gl_item = elm_genlist_item_append(genlist, itc_slider, (void *) index, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
              elm_genlist_item_select_mode_set(gl_item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
              break;
           case 2:
              gl_item = elm_genlist_item_append(genlist, itc_guide, (void *) index, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
              elm_genlist_item_select_mode_set(gl_item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
              break;
           case 3:
              gl_item = elm_genlist_item_append(genlist, itc_sep, (void *) index, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
              elm_genlist_item_select_mode_set(gl_item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
              break;
        }
   }
   elm_genlist_item_class_unref(itc_name);
   elm_genlist_item_class_unref(itc_slider);
   elm_genlist_item_class_unref(itc_guide);
   elm_genlist_item_class_unref(itc_sep);
   return genlist;
}

void config_scrolling_cb(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *genlist, *button, *icon;
   Elm_Object_Item *navi_it;
   struct appdata *ad = (struct appdata *) data;
   if (ad == NULL) return;

   genlist = _create_genlist(ad->nf);
   navi_it = elm_naviframe_item_push(ad->nf, _("Scrolling"), NULL, NULL, genlist, NULL);

   button = elm_button_add(ad->nf);
   elm_object_style_set(button, "naviframe/title_icon");
   icon = elm_image_add(ad->nf);
   elm_image_file_set(icon, ICON_DIR"/reset.png", NULL);
   evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   elm_object_part_content_set(button, "icon", icon);
   elm_object_item_part_content_set(navi_it, "title_right_btn", button);
   evas_object_smart_callback_add(button, "clicked", _reset_button_cb, genlist);
}

