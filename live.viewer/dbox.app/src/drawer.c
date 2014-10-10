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
#include "drawer.h"

/*********************************************************
  Drawer
 ********************************************************/
static void _left_drawer_cb(void *data, Evas_Object * obj, void *event_info);
static void _left_right_drawer_cb(void *data, Evas_Object * obj, void *event_info);

static struct _menu_item _menu_its[] = {
	{"Left Drawer Style", _left_drawer_cb},
	{"Left Right Drawer Style", _left_right_drawer_cb},
	/* do not delete below */
	{NULL, NULL}
};

static void _list_click(void *data, Evas_Object * obj, void *event_info)
{
	Elm_Object_Item *it = (Elm_Object_Item *) elm_list_selected_item_get(obj);

	if (!it) {
		fprintf(stderr, "list item is NULL\n");
		return;
	}

	elm_list_item_selected_set(it, EINA_FALSE);
}

static Evas_Object *_create_list_winset(struct appdata *ad)
{
	Evas_Object *li;
	struct _menu_item *menu_its;
	int idx = 0;

	if (!ad) return NULL;

	li = elm_list_add(ad->nf);
	elm_list_mode_set(li, ELM_LIST_COMPRESS);
	evas_object_smart_callback_add(li, "selected", _list_click, NULL);

	menu_its = _menu_its;
	while (menu_its[idx].name != NULL) {
		elm_list_item_append(li, menu_its[idx].name, NULL, NULL, menu_its[idx].func, ad);
		++idx;
	}
	elm_list_go(li);

	return li;
}

static Evas_Object *_create_nocontent(Evas_Object *parent, const char *text)
{
	Evas_Object *grid, *bg, *noc, *btn;

	//Grid
	grid = elm_grid_add(parent);
	evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(grid);

	//Bg
	bg = elm_bg_add(grid);
	elm_grid_pack(grid, bg, 0, 0, 100, 100);
	evas_object_show(bg);

	//NoContent
	noc = elm_layout_add(grid);
	elm_layout_theme_set(noc, "layout", "nocontents", "text");
	evas_object_size_hint_weight_set(noc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(noc, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_part_text_set(noc, "elm.text", text);
	elm_layout_signal_emit(noc, "align.center", "elm");
	elm_grid_pack(grid, noc, 0, 0, 100, 100);
	evas_object_show(noc);

   //Button
	btn = elm_button_add(noc);
	elm_object_style_set(btn, "style1");
	elm_object_text_set(btn, "Text button");
	evas_object_show(btn);
	elm_object_part_content_set(noc, "swallow_area", btn);

	return grid;
}

static void
_panel_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *panel = data;
	elm_panel_hidden_set(panel, EINA_TRUE);
}

static Evas_Object *_create_panel(Evas_Object *parent)
{
	Evas_Object *panel, *list;
	int i;
	char buf[64];

	//Panel
	panel = elm_panel_add(parent);
	elm_panel_scrollable_set(panel, EINA_TRUE);

	//Default is visible, hide the content in default.
	elm_panel_hidden_set(panel, EINA_TRUE);
	evas_object_show(panel);

	//Panel content
	list = elm_list_add(panel);
	ea_object_event_callback_add(list, EA_CALLBACK_BACK, _panel_back_cb, panel);
	evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(list, EVAS_HINT_FILL, EVAS_HINT_FILL);
	for (i = 0; i < 20; i++) {
		sprintf(buf, "list item %d", i);
		elm_list_item_append(list, buf, NULL, NULL, NULL, NULL);
	}

	evas_object_show(list);

	elm_object_content_set(panel, list);

	return panel;
}

Evas_Object *_create_drawer_layout(Evas_Object *parent)
{
	Evas_Object *layout;
	layout = elm_layout_add(parent);
	elm_layout_theme_set(layout, "layout", "drawer", "panel");
	evas_object_show(layout);

	return layout;
}

Evas_Object *_create_bg(Evas_Object *parent)
{
	Evas_Object *rect;
	rect = evas_object_rectangle_add(evas_object_evas_get(parent));
	evas_object_color_set(rect, 0, 0, 0, 0);
	evas_object_show(rect);

	return rect;
}

static void _btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *panel = data;
	if (!elm_object_disabled_get(panel)) elm_panel_toggle(panel);
}

static void
_panel_scroll_cb(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Panel_Scroll_Info *ev = event_info;
	Evas_Object *bg = data;
	int col = 127 * ev->rel_x;

	evas_object_color_set(bg, 0, 0, 0, col);
}

static void
_left_unhold_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	Evas_Object *panel = data;
	if (!panel) return;
	elm_object_disabled_set(panel, EINA_TRUE);
}

static void
_left_hold_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	Evas_Object *panel = data;
	if (!panel) return;
	elm_object_disabled_set(panel, EINA_FALSE);
}

static void
_right_unhold_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	Evas_Object *panel = data;
	elm_object_disabled_set(panel, EINA_TRUE);
}

static void
_right_hold_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	Evas_Object *panel = data;
	elm_object_disabled_set(panel, EINA_FALSE);
}

static void
_left_active_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	printf("left drawer active!\n");
}

static void
_left_inactive_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	printf("left drawer inactive!\n");
}

static void
_right_active_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	printf("right drawer active!\n");
}

static void
_right_inactive_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	printf("right drawer inactive!\n");
}

static void
_left_right_drawer_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	Evas_Object *layout, *bg, *center, *left_panel, *right_panel, *btn;
	Elm_Object_Item *navi_it;

	if (ad == NULL) return;

	layout = _create_drawer_layout(ad->conform);

	bg = _create_bg(layout);
	elm_object_part_content_set(layout, "elm.swallow.bg", bg);

	center = _create_nocontent(layout, "Left Right Drawer Demo");
	elm_object_part_content_set(layout, "elm.swallow.content", center);

	//Left Panel
	left_panel = _create_panel(layout);
	elm_panel_orient_set(left_panel, ELM_PANEL_ORIENT_LEFT);
	evas_object_smart_callback_add(left_panel, "scroll", _panel_scroll_cb, bg);
	elm_object_part_content_set(layout, "elm.swallow.left", left_panel);

	//Right Panel
	right_panel = _create_panel(layout);
	elm_panel_orient_set(right_panel, ELM_PANEL_ORIENT_RIGHT);
	evas_object_smart_callback_add(right_panel, "scroll", _panel_scroll_cb, bg);
	elm_object_part_content_set(layout, "elm.swallow.right", right_panel);

	navi_it = elm_naviframe_item_push(ad->nf, _("Left Right Drawer"), NULL, NULL, layout,
												 NULL);

	//hold & unhold signal callback
	elm_object_signal_callback_add(left_panel, "elm,state,hold", "elm", _left_hold_cb, right_panel);
	elm_object_signal_callback_add(left_panel, "elm,state,unhold", "elm", _left_unhold_cb, right_panel);
	elm_object_signal_callback_add(right_panel, "elm,state,hold", "elm", _right_hold_cb, left_panel);
	elm_object_signal_callback_add(right_panel, "elm,state,unhold", "elm", _right_unhold_cb, left_panel);

	//active & inactive signal callback
	elm_object_signal_callback_add(left_panel, "elm,state,active", "elm", _left_active_cb, NULL);
	elm_object_signal_callback_add(left_panel, "elm,state,inactive", "elm", _left_inactive_cb, NULL);
	elm_object_signal_callback_add(right_panel, "elm,state,active", "elm", _right_active_cb, NULL);
	elm_object_signal_callback_add(right_panel, "elm,state,inactive", "elm", _right_inactive_cb, NULL);

	//left panel toggle button
	btn = elm_button_add(ad->nf);
	elm_object_style_set(btn, "naviframe/title_text");
	elm_object_text_set(btn, "left");
	evas_object_smart_callback_add(btn, "clicked", _btn_cb, left_panel);
	elm_object_item_part_content_set(navi_it, "title_left_btn", btn);

	//right panel toggle button
	btn = elm_button_add(ad->nf);
	elm_object_style_set(btn, "naviframe/title_text");
	elm_object_text_set(btn, "right");
	evas_object_smart_callback_add(btn, "clicked", _btn_cb, right_panel);
	elm_object_item_part_content_set(navi_it, "title_right_btn", btn);
}

static void
_left_drawer_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	Evas_Object *layout, *bg, *center, *left_panel, *btn;
	Elm_Object_Item *navi_it;

	if (ad == NULL) return;

	layout = _create_drawer_layout(ad->conform);

	bg = _create_bg(layout);
	elm_object_part_content_set(layout, "elm.swallow.bg", bg);

	center = _create_nocontent(layout, "Left Drawer Demo");
	elm_object_part_content_set(layout, "elm.swallow.content", center);

	//Left Panel
	left_panel = _create_panel(layout);
	elm_panel_orient_set(left_panel, ELM_PANEL_ORIENT_LEFT);
	evas_object_smart_callback_add(left_panel, "scroll", _panel_scroll_cb, bg);
	elm_object_part_content_set(layout, "elm.swallow.left", left_panel);

	navi_it = elm_naviframe_item_push(ad->nf, _("Left Drawer"), NULL, NULL, layout,
												 "drawers");

	//hold & unhold signal callback
	elm_object_signal_callback_add(left_panel, "elm,state,hold", "elm", _left_hold_cb, NULL);
	elm_object_signal_callback_add(left_panel, "elm,state,unhold", "elm", _left_unhold_cb, NULL);

	//active & inactive signal callback
	elm_object_signal_callback_add(left_panel, "elm,state,active", "elm", _left_active_cb, NULL);
	elm_object_signal_callback_add(left_panel, "elm,state,inactive", "elm", _left_inactive_cb, NULL);

	//left panel toggle button
	btn = elm_button_add(ad->nf);
	elm_object_style_set(btn, "naviframe/drawers");
	evas_object_smart_callback_add(btn, "clicked", _btn_cb, left_panel);
	elm_object_item_part_content_set(navi_it, "drawers", btn);
}

void drawer_cb(void *data, Evas_Object * obj, void *event_info)
{
	struct appdata *ad;

	Evas_Object *list;

	ad = (struct appdata *)data;
	if (!ad) return;

	list = _create_list_winset(ad);
	elm_naviframe_item_push(ad->nf, _("Drawer"), NULL, NULL, list, NULL);
}
