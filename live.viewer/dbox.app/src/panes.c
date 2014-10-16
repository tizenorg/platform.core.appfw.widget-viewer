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
#include "panes.h"

/*********************************************************
 panes
 ********************************************************/
static void _panes_default_cb(void *data, Evas_Object *obj, void *event_info);

static void _panes_fixed_cb(void *data, Evas_Object *obj, void *event_info);

static Evas_Object *_create_default_panes(Evas_Object *parent);

static Evas_Object *_create_fixed_panes(Evas_Object *parent);

static double size = 0.0;

static struct _menu_item menu_its[] = {
	{"Movable Pane", _panes_default_cb},
	{"Fixed Pane", _panes_fixed_cb},
	/* do not delete below */
	{NULL, NULL}
};

typedef struct _Panes_Data Panes_Data;

struct _Panes_Data
{
	Evas_Object *parent;
	Evas_Object *panes;
	Evas_Object *content_right;
	Eina_Bool panes_pressed;
};

static Panes_Data pd;

static void
_list_click(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *it = (Elm_Object_Item *) elm_list_selected_item_get(obj);

	if (it == NULL) {
		fprintf(stderr, "list item is NULL\n");
		return;
	}

	elm_list_item_selected_set(it, EINA_FALSE);
}

static Eina_Bool
_pop_cb(void *data, Elm_Object_Item *it)
{
	Panes_Data *pd = (Panes_Data *) data;

	set_rotate_cb_for_winset(NULL, NULL);
	if (pd->content_right) {
		elm_object_part_content_unset(pd->panes, "right");
		evas_object_del(pd->content_right);
	}

	return EINA_TRUE;
}

static int
_rotate_panes_cb(enum appcore_rm rotmode, void *data)
{
	Panes_Data *pd = (Panes_Data *) data;

	if (pd->panes_pressed)
		evas_event_feed_mouse_up(evas_object_evas_get(pd->panes), 1, EVAS_BUTTON_NONE, 0, NULL);

	switch (rotmode) {
		case APPCORE_RM_PORTRAIT_NORMAL:
		case APPCORE_RM_PORTRAIT_REVERSE:
			elm_object_part_content_set(pd->panes, "right", pd->content_right);
			break;

		case APPCORE_RM_LANDSCAPE_NORMAL:
		case APPCORE_RM_LANDSCAPE_REVERSE:
			elm_object_part_content_set(pd->panes, "right", pd->content_right);
			break;

		case APPCORE_RM_UNKNOWN:
			elm_panes_content_left_size_set(pd->panes, 1.0);
			elm_object_part_content_unset(pd->panes, "right");
			evas_object_hide(pd->content_right);
			break;
	}
	return 0;
}

static Evas_Object *
_create_list_winset(struct appdata *ad)
{
	Evas_Object *li;

	if (ad == NULL) return NULL;
	li = elm_list_add(ad->nf);
	elm_list_mode_set(li, ELM_LIST_COMPRESS);
	evas_object_smart_callback_add(li, "selected", _list_click, NULL);

	int idx = 0;

	while (menu_its[idx].name != NULL) {
		elm_list_item_append(li, menu_its[idx].name, NULL, NULL, menu_its[idx].func, ad);
		++idx;
	}
	elm_list_go(li);

	return li;
}

static void
_panes_default_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *panes;
	Elm_Object_Item *it;
	struct appdata *ad = (struct appdata *)data;

	if (ad == NULL) return;
	panes = _create_default_panes(ad->nf);
	it = elm_naviframe_item_push(ad->nf, _("Movable"), NULL, NULL, panes, NULL);
	elm_naviframe_item_pop_cb_set(it, _pop_cb, (void *)&pd);
}

static void
_panes_fixed_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *panes;
	Elm_Object_Item *it;
	struct appdata *ad = (struct appdata *)data;

	if (ad == NULL) return;
	panes = _create_fixed_panes(ad->nf);
	it = elm_naviframe_item_push(ad->nf, _("Fixed"), NULL, NULL, panes, NULL);
	elm_naviframe_item_pop_cb_set(it, _pop_cb, (void *)&pd);
}

static void
_press(void *data, Evas_Object *obj, void *event_info)
{
	pd.panes_pressed = EINA_TRUE;
	printf("press\n");
}

static void
_unpress(void *data, Evas_Object *obj, void *event_info)
{
	pd.panes_pressed = EINA_FALSE;
	printf("unpress, size : %f\n", elm_panes_content_left_size_get(obj));
}

static void
_clicked(void *data, Evas_Object *obj, void *event_info)
{
	printf("clicked\n");
}

static void
_clicked_double(void *data, Evas_Object *obj, void *event_info)
{
	printf("clicked double\n");
	if (elm_panes_content_left_size_get(obj) > 0) {
		size = elm_panes_content_left_size_get(obj);
		elm_panes_content_left_size_set(obj, 0.0);
	}
	else
		elm_panes_content_left_size_set(obj, size);
}

static Evas_Object *
_create_default_panes(Evas_Object *parent)
{
	Evas_Object *panes, *panes_h, *btn;
	Eina_Bool current_mode;

	panes = elm_panes_add(parent);
	pd.parent = parent;
	pd.panes = panes;
	//by default, pane's type is vertical
	//setting the expansion weight hints for the pane
	evas_object_size_hint_weight_set(panes, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(panes, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(panes);

	//handlers for the signals emitted by the outer pane
	evas_object_smart_callback_add(panes, "clicked", _clicked, panes);
	evas_object_smart_callback_add(panes, "clicked,double", _clicked_double, panes);

	evas_object_smart_callback_add(panes, "press", _press, panes);
	evas_object_smart_callback_add(panes, "unpress", _unpress, panes);

	//creating a control to be set as one of the contents of the pane
	btn = elm_button_add(panes);
	elm_object_text_set(btn, "Left");
	evas_object_size_hint_weight_set(btn, 1.0, 1.0);
	evas_object_size_hint_align_set(btn, -1.0, -1.0);
	//setting the relative size of the left view of the pane
	elm_panes_content_left_size_set(panes, 0.5);
	//setting the left content of the pane
	elm_object_part_content_set(panes, "left", btn);

	//right content of the pane
	panes_h = elm_panes_add(panes);
	elm_panes_horizontal_set(panes_h, EINA_TRUE);
	evas_object_size_hint_weight_set(panes_h, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(panes_h, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(panes_h);

	evas_object_smart_callback_add(panes_h, "clicked", _clicked, panes_h);
	evas_object_smart_callback_add(panes_h, "clicked,double", _clicked_double, panes_h);

	evas_object_smart_callback_add(panes_h, "press", _press, panes_h);
	evas_object_smart_callback_add(panes_h, "unpress", _unpress, panes_h);

	btn = elm_button_add(panes_h);
	elm_object_text_set(btn, "Up");
	evas_object_size_hint_weight_set(btn, 1.0, 1.0);
	evas_object_size_hint_align_set(btn, -1.0, -1.0);
	//setting the relative size of the left view of the pane
	elm_panes_content_left_size_set(panes_h, 0.3);
	//setting the top content of the horizontal pane
	elm_object_part_content_set(panes_h, "left", btn);

	btn = elm_button_add(panes_h);
	elm_object_text_set(btn, "Down");
	evas_object_size_hint_weight_set(btn, 1.0, 1.0);
	evas_object_size_hint_align_set(btn, -1.0, -1.0);
	//setting the bottom content of the horizontal pane
	elm_object_part_content_set(panes_h, "right", btn);
	pd.content_right = panes_h;
	set_rotate_cb_for_winset(_rotate_panes_cb, &pd);
	current_mode = is_portrait_mode();
	if (current_mode == EINA_TRUE) {
		_rotate_panes_cb(APPCORE_RM_PORTRAIT_NORMAL, &pd);
	}
	else {
		_rotate_panes_cb(APPCORE_RM_LANDSCAPE_NORMAL, &pd);
	}
	return panes;
}

static Evas_Object *
_create_fixed_panes(Evas_Object *parent)
{
	Evas_Object *panes, *panes_h, *btn;
	Eina_Bool current_mode;

	panes = elm_panes_add(parent);
	pd.parent = parent;
	pd.panes = panes;
	//by default, pane's type is vertical
	//setting the expansion weight hints for the pane
	evas_object_size_hint_weight_set(panes, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(panes, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(panes);
	//restrict the movement of handler with user interaction
	elm_panes_fixed_set(panes, EINA_TRUE);

	//handlers for the signals emitted by the outer pane
	evas_object_smart_callback_add(panes, "clicked", _clicked, panes);
	evas_object_smart_callback_add(panes, "clicked,double", _clicked_double, panes);

	evas_object_smart_callback_add(panes, "press", _press, panes);
	evas_object_smart_callback_add(panes, "unpress", _unpress, panes);

	//creating a control to be set as one of the contents of the pane
	btn = elm_button_add(panes);
	elm_object_text_set(btn, "Left");
	evas_object_size_hint_weight_set(btn, 1.0, 1.0);
	evas_object_size_hint_align_set(btn, -1.0, -1.0);
	//setting the relative size of the left view of the pane
	elm_panes_content_left_size_set(panes, 0.5);
	//setting the left content of the pane
	elm_object_part_content_set(panes, "left", btn);

	//right content of the pane
	panes_h = elm_panes_add(panes);
	elm_panes_horizontal_set(panes_h, EINA_TRUE);
	evas_object_size_hint_weight_set(panes_h, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(panes_h, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(panes_h);
	//restrict the movement of handler with user interaction
	elm_panes_fixed_set(panes_h, EINA_TRUE);

	evas_object_smart_callback_add(panes_h, "clicked", _clicked, panes_h);
	evas_object_smart_callback_add(panes_h, "clicked,double", _clicked_double, panes_h);

	evas_object_smart_callback_add(panes_h, "press", _press, panes_h);
	evas_object_smart_callback_add(panes_h, "unpress", _unpress, panes_h);

	btn = elm_button_add(panes_h);
	elm_object_text_set(btn, "Up");
	evas_object_size_hint_weight_set(btn, 1.0, 1.0);
	evas_object_size_hint_align_set(btn, -1.0, -1.0);
	//setting the relative size of the left view of the pane
	elm_panes_content_left_size_set(panes_h, 0.3);
	//setting the top content of the horizontal pane
	elm_object_part_content_set(panes_h, "left", btn);

	btn = elm_button_add(panes_h);
	elm_object_text_set(btn, "Down");
	evas_object_size_hint_weight_set(btn, 1.0, 1.0);
	evas_object_size_hint_align_set(btn, -1.0, -1.0);
	//setting the bottom content of the horizontal pane
	elm_object_part_content_set(panes_h, "right", btn);
	pd.content_right = panes_h;
	set_rotate_cb_for_winset(_rotate_panes_cb, &pd);
	current_mode = is_portrait_mode();
	if (current_mode == EINA_TRUE) {
		_rotate_panes_cb(APPCORE_RM_PORTRAIT_NORMAL, &pd);
	}
	else {
		_rotate_panes_cb(APPCORE_RM_LANDSCAPE_NORMAL, &pd);
	}
	return panes;
}

void
panes_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *list;
	struct appdata *ad = (struct appdata *)data;

	if (ad == NULL) return;

	list = _create_list_winset(ad);
	elm_naviframe_item_push(ad->nf, _("Split View"), NULL, NULL, list, NULL);
}
