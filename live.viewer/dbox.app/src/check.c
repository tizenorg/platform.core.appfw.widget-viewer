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
#include "check.h"

/*********************************************************
 check
 ********************************************************/
static Evas_Object *_create_scroller(Evas_Object *parent);
static Evas_Object *_create_checks(Evas_Object *parent);
static Evas_Object *_create_extended_style_checks(Evas_Object *parent);
static void _style_check_cb(void *data, Evas_Object * obj, void *event_info);
static void _extended_style_check_cb(void *data, Evas_Object * obj, void *event_info);


static struct _menu_item _menu_its[] = {
	{"Normal Styles", _style_check_cb},
	{"Extended Styles", _extended_style_check_cb},
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

static void _go_check_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	int param = (int)(long)(data);

	Eina_Bool state = elm_check_state_get(obj);

	printf("Check %d:%d\n", param, state);
}

static Evas_Object *_create_extended_style_checks(Evas_Object *parent)
{
	Evas_Object *check;

	Evas_Object *layout;

	layout = elm_layout_add(parent);
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "elmdemo-test/check-extended");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	check = elm_check_add(layout);
	elm_check_state_set(check, EINA_TRUE);
	elm_object_part_content_set(layout, "check1", check);
	elm_object_style_set(check, "default/extended");
	evas_object_smart_callback_add(check, "changed", _go_check_clicked_cb, (void *)1);

	check = elm_check_add(layout);
	elm_check_state_set(check, EINA_FALSE);
	elm_object_part_content_set(layout, "check2", check);
	elm_object_style_set(check, "favorite/extended");
	evas_object_smart_callback_add(check, "changed", _go_check_clicked_cb, (void *)2);

	return layout;
}

static Evas_Object *_create_checks(Evas_Object *parent)
{
	Evas_Object *check;

	Evas_Object *layout;

	layout = elm_layout_add(parent);
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "elmdemo-test/check");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	check = elm_check_add(layout);
	elm_check_state_set(check, EINA_TRUE);
	elm_object_part_content_set(layout, "check1", check);
	evas_object_smart_callback_add(check, "changed", _go_check_clicked_cb, (void *)1);

	check = elm_check_add(layout);
	elm_check_state_set(check, EINA_FALSE);
	elm_object_part_content_set(layout, "check2", check);
	elm_object_style_set(check, "favorite");
	evas_object_smart_callback_add(check, "changed", _go_check_clicked_cb, (void *)2);

	check = elm_check_add(layout);
	elm_check_state_set(check, EINA_FALSE);
	elm_object_part_content_set(layout, "check3", check);
	elm_object_style_set(check, "on&off");
	evas_object_smart_callback_add(check, "changed", _go_check_clicked_cb, (void *)3);

	check = elm_check_add(layout);
	elm_check_state_set(check, EINA_TRUE);
	elm_object_part_content_set(layout, "check4", check);
	elm_object_style_set(check, "favorite_small");
	evas_object_smart_callback_add(check, "changed", _go_check_clicked_cb, (void *)4);

	return layout;
}

static Evas_Object *_create_scroller(Evas_Object *parent)
{
	Evas_Object *scroller = elm_scroller_add(parent);

	elm_scroller_bounce_set(scroller, EINA_FALSE, EINA_TRUE);
	elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	evas_object_show(scroller);

	return scroller;
}

static void _extended_style_check_cb(void *data, Evas_Object * obj, void *event_info)
{
	Evas_Object *scroller, *layout_inner;

	struct appdata *ad = (struct appdata *)data;

	if (ad == NULL) return;

	scroller = _create_scroller(ad->nf);
	elm_naviframe_item_push(ad->nf, _("Extended Styles"), NULL, NULL, scroller, NULL);

	layout_inner = _create_extended_style_checks(ad->nf);
	elm_object_content_set(scroller, layout_inner);
}



static void _style_check_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *scroller, *layout_inner;

	struct appdata *ad = (struct appdata *)data;

	if (ad == NULL) return;

	scroller = _create_scroller(ad->nf);
	elm_naviframe_item_push(ad->nf, _("Normal styles"), NULL, NULL, scroller, NULL);

	layout_inner = _create_checks(ad->nf);
	elm_object_content_set(scroller, layout_inner);
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

void check_cb(void *data, Evas_Object * obj, void *event_info)
{
	struct appdata *ad;

	Evas_Object *list;

	ad = (struct appdata *)data;
	if (ad == NULL) return;

	list = _create_list_winset(ad);
	elm_naviframe_item_push(ad->nf, _("Check"), NULL, NULL, list, NULL);
}
