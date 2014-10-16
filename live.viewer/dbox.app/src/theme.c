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

#include "elmdemo_util.h"
#include "elmdemo_test.h"
#include "theme.h"

#define MAX_NUM_OF_RECT 7

static Evas_Object *_selected_rect = NULL;

static void _button_clicked(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad;
	Elm_Object_Item *it = (Elm_Object_Item *)event_info;
	const char *text;
	int index, r, g, b;

	if (!data) return;
	ad = (struct appdata *)data;

	// if color is not selected, return
	evas_object_color_get(_selected_rect, &r, &g, &b, NULL);
	if (r == 255 && g == 255 && b == 255) return;

	index = (int)evas_object_data_get(_selected_rect, "index");
	printf("theme index: %d\n", index);

	text = elm_object_item_text_get(it);
	if (text && strstr(text, "Dark"))
		ea_theme_fixed_style_set(index, EA_THEME_STYLE_DARK);
	else if (text && strstr(text, "Light"))
		ea_theme_fixed_style_set(index, EA_THEME_STYLE_LIGHT);

	elm_naviframe_item_pop(ad->nf);
}

static void _rect_selected(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	int r, g, b, a;

	evas_object_color_get(obj, &r, &g, &b, &a);
	evas_object_color_set(_selected_rect, r, g, b, a);
	evas_object_data_set(_selected_rect, "index", data);
}

static Evas_Object *_create_color_box(struct appdata *ad)
{
	Evas_Object *box, *lb, *rect, *hbox, *hbox2, *hbox3, *cur_box;
	Eina_List *colors;
	Ea_Theme_Color_hsv *color;
	int start, r, g, b;

	box = elm_box_add(ad->nf);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(box, EVAS_HINT_FILL, 0.5);

	lb = elm_label_add(box);
	elm_object_text_set(lb, " Theme Colors");
	evas_object_size_hint_weight_set(lb, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(lb, EVAS_HINT_FILL, 0.5);
	elm_box_pack_end(box, lb);
	evas_object_show(lb);

	hbox = elm_box_add(ad->nf);
	elm_box_horizontal_set(hbox, EINA_TRUE);
	evas_object_size_hint_weight_set(hbox, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(hbox, EVAS_HINT_FILL, 0.5);
	elm_box_pack_end(box, hbox);
	evas_object_show(hbox);

	hbox2 = elm_box_add(ad->nf);
	elm_box_horizontal_set(hbox2, EINA_TRUE);
	evas_object_size_hint_weight_set(hbox2, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(hbox2, EVAS_HINT_FILL, 0.5);
	elm_box_pack_end(box, hbox2);
	evas_object_show(hbox2);

	hbox3 = elm_box_add(ad->nf);
	elm_box_horizontal_set(hbox3, EINA_TRUE);
	evas_object_size_hint_weight_set(hbox3, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(hbox3, EVAS_HINT_FILL, 0.5);
	elm_box_pack_end(box, hbox3);
	evas_object_show(hbox3);

	cur_box = hbox;

	start = EA_THEME_COLOR_TABLE_1;
	colors = ea_theme_input_colors_get(start);
	while(colors)
	{
		rect = evas_object_rectangle_add(evas_object_evas_get(cur_box));
		evas_object_size_hint_min_set(rect, 100, 100);
		evas_object_event_callback_add(rect, EVAS_CALLBACK_MOUSE_DOWN, _rect_selected, (void *)start);
		elm_box_pack_end(cur_box, rect);
		evas_object_show(rect);

		// set main color to rect
		color = eina_list_data_get(colors);
		if (color)
		{
			evas_color_hsv_to_rgb((float)color->h, color->s / 100.0, color->v / 100.0, &r, &g, &b);
			evas_object_color_set(rect, r, g, b, 255);
		}

		// free colors
		EINA_LIST_FREE(colors, color)
			free(color);
		start++;
		if(start == MAX_NUM_OF_RECT) cur_box = hbox2;
		colors = ea_theme_input_colors_get(start);
	}

	start = EA_THEME_COLOR_TABLE_2;
	colors = ea_theme_input_colors_get(start);
	while(colors)
	{
		rect = evas_object_rectangle_add(evas_object_evas_get(cur_box));
		evas_object_size_hint_min_set(rect, 100, 100);
		evas_object_event_callback_add(rect, EVAS_CALLBACK_MOUSE_UP, _rect_selected, (void *)start);
		elm_box_pack_end(cur_box, rect);
		evas_object_show(rect);

		// set main color to rect
		color = eina_list_data_get(colors);
		if (color)
		{
			evas_color_hsv_to_rgb((float)color->h, color->s / 100.0, color->v / 100.0, &r, &g, &b);
			evas_object_color_set(rect, r, g, b, 255);
		}

		// free colors;
		EINA_LIST_FREE(colors, color)
			free(color);
		start++;
		colors = ea_theme_input_colors_get(start);
	}

	start = EA_THEME_COLOR_TABLE_3;
	colors = ea_theme_input_colors_get(start);
	cur_box = hbox3;
	while(colors)
	{
		rect = evas_object_rectangle_add(evas_object_evas_get(cur_box));
		evas_object_size_hint_min_set(rect, 100, 100);
		evas_object_event_callback_add(rect, EVAS_CALLBACK_MOUSE_UP, _rect_selected, (void *)start);
		elm_box_pack_end(cur_box, rect);
		evas_object_show(rect);

		// set main color to rect
		color = eina_list_data_get(colors);
		if (color)
		{
			evas_color_hsv_to_rgb((float)color->h, color->s / 100.0, color->v / 100.0, &r, &g, &b);
			evas_object_color_set(rect, r, g, b, 255);
		}

		// free colors;
		EINA_LIST_FREE(colors, color)
			free(color);
		start++;
		colors = ea_theme_input_colors_get(start);
	}

	return box;
}

static Evas_Object *_create_selected_box(struct appdata *ad)
{
	Evas_Object *box, *lb, *rect;

	box = elm_box_add(ad->nf);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(box, EVAS_HINT_FILL, 0.5);

	lb = elm_label_add(box);
	elm_object_text_set(lb, " Selected Color");
	evas_object_size_hint_weight_set(lb, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(lb, EVAS_HINT_FILL, 0.5);
	elm_box_pack_end(box, lb);
	evas_object_show(lb);

	rect = evas_object_rectangle_add(evas_object_evas_get(box));
	evas_object_size_hint_min_set(rect, 700, 300);
	elm_box_pack_end(box, rect);
	evas_object_show(rect);
	_selected_rect = rect;

	return box;
}

static Evas_Object *_create_layout(struct appdata *ad)
{
	Evas_Object *box, *sbox, *cbox;

	box = elm_box_add(ad->nf);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(box);

	// selected box
	sbox = _create_selected_box(ad);
	elm_box_pack_end(box, sbox);
	evas_object_show(sbox);

	// color box
	cbox = _create_color_box(ad);
	elm_box_pack_end(box, cbox);
	evas_object_show(cbox);

	return box;
}

void theme_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *ly, *tb, *sc;
	Elm_Object_Item *navi_it;
	struct appdata *ad = (struct appdata *) data;
	if (ad == NULL) return;

	sc = elm_scroller_add(ad->nf);
	elm_scroller_bounce_set(sc, EINA_FALSE, EINA_TRUE);
	elm_scroller_policy_set(sc, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	evas_object_show(sc);

	ly = _create_layout(ad);
	elm_object_content_set(sc, ly);
	navi_it = elm_naviframe_item_push(ad->nf, _("Theme"), NULL, NULL, sc, NULL);

	tb = elm_toolbar_add(ad->nf);
	elm_toolbar_shrink_mode_set(tb, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_transverse_expanded_set(tb, EINA_TRUE);
	elm_toolbar_select_mode_set(tb, ELM_OBJECT_SELECT_MODE_NONE);
	elm_toolbar_item_append(tb, NULL, "Dark", _button_clicked, ad);
	elm_toolbar_item_append(tb, NULL, "Light", _button_clicked, ad);
	elm_object_item_part_content_set(navi_it, "toolbar", tb);
}
