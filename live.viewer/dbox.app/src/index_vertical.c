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

typedef struct _page_data page_data;
struct _page_data
{
	Evas_Object *scroller;
	Evas_Object *box;
	Evas_Object *index;
	Evas_Object *page[3];
	Evas_Object *page_layout[3];

	int current_page;
	int page_width;

	Elm_Object_Item *last_it;
	Elm_Object_Item *new_it;
};

static void
_on_index_mouse_down_cb(void *data, Evas *e, Evas_Object *o, void *event_info)
{
	page_data *pd = (page_data *)data;

	/* Keep the last index item active and save the selected index item */
	if (!pd->last_it) return;

	int level = elm_index_item_level_get(o);
	pd->new_it = elm_index_selected_item_get(o, level);
	elm_index_item_selected_set(pd->last_it, EINA_TRUE);
}

static void
_on_index_mouse_up_cb(void *data, Evas *e, Evas_Object *o, void *event_info)
{
	page_data *pd = (page_data *)data;

	/* Keep the last index item active and move to the page of the currently selected index item */
	if (!pd->last_it) return;
	elm_index_item_selected_set(pd->last_it, EINA_TRUE);

	if (!pd->new_it) return;

	int idx = (int) elm_object_item_data_get(pd->new_it);
	if (idx == pd->current_page) return;

	elm_scroller_page_bring_in(pd->scroller, 0, idx);
}

static void
_on_index_mouse_move_cb(void *data, Evas *e, Evas_Object *o, void *event_info)
{
	page_data *pd = (page_data *)data;

	/* Keep the last index item active and save the currently selected index item */
	if (!pd->last_it) return;

	int level = elm_index_item_level_get(o);
	pd->new_it = elm_index_selected_item_get(o, level);
	elm_index_item_selected_set(pd->last_it, EINA_TRUE);
}

static void
_index_sync(void *data)
{
	page_data *pd = (page_data *)data;
	Elm_Object_Item *it;
	it = elm_index_item_find(pd->index, (void *)pd->current_page);
	if (it) {
		elm_index_item_selected_set(it, EINA_TRUE);
		pd->last_it = it;
		pd->new_it = it;
	}
}

static void
_scroll_cb(void *data, Evas_Object *scroller, void *event_info)
{
	page_data *pd = data;
	int page_no;
	elm_scroller_current_page_get(pd->scroller, NULL, &page_no);
	if (pd->current_page != page_no) {
		pd->current_page = page_no;
		_index_sync(pd);
	}
}

static void
_layout_del_cb(void *data , Evas *e, Evas_Object *obj, void *event_info)
{
	page_data *pd = data;
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
	elm_scroller_page_size_set(pd->scroller, w, h);
	elm_scroller_page_show(pd->scroller, pd->current_page, 0);
}

static Evas_Object*
_create_index(Evas_Object* parent)
{
	Evas_Object *layout, *scroller, *box, *index, *page_layout, *page;
	Elm_Object_Item *it;

	if (parent == NULL) return NULL;
	/* Create Layout */
	layout = elm_layout_add(parent);
	if (layout == NULL) return NULL;
	page_data *pd = calloc(1, sizeof(page_data));
	if (pd == NULL) return NULL;
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "elmdemo-test/index");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(layout);

	evas_object_event_callback_add(layout, EVAS_CALLBACK_RESIZE, _layout_resize_cb, pd);
	evas_object_event_callback_add(layout, EVAS_CALLBACK_DEL, _layout_del_cb, pd);

	/* Create Scroller */
	scroller = elm_scroller_add(layout);
	elm_scroller_loop_set(scroller, EINA_FALSE, EINA_FALSE);
	evas_object_size_hint_weight_set(scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(scroller, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_scroller_page_relative_set(scroller, 0.0, 1.0);
	elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
	elm_scroller_page_scroll_limit_set(scroller, 0, 1);
	elm_object_scroll_lock_x_set(scroller, EINA_TRUE);
	elm_object_part_content_set(layout, "scroller", scroller);
	evas_object_show(scroller);

	pd->scroller = scroller;

	evas_object_smart_callback_add(scroller, "scroll", _scroll_cb, pd);

	/* Create Box */
	box = elm_box_add(scroller);
	elm_box_horizontal_set(box, EINA_FALSE);
	elm_object_content_set(scroller, box);
	evas_object_show(box);

	pd->box = box;

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

	pd->current_page = 0;

	/* Create Index */
	index = elm_index_add(layout);
	evas_object_size_hint_weight_set(index, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(index, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_style_set(index, "pagecontrol");
	elm_index_horizontal_set(index, EINA_TRUE);
	elm_index_autohide_disabled_set(index, EINA_TRUE);
	elm_object_part_content_set(layout, "controller", index);

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

	return layout;
}

void vertical_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad;
	Evas_Object *layout_inner;

	ad = (struct appdata *)data;
	if (ad == NULL) return;

	layout_inner = _create_index(ad->nf);
	elm_naviframe_item_push(ad->nf, _("PageControl") , NULL, NULL, layout_inner, NULL);
}

