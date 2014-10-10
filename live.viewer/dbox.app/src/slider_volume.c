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
#include "slider_volume.h"

/*********************************************************
	Slider_Volume
 ********************************************************/

#define VOLUME_POPUP_X 330
#define VOLUME_POPUP_Y 424

typedef struct _Slider_Data Slider_Data;
struct _Slider_Data
{
	Evas_Object *parent;
	Evas_Object *layout;
	Evas_Object *button;
	Evas_Object *volume_hor;
	Evas_Object *volume_ver;
	Evas_Object *volume_popup;
	Evas_Object *icon;
	Ecore_Timer *del_timer;
};

// when time is over, volume popup is hidden
static Eina_Bool _del_timer_cb(void *data)
{
	Slider_Data *sd = (Slider_Data*)data;

	evas_object_hide(sd->volume_popup);

	if(sd->del_timer) {
		ecore_timer_del(sd->del_timer);
		sd->del_timer = NULL;
	}
	return ECORE_CALLBACK_CANCEL;
}

// when the back button of naviframe item is pushed, volume popup is deleted
static Eina_Bool _pop_cb(void *data, Elm_Object_Item *it)
{
	Slider_Data *sd = (Slider_Data*)data;

	if(sd->del_timer) {
		ecore_timer_del(sd->del_timer);
		sd->del_timer = NULL;
	}
	evas_object_del(sd->volume_popup);

	return EINA_TRUE;
}

// when the sound button is clicked, popup is shown
static void _button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	Slider_Data *sd = (Slider_Data*)data;
	Evas_Coord x, y, w, h;
	int pos_x, pos_y;

	evas_object_geometry_get(sd->layout, &x, &y, &w, &h);
	pos_x = (x + w) / 2 - (int)(((double)VOLUME_POPUP_X * elm_config_scale_get()) / 2);
	pos_y = (y + h) / 2 - (int)(((double)VOLUME_POPUP_Y * elm_config_scale_get()) / 2);
	evas_object_move(sd->volume_popup, pos_x, pos_y);
	evas_object_show(sd->volume_popup);

	if(sd->del_timer) {
		ecore_timer_del(sd->del_timer);
		sd->del_timer = NULL;
	}
	sd->del_timer = ecore_timer_add(3, _del_timer_cb, sd);
}

// callback function for the value change of horizontal volume slider
static void _volume_hor_change_cb(void *data, Evas_Object *obj, void *event_info)
{
	Slider_Data *sd = (Slider_Data*)data;
	double value;
	char buf[50];

	value = elm_slider_value_get(sd->volume_hor);
	if (value > 14) value = 14;
	sprintf(buf, "Volume : %.0lf", value);
	edje_object_part_text_set(_EDJ(sd->layout), "text", buf);
	elm_slider_value_set(sd->volume_ver, (int)value);
	if(sd->del_timer) {
		ecore_timer_del(sd->del_timer);
		sd->del_timer = NULL;
	}
	sd->del_timer = ecore_timer_add(3, _del_timer_cb, sd);
}

// callback function for the value change of vertical volume slider
static void _volume_ver_change_cb(void *data, Evas_Object *obj, void *event_info)
{
	Slider_Data *sd = (Slider_Data*)data;
	double value;
	char buf[50];

	value = elm_slider_value_get(sd->volume_ver);
	if (value > 14) value = 14;
	sprintf(buf, "Volume : %.0lf", value);
	edje_object_part_text_set(_EDJ(sd->layout), "text", buf);
	elm_slider_value_set(sd->volume_hor, (int)value);
	if(sd->del_timer) {
		ecore_timer_del(sd->del_timer);
		sd->del_timer = NULL;
	}
	sd->del_timer = ecore_timer_add(3, _del_timer_cb, sd);
}

// when popup is shown, and rotation is occured.
static int _rotate_volume_cb(enum appcore_rm rotmode, void* data)
{
	Slider_Data *sd = (Slider_Data*)data;
	Evas_Coord x, y, w, h;
	int pos_x, pos_y;

	evas_object_geometry_get(sd->layout, &y, &x, &h, &w);
	pos_x = (x + w) / 2 - (int)(((double)VOLUME_POPUP_X * elm_config_scale_get()) / 2);
	pos_y = (y + h) / 2 - (int)(((double)VOLUME_POPUP_Y * elm_config_scale_get()) / 2);
	evas_object_move(sd->volume_popup, pos_x, pos_y);

	return 0;
}
static Slider_Data* _create_slider(Evas_Object* parent)
{
	static Slider_Data sd ;

	sd.parent = parent;

	// make a base layout
	sd.layout = elm_layout_add(parent);
	elm_layout_file_set(sd.layout, ELM_DEMO_EDJ, "slider_volume_layout");
	edje_object_part_text_set(_EDJ(sd.layout), "text", _("Volume : 0"));
	evas_object_show(sd.layout);

	// make a icon for a sound button
	sd.icon = elm_image_add(parent);
	elm_image_file_set(sd.icon, ICON_DIR"/00_volume_icon.png", NULL);
	evas_object_size_hint_aspect_set(sd.icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(sd.icon, 1, 1);

	// make a sound button
	sd.button = elm_button_add(parent);
	elm_object_style_set(sd.button, "style1");
	elm_object_part_content_set(sd.layout, "volume_button", sd.button);
	elm_object_content_set(sd.button, sd.icon);
	evas_object_smart_callback_add(sd.button, "clicked",_button_clicked_cb, &sd);

	// make a layout for the volume popup
	sd.volume_popup = elm_layout_add(parent);
	elm_layout_file_set(sd.volume_popup, ELM_DEMO_EDJ, "slider_volume_popup");
	edje_object_part_text_set(_EDJ(sd.volume_popup), "title_text", _("Automatically disappears after 3 seconds"));
	evas_object_resize(sd.volume_popup,
			(int)((double)VOLUME_POPUP_X * elm_config_scale_get()),
			(int)((double)VOLUME_POPUP_Y * elm_config_scale_get()));

	// add a horizontal volume
	sd.volume_hor = elm_slider_add(parent);
	elm_object_style_set(sd.volume_hor, "volume");
     	elm_slider_min_max_set(sd.volume_hor, 0, 15);
	evas_object_smart_callback_add(sd.volume_hor, "changed", _volume_hor_change_cb,  &sd);
	elm_object_part_content_set(sd.volume_popup, "volume_hor", sd.volume_hor);

	// add a vertical volume
	sd.volume_ver = elm_slider_add(parent);
	elm_object_style_set(sd.volume_ver, "volume");
	elm_slider_inverted_set(sd.volume_ver, EINA_TRUE);
	elm_slider_horizontal_set(sd.volume_ver, EINA_FALSE);
     	elm_slider_min_max_set(sd.volume_ver, 0, 15);
	evas_object_smart_callback_add(sd.volume_ver, "changed", _volume_ver_change_cb,  &sd);
	elm_object_part_content_set(sd.volume_popup, "volume_ver", sd.volume_ver);

	set_rotate_cb_for_winset(_rotate_volume_cb, &sd);

	return &sd;
}

static Evas_Object* _create_layout(Evas_Object *parent)
{
	Evas_Object *layout;
	layout = elm_layout_add(parent);
	elm_layout_theme_set(layout, "layout", "application", "default");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	return layout;
}

void slider_volume_cb(void *data, Evas_Object *obj, void *event_info)
{
	Slider_Data *sd;
	Evas_Object *layout;
	Elm_Object_Item *it;
	struct appdata *ad = (struct appdata *) data;
	if (ad == NULL) return;

	layout = _create_layout(ad->nf);
	sd = _create_slider(ad->nf);
	elm_object_part_content_set(layout, "elm.swallow.content", sd->layout);
	it = elm_naviframe_item_push(ad->nf, _("Volume Style"), NULL, NULL, layout, NULL);
	elm_naviframe_item_pop_cb_set(it, _pop_cb, sd);
}
