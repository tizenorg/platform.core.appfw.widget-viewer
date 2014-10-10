/*
 * Copyright (c) 2013 Samsung Electronics Co., Ltd All Rights Reserved
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
	colorselector - plane
 ********************************************************/

typedef struct _Colorplane_Data Colorplane_Data;
struct _Colorplane_Data
{
	Evas_Object *layout;
	Evas_Object *rect;
	Evas_Object *colorselector;
	Elm_Object_Item *it_last;
	Elm_Object_Item *sel_it;
	struct appdata *ad;
	Eina_Bool changed;
	int r, g, b, a;
};

static void _colorpalette_cb(void *data, Evas_Object *obj, void *event_info)
{
	int r = 0, g = 0, b = 0 ,a = 0;
	Colorplane_Data *cp = (Colorplane_Data *)data;
	Elm_Object_Item *color_it = (Elm_Object_Item *) event_info;
	cp->sel_it = color_it;
	elm_colorselector_palette_item_color_get(color_it, &r, &g, &b, &a);
	evas_object_color_set(cp->rect, r, g, b , a);
	elm_colorselector_color_set(cp->colorselector, r, g, b, a);
}

static void _colorplane_cb(void *data, Evas_Object *obj, void *event_info)
{
	int r, g, b, a;
	Colorplane_Data *cp = (Colorplane_Data*)data;
	Evas_Object* rect = cp->rect;
	if (!cp->changed) {
		elm_object_item_signal_emit(cp->it_last, "elm,state,custom,hide", "");
		cp->changed = EINA_TRUE;
	}
	elm_colorselector_color_get(cp->colorselector, &r, &g, &b, &a);
	cp->r = r;
	cp->g = g;
	cp->b = b;
	cp->a = a;
	evas_object_color_set(rect, r, g, b, a);
	elm_colorselector_palette_item_color_set(cp->it_last, r, g, b, a);
	if (cp->sel_it != cp->it_last)
		elm_object_item_signal_emit(cp->it_last, "elm,state,selected", "elm");
}

static void _colorpalette(Evas_Object *layout, Colorplane_Data *cp)
{
	/* add color palette widget */
	Eina_List *color_list, *last_list;

	cp->colorselector = elm_colorselector_add(layout);
	elm_object_style_set(cp->colorselector, "colorplane");
	elm_colorselector_mode_set(cp->colorselector, ELM_COLORSELECTOR_PALETTE_PLANE);
	evas_object_size_hint_fill_set(cp->colorselector, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(cp->colorselector, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_part_content_set(layout, "colorpalette", cp->colorselector);
	evas_object_smart_callback_add(cp->colorselector, "color,item,selected", _colorpalette_cb, cp);
	color_list = elm_colorselector_palette_items_get(cp->colorselector);
	last_list = eina_list_last(color_list);
	cp->it_last = eina_list_data_get(last_list);
	elm_object_item_signal_emit(cp->it_last, "elm,state,custom,show", "");
	cp->changed = EINA_FALSE;
	evas_object_smart_callback_add(cp->colorselector, "changed", _colorplane_cb, cp);
	cp->sel_it = eina_list_nth(color_list, 3);
	elm_object_item_signal_emit(cp->sel_it, "elm,state,selected", "elm");
}

static Evas_Object *_create_colorplane(Evas_Object *scroller, Colorplane_Data *cp)
{
	Evas_Object *box = elm_box_add(scroller);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(box, EVAS_HINT_FILL, 0.0);
	evas_object_show(box);

	Evas_Object *layout = elm_layout_add(box);
	elm_layout_theme_set(layout, "layout", "dialogue", "1icon");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, 0.0);
	elm_object_signal_emit(layout, "elm,state,top", "");
	elm_box_pack_end(box, layout);
	evas_object_show(layout);

	/* make base layout for a rectangle*/
	Evas_Object *layout1 = elm_layout_add(layout);
	elm_layout_file_set(layout1, ELM_DEMO_EDJ, "colorplane/rect");
	evas_object_size_hint_weight_set(layout1, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout1, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(layout1);
	elm_object_part_content_set(layout, "elm.icon", layout1);

	/* add color rectangle */
	cp->rect = elm_layout_add(box);
	elm_layout_file_set(cp->rect, ELM_DEMO_EDJ, "item/colorpalette");
	evas_object_size_hint_weight_set(cp->rect, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(cp->rect, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_part_content_set(layout1, "rect", cp->rect);

	Evas_Object *layout2 = elm_layout_add(box);
	elm_layout_theme_set(layout2, "layout", "dialogue", "1icon");
	evas_object_size_hint_weight_set(layout2, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(layout2, EVAS_HINT_FILL, 0.0);
	elm_box_pack_end(box, layout2);
	elm_object_signal_emit(layout2, "elm,state,bottom", "");
	evas_object_show(layout2);

	/* make base layout for a colorpalette and colorplane*/
	cp->layout = elm_layout_add(layout2);
	elm_layout_file_set(cp->layout, ELM_DEMO_EDJ, "colorplane");
	evas_object_size_hint_weight_set(cp->layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(cp->layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(cp->layout);
	elm_object_part_content_set(layout2, "elm.icon", cp->layout);

	_colorpalette(cp->layout, cp);
	evas_object_color_set(cp->rect, 255, 59, 91, 255);

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

void colorselector_plane_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *box, *scroller;
	static Colorplane_Data cp;

	if (data == NULL) return;
	cp.ad = (struct appdata *) data;
	scroller = _create_scroller(cp.ad->nf);
	elm_naviframe_item_push(cp.ad->nf, _("Colorselector plane"), NULL, NULL, scroller, NULL);
	box = _create_colorplane(scroller, &cp);
	elm_object_content_set(scroller, box);
}
