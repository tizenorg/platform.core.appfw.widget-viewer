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

#define MAX_PAGE (double)3.0
#define SLIDER_POPUP_X_Y 133

typedef struct _page_data page_data;
struct _page_data
{
	Evas_Object *index_scroller;
	Evas_Object *slider_scroller;
	Evas_Object *box;
	Evas_Object *index_slider;
	Evas_Object *index;
	Evas_Object *page[3];
	Evas_Object *page_layout[3];
	Evas_Object *slider_popup;
	Evas_Object *index_layout;
	Ecore_Timer *longpress_timer;
	double page_width;
	double current_page;
	double dx;
	int page_height;
	Eina_Bool slider_show;
	Eina_Bool is_index;
	Elm_Object_Item *last_it;
	Elm_Object_Item *new_it;
};

static void
_index_sync(void *data)
{
	page_data *pd = (page_data *)data;
	Elm_Object_Item *it;
	it = elm_index_item_find(pd->index, (void *)(int)pd->current_page);
	if (it) {
		elm_index_item_selected_set(it, EINA_TRUE);
		pd->last_it = it;
		pd->new_it = it;
	}
}

static void
_slider_popup_position_set(page_data *pd)
{
	Evas_Coord x = 0, y = 0, w = 0, h = 0;
	int pos_x, pos_y;

	evas_object_geometry_get(pd->index, &x, &y, &w, &h);
	pos_x = x + (w / 2) - (int)(((double)SLIDER_POPUP_X_Y * elm_config_scale_get()) / 2);
	pos_y = y - (int)((double)SLIDER_POPUP_X_Y * elm_config_scale_get()) - 25;
	evas_object_move(pd->slider_popup, pos_x, pos_y);
}

static void
_page_position_calculate(page_data *pd)
{
	elm_scroller_region_show(pd->slider_scroller, pd->page_width * (int)pd->current_page, 0, pd->page_width, pd->page_height);
}

static void
_scroll_cb(void *data, Evas_Object *scroller, void *event_info)
{
	page_data *pd = data;
	double page_no;
	int page;
	char buf[50];
	Evas_Coord x, y, w, h;

	if (pd->is_index) {
		elm_scroller_current_page_get(pd->index_scroller, &page, NULL);
		if ((int)pd->current_page != page) {
			pd->current_page = page;
			_index_sync(pd);
		}
	}
	else {
		elm_scroller_region_get(pd->slider_scroller, &x, &y, &w, &h);
		page_no = x / pd->page_width;
		pd->current_page = page_no + 0.5; //To calculate 50% of the page scroll
		pd->page_width = w;
		pd->page_height = h;
		sprintf(buf, "%d", (int)(pd->current_page + 1.0));
		edje_object_part_text_set(_EDJ(pd->slider_popup), "page_no", buf);
		_index_sync(pd);
	}

}

static void
_slider_up(page_data *pd)
{
	Evas_Object *slider;
	slider = elm_object_part_content_unset(pd->index_layout, "controller");
	evas_object_hide(slider);
	elm_object_part_content_set(pd->index_layout, "controller", pd->index);
	evas_object_show(pd->index);
	evas_object_hide(pd->slider_popup);
	pd->slider_show = EINA_FALSE;
}

static void
_slider_move(page_data *pd)
{
	double idx;
	double normal;
	Evas_Coord y, w, h;
	char buf[50];

	evas_object_geometry_get(pd->box, NULL, &y, &w, &h);
	idx = pd->dx * (MAX_PAGE - 1);
	normal = (double)(w / MAX_PAGE);
	int unit = (int) (normal * idx);
	idx = idx + 0.5;
	pd->current_page = (int)idx;
	elm_scroller_region_show(pd->slider_scroller, unit, y, (w / MAX_PAGE), h);
	sprintf(buf, "%d", (int)(pd->current_page + 1.0));
	edje_object_part_text_set(_EDJ(pd->slider_popup), "page_no", buf);
}

static Evas_Object *
_create_scroller(Evas_Object *parent, page_data *pd, Eina_Bool is_index)
{
	/* Create Scroller */
	Evas_Object *scroller;
	scroller = elm_scroller_add(parent);
	elm_scroller_loop_set(scroller, EINA_FALSE, EINA_FALSE);
	evas_object_size_hint_weight_set(scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(scroller, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
	elm_object_scroll_lock_y_set(scroller, EINA_TRUE);
	if (is_index) {
		elm_scroller_page_relative_set(scroller, 1.0, 0.0);
		elm_scroller_page_scroll_limit_set(scroller, 1, 0);
	}
	evas_object_smart_callback_add(scroller, "scroll", _scroll_cb, pd);
	return scroller;
}

static Evas_Object *
_create_box(Evas_Object *parent, page_data *pd)
{
	Evas_Object *box, *page_layout, *page;
	box = elm_box_add(parent);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_box_horizontal_set(box, EINA_TRUE);

	/* Create Pages */
	// page 1 layout
	page_layout = elm_layout_add(box);
	elm_layout_file_set(page_layout, ELM_DEMO_EDJ, "elmdemo-test/pagecontrol/page");
	evas_object_size_hint_weight_set(page_layout, 0, 0);
	evas_object_size_hint_align_set(page_layout, 0, EVAS_HINT_FILL);
	evas_object_show(page_layout);
	pd->page_layout[0] = page_layout;

	// page 1 content
	page = evas_object_rectangle_add(evas_object_evas_get(page_layout));
	evas_object_color_set(page, 50, 0, 0, 50);
	pd->page[0] = page;

	elm_object_part_content_set(page_layout, "page", page);
	elm_object_part_text_set(page_layout, "text", "Page1");
	elm_box_pack_end(box, page_layout);

	// page 2 layout
	page_layout = elm_layout_add(box);
	evas_object_size_hint_weight_set(page_layout, 0, 0);
	evas_object_size_hint_align_set(page_layout, 0, EVAS_HINT_FILL);
	elm_layout_file_set(page_layout, ELM_DEMO_EDJ, "elmdemo-test/pagecontrol/page");
	evas_object_show(page_layout);
	pd->page_layout[1] = page_layout;

	// page 2 content
	page = evas_object_rectangle_add(evas_object_evas_get(page_layout));
	evas_object_color_set(page, 0, 50, 0, 50);
	pd->page[1] = page;

	elm_object_part_content_set(page_layout, "page", page);
	elm_object_part_text_set(page_layout, "text", "Page2");
	elm_box_pack_end(box, page_layout);

	// page 3 layout
	page_layout = elm_layout_add(box);
	evas_object_size_hint_weight_set(page_layout, 0, 0);
	evas_object_size_hint_align_set(page_layout, 0, EVAS_HINT_FILL);
	elm_layout_file_set(page_layout, ELM_DEMO_EDJ, "elmdemo-test/pagecontrol/page");
	evas_object_show(page_layout);
	pd->page_layout[2] = page_layout;

	// page 3 content
	page = evas_object_rectangle_add(evas_object_evas_get(page_layout));
	evas_object_color_set(page, 0, 0, 50, 50);
	pd->page[2] = page;

	elm_object_part_content_set(page_layout, "page", page);
	elm_object_part_text_set(page_layout, "text", "Page3");

	elm_box_pack_end(box, page_layout);
	return box;
}

static Eina_Bool
_pop_cb(void *data, Elm_Object_Item *it)
{
	page_data *pd = data;
	evas_object_del(pd->index_layout);
	return EINA_TRUE;
}

static void
_layout_del_cb(void *data , Evas *e, Evas_Object *obj, void *event_info)
{
	page_data *pd = data;
	if (pd->slider_popup)
		evas_object_del(pd->slider_popup);
	free(pd);
}

static void
_layout_resize_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	page_data *pd = (page_data *)data;
	Evas_Coord w, h;

	evas_object_geometry_get(obj, NULL, NULL, &w, &h);
	evas_object_size_hint_min_set(pd->page[0], w, h);
	evas_object_size_hint_min_set(pd->page[1], w, h);
	evas_object_size_hint_min_set(pd->page[2], w, h);
	pd->page_width = w;
	elm_scroller_page_size_set(pd->index_scroller, w, h);
	elm_scroller_page_show(pd->index_scroller, (int)pd->current_page, 0);
	_slider_popup_position_set(pd);
}

static double
_step_size_calculate(Evas_Object *obj, double min, double max)
{
	double step = 0.0;
	int steps = 0;

	steps = max - min;
	if (steps) step = (1.0 / steps);
	return step;
}

static Evas_Object*
_create_index_slider(page_data *pd)
{
	/* Create Index Slider */
	double step;
	Evas_Object *index_slider;
	index_slider = elm_slider_add(pd->index_layout);
	elm_object_style_set(index_slider, "pagecontrol");
	evas_object_size_hint_weight_set(index_slider, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(index_slider, EVAS_HINT_FILL, 0.5);
	elm_slider_indicator_show_set(index_slider, EINA_FALSE);
	elm_slider_min_max_set(index_slider, 0, (MAX_PAGE - 1));
	step = _step_size_calculate(index_slider, 0, (MAX_PAGE - 1));
	elm_slider_step_set(index_slider, step);
	elm_slider_value_set(index_slider, 0);
	return index_slider;
}

Eina_Bool
_longpress_timer_cb(void *data)
{
	page_data *pd = (page_data *)data;
	Evas_Object *obj, *content;

	elm_object_content_unset(pd->index_scroller);
	content = elm_object_part_content_unset(pd->index_layout, "scroller");
	evas_object_hide(content);
	evas_object_hide(pd->index_scroller);

	elm_object_content_set(pd->slider_scroller, pd->box);
	elm_object_part_content_set(pd->index_layout, "scroller", pd->slider_scroller);
	evas_object_show(pd->slider_scroller);

	pd->is_index = EINA_FALSE;
	obj = elm_object_part_content_unset(pd->index_layout, "controller");
	evas_object_hide(obj);
	elm_object_part_content_set(pd->index_layout, "controller", pd->index_slider);
	evas_object_show(pd->index_slider);
	pd->longpress_timer = NULL;
	elm_slider_value_set(pd->index_slider, pd->dx * (MAX_PAGE -1));
	_slider_move(pd);
	pd->slider_show = EINA_TRUE;
	evas_object_show(pd->slider_popup);
	return ECORE_CALLBACK_CANCEL;
}

static void
_on_index_mouse_down_cb(void *data, Evas *e, Evas_Object *o, void *event_info)
{
	page_data *pd = (page_data *)data;

	/* Keep the last index item active and save the selected index item */
	if (!pd->last_it) return;

	int level = elm_index_item_level_get(o);
	pd->new_it = elm_index_selected_item_get(o, level);
	elm_index_item_selected_set(pd->last_it, EINA_TRUE);
	if (!pd->longpress_timer)
		pd->longpress_timer = ecore_timer_add(elm_config_longpress_timeout_get(), _longpress_timer_cb, pd);
}

static void
_on_index_mouse_up_cb(void *data, Evas *e, Evas_Object *o, void *event_info)
{
	page_data *pd = (page_data *)data;
	Evas_Object *content;
	if (pd->longpress_timer) {
		ecore_timer_del(pd->longpress_timer);
		pd->longpress_timer = NULL;
	}
	/* Keep the last index item active and move to the page of the currently selected index item */
	if (!pd->last_it) return;
	elm_index_item_selected_set(pd->last_it, EINA_TRUE);

	if (!pd->new_it) return;

	int idx = (int) elm_object_item_data_get(pd->new_it);
	pd->current_page = idx;

	_page_position_calculate(pd);
	_index_sync(pd);
	_slider_up(pd);

	if (pd->is_index) {
		elm_scroller_page_bring_in(pd->index_scroller, idx, 0);
	}
	else {
		elm_object_content_unset(pd->slider_scroller);
		content = elm_object_part_content_unset(pd->index_layout, "scroller");
		evas_object_hide(content);
		evas_object_hide(pd->slider_scroller);

		elm_object_content_set(pd->index_scroller, pd->box);
		elm_object_part_content_set(pd->index_layout, "scroller", pd->index_scroller);
		evas_object_show(pd->index_scroller);
		elm_scroller_page_show(pd->index_scroller, idx, 0);
	}
	pd->is_index = EINA_TRUE;
}

static void
_on_index_mouse_move_cb(void *data, Evas *e, Evas_Object *o, void *event_info)
{
	page_data *pd = (page_data *)data;
	if (!pd->last_it) return;

	int level = elm_index_item_level_get(o);
	pd->new_it = elm_index_selected_item_get(o, level);
	elm_index_item_selected_set(pd->last_it, EINA_TRUE);
}

static void
_on_rect_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	page_data *pd = (page_data *)data;
	Evas_Event_Mouse_Down *ev = event_info;
	Evas_Coord x, y, w, h;
	evas_object_geometry_get(obj, &x, &y, &w, &h);
	pd->dx = ((double)ev->canvas.x - (double)x) / (double)w;
}

static void
_on_rect_mouse_move_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	page_data *pd = (page_data *)data;
	Evas_Event_Mouse_Down *ev = event_info;
	Evas_Coord x, y, w, h;
	evas_object_geometry_get(obj, &x, &y, &w, &h);
	pd->dx = ((double)ev->canvas.x - (double)x) / (double)w;
	if (pd->slider_show && pd->dx <= 1.0) {
		elm_slider_value_set(pd->index_slider, pd->dx * (MAX_PAGE -1));
		_slider_move(pd);
	}
}

static void
_create_index(Evas_Object* parent, page_data *pd)
{
	Evas_Object *index, *rect;
	Elm_Object_Item *it;

	if (parent == NULL) return;
	/* Create Layout */
	if (pd == NULL) return;
	pd->index_layout = elm_layout_add(parent);
	if (pd->index_layout == NULL) return;
	elm_layout_file_set(pd->index_layout, ELM_DEMO_EDJ, "elmdemo-test/index_slider");
	evas_object_size_hint_weight_set(pd->index_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(pd->index_layout);

	evas_object_event_callback_add(pd->index_layout, EVAS_CALLBACK_RESIZE, _layout_resize_cb, pd);
	evas_object_event_callback_add(pd->index_layout, EVAS_CALLBACK_DEL, _layout_del_cb, pd);

	rect = evas_object_rectangle_add(evas_object_evas_get(parent));
	evas_object_color_set(rect, 0, 0, 0, 0);
	evas_object_pass_events_set(rect, EINA_TRUE);
	elm_object_part_content_set(pd->index_layout, "controller_rect", rect);
	evas_object_event_callback_add(rect, EVAS_CALLBACK_MOUSE_DOWN, _on_rect_down_cb, pd);
	evas_object_event_callback_add(rect, EVAS_CALLBACK_MOUSE_MOVE, _on_rect_mouse_move_cb, pd);

	pd->is_index = EINA_TRUE;
	pd->index_scroller = _create_scroller(pd->index_layout, pd, pd->is_index);
	elm_object_part_content_set(pd->index_layout, "scroller", pd->index_scroller);

	pd->box = _create_box(pd->index_scroller, pd);

	elm_object_content_set(pd->index_scroller, pd->box);
	evas_object_show(pd->box);
	evas_object_show(pd->index_scroller);

	pd->current_page = 0.0;

	/* Create Index */
	index = elm_index_add(pd->index_layout);
	evas_object_size_hint_weight_set(index, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(index, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_style_set(index, "pagecontrol");
	elm_index_horizontal_set(index, EINA_TRUE);
	elm_index_autohide_disabled_set(index, EINA_TRUE);
	elm_object_part_content_set(pd->index_layout, "controller", index);

	it = elm_index_item_append(index, "1", NULL, (void *) 0);
	elm_index_item_append(index, "2", NULL, (void *) 1);
	elm_index_item_append(index, "3", NULL, (void *) 2);

	elm_index_level_go(index, 0);
	elm_index_item_selected_set(it, EINA_TRUE);
	pd->index = index;
	pd->last_it = it;

	evas_object_event_callback_add(index, EVAS_CALLBACK_MOUSE_DOWN, _on_index_mouse_down_cb, pd);
	evas_object_event_callback_add(index, EVAS_CALLBACK_MOUSE_MOVE, _on_index_mouse_move_cb, pd);
	evas_object_event_callback_add(index, EVAS_CALLBACK_MOUSE_UP, _on_index_mouse_up_cb, pd);

	pd->index_slider = _create_index_slider(pd);
	pd->slider_scroller = _create_scroller(pd->index_layout, pd, !pd->is_index);
}

void index_slider_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad;
	Elm_Object_Item *navi_it;

	ad = (struct appdata *)data;
	if (ad == NULL) return;
	page_data *pd = calloc(1, sizeof(page_data));
	//slider popup layout
	pd->slider_popup = elm_layout_add(ad->nf);
	elm_layout_file_set(pd->slider_popup, ELM_DEMO_EDJ, "elmdemo-test/index_slider_popup");
	evas_object_resize(pd->slider_popup,
			(int)((double)SLIDER_POPUP_X_Y * elm_config_scale_get()),
			(int)((double)SLIDER_POPUP_X_Y * elm_config_scale_get()));
	_create_index(ad->nf, pd);
	navi_it = elm_naviframe_item_push(ad->nf, _("Slider PageControl") , NULL, NULL, pd->index_layout, NULL);
	elm_naviframe_item_pop_cb_set(navi_it, _pop_cb, pd);
}
