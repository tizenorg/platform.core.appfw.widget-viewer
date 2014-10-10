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
	colorselector - components
 ********************************************************/

static void _colorselector_cb(void *data, Evas_Object *obj, void *event_info)
{
	int r, g, b, a;
	Evas_Object* rect = (Evas_Object*)data;
	elm_colorselector_color_get(obj, &r, &g, &b, &a);
	evas_object_color_set(rect, r, g, b, a);
}

static Evas_Object *_create_colorselector(Evas_Object* parent)
{
	Evas_Object *layout, *rect, *colorselector, *layoutp;
	layoutp = elm_layout_add(parent);
	elm_layout_theme_set(layoutp, "layout", "dialogue", "1icon/no_padding");
	evas_object_size_hint_weight_set(layoutp, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(layoutp, EVAS_HINT_FILL, 0.0);
	evas_object_show(layoutp);

	// make base layout
	layout = elm_layout_add(layoutp);
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "colorselector");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(layout);
	elm_object_part_content_set(layoutp, "elm.icon", layout);


	rect = elm_layout_add(layoutp);
	elm_layout_file_set(rect, ELM_DEMO_EDJ, "item/colorpalette");
	evas_object_size_hint_weight_set(rect, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(rect, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_part_content_set(layout, "rect", rect);
	evas_object_color_set(rect, 231, 28, 28, 255);

	// add a color picker widget
	colorselector = elm_colorselector_add(parent);
	elm_colorselector_mode_set(colorselector, ELM_COLORSELECTOR_COMPONENTS);
	elm_colorselector_color_set(colorselector, 255, 255, 0, 255);
	evas_object_size_hint_fill_set(colorselector, EVAS_HINT_FILL, 0);
	evas_object_size_hint_weight_set(colorselector, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_part_content_set(layout, "colorselector", colorselector);
	evas_object_smart_callback_add(colorselector, "changed", _colorselector_cb, rect);

	return layoutp;
}

static Evas_Object *_create_scroller(Evas_Object *parent)
{
	Evas_Object* scroller = elm_scroller_add(parent);
	elm_scroller_bounce_set(scroller, EINA_FALSE, EINA_TRUE);
	elm_scroller_policy_set(scroller,ELM_SCROLLER_POLICY_OFF,ELM_SCROLLER_POLICY_AUTO);
	evas_object_show(scroller);

	return scroller;
}

void colorselector_components_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *layout, *scroller;
	struct appdata *ad = (struct appdata *) data;
	if (ad == NULL) return;

	scroller = _create_scroller(ad->nf);
	elm_naviframe_item_push(ad->nf, _("Colorselector components"), NULL, NULL, scroller, NULL);
	layout = _create_colorselector(ad->nf);
	elm_object_content_set(scroller, layout);
}
