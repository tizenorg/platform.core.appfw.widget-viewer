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
#include "colorselector.h"

/*********************************************************
	colorselector - palette
 ********************************************************/

struct appdata *gad = NULL;
typedef struct _Colorpalette_Data Colorpalette_Data;
struct _Colorpalette_Data
{
	Evas_Object *layout;
	Evas_Object *rect;
	Evas_Object *colorpalette;
};
static void _rotate_colorpalette_cb(void *data, Evas_Object *obj, void *event_info );

static void _colorpalette_cb(void *data, Evas_Object *obj, void *event_info)
{
	int r = 0, g = 0, b = 0 ,a = 0;
	Elm_Object_Item *color_it = (Elm_Object_Item *) event_info;
	elm_colorselector_palette_item_color_get(color_it, &r, &g, &b, &a);
	evas_object_color_set((Evas_Object *) data, r, g, b , a);
}

static Eina_Bool _pop_cb(void *data, Elm_Object_Item *it)
{
	evas_object_smart_callback_del(data, "rotation,changed", _rotate_colorpalette_cb);
	return EINA_TRUE;
}

static void _colorpalette(Evas_Object *layout, Colorpalette_Data *cp)
{
	/* add color palette widget */
	int rotation;
	cp->colorpalette = elm_colorselector_add(layout);
	elm_colorselector_palette_name_set(cp->colorpalette, "painting");
	elm_colorselector_mode_set(cp->colorpalette, ELM_COLORSELECTOR_PALETTE);
	evas_object_size_hint_fill_set(cp->colorpalette, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(cp->colorpalette, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_part_content_set(layout, "colorpalette", cp->colorpalette);
	evas_object_smart_callback_add(cp->colorpalette, "color,item,selected", _colorpalette_cb, cp->rect);
	rotation = elm_win_rotation_get(gad->win_main);
	if (abs(rotation) == 90 || rotation == 270) {
		elm_object_style_set(cp->colorpalette, "landscape");
		elm_object_signal_emit(layout, "landscape", "");
	}
	else
		elm_object_signal_emit(layout, "portrait", "");
	//Needs to be called as recent changes in layout causes signals not to be processed immediately
	edje_object_message_signal_process(elm_layout_edje_get(layout));
	elm_colorselector_palette_color_add(cp->colorpalette, 231, 28, 28, 255);
	elm_colorselector_palette_color_add(cp->colorpalette, 215, 182, 30, 255);
	elm_colorselector_palette_color_add(cp->colorpalette, 171, 176, 41, 255);
	elm_colorselector_palette_color_add(cp->colorpalette, 53, 164, 72, 255);
	elm_colorselector_palette_color_add(cp->colorpalette, 73, 165, 157, 255);
	elm_colorselector_palette_color_add(cp->colorpalette, 34, 129, 157, 255);
	elm_colorselector_palette_color_add(cp->colorpalette, 37, 58, 119, 255);
	elm_colorselector_palette_color_add(cp->colorpalette, 128, 58, 177, 255);
	elm_colorselector_palette_color_add(cp->colorpalette, 194, 81, 182, 255);
	elm_colorselector_palette_color_add(cp->colorpalette, 189, 21, 92, 255);
}

static void _rotate_colorpalette_cb(void *data, Evas_Object *obj, void *event_info )
{
	Colorpalette_Data *cp = (Colorpalette_Data*)data;
	evas_object_del(cp->colorpalette);
	_colorpalette(cp->layout, cp);
}

static Evas_Object *_create_colorpalette(Evas_Object *scroller)
{
	static Colorpalette_Data cp;

	Evas_Object *box = elm_box_add(scroller);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(box, EVAS_HINT_FILL, 0.0);
	evas_object_show(box);

	Evas_Object *layout = elm_layout_add(box);
	elm_layout_theme_set(layout, "layout", "dialogue", "1icon/no_padding");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, 0.0);
	elm_object_signal_emit(layout, "elm,state,top", "");
	elm_box_pack_end(box, layout);
	evas_object_show(layout);

	/* make base layout for a rectangle and a colorpalette */
	cp.layout = elm_layout_add(layout);
	elm_layout_file_set(cp.layout, ELM_DEMO_EDJ, "colorpalette/rect");
	evas_object_size_hint_weight_set(cp.layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(cp.layout);
	elm_object_part_content_set(layout, "elm.icon", cp.layout);

	/* add color rectangle */
	cp.rect = elm_layout_add(box);
	elm_layout_file_set(cp.rect, ELM_DEMO_EDJ, "item/colorpalette");
	evas_object_size_hint_weight_set(cp.rect, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(cp.rect, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_part_content_set(cp.layout, "rect", cp.rect);
	evas_object_color_set(cp.rect, 231, 28, 28, 255);


	Evas_Object *layout1 = elm_layout_add(box);
	elm_layout_theme_set(layout1, "layout", "dialogue", "1icon/no_padding");
	evas_object_size_hint_weight_set(layout1, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(layout1, EVAS_HINT_FILL, 0.0);
	elm_box_pack_end(box, layout1);
	elm_object_signal_emit(layout1, "elm,state,bottom", "");
	evas_object_show(layout1);

	/* make base layout for a rectangle and a colorpalette */
	cp.layout = elm_layout_add(layout1);
	elm_layout_file_set(cp.layout, ELM_DEMO_EDJ, "colorpalette");
	evas_object_size_hint_weight_set(cp.layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(cp.layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(cp.layout);
	elm_object_part_content_set(layout1, "elm.icon", cp.layout);

	_colorpalette(cp.layout, &cp);
	evas_object_smart_callback_add(gad->win_main, "rotation,changed", _rotate_colorpalette_cb, &cp);

	return box;
}

static Evas_Object *_create_scroller(Evas_Object *parent)
{
	Evas_Object* scroller = elm_scroller_add(parent);
	elm_object_style_set(scroller, "dialogue");
	elm_scroller_bounce_set(scroller, EINA_FALSE, EINA_TRUE);
	elm_scroller_policy_set(scroller,ELM_SCROLLER_POLICY_OFF,ELM_SCROLLER_POLICY_AUTO);
	evas_object_show(scroller);

	return scroller;
}

void colorselector_palette_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *box, *scroller;
	Elm_Object_Item *it;
	struct appdata *ad = (struct appdata *) data;
	if(ad == NULL) return;
	gad = ad;
	scroller = _create_scroller(ad->nf);
	it = elm_naviframe_item_push(ad->nf, _("Colorselector palette"), NULL, NULL, scroller, NULL);
	box = _create_colorpalette(scroller);
	elm_naviframe_item_pop_cb_set(it, _pop_cb, ad->win_main);
	elm_object_content_set(scroller, box);
}
