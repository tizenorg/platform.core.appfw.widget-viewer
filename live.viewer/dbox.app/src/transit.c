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
#include "transit.h"

struct custom_effect
{
	struct _size
	{
		Evas_Coord w, h;
	}from, to;
};

static void _list_click(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *it = (Elm_Object_Item *) elm_list_selected_item_get(obj);
	if(!it) return;
	elm_list_item_selected_set(it, EINA_FALSE);
}

static void _transit_del_cb(void *data, Elm_Transit *transit)
{
	set_rotation_mode(EINA_TRUE);
}

//Fade Effect Sample
static void _fade_cb(void *data, Evas_Object *obj, void *event_info)
{
	set_rotation_mode(EINA_FALSE);

	Evas_Object *layout = (Evas_Object *) data;
	Elm_Transit *transit = elm_transit_add();
	//Fade in and out with layout object.
	elm_transit_object_add(transit, layout);
	elm_transit_object_add(transit, layout);
	elm_transit_effect_fade_add(transit);
	elm_transit_duration_set(transit, 0.5);
	elm_transit_del_cb_set(transit, _transit_del_cb, NULL);
	elm_transit_go(transit);
}

//Translation Effect Sample
static void _translation_cb(void *data, Evas_Object *obj, void *event_info)
{
	set_rotation_mode(EINA_FALSE);

	Evas_Object *layout = (Evas_Object *) data;
	Evas_Coord w, h;
	evas_object_geometry_get(layout, NULL, NULL, &w, &h);

	//Step 1. Translate to left side.
	Elm_Transit *transit = elm_transit_add();
	elm_transit_object_add(transit, layout);
	elm_transit_effect_translation_add(transit, 0, 0, -w, 0);
	elm_transit_duration_set(transit, 0.5);

	//Step 2. Translate to up side.
	Elm_Transit *transit2 = elm_transit_add();
	elm_transit_object_add(transit2, layout);
	elm_transit_effect_translation_add(transit2, 0, h, 0, 0);
	elm_transit_duration_set(transit2, 0.5);
	elm_transit_del_cb_set(transit2, _transit_del_cb, NULL);

	elm_transit_chain_transit_add(transit, transit2);
	elm_transit_go(transit);
}

//Rotation Effect Sample
static void _rotation_cb(void *data, Evas_Object *obj, void *event_info)
{
	set_rotation_mode(EINA_FALSE);

	Evas_Object *nf = (Evas_Object *) data;
	Elm_Transit *transit = elm_transit_add();
	//360 degree rotation effect with clock wise direction.
	elm_transit_object_add(transit, nf);
	elm_transit_effect_rotation_add(transit, 0, 360);
	elm_transit_duration_set(transit, 1);
	elm_transit_del_cb_set(transit, _transit_del_cb, NULL);
	elm_transit_go(transit);
}

//Color Effect Sample
static void _color_cb(void *data, Evas_Object *obj, void *event_info)
{
	set_rotation_mode(EINA_FALSE);

	Evas_Object *layout = (Evas_Object *) data;
	//1. to be Red color.
	Elm_Transit *transit1 = elm_transit_add();
	elm_transit_object_add(transit1, layout);
	elm_transit_effect_color_add(transit1, 255, 255, 255, 255, 255, 0, 0, 255);
	elm_transit_duration_set(transit1, 0.5);

	//2. to be Green color.
	Elm_Transit *transit2 = elm_transit_add();
	elm_transit_object_add(transit2, layout);
	elm_transit_effect_color_add(transit2, 255, 0, 0, 255, 255, 255, 0, 255);
	elm_transit_duration_set(transit2, 0.5);

  	//3. to be original color.
	Elm_Transit *transit3 = elm_transit_add();
	elm_transit_object_add(transit3, layout);
	elm_transit_effect_color_add(transit3, 255, 255, 0, 255, 255, 255, 255, 255);
	elm_transit_duration_set(transit3, 0.5);
	elm_transit_del_cb_set(transit3, _transit_del_cb, NULL);

	elm_transit_chain_transit_add(transit1, transit2);
	elm_transit_chain_transit_add(transit2, transit3);
	elm_transit_go(transit1);
}

//Blend Effect Sample
static void _blend_cb(void *data, Evas_Object *obj, void *event_info)
{
	set_rotation_mode(EINA_FALSE);

	Evas_Object *layout = (Evas_Object *) data;
	//Let's Blend with layout and one image.
	//Create one image object.
	Evas_Object *img = evas_object_image_filled_add(evas_object_evas_get(layout));
	evas_object_image_file_set(img, ICON_DIR"/logo.png", NULL);
	evas_object_image_smooth_scale_set(img, EINA_TRUE);

	Evas_Coord x, y, w, h;
	evas_object_geometry_get(layout, &x, &y, &w, &h);
	evas_object_move(img, x, y);
	evas_object_resize(img, w ,h);

	//Let's blend ! You can blend with all widgets as well.
	Elm_Transit *transit = elm_transit_add();
	elm_transit_object_add(transit, layout);
  	elm_transit_object_add(transit, img);
	elm_transit_effect_blend_add(transit);
	elm_transit_duration_set(transit, 0.5);
	elm_transit_del_cb_set(transit, _transit_del_cb, NULL);

	elm_transit_go(transit);
}

//Flip Effect Sample
static void _flip_cb(void *data, Evas_Object *obj, void *event_info)
{
	set_rotation_mode(EINA_FALSE);

	Evas_Object *layout = (Evas_Object *) data;
	Elm_Transit* transit = elm_transit_add();
	elm_transit_smooth_set(transit, EINA_FALSE);
	elm_transit_object_add(transit, layout);
	elm_transit_object_add(transit, layout);
	//Flip based on Axis Y. and clock wise rotation direction.
	elm_transit_effect_flip_add(transit, ELM_TRANSIT_EFFECT_FLIP_AXIS_Y, EINA_TRUE);
	elm_transit_duration_set(transit, 0.8);
	elm_transit_del_cb_set(transit, _transit_del_cb, NULL);
	elm_transit_go(transit);
}

//Zoom Effect Sample
static void _zoom_cb(void *data, Evas_Object *obj, void *event_info)
{
	set_rotation_mode(EINA_FALSE);

	Evas_Object *layout = (Evas_Object *) data;
	//Zoom out to scale 0.6.
	Elm_Transit *transit = elm_transit_add();
	elm_transit_smooth_set(transit, EINA_FALSE);
	elm_transit_object_add(transit, layout);
	elm_transit_effect_zoom_add(transit, 1.0, 0.4);
	elm_transit_duration_set(transit, 0.5);

	//Zoom in to be original size.
	Elm_Transit *transit2 = elm_transit_add();
	elm_transit_smooth_set(transit2, EINA_FALSE);
	elm_transit_object_add(transit2, layout);
	elm_transit_effect_zoom_add(transit2, 0.4, 1.0);
	elm_transit_duration_set(transit2, 0.5);
	elm_transit_del_cb_set(transit2, _transit_del_cb, NULL);

	elm_transit_chain_transit_add(transit, transit2);
	elm_transit_go(transit);
}

//Wipe Effect Sample
static void _wipe_cb(void* data, Evas_Object* obj, void* event_info)
{
	set_rotation_mode(EINA_FALSE);

	Evas_Object *layout = (Evas_Object *) data;
	//Hide wipe effect. and direction is up.
	Elm_Transit* transit = elm_transit_add();
	elm_transit_object_add(transit, layout);
	elm_transit_effect_wipe_add(transit, ELM_TRANSIT_EFFECT_WIPE_TYPE_HIDE, ELM_TRANSIT_EFFECT_WIPE_DIR_UP);
	elm_transit_duration_set(transit, 0.5);

	//One more time wipe effect. In this case show up object and direction is right.
  	Elm_Transit* transit2 = elm_transit_add();
	elm_transit_object_add(transit2, layout);
	elm_transit_effect_wipe_add(transit2, ELM_TRANSIT_EFFECT_WIPE_TYPE_SHOW, ELM_TRANSIT_EFFECT_WIPE_DIR_RIGHT);
	elm_transit_duration_set(transit2, 0.5);
        elm_transit_del_cb_set(transit2, _transit_del_cb, NULL);

	elm_transit_chain_transit_add(transit, transit2);
	elm_transit_go(transit);
}

//ImageAnimation Effect Sample
static void _imageanimation_cb(void *data, Evas_Object *obj, void *event_info)
{
	set_rotation_mode(EINA_FALSE);

	const char *calendar_images[] = {
		ICON_DIR"/animatedicon/calendar_00.png",
		ICON_DIR"/animatedicon/calendar_01.png",
		ICON_DIR"/animatedicon/calendar_02.png",
		ICON_DIR"/animatedicon/calendar_03.png",
		ICON_DIR"/animatedicon/calendar_04.png",
		ICON_DIR"/animatedicon/calendar_05.png",
		ICON_DIR"/animatedicon/calendar_06.png",
		ICON_DIR"/animatedicon/calendar_07.png",
		ICON_DIR"/animatedicon/calendar_08.png",
		ICON_DIR"/animatedicon/calendar_09.png",
		ICON_DIR"/animatedicon/calendar_10.png",
		ICON_DIR"/animatedicon/calendar_11.png",
		NULL
	};

	const char *message_images[] = {
		ICON_DIR"/animatedicon/message_00.png",
		ICON_DIR"/animatedicon/message_01.png",
		ICON_DIR"/animatedicon/message_02.png",
		ICON_DIR"/animatedicon/message_03.png",
		ICON_DIR"/animatedicon/message_04.png",
		ICON_DIR"/animatedicon/message_05.png",
		ICON_DIR"/animatedicon/message_06.png",
		ICON_DIR"/animatedicon/message_07.png",
		ICON_DIR"/animatedicon/message_08.png",
		ICON_DIR"/animatedicon/message_09.png",
		ICON_DIR"/animatedicon/message_10.png",
		ICON_DIR"/animatedicon/message_11.png",
		ICON_DIR"/animatedicon/message_12.png",
		ICON_DIR"/animatedicon/message_13.png",
		ICON_DIR"/animatedicon/message_14.png",
		ICON_DIR"/animatedicon/message_15.png",
		ICON_DIR"/animatedicon/message_16.png",
		ICON_DIR"/animatedicon/message_17.png",
		NULL
	};

	Evas_Object *nf = (Evas_Object *) data;

	//layout
	Evas_Object *layout = elm_layout_add(nf);
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "group_imageanimation");
	elm_naviframe_item_push(nf, _("ImageAnimation"), NULL, NULL, layout, NULL);

	Elm_Transit *transit;
	Eina_List *images = NULL;
	Eina_List *images2 = NULL;
	Evas_Object *icon;
	int i;

	//Transit - Calendar
	icon = elm_image_add(nf);
	elm_object_part_content_set(layout, "calendar_icon", icon);

	for(i = 0; i <12; ++i) {
		images = eina_list_append(images, eina_stringshare_add(calendar_images[i]));
	}

	transit = elm_transit_add();
	elm_transit_object_add(transit, icon);
	elm_transit_effect_image_animation_add(transit, images);
	elm_transit_auto_reverse_set(transit, EINA_TRUE);
	elm_transit_objects_final_state_keep_set(transit, EINA_TRUE);
	elm_transit_duration_set(transit, 1);
	elm_transit_go(transit);

	//Transit - Message
	icon = elm_image_add(nf);
	elm_object_part_content_set(layout, "message_icon", icon);

	for(i = 0; i <18; ++i) {
		images2 = eina_list_append(images2, eina_stringshare_add(message_images[i]));
	}

	transit = elm_transit_add();
	elm_transit_object_add(transit, icon);
	elm_transit_effect_image_animation_add(transit, images2);
	elm_transit_auto_reverse_set(transit, EINA_TRUE);
	elm_transit_objects_final_state_keep_set(transit, EINA_TRUE);
	elm_transit_duration_set(transit, 1);
	elm_transit_del_cb_set(transit, _transit_del_cb, NULL);
	elm_transit_go(transit);
}

//Resizable Flip Effect Sample
static void _resizable_flip_del(void *data, Elm_Transit *trans)
{
	//Delete button object.
	evas_object_del(data);
	set_rotation_mode(EINA_TRUE);
}

static void _resizable_flip_cb(void *data, Evas_Object *obj, void *event_info)
{
	set_rotation_mode(EINA_FALSE);

	Evas_Object* layout = (Evas_Object *) data;
	//Let's Flip with one button object and original naviframe object.
	//Create a button object.
	Evas_Object *btn = elm_button_add(layout);
	elm_object_text_set(btn, "Front");

	Evas_Coord w, h;
	evas_object_geometry_get(layout, NULL, NULL, &w, &h);

	//Compute front face object size and position for scalability.
	evas_object_move(btn, (w - w * 0.25) / 2, (h - h * 0.25) / 2);
	evas_object_resize(btn, w * 0.25, h * 0.25);
	evas_object_show(btn);

	Elm_Transit* transit = elm_transit_add();
	elm_transit_smooth_set(transit, EINA_FALSE);
	elm_transit_object_add(transit, btn);
	elm_transit_object_add(transit, layout);
	elm_transit_effect_resizable_flip_add(transit, ELM_TRANSIT_EFFECT_FLIP_AXIS_Y, EINA_TRUE);
	elm_transit_del_cb_set(transit, _resizable_flip_del, btn);
	elm_transit_duration_set(transit, 0.8);
	elm_transit_go(transit);
}

//Custom Effect Sample
static void _custom_context_free(Elm_Transit_Effect *effect, Elm_Transit *transit)
{
	struct custom_effect *custom_effect = effect;
	free(custom_effect);
}

static void _custom_op(Elm_Transit_Effect *effect, Elm_Transit *transit, double progress)
{
	if (!effect) return;

	Evas_Coord w, h;
	Evas_Object *obj;
	const Eina_List *elist;

	struct custom_effect *custom_effect = (struct custom_effect *) effect;
	const Eina_List *objs = elm_transit_objects_get(transit);

	if (progress < 0.5) {
		h = custom_effect->from.h + (custom_effect->to.h * progress * 2);
		w = custom_effect->from.w;
	}else {
		h = custom_effect->from.h + custom_effect->to.h;
		w = custom_effect->from.w + (custom_effect->to.w * (progress - 0.5) * 2);
	}

	EINA_LIST_FOREACH(objs, elist, obj) {
		evas_object_resize(obj, w, h);
	}
}

static Elm_Transit_Effect * _custom_context_new(Evas_Coord from_w, Evas_Coord from_h, Evas_Coord to_w, Evas_Coord to_h)
{
	struct custom_effect *custom_effect = calloc( 1, sizeof( struct custom_effect ) );
	if (!custom_effect) return NULL;

	custom_effect->from.w = from_w;
	custom_effect->from.h = from_h;
	custom_effect->to.w = to_w - from_w;
	custom_effect->to.h = to_h - from_h;
	return custom_effect;
}

static void _custom_cb(void *data, Evas_Object *obj, void *event_info)
{
	set_rotation_mode(EINA_FALSE);

	Evas_Object *layout = (Evas_Object *) data;

	Evas_Coord w, h;
	evas_object_geometry_get(layout, NULL, NULL, &w, &h);
	Elm_Transit_Effect *effect_context = _custom_context_new(w, h, w / 1.5, h / 1.5);

	Elm_Transit *transit = elm_transit_add();
	elm_transit_object_add(transit, layout);
	elm_transit_tween_mode_set(transit, ELM_TRANSIT_TWEEN_MODE_DECELERATE);
	elm_transit_effect_add(transit, _custom_op, effect_context, _custom_context_free);
	elm_transit_auto_reverse_set(transit, EINA_TRUE);
	elm_transit_duration_set(transit, 1.2);
	elm_transit_del_cb_set(transit, _transit_del_cb, NULL);
	elm_transit_go(transit);
}

void effect_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *list;

	struct appdata *ad = (struct appdata *) data;

	//List
	list = elm_list_add(ad->nf);
	elm_list_mode_set(list, ELM_LIST_COMPRESS);
	evas_object_smart_callback_add(list, "selected", _list_click, NULL);

	elm_list_item_append(list, "Blend", NULL, NULL, _blend_cb, ad->layout_main);
	elm_list_item_append(list, "Color", NULL, NULL, _color_cb, ad->layout_main);
	elm_list_item_append(list, "Fade", NULL, NULL, _fade_cb, ad->layout_main);
	elm_list_item_append(list, "ImageAnimation", NULL, NULL, _imageanimation_cb, ad->nf);
	elm_list_item_append(list, "Flip", NULL, NULL, _flip_cb, ad->layout_main);
	elm_list_item_append(list, "Rotation", NULL, NULL, _rotation_cb, ad->layout_main);
	elm_list_item_append(list, "ResizableFlip", NULL, NULL, _resizable_flip_cb, ad->layout_main);
	elm_list_item_append(list, "Translation", NULL, NULL, _translation_cb, ad->layout_main);
	elm_list_item_append(list, "Wipe", NULL, NULL, _wipe_cb, ad->layout_main);
	elm_list_item_append(list, "Zoom", NULL, NULL, _zoom_cb, ad->layout_main);
	elm_list_item_append(list, "Custom", NULL, NULL, _custom_cb, ad->layout_main);
	elm_list_go(list);

	elm_naviframe_item_push(ad->nf, _("Transit"), NULL, NULL, list, NULL);
}

