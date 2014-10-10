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
#include "vi.h"

/*********************************************************
VI
 ********************************************************/
static void _interpolation_cb(void *data, Evas_Object * obj, void *event_info);
static void _fade_in_out_cb(void *data, Evas_Object * obj, void *event_info);
static void _zoom_in_out_cb(void *data, Evas_Object * obj, void *event_info);
static void _rotate_cb(void *data, Evas_Object * obj, void *event_info);
static void _wipe_cb(void *data, Evas_Object * obj, void *event_info);
static void _flip_cb(void *data, Evas_Object * obj, void *event_info);
static void _custom_cb(void *data, Evas_Object * obj, void *event_info);


static struct _menu_item _menu_its[] = {
	{"Interpolation", _interpolation_cb},
	{"Fade", _fade_in_out_cb},
	{"Zoom", _zoom_in_out_cb},
	{"Rotate", _rotate_cb},
	{"Wipe", _wipe_cb},
	{"Flip", _flip_cb},
	{"Custom", _custom_cb},
	/* do not delete below */
	{NULL, NULL}
};

static void _list_click(void *data, Evas_Object * obj, void *event_info)
{
	Elm_Object_Item *it = (Elm_Object_Item *) elm_list_selected_item_get(obj);

	if (it == NULL) {
		fprintf(stderr, "list item is NULL\n");
		return;
	}

	elm_list_item_selected_set(it, EINA_FALSE);
}

static Evas_Object *_create_scroller(Evas_Object *parent)
{
	Evas_Object *scroller = elm_scroller_add(parent);

	elm_scroller_bounce_set(scroller, EINA_FALSE, EINA_TRUE);
	elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	evas_object_show(scroller);

	return scroller;
}


///////////////////////////////////////////////////////////////////////////////////////////////
// INTERPOLATION
///////////////////////////////////////////////////////////////////////////////////////////////

static void _interpolation_cb1(void *data, Evas_Object *obj, void *event_info)
{
	elm_layout_signal_emit(data, "elm,state,show1", "elm");
}

static void _interpolation_cb2(void *data, Evas_Object *obj, void *event_info)
{
	elm_layout_signal_emit(data, "elm,state,show2", "elm");
}

static void _interpolation_cb3(void *data, Evas_Object *obj, void *event_info)
{
	elm_layout_signal_emit(data, "elm,state,show3", "elm");
}

static void _interpolation_cb4(void *data, Evas_Object *obj, void *event_info)
{
	elm_layout_signal_emit(data, "elm,state,show4", "elm");
}

static void _interpolation_cb5(void *data, Evas_Object *obj, void *event_info)
{
	elm_layout_signal_emit(data, "elm,state,show5", "elm");
}

static void _interpolation_cb6(void *data, Evas_Object *obj, void *event_info)
{
	elm_layout_signal_emit(data, "elm,state,show6", "elm");
}

static void _interpolation_cb7(void *data, Evas_Object *obj, void *event_info)
{
	elm_layout_signal_emit(data, "elm,state,show7", "elm");
}

static void _interpolation_cb8(void *data, Evas_Object *obj, void *event_info)
{
	elm_layout_signal_emit(data, "elm,state,show8", "elm");
}

static void _interpolation_cb9(void *data, Evas_Object *obj, void *event_info)
{
	elm_layout_signal_emit(data, "elm,state,show9", "elm");
}

static void _interpolation_cb10(void *data, Evas_Object *obj, void *event_info)
{
	elm_layout_signal_emit(data, "elm,state,show10", "elm");
}

static void _interpolation_start(void *data, Evas_Object *obj, void *event_info)
{
	elm_layout_signal_emit(data, "elm,state,show1", "elm");
	elm_layout_signal_emit(data, "elm,state,show2", "elm");
	elm_layout_signal_emit(data, "elm,state,show3", "elm");
	elm_layout_signal_emit(data, "elm,state,show4", "elm");
	elm_layout_signal_emit(data, "elm,state,show5", "elm");
	elm_layout_signal_emit(data, "elm,state,show6", "elm");
	elm_layout_signal_emit(data, "elm,state,show7", "elm");
	elm_layout_signal_emit(data, "elm,state,show8", "elm");
	elm_layout_signal_emit(data, "elm,state,show9", "elm");
	elm_layout_signal_emit(data, "elm,state,show10", "elm");
}

static void _interpolation_reset(void *data, Evas_Object *obj, void *event_info)
{
	elm_layout_signal_emit(data, "elm,state,reset", "elm");
}

static Evas_Object *_test_interpolation(Evas_Object *parent)
{
	char buf[255];
	Evas_Object *layout, *icon, *btn;

	layout = elm_layout_add(parent);
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "vi/interpolation");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	/***** 1. sine i 33 *****/
	icon = elm_image_add(layout);
	snprintf(buf, sizeof(buf), "%s/sine_i_33.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(layout, "image1", icon);

	icon = elm_image_add(layout);
	snprintf(buf, sizeof(buf), "%s/00_mover.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(layout, "mover1", icon);

	btn = elm_button_add(layout);
	elm_object_style_set(btn, "naviframe/title_icon");
	icon = elm_image_add(layout);
	snprintf(buf, sizeof(buf), "%s/play.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(btn, "icon", icon);
	elm_object_part_content_set(layout, "btn1", btn);
	evas_object_smart_callback_add(btn, "clicked", _interpolation_cb1, layout);

	/***** 2. sine o 33 *****/
	icon = elm_image_add(layout);
	snprintf(buf, sizeof(buf), "%s/sine_o_33.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(layout, "image2", icon);

	icon = elm_image_add(layout);
	snprintf(buf, sizeof(buf), "%s/00_mover.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(layout, "mover2", icon);

	btn = elm_button_add(layout);
	elm_object_style_set(btn, "naviframe/title_icon");
	icon = elm_image_add(layout);
	snprintf(buf, sizeof(buf), "%s/play.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(btn, "icon", icon);
	elm_object_part_content_set(layout, "btn2", btn);
	evas_object_smart_callback_add(btn, "clicked", _interpolation_cb2, layout);

	/***** 3. sine io 33 *****/
	icon = elm_image_add(layout);
	snprintf(buf, sizeof(buf), "%s/sine_io_33.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(layout, "image3", icon);

	icon = elm_image_add(layout);
	snprintf(buf, sizeof(buf), "%s/00_mover.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(layout, "mover3", icon);

	btn = elm_button_add(layout);
	elm_object_style_set(btn, "naviframe/title_icon");
	icon = elm_image_add(layout);
	snprintf(buf, sizeof(buf), "%s/play.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(btn, "icon", icon);
	elm_object_part_content_set(layout, "btn3", btn);
	evas_object_smart_callback_add(btn, "clicked", _interpolation_cb3, layout);

	/***** 4. sine io 50 *****/
	icon = elm_image_add(layout);
	snprintf(buf, sizeof(buf), "%s/sine_io_50.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(layout, "image4", icon);

	icon = elm_image_add(layout);
	snprintf(buf, sizeof(buf), "%s/00_mover.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(layout, "mover4", icon);

	btn = elm_button_add(layout);
	elm_object_style_set(btn, "naviframe/title_icon");
	icon = elm_image_add(layout);
	snprintf(buf, sizeof(buf), "%s/play.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(btn, "icon", icon);
	elm_object_part_content_set(layout, "btn4", btn);
	evas_object_smart_callback_add(btn, "clicked", _interpolation_cb4, layout);

	/***** 5. sine io 60 *****/
	icon = elm_image_add(layout);
	snprintf(buf, sizeof(buf), "%s/sine_io_60.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(layout, "image5", icon);

	icon = elm_image_add(layout);
	snprintf(buf, sizeof(buf), "%s/00_mover.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(layout, "mover5", icon);

	btn = elm_button_add(layout);
	elm_object_style_set(btn, "naviframe/title_icon");
	icon = elm_image_add(layout);
	snprintf(buf, sizeof(buf), "%s/play.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(btn, "icon", icon);
	elm_object_part_content_set(layout, "btn5", btn);
	evas_object_smart_callback_add(btn, "clicked", _interpolation_cb5, layout);

	/***** 6. sine io 70 *****/
	icon = elm_image_add(layout);
	snprintf(buf, sizeof(buf), "%s/sine_io_70.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(layout, "image6", icon);

	icon = elm_image_add(layout);
	snprintf(buf, sizeof(buf), "%s/00_mover.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(layout, "mover6", icon);

	btn = elm_button_add(layout);
	elm_object_style_set(btn, "naviframe/title_icon");
	icon = elm_image_add(layout);
	snprintf(buf, sizeof(buf), "%s/play.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(btn, "icon", icon);
	elm_object_part_content_set(layout, "btn6", btn);
	evas_object_smart_callback_add(btn, "clicked", _interpolation_cb6, layout);

	/***** 7. sine io 80 *****/
	icon = elm_image_add(layout);
	snprintf(buf, sizeof(buf), "%s/sine_io_80.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(layout, "image7", icon);

	icon = elm_image_add(layout);
	snprintf(buf, sizeof(buf), "%s/00_mover.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(layout, "mover7", icon);

	btn = elm_button_add(layout);
	elm_object_style_set(btn, "naviframe/title_icon");
	icon = elm_image_add(layout);
	snprintf(buf, sizeof(buf), "%s/play.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(btn, "icon", icon);
	elm_object_part_content_set(layout, "btn7", btn);
	evas_object_smart_callback_add(btn, "clicked", _interpolation_cb7, layout);

	/***** 8. sine io 90 *****/
	icon = elm_image_add(layout);
	snprintf(buf, sizeof(buf), "%s/sine_io_90.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(layout, "image8", icon);

	icon = elm_image_add(layout);
	snprintf(buf, sizeof(buf), "%s/00_mover.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(layout, "mover8", icon);

	btn = elm_button_add(layout);
	elm_object_style_set(btn, "naviframe/title_icon");
	icon = elm_image_add(layout);
	snprintf(buf, sizeof(buf), "%s/play.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(btn, "icon", icon);
	elm_object_part_content_set(layout, "btn8", btn);
	evas_object_smart_callback_add(btn, "clicked", _interpolation_cb8, layout);

	/***** 9. quint io 50 *****/
	icon = elm_image_add(layout);
	snprintf(buf, sizeof(buf), "%s/quint_o_50.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(layout, "image9", icon);

	icon = elm_image_add(layout);
	snprintf(buf, sizeof(buf), "%s/00_mover.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(layout, "mover9", icon);

	btn = elm_button_add(layout);
	elm_object_style_set(btn, "naviframe/title_icon");
	icon = elm_image_add(layout);
	snprintf(buf, sizeof(buf), "%s/play.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(btn, "icon", icon);
	elm_object_part_content_set(layout, "btn9", btn);
	evas_object_smart_callback_add(btn, "clicked", _interpolation_cb9, layout);

	/***** 10. quint io 80 *****/
	icon = elm_image_add(layout);
	snprintf(buf, sizeof(buf), "%s/quint_o_80.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(layout, "image10", icon);

	icon = elm_image_add(layout);
	snprintf(buf, sizeof(buf), "%s/00_mover.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(layout, "mover10", icon);

	btn = elm_button_add(layout);
	elm_object_style_set(btn, "naviframe/title_icon");
	icon = elm_image_add(layout);
	snprintf(buf, sizeof(buf), "%s/play.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(btn, "icon", icon);
	elm_object_part_content_set(layout, "btn10", btn);
	evas_object_smart_callback_add(btn, "clicked", _interpolation_cb10, layout);

	return layout;
}

static void _interpolation_cb(void *data, Evas_Object * obj, void *event_info)
{
	char buf[255];
	Evas_Object *scroller, *layout_inner, *btn, *icon;
	Elm_Object_Item *navi_it;

	struct appdata *ad = (struct appdata *)data;

	if (ad == NULL) return;

	scroller = _create_scroller(ad->nf);
	navi_it = elm_naviframe_item_push(ad->nf, _("Interpolation"), NULL, NULL, scroller, NULL);

	layout_inner = _test_interpolation(ad->nf);
	elm_object_content_set(scroller, layout_inner);

	btn = elm_button_add(ad->nf);
	elm_object_style_set(btn, "naviframe/title_icon");
	icon = elm_image_add(ad->nf);
	snprintf(buf, sizeof(buf), "%s/play.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(btn, "icon", icon);
	elm_object_item_part_content_set(navi_it, "title_left_btn", btn);
	evas_object_smart_callback_add(btn, "clicked", _interpolation_start, layout_inner);

	btn = elm_button_add(ad->nf);
	elm_object_style_set(btn, "naviframe/title_icon");
	icon = elm_image_add(ad->nf);
	snprintf(buf, sizeof(buf), "%s/reset.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(btn, "icon", icon);
	elm_object_item_part_content_set(navi_it, "title_right_btn", btn);
	evas_object_smart_callback_add(btn, "clicked", _interpolation_reset, layout_inner);
}

///////////////////////////////////////////////////////////////////////////////////////////////
// FADE
///////////////////////////////////////////////////////////////////////////////////////////////

static void _fade_in_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_layout_signal_emit(data, "elm,action,in", "elm");
}

static void _fade_out_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_layout_signal_emit(data, "elm,action,out", "elm");
}

static void _fade_default_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_layout_signal_emit(data, "elm,action,reset", "elm");
}

static Evas_Object *_test_fade(Evas_Object *parent)
{
	char buf[255];
	Evas_Object *layout, *icon;

	layout = elm_layout_add(parent);
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "vi/fade");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	icon = elm_image_add(layout);
	snprintf(buf, sizeof(buf), "%s/g5.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(layout, "elm.swallow.content", icon);

	return layout;
}

static void _fade_in_out_cb(void *data, Evas_Object * obj, void *event_info)
{
	char buf[255];
	Evas_Object *scroller, *layout_inner, *btn, *icon, *toolbar;
	Elm_Object_Item *navi_it;

	struct appdata *ad = (struct appdata *)data;

	if (ad == NULL) return;

	scroller = _create_scroller(ad->nf);
	navi_it = elm_naviframe_item_push(ad->nf, _("Fade In/Out"), NULL, NULL, scroller, NULL);

	layout_inner = _test_fade(ad->nf);
	elm_object_content_set(scroller, layout_inner);

	btn = elm_button_add(ad->nf);
	elm_object_style_set(btn, "naviframe/title_icon");
	icon = elm_image_add(ad->nf);
	snprintf(buf, sizeof(buf), "%s/reset.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(btn, "icon", icon);
	elm_object_item_part_content_set(navi_it, "title_right_btn", btn);
	evas_object_smart_callback_add(btn, "clicked", _fade_default_cb, layout_inner);

	toolbar = elm_toolbar_add(ad->nf);
	elm_toolbar_shrink_mode_set(toolbar, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_transverse_expanded_set(toolbar, EINA_TRUE);
	elm_toolbar_select_mode_set(toolbar, ELM_OBJECT_SELECT_MODE_NONE);
	elm_toolbar_item_append(toolbar, NULL, "in", _fade_in_cb, layout_inner);
	elm_toolbar_item_append(toolbar, NULL, "out", _fade_out_cb, layout_inner);
	elm_object_item_part_content_set(navi_it, "toolbar", toolbar);
}

///////////////////////////////////////////////////////////////////////////////////////////////
// ZOOM
///////////////////////////////////////////////////////////////////////////////////////////////

static void _zoom_1(void *data, Evas_Object *obj, void *event_info)
{
	elm_layout_signal_emit(data, "elm,action,zoom_1", "elm");
}

static void _zoom_2(void *data, Evas_Object *obj, void *event_info)
{
	elm_layout_signal_emit(data, "elm,action,zoom_2", "elm");
}

static void _zoom_3(void *data, Evas_Object *obj, void *event_info)
{
	elm_layout_signal_emit(data, "elm,action,zoom_3", "elm");
}

static void _zoom_4(void *data, Evas_Object *obj, void *event_info)
{
	elm_layout_signal_emit(data, "elm,action,zoom_4", "elm");
}

static void _zoom_default_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_layout_signal_emit(data, "elm,action,reset", "elm");
}

static Evas_Object *_test_zoom(Evas_Object *parent)
{
	char buf[255];
	Evas_Object *layout, *icon;

	layout = elm_layout_add(parent);
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "vi/zoom");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	icon = elm_image_add(layout);
	snprintf(buf, sizeof(buf), "%s/g4.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(layout, "elm.swallow.content", icon);

	return layout;
}

static void _zoom_in_out_cb(void *data, Evas_Object * obj, void *event_info)
{
	char buf[255];
	Evas_Object *scroller, *layout_inner, *btn, *icon;
	Elm_Object_Item *navi_it;

	struct appdata *ad = (struct appdata *)data;

	if (ad == NULL) return;

	scroller = _create_scroller(ad->nf);
	navi_it = elm_naviframe_item_push(ad->nf, _("Zoom In/Out"), NULL, NULL, scroller, NULL);

	layout_inner = _test_zoom(ad->nf);
	elm_object_content_set(scroller, layout_inner);

	btn = elm_button_add(ad->nf);
	elm_object_style_set(btn, "naviframe/title_icon");
	icon = elm_image_add(ad->nf);
	snprintf(buf, sizeof(buf), "%s/reset.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(btn, "icon", icon);
	elm_object_item_part_content_set(navi_it, "title_right_btn", btn);
	evas_object_smart_callback_add(btn, "clicked", _zoom_default_cb, layout_inner);

	btn = elm_button_add(ad->nf);
	elm_object_text_set(btn, "x1/4");
	elm_object_part_content_set(layout_inner, "btn1", btn);
	evas_object_smart_callback_add(btn, "clicked", _zoom_1, layout_inner);

	btn = elm_button_add(ad->nf);
	elm_object_text_set(btn, "x1/2");
	elm_object_part_content_set(layout_inner, "btn2", btn);
	evas_object_smart_callback_add(btn, "clicked", _zoom_2, layout_inner);

	btn = elm_button_add(ad->nf);
	elm_object_text_set(btn, "x2");
	elm_object_part_content_set(layout_inner, "btn3", btn);
	evas_object_smart_callback_add(btn, "clicked", _zoom_3, layout_inner);

	btn = elm_button_add(ad->nf);
	elm_object_text_set(btn, "x4");
	elm_object_part_content_set(layout_inner, "btn4", btn);
	evas_object_smart_callback_add(btn, "clicked", _zoom_4, layout_inner);

	btn = elm_button_add(ad->nf);
	elm_object_style_set(btn, "naviframe/title_icon");
	icon = elm_image_add(ad->nf);
	snprintf(buf, sizeof(buf), "%s/reset.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(btn, "icon", icon);
	elm_object_item_part_content_set(navi_it, "title_right_btn", btn);
	evas_object_smart_callback_add(btn, "clicked", _zoom_default_cb, layout_inner);
}

///////////////////////////////////////////////////////////////////////////////////////////////
// ROTATE
///////////////////////////////////////////////////////////////////////////////////////////////

static void _rotate_default(void *data, Evas_Object *obj, void *event_info)
{
	elm_layout_signal_emit(data, "elm,action,reset", "elm");
}

static void _rotate_90(void *data, Evas_Object *obj, void *event_info)
{
	elm_layout_signal_emit(data, "elm,action,90", "elm");
}

static void _rotate_180(void *data, Evas_Object *obj, void *event_info)
{
	elm_layout_signal_emit(data, "elm,action,180", "elm");
}

static void _rotate_270(void *data, Evas_Object *obj, void *event_info)
{
	elm_layout_signal_emit(data, "elm,action,270", "elm");
}

static void _rotate_360(void *data, Evas_Object *obj, void *event_info)
{
	elm_layout_signal_emit(data, "elm,action,360", "elm");
}

static Evas_Object *_test_rotate(Evas_Object *parent)
{
	char buf[255];
	Evas_Object *layout, *icon;

	layout = elm_layout_add(parent);
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "vi/rotate");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	icon = elm_image_add(layout);
	snprintf(buf, sizeof(buf), "%s/icon3.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(layout, "content", icon);

	return layout;
}

static void _rotate_cb(void *data, Evas_Object * obj, void *event_info)
{
	char buf[255];
	Evas_Object *scroller, *layout_inner, *btn, *icon;
	Elm_Object_Item *navi_it;

	struct appdata *ad = (struct appdata *)data;

	if (ad == NULL) return;

	scroller = _create_scroller(ad->nf);
	navi_it = elm_naviframe_item_push(ad->nf, _("Rotate"), NULL, NULL, scroller, NULL);

	layout_inner = _test_rotate(ad->nf);
	elm_object_content_set(scroller, layout_inner);

	btn = elm_button_add(ad->nf);
	elm_object_text_set(btn, "90");
	elm_object_part_content_set(layout_inner, "btn1", btn);
	evas_object_smart_callback_add(btn, "clicked", _rotate_90, layout_inner);

	btn = elm_button_add(ad->nf);
	elm_object_text_set(btn, "180");
	elm_object_part_content_set(layout_inner, "btn2", btn);
	evas_object_smart_callback_add(btn, "clicked", _rotate_180, layout_inner);

	btn = elm_button_add(ad->nf);
	elm_object_text_set(btn, "270");
	elm_object_part_content_set(layout_inner, "btn3", btn);
	evas_object_smart_callback_add(btn, "clicked", _rotate_270, layout_inner);

	btn = elm_button_add(ad->nf);
	elm_object_text_set(btn, "360");
	elm_object_part_content_set(layout_inner, "btn4", btn);
	evas_object_smart_callback_add(btn, "clicked", _rotate_360, layout_inner);

	btn = elm_button_add(ad->nf);
	elm_object_style_set(btn, "naviframe/title_icon");
	icon = elm_image_add(ad->nf);
	snprintf(buf, sizeof(buf), "%s/reset.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(btn, "icon", icon);
	elm_object_item_part_content_set(navi_it, "title_right_btn", btn);
	evas_object_smart_callback_add(btn, "clicked", _rotate_default, layout_inner);
}

///////////////////////////////////////////////////////////////////////////////////////////////
// WIPE
///////////////////////////////////////////////////////////////////////////////////////////////

static void _wipe_start(void *data, Evas_Object *obj, void *event_info)
{
	elm_layout_signal_emit(data, "elm,state,show", "elm");
}

static void _wipe_reset(void *data, Evas_Object *obj, void *event_info)
{
	elm_layout_signal_emit(data, "elm,state,reset", "elm");
}

static Evas_Object *_test_wipe(Evas_Object *parent)
{
	char buf[255];
	Evas_Object *layout, *icon;

	layout = elm_layout_add(parent);
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "vi/wipe");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	icon = elm_image_add(layout);
	snprintf(buf, sizeof(buf), "%s/g2.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(layout, "bottom2top", icon);

	icon = elm_image_add(layout);
	snprintf(buf, sizeof(buf), "%s/g2.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(layout, "top2bottom", icon);

	icon = elm_image_add(layout);
	snprintf(buf, sizeof(buf), "%s/g2.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(layout, "left2right", icon);

	icon = elm_image_add(layout);
	snprintf(buf, sizeof(buf), "%s/g2.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(layout, "right2left", icon);

	icon = elm_image_add(layout);
	snprintf(buf, sizeof(buf), "%s/g2.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(layout, "leftbottom2righttop", icon);

	icon = elm_image_add(layout);
	snprintf(buf, sizeof(buf), "%s/g2.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(layout, "rightbottom2lefttop", icon);

	icon = elm_image_add(layout);
	snprintf(buf, sizeof(buf), "%s/g2.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(layout, "lefttop2rightbottom", icon);

	icon = elm_image_add(layout);
	snprintf(buf, sizeof(buf), "%s/g2.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(layout, "righttop2leftbottom", icon);

	return layout;
}

static void _wipe_cb(void *data, Evas_Object * obj, void *event_info)
{
	char buf[255];
	Evas_Object *scroller, *layout_inner, *btn, *icon, *toolbar;
	Elm_Object_Item *navi_it;

	struct appdata *ad = (struct appdata *)data;

	if (ad == NULL) return;

	scroller = _create_scroller(ad->nf);
	navi_it = elm_naviframe_item_push(ad->nf, _("Wipe"), NULL, NULL, scroller, NULL);

	layout_inner = _test_wipe(ad->nf);
	elm_object_content_set(scroller, layout_inner);

	btn = elm_button_add(ad->nf);
	elm_object_style_set(btn, "naviframe/title_icon");
	icon = elm_image_add(ad->nf);
	snprintf(buf, sizeof(buf), "%s/play.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(btn, "icon", icon);
	elm_object_item_part_content_set(navi_it, "title_left_btn", btn);
	evas_object_smart_callback_add(btn, "clicked", _wipe_start, layout_inner);

	btn = elm_button_add(ad->nf);
	elm_object_style_set(btn, "naviframe/title_icon");
	icon = elm_image_add(ad->nf);
	snprintf(buf, sizeof(buf), "%s/reset.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(btn, "icon", icon);
	elm_object_item_part_content_set(navi_it, "title_right_btn", btn);
	evas_object_smart_callback_add(btn, "clicked", _wipe_reset, layout_inner);

	toolbar = elm_toolbar_add(ad->nf);
	elm_toolbar_shrink_mode_set(toolbar, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_transverse_expanded_set(toolbar, EINA_TRUE);
	elm_toolbar_select_mode_set(toolbar, ELM_OBJECT_SELECT_MODE_NONE);
	elm_toolbar_item_append(toolbar, NULL, "start", _wipe_start, layout_inner);
	elm_toolbar_item_append(toolbar, NULL, "reset", _wipe_reset, layout_inner);
	elm_object_item_part_content_set(navi_it, "toolbar", toolbar);
}

///////////////////////////////////////////////////////////////////////////////////////////////
// FLIP
///////////////////////////////////////////////////////////////////////////////////////////////

static void _flip_default(void *data, Evas_Object *obj, void *event_info)
{
	elm_layout_signal_emit(data, "elm,state,reset", "elm");
}

static void _flip_x(void *data, Evas_Object *obj, void *event_info)
{
	elm_layout_signal_emit(data, "elm,state,flipx", "elm");
}

static void _flip_y(void *data, Evas_Object *obj, void *event_info)
{
	elm_layout_signal_emit(data, "elm,state,flipy", "elm");
}

static Evas_Object *_test_flip(Evas_Object *parent)
{
	char buf[255];
	Evas_Object *layout, *icon;

	layout = elm_layout_add(parent);
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "vi/flip");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	icon = elm_image_add(layout);
	snprintf(buf, sizeof(buf), "%s/00_flip.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(layout, "content", icon);

	return layout;
}

static void _flip_cb(void *data, Evas_Object * obj, void *event_info)
{
	char buf[255];
	Evas_Object *scroller, *layout_inner, *btn, *icon;
	Elm_Object_Item *navi_it;

	struct appdata *ad = (struct appdata *)data;

	if (ad == NULL) return;

	scroller = _create_scroller(ad->nf);
	navi_it = elm_naviframe_item_push(ad->nf, _("Flip"), NULL, NULL, scroller, NULL);

	layout_inner = _test_flip(ad->nf);
	elm_object_content_set(scroller, layout_inner);

	btn = elm_button_add(ad->nf);
	elm_object_text_set(btn, "Default");
	elm_object_part_content_set(layout_inner, "btn1", btn);
	evas_object_smart_callback_add(btn, "clicked", _flip_default, layout_inner);

	btn = elm_button_add(ad->nf);
	elm_object_text_set(btn, "X flip");
	elm_object_part_content_set(layout_inner, "btn2", btn);
	evas_object_smart_callback_add(btn, "clicked", _flip_x, layout_inner);

	btn = elm_button_add(ad->nf);
	elm_object_text_set(btn, "Y flip");
	elm_object_part_content_set(layout_inner, "btn3", btn);
	evas_object_smart_callback_add(btn, "clicked", _flip_y, layout_inner);

	btn = elm_button_add(ad->nf);
	elm_object_style_set(btn, "naviframe/title_icon");
	icon = elm_image_add(ad->nf);
	snprintf(buf, sizeof(buf), "%s/reset.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(btn, "icon", icon);
	elm_object_item_part_content_set(navi_it, "title_right_btn", btn);
	evas_object_smart_callback_add(btn, "clicked", _flip_default, layout_inner);

}

///////////////////////////////////////////////////////////////////////////////////////////////
// CUSTOM
///////////////////////////////////////////////////////////////////////////////////////////////

static void _custom_in_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_layout_signal_emit(data, "elm,action,_zoom_in", "elm");
	elm_layout_signal_emit(data, "elm,action,_fade_in", "elm");
}

static void _custom_out_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_layout_signal_emit(data, "elm,action,_zoom_out", "elm");
	elm_layout_signal_emit(data, "elm,action,_fade_out", "elm");
}

static void _custom_default_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_layout_signal_emit(data, "elm,action,reset", "elm");
}

static Evas_Object *_test_custom(Evas_Object *parent)
{
	char buf[255];
	Evas_Object *layout, *icon;

	layout = elm_layout_add(parent);
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "vi/custom");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	icon = elm_image_add(layout);
	snprintf(buf, sizeof(buf), "%s/sky_01.jpg", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(layout, "elm.swallow.content", icon);

	return layout;
}

static void _custom_cb(void *data, Evas_Object * obj, void *event_info)
{
	char buf[255];
	Evas_Object *scroller, *layout_inner, *btn, *icon, *toolbar;
	Elm_Object_Item *navi_it;

	struct appdata *ad = (struct appdata *)data;

	if (ad == NULL) return;

	scroller = _create_scroller(ad->nf);
	navi_it = elm_naviframe_item_push(ad->nf, _("Fade, Zoom and Move"), NULL, NULL, scroller, NULL);

	layout_inner = _test_custom(ad->nf);
	elm_object_content_set(scroller, layout_inner);

	btn = elm_button_add(ad->nf);
	elm_object_style_set(btn, "naviframe/title_icon");
	icon = elm_image_add(ad->nf);
	snprintf(buf, sizeof(buf), "%s/reset.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(btn, "icon", icon);
	elm_object_item_part_content_set(navi_it, "title_right_btn", btn);
	evas_object_smart_callback_add(btn, "clicked", _custom_default_cb, layout_inner);

	toolbar = elm_toolbar_add(ad->nf);
	elm_toolbar_shrink_mode_set(toolbar, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_transverse_expanded_set(toolbar, EINA_TRUE);
	elm_toolbar_select_mode_set(toolbar, ELM_OBJECT_SELECT_MODE_NONE);
	elm_toolbar_item_append(toolbar, NULL, "in", _custom_in_cb, layout_inner);
	elm_toolbar_item_append(toolbar, NULL, "out", _custom_out_cb, layout_inner);
	elm_object_item_part_content_set(navi_it, "toolbar", toolbar);
}

static Evas_Object *_create_list_winset(struct appdata *ad)
{
	Evas_Object *li;
	struct _menu_item *menu_its;
	if (ad == NULL) return NULL;

	li = elm_list_add(ad->nf);
	elm_list_mode_set(li, ELM_LIST_COMPRESS);
	evas_object_smart_callback_add(li, "selected", _list_click, NULL);

	int idx = 0;
	menu_its = _menu_its;
	while (menu_its[idx].name != NULL) {
		elm_list_item_append(li, menu_its[idx].name, NULL, NULL, menu_its[idx].func, ad);
		++idx;
	}
	elm_list_go(li);

	return li;
}

void vi_cb(void *data, Evas_Object * obj, void *event_info)
{
	struct appdata *ad;

	Evas_Object *list;

	ad = (struct appdata *)data;
	if (ad == NULL) return;

	list = _create_list_winset(ad);
	elm_naviframe_item_push(ad->nf, _("VI"), NULL, NULL, list, NULL);
}

