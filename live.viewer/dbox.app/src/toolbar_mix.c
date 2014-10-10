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
#include <math.h>

Ecore_Timer *timer, *timer2;

#define ELM_MAX(v1, v2)    (((v1) > (v2)) ? (v1) : (v2))

static void segment_style_cb(void *data, Evas_Object *obj, void *event_info);
static void progressbar_style_cb(void *data, Evas_Object *obj, void *event_info);
static void loadingbar_style_cb(void *data, Evas_Object *obj, void *event_info);

static Eina_Bool _fn_pb_timer_bar(void *data);

static Evas_Object* _create_list_winset(Evas_Object* parent, struct _menu_item *menu, struct appdata* ad);

static Evas_Object *create_segment(struct appdata *ad);
static Evas_Object *create_progressbar(struct appdata *ad);
static Evas_Object *create_loadingbar(struct appdata *ad);

static struct _menu_item menu_main[] = {
   { "Segment Style", segment_style_cb},
   { "ProgressBar Style", progressbar_style_cb},
   { "LoadingBar Style", loadingbar_style_cb},
   /* do not delete below */
   { NULL, NULL }
};

static void _delete_timer()
{
   if (timer){
      ecore_timer_del(timer);
      timer = NULL;
   }

   if (timer2){
      ecore_timer_del(timer2);
      timer = NULL;
   }
}

static Eina_Bool _pop_cb(void *data, Elm_Object_Item *it)
{
   _delete_timer();
   return EINA_TRUE;
}

static void _response_cb(void *data, Evas_Object *obj, void *event_info)
{
   evas_object_del(data);
}

static void _toolbar_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
   struct appdata *ad = (struct appdata *)data;
   Evas_Object *popup, *btn1, *btn2;

   popup = elm_popup_add(ad->win_main);
   evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_object_text_set(popup,"Toolbar item was clicked. if you want to go back, press any button.");
   btn1 = elm_button_add(popup);
   elm_object_style_set(btn1, "popup");
   elm_object_text_set(btn1, "OK");
   elm_object_part_content_set(popup, "button1", btn1);
   evas_object_smart_callback_add(btn1, "clicked", _response_cb, popup);
   btn2 = elm_button_add(popup);
   elm_object_style_set(btn2, "popup");
   elm_object_text_set(btn2, "Cancel");
   elm_object_part_content_set(popup, "button2", btn2);
   evas_object_smart_callback_add(btn2, "clicked", _response_cb, popup);
   evas_object_show(popup);
}

static void segment_style_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *toolbar;
	Elm_Object_Item *navi_it;
	struct appdata *ad;

	ad = (struct appdata *) data;
	if (ad == NULL) return;

	_delete_timer();

	navi_it = elm_naviframe_top_item_get(ad->nf);
	toolbar = create_segment(ad);
	elm_object_item_part_content_set(navi_it, "controlbar", toolbar);
}

static void progressbar_style_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *toolbar;
	Elm_Object_Item *navi_it;
	struct appdata *ad;

	ad = (struct appdata *) data;
	if (ad == NULL) return;

	_delete_timer();

	navi_it = elm_naviframe_top_item_get(ad->nf);
	toolbar = create_progressbar(ad);
	elm_object_item_part_content_set(navi_it, "controlbar", toolbar);
}

static void loadingbar_style_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *toolbar;
	Elm_Object_Item *navi_it;
	struct appdata *ad;

	ad = (struct appdata *) data;
	if (ad == NULL) return;

	_delete_timer();

	navi_it = elm_naviframe_top_item_get(ad->nf);
	toolbar = create_loadingbar(ad);
	elm_object_item_part_content_set(navi_it, "controlbar", toolbar);
}

static void _list_click(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Object_Item *it = (Elm_Object_Item *) elm_list_selected_item_get(obj);
   if (it == NULL) return;

   elm_list_item_selected_set(it, EINA_FALSE);
}

static Evas_Object* _create_list_winset(Evas_Object* parent, struct _menu_item *menu, struct appdata *ad)
{
   Evas_Object *li;

   if (parent == NULL || ad == NULL) return NULL;

   li = elm_list_add(parent);
   elm_list_mode_set(li, ELM_LIST_COMPRESS);
   evas_object_smart_callback_add(li, "selected", _list_click, NULL);

   int idx = 0;

   while (menu[ idx ].name != NULL) {

          elm_list_item_append(
                          li,
                          menu[ idx ].name,
                          NULL,
                          NULL,
                          menu[ idx ].func,
                          ad);
          ++idx;
   }

   elm_list_go(li);

   return li;
}

static Eina_Bool _fn_pb_timer_bar(void *data)
{
   double value=0.0;
   Evas_Object *progressbar = (Evas_Object*) data;

   value = elm_progressbar_value_get(progressbar);
   if (value == 1.0)
          value = 0.0;
   value = value + 0.01;
   elm_progressbar_value_set(progressbar, value);
   return ECORE_CALLBACK_RENEW;
}

static Evas_Object *create_segment(struct appdata *ad)
{
   Evas_Object *obj;
   Evas_Object *segment;
   Evas_Object *box;
   Elm_Object_Item *item[3], *seg_it;

   /* create toolbar */
   obj = elm_toolbar_add(ad->nf);
   if (obj == NULL) return NULL;
   elm_toolbar_shrink_mode_set(obj, ELM_TOOLBAR_SHRINK_EXPAND);
   elm_toolbar_homogeneous_set(obj, EINA_FALSE);

   item[0] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_songs.png", "Songs", _toolbar_clicked_cb, ad);

   box = elm_box_add(obj);
   elm_box_horizontal_set(box, EINA_FALSE);
   evas_object_size_hint_min_set(box, 180, 20);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(box);

   segment = elm_segment_control_add(box);
   evas_object_size_hint_weight_set(segment, EVAS_HINT_EXPAND, 0.5);
   evas_object_size_hint_align_set(segment, EVAS_HINT_FILL, 0.5);
   seg_it = elm_segment_control_item_add(segment, NULL, "All");
   elm_segment_control_item_add(segment, NULL, "Call");
   elm_segment_control_item_add(segment, NULL, "Message");
   elm_segment_control_item_selected_set(seg_it, EINA_TRUE);
   evas_object_show(segment);
   elm_box_pack_end(box, segment);

   item[1] = elm_toolbar_item_append(obj, NULL, NULL, NULL, NULL);
   elm_object_item_part_content_set(item[1], "object", box);

   return obj;
}

static Evas_Object *create_progressbar(struct appdata *ad)
{
   Evas_Object *obj;
   Evas_Object *progressbar;
   Evas_Object *box;
   Evas_Object *label;
   Elm_Object_Item *item[3];

   /* create toolbar */
   obj = elm_toolbar_add(ad->nf);
   if (obj == NULL) return NULL;
   elm_toolbar_shrink_mode_set(obj, ELM_TOOLBAR_SHRINK_EXPAND);
   elm_toolbar_homogeneous_set(obj, EINA_FALSE);

   item[0] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_songs.png", "Songs", _toolbar_clicked_cb, ad);

   box = elm_box_add(obj);
   elm_box_horizontal_set(box, EINA_FALSE);
   evas_object_size_hint_min_set(box, 180, 20);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(box);

   progressbar = elm_progressbar_add(box);
   elm_object_style_set(progressbar, "list_progress");
   elm_progressbar_horizontal_set(progressbar, EINA_TRUE);
   evas_object_size_hint_weight_set(progressbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(progressbar, EVAS_HINT_FILL, 1.0);
   elm_progressbar_span_size_set(progressbar, 150);
   elm_progressbar_value_set(progressbar, 0.0);
   timer = ecore_timer_add(0.1, _fn_pb_timer_bar, progressbar);
   evas_object_show(progressbar);
   elm_box_pack_end(box, progressbar);

   label = elm_label_add(box);
   elm_object_text_set(label, "<font_size=18><color=#ffffff>progressbar</color></font_size>");
   evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(label, 0.5, 0.5);
   evas_object_show(label);
   elm_box_pack_end(box, label);

   item[1] = elm_toolbar_item_append(obj, NULL, NULL, NULL, NULL);
   elm_object_item_part_content_set(item[1], "object", box);

   return obj;
}

static Evas_Object *create_loadingbar(struct appdata *ad)
{
   Evas_Object *obj;
   Evas_Object *progressbar;
   Evas_Object *box;
   Evas_Object *label;
   Elm_Object_Item *item[3];

   /* create toolbar */
   obj = elm_toolbar_add(ad->nf);
   if (obj == NULL) return NULL;
   elm_toolbar_shrink_mode_set(obj, ELM_TOOLBAR_SHRINK_EXPAND);
   elm_toolbar_homogeneous_set(obj, EINA_FALSE);

   item[0] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_songs.png", "Songs", _toolbar_clicked_cb, ad);

   box = elm_box_add(obj);
   elm_box_horizontal_set(box, EINA_TRUE);
   elm_box_homogeneous_set(box, EINA_FALSE);
   evas_object_size_hint_min_set(box, 180, 20);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(box, 0.5, EVAS_HINT_FILL);
   evas_object_show(box);

   progressbar = elm_progressbar_add(box);
   elm_object_style_set(progressbar, "toolbar_process");
   elm_progressbar_horizontal_set(progressbar, EINA_TRUE);
   evas_object_size_hint_weight_set(progressbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(progressbar, 0.5, 0.5);
   elm_progressbar_span_size_set(progressbar, 60);
   elm_progressbar_pulse(progressbar, EINA_TRUE);
   evas_object_show(progressbar);
   elm_box_pack_end(box, progressbar);

   label = elm_label_add(box);
   elm_object_text_set(label, "<font_size=14><color=#ffffff>Activity Indicator Progressbar</color></font_size>");
   evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(label, 0.5, 0.5);
   evas_object_show(label);
   elm_box_pack_end(box, label);

   item[1] = elm_toolbar_item_append(obj, NULL, NULL, NULL, NULL);
   elm_object_item_part_content_set(item[1], "object", box);

   return obj;
}

void mix_cb(void *data, Evas_Object *obj, void *event_info)
{
   struct appdata *ad;
   Evas_Object *sub_view = NULL;
   Evas_Object *toolbar;
	Elm_Object_Item *navi_it;

   ad = (struct appdata *) data;
   if (ad == NULL) return;

   sub_view = _create_list_winset(obj, menu_main, ad);
   navi_it = elm_naviframe_item_push(ad->nf, _("Mixbar"), NULL, NULL, sub_view, NULL);
	elm_naviframe_item_pop_cb_set(navi_it, _pop_cb, ad);
   toolbar = create_segment(ad);
	elm_object_item_part_content_set(navi_it, "controlbar", toolbar);
}
