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

/*********************************************************
Segment Control
 ********************************************************/

typedef struct _Layout_info Layout_info;
struct _Layout_info
{
   Evas_Object* btn1;
   Evas_Object* btn2;
   Evas_Object* btn3;
   Evas_Object* ctrlb_segment;
   Evas_Object* segment1;
   Evas_Object* segment2;
   Evas_Object* disabled_seg1;
   Evas_Object* segment4;
   Evas_Object* disabled_seg2;
};

static void
_cb1(void* data, Evas_Object* obj, void* event_info)
{
   Elm_Object_Item * it = NULL;
   Layout_info *info = data;

   int count = elm_segment_control_item_count_get(info->segment1);
   if (count > 1)
     {
        it = elm_segment_control_item_selected_get(info->segment1);
        if (it)
          elm_object_item_del(it);
     }
   count = elm_segment_control_item_count_get(info->segment2);
   if (count > 1)
     {
        it = elm_segment_control_item_selected_get(info->segment2);
        if (it)
          elm_object_item_del(it);
     }
   count = elm_segment_control_item_count_get(info->disabled_seg1);
   if (count > 1)
     {
        it = elm_segment_control_item_selected_get(info->disabled_seg1);
        if (it)
          elm_object_item_del(it);
     }
   count = elm_segment_control_item_count_get(info->segment4);
   if (count > 1)
     {
        it = elm_segment_control_item_selected_get(info->segment4);
        if (it)
          elm_object_item_del(it);
     }
   count = elm_segment_control_item_count_get(info->disabled_seg2);
   if (count > 1)
     {
        it = elm_segment_control_item_selected_get(info->disabled_seg2);
        if (it)
          elm_object_item_del(it);
     }
   count = elm_segment_control_item_count_get(info->ctrlb_segment);
   if (count > 1)
     {
        it = elm_segment_control_item_selected_get(info->ctrlb_segment);
        if (it)
          elm_object_item_del(it);
     }
   return;
}

static void
_cb3(void* data, Evas_Object* obj, void* event_info)
{
   Layout_info *info = data;

   int count = elm_segment_control_item_count_get(info->segment1);
   if (count < 7)
     {
        char buf[PATH_MAX];
        Evas_Object *ic;

        ic = elm_image_add(info->segment1);
        snprintf(buf, sizeof(buf), "%s/logo_small.png", ICON_DIR);
        elm_image_file_set(ic, buf, NULL);
        evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_BOTH, 1, 1);

        elm_segment_control_item_insert_at(info->segment1, ic, "ins", 1);
     }
   count = elm_segment_control_item_count_get(info->segment2);
   if (count < 7)
     elm_segment_control_item_insert_at(info->segment2, NULL, "ins", 1);
   count = elm_segment_control_item_count_get(info->disabled_seg1);
   if (count < 7)
     elm_segment_control_item_insert_at(info->disabled_seg1, NULL, "disabled ins", 1);
   count = elm_segment_control_item_count_get(info->segment4);
   if (count < 7)
     elm_segment_control_item_insert_at(info->segment4, NULL, "ins", 1);
   count = elm_segment_control_item_count_get(info->disabled_seg2);
   if (count < 7)
     elm_segment_control_item_insert_at(info->disabled_seg2, NULL, "disabled ins", 1);
   count = elm_segment_control_item_count_get(info->ctrlb_segment);
   if (count < 7)
     elm_segment_control_item_insert_at(info->ctrlb_segment, NULL, "ins", 1);
   return;
}

static void
_cb2(void* data, Evas_Object* obj, void* event_info)
{
   Elm_Object_Item * it = NULL;
   Layout_info *info = data;

   int count = elm_segment_control_item_count_get(info->segment1);
   if (count < 7)
     {
        char buf[PATH_MAX];
        Evas_Object *ic;

        ic = elm_image_add(info->segment1);
        snprintf(buf, sizeof(buf), "%s/logo_small.png", ICON_DIR);
        elm_image_file_set(ic, buf, NULL);
        evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_BOTH, 1, 1);

        elm_segment_control_item_add(info->segment1, ic, NULL);
     }
   count = elm_segment_control_item_count_get(info->segment2);
   if (count < 7)
     {
        it = elm_segment_control_item_add(info->segment2, NULL, "Add");
        elm_object_item_part_text_set(it, "elm.text.badge", "99");
     }
   count = elm_segment_control_item_count_get(info->disabled_seg1);
   if (count < 7)
     elm_segment_control_item_add(info->disabled_seg1, NULL, "disabled add");
   count = elm_segment_control_item_count_get(info->segment4);
   if (count < 7)
     {
        it = elm_segment_control_item_add(info->segment4, NULL, "Add");
        elm_object_item_part_text_set(it, "elm.text.badge", "999");
     }
   count = elm_segment_control_item_count_get(info->disabled_seg2);
   if (count < 7)
      elm_segment_control_item_add(info->disabled_seg2, NULL, "disabled add");
   count = elm_segment_control_item_count_get(info->ctrlb_segment);
   if (count < 7)
      elm_segment_control_item_add(info->ctrlb_segment, NULL, "add");
   return;
}

static Eina_Bool _pop_cb(void *data, Elm_Object_Item *it)
{
   Layout_info *info = data;
   if (info)
     {
        free (info);
        info = NULL;
     }

   return EINA_TRUE;
}

static void _more_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Object_Item *navi_it = (Elm_Object_Item *) data;
   int oh_closed = (int) elm_object_item_data_get(navi_it);

   if (oh_closed)
     elm_object_item_signal_emit(navi_it, "elm,state,optionheader,open", "");
   else
     elm_object_item_signal_emit(navi_it, "elm,state,optionheader,close", "");

   elm_object_item_data_set(navi_it, (void *) !oh_closed);
}

void segmented_control_cb(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Object_Item *it[5];
   Evas_Object *ic[9];
   Evas_Object *in_layout;
   Evas_Object *toolbar;
   Elm_Object_Item *item[3];
   Evas_Object *scroller;
   Evas_Object *more_btn;
   Elm_Object_Item *navi_it;
   char buf[9][PATH_MAX];
   Layout_info *info;

   struct appdata *ad = (struct appdata *) data;
   if(ad == NULL) return;

   in_layout = elm_layout_add(ad->nf);
   elm_layout_file_set(in_layout, ELM_DEMO_EDJ, "elmdemo-test/segment");
   evas_object_size_hint_weight_set(in_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

   info = calloc(1, sizeof(Layout_info));
   info->segment1 = elm_segment_control_add(in_layout);
   elm_object_style_set(info->segment1, "body_style");
   ic[0] = elm_image_add(info->segment1);
   snprintf(buf[0], sizeof(buf[0]), "%s/logo.png", ICON_DIR);
   elm_image_file_set(ic[0], buf[0], NULL);
   evas_object_size_hint_aspect_set(ic[0], EVAS_ASPECT_CONTROL_BOTH, 1, 1);

   ic[1] = elm_image_add(info->segment1);
   snprintf(buf[1], sizeof(buf[1]), "%s/logo_small.png", ICON_DIR);
   elm_image_file_set(ic[1], buf[1], NULL);
   evas_object_size_hint_aspect_set(ic[1], EVAS_ASPECT_CONTROL_BOTH, 1, 1);

   ic[2] = elm_image_add(info->segment1);
   snprintf(buf[2], sizeof(buf[2]), "%s/logo.png", ICON_DIR);
   elm_image_file_set(ic[2], buf[2], NULL);
   evas_object_size_hint_aspect_set(ic[2], EVAS_ASPECT_CONTROL_BOTH, 1, 1);

   ic[3] = elm_image_add(info->segment1);
   snprintf(buf[3], sizeof(buf[3]), "%s/logo_small.png", ICON_DIR);
   elm_image_file_set(ic[3], buf[3], NULL);
   evas_object_size_hint_aspect_set(ic[3], EVAS_ASPECT_CONTROL_BOTH, 1, 1);

   ic[4] = elm_image_add(info->segment1);
   snprintf(buf[4], sizeof(buf[4]), "%s/logo.png", ICON_DIR);
   elm_image_file_set(ic[4], buf[4], NULL);
   evas_object_size_hint_aspect_set(ic[4], EVAS_ASPECT_CONTROL_BOTH, 1, 1);

   it[0] = elm_segment_control_item_add(info->segment1, ic[0], "Text Is Ellipsizing");
   elm_object_item_part_text_set(it[0], "elm.text.badge", "9");
   it[1] = elm_segment_control_item_add(info->segment1, ic[1], NULL);
   elm_object_item_part_text_set(it[1], "elm.text.badge", "99");
   it[2] = elm_segment_control_item_add(info->segment1, ic[2], NULL);
   elm_object_item_part_text_set(it[2], "elm.text.badge", "999");
   it[3] = elm_segment_control_item_add(info->segment1, ic[3], "Message");
   elm_object_item_part_text_set(it[3], "elm.text.badge", "9");
   it[4] = elm_segment_control_item_add(info->segment1, ic[4], NULL);
   elm_object_item_part_text_set(it[4], "elm.text.badge", "99");
   elm_segment_control_item_selected_set(it[4], EINA_TRUE);

   info->segment2 = elm_segment_control_add(in_layout);
   elm_object_style_set(info->segment2, "multiline");

   ic[5] = elm_image_add(info->segment2);
   snprintf(buf[5], sizeof(buf[5]), "%s/logo_small.png", ICON_DIR);
   elm_image_file_set(ic[5], buf[5], NULL);
   evas_object_size_hint_aspect_set(ic[5], EVAS_ASPECT_CONTROL_BOTH, 1, 1);

   it[0] = elm_segment_control_item_add(info->segment2, NULL, "All");
   elm_object_item_part_text_set(it[0], "elm.text.badge", "9999");
   it[1] = elm_segment_control_item_add(info->segment2, NULL, "Text Is Ellipsizing");
   it[2] = elm_segment_control_item_add(info->segment2, ic[5], "Message");
   it[3] = elm_segment_control_item_add(info->segment2, NULL, "Segment Control");
   it[4] = elm_segment_control_item_add(info->segment2, NULL, "Phone");
   elm_object_item_part_text_set(it[4], "elm.text.badge", "99999");
   elm_segment_control_item_selected_set(it[2], EINA_TRUE);

   info->disabled_seg1 = elm_segment_control_add(in_layout);
   elm_object_style_set(info->disabled_seg1, "multiline");

   ic[6] = elm_image_add(info->disabled_seg1);
   snprintf(buf[6], sizeof(buf[6]), "%s/logo_small.png", ICON_DIR);
   elm_image_file_set(ic[6], buf[6], NULL);
   evas_object_size_hint_aspect_set(ic[6], EVAS_ASPECT_CONTROL_BOTH, 1, 1);

   it[0] = elm_segment_control_item_add(info->disabled_seg1, NULL, "Disabled");
   elm_object_item_part_text_set(it[0], "elm.text.badge", "99");
   it[1] = elm_segment_control_item_add(info->disabled_seg1, NULL, "DisabledEllipsizing");
   it[2] = elm_segment_control_item_add(info->disabled_seg1, NULL, "Disabled");
   it[3] = elm_segment_control_item_add(info->disabled_seg1, ic[6], "Disabled");
   elm_object_item_part_text_set(it[3], "elm.text.badge", "999");
   it[4] = elm_segment_control_item_add(info->disabled_seg1, NULL, "Disabled");
   elm_segment_control_item_selected_set(it[0], EINA_TRUE);
   elm_object_disabled_set(info->disabled_seg1, EINA_TRUE);

   info->segment4 = elm_segment_control_add(in_layout);
   elm_object_style_set(info->segment4, "title_multiline");

   ic[7] = elm_image_add(info->segment4);
   snprintf(buf[7], sizeof(buf[7]), "%s/logo_small.png", ICON_DIR);
   elm_image_file_set(ic[7], buf[7], NULL);
   evas_object_size_hint_aspect_set(ic[7], EVAS_ASPECT_CONTROL_BOTH, 1, 1);

   it[0] = elm_segment_control_item_add(info->segment4, NULL, "Text Is Ellipsizing");
   it[1] = elm_segment_control_item_add(info->segment4, NULL, "Call");
   elm_object_item_part_text_set(it[1], "elm.text.badge", "9");
   it[2] = elm_segment_control_item_add(info->segment4, NULL, "Segment Control");
   elm_object_item_part_text_set(it[2], "elm.text.badge", "999999");
   it[3] = elm_segment_control_item_add(info->segment4, ic[7], "Phone");
   elm_segment_control_item_selected_set(it[0], EINA_TRUE);

   info->disabled_seg2 = elm_segment_control_add(in_layout);
   elm_object_style_set(info->disabled_seg2, "multiline");

   ic[8] = elm_image_add(info->disabled_seg2);
   snprintf(buf[8], sizeof(buf[8]), "%s/logo_small.png", ICON_DIR);
   elm_image_file_set(ic[8], buf[8], NULL);
   evas_object_size_hint_aspect_set(ic[8], EVAS_ASPECT_CONTROL_BOTH, 1, 1);

   it[0] = elm_segment_control_item_add(info->disabled_seg2, ic[8], "Disabled");
   it[1] = elm_segment_control_item_add(info->disabled_seg2, NULL, "Disabled");
   elm_object_item_part_text_set(it[0], "elm.text.badge", "999");
   it[2] = elm_segment_control_item_add(info->disabled_seg2, NULL, "DisabledEllipsizing");
   it[3] = elm_segment_control_item_add(info->disabled_seg2, NULL, "Disabled");
   elm_segment_control_item_selected_set(it[2], EINA_TRUE);
   elm_object_disabled_set(info->disabled_seg2, EINA_TRUE);

   /* create toolbar */
   toolbar = elm_toolbar_add(ad->nf);
   elm_toolbar_shrink_mode_set(toolbar, ELM_TOOLBAR_SHRINK_EXPAND);
   elm_object_style_set(toolbar, "naviframe");

   info->ctrlb_segment = elm_segment_control_add(toolbar);
   it[0] = elm_segment_control_item_add(info->ctrlb_segment, NULL, "All");
   elm_object_item_part_text_set(it[0], "elm.text.badge", "9");
   it[1] = elm_segment_control_item_add(info->ctrlb_segment, NULL, "Call");
   elm_object_item_part_text_set(it[1], "elm.text.badge", "99");
   it[2] = elm_segment_control_item_add(info->ctrlb_segment, NULL, "Message");
   elm_object_item_part_text_set(it[2], "elm.text.badge", "999");
   it[3] = elm_segment_control_item_add(info->ctrlb_segment, NULL, "Phone");
   elm_object_item_part_text_set(it[3], "elm.text.badge", "9");
   it[4] = elm_segment_control_item_add(info->ctrlb_segment, NULL, "Last Call");
   elm_object_item_part_text_set(it[4], "elm.text.badge", "99");
   elm_segment_control_item_selected_set(it[4], EINA_TRUE);

   info->btn1 = elm_button_add(ad->nf);
   elm_object_text_set(info->btn1, "Del (Selected one)");
   evas_object_show(info->btn1);

   info->btn2 = elm_button_add(ad->nf);
   elm_object_text_set(info->btn2, "Add (till item count 7)");
   evas_object_show(info->btn2);

   info->btn3 = elm_button_add(ad->nf);
   elm_object_text_set(info->btn3, "Insert (till item count 7)");
   evas_object_show(info->btn3);

   evas_object_smart_callback_add(info->btn1, "clicked", _cb1, info);
   evas_object_smart_callback_add(info->btn2, "clicked", _cb2, info);
   evas_object_smart_callback_add(info->btn3, "clicked", _cb3, info);

   elm_object_part_content_set(in_layout, "segment1", info->segment1);
   elm_object_part_content_set(in_layout, "segment2", info->segment2);
   elm_object_part_content_set(in_layout, "disabled_seg1", info->disabled_seg1);
   elm_object_part_content_set(in_layout, "segment4", info->segment4);
   elm_object_part_content_set(in_layout, "disabled_seg2", info->disabled_seg2);
   elm_object_part_content_set(in_layout, "add", info->btn2);
   elm_object_part_content_set(in_layout, "del", info->btn1);
   elm_object_part_content_set(in_layout, "insert", info->btn3);

   scroller = elm_scroller_add(ad->nf);
   elm_scroller_bounce_set(scroller, EINA_FALSE, EINA_TRUE);
   elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
   evas_object_show(scroller);
   elm_object_content_set(scroller, in_layout);

   item[0] = elm_toolbar_item_append(toolbar, NULL, NULL, NULL, NULL);
   elm_object_item_part_content_set(item[0], "object", info->ctrlb_segment);
   navi_it = elm_naviframe_item_push(ad->nf, _("SegmentControl"), NULL, NULL, scroller, NULL);
   elm_naviframe_item_pop_cb_set(navi_it, _pop_cb, info);
   elm_object_item_part_content_set(navi_it, "optionheader", toolbar);

   //More Button
   more_btn = elm_button_add(ad->nf);
   elm_object_style_set(more_btn, "naviframe/more/default");
   evas_object_smart_callback_add(more_btn, "clicked", _more_btn_cb, navi_it);
   elm_object_item_part_content_set(navi_it, "title_more_btn", more_btn);
   elm_object_item_data_set(navi_it, (void *) 0);

   return;
}

