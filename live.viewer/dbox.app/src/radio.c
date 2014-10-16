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
#include "radio.h"

/*********************************************************
 button
 ********************************************************/
static Evas_Object *_create_scroller(Evas_Object *parent);
static Evas_Object *_create_radios(Evas_Object *parent);
static Evas_Object *_create_custom_radios(Evas_Object *parent);
static void _style_radio_cb(void *data, Evas_Object * obj, void *event_info);
static void _custom_style_radio_cb(void *data, Evas_Object * obj, void *event_info);

static struct _menu_item _menu_its[] = {
	{"Normal Styles", _style_radio_cb},
	{"Custom Styles", _custom_style_radio_cb},
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

static void _radio_cb(void *data, Evas_Object *obj, void *event_info)
{
	int x = 0;
	printf("Selected Radio:%d\n", (int)(long)data);
	x = elm_radio_value_get(obj);
	//will print the value assigned to the currently selected radio
	printf("selected value for the group:%d\n", x);
}

static Evas_Object *_create_radios(Evas_Object *parent)
{
	Evas_Object *radio, *rdg = NULL;
	Evas_Object *layout;

	layout = elm_layout_add(parent);
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "elmdemo-test/radio");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	//adding a radio instance
	radio = elm_radio_add(layout);
	elm_object_part_content_set(layout, "radio1", radio);
	//assigning a unique value(within the group) to the radio instance
	elm_radio_state_value_set(radio, 10);
	evas_object_smart_callback_add(radio, "changed", _radio_cb, (void *)1);
	//creating a radio group with first radio
	rdg = radio;

	radio = elm_radio_add(layout);
	//assigning a unique value(within the group) to the radio instance
	elm_radio_state_value_set(radio, 20);
	//adding this radio to the group containing the first radio
	elm_radio_group_add(radio, rdg);
	elm_object_part_content_set(layout, "radio2", radio);
	evas_object_smart_callback_add(radio, "changed", _radio_cb, (void *)2);

	radio = elm_radio_add(layout);
	//assigning a unique value(within the group) to the radio instance
	elm_radio_state_value_set(radio, 30);
	//adding this radio to the group containing the first radio
	elm_radio_group_add(radio, rdg);
	elm_object_part_content_set(layout, "radio3", radio);
	evas_object_smart_callback_add(radio, "changed", _radio_cb, (void *)3);

	radio = elm_radio_add(layout);
	//assigning a unique value(within the group) to the radio instance
	elm_radio_state_value_set(radio, 40);
	//adding this radio to the group containing the first radio
	elm_radio_group_add(radio, rdg);
	elm_object_part_content_set(layout, "radio4", radio);
	elm_object_disabled_set(radio, EINA_TRUE);
	evas_object_smart_callback_add(radio, "changed", _radio_cb, (void *)4);

	//selecting the second radio in the group with value set to 10. This will set the 2nd radio instance
	elm_radio_value_set(rdg, 10);
	return layout;
}

static Evas_Object *_create_custom_radios(Evas_Object *parent)
{
	Evas_Object *radio, *rdg = NULL;
	Evas_Object *layout;

	layout = elm_layout_add(parent);
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "elmdemo-test/radio-custom");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	//adding a radio instance
	radio = elm_radio_add(layout);
	elm_object_part_content_set(layout, "radio1", radio);
	//assigning a unique value(within the group) to the radio instance
	elm_radio_state_value_set(radio, 10);
	elm_object_style_set(radio, "water");
        elm_object_signal_emit(radio, "elm,state,radio,begin", "");
	evas_object_smart_callback_add(radio, "changed", _radio_cb, (void *)1);
	//creating a radio group with first radio
	rdg = radio;

	radio = elm_radio_add(layout);
	//assigning a unique value(within the group) to the radio instance
	elm_radio_state_value_set(radio, 20);
	//adding this radio to the group containing the first radio
	elm_radio_group_add(radio, rdg);
	elm_object_part_content_set(layout, "radio2", radio);
	elm_object_style_set(radio, "water");
	evas_object_smart_callback_add(radio, "changed", _radio_cb, (void *)2);

	radio = elm_radio_add(layout);
	//assigning a unique value(within the group) to the radio instance
	elm_radio_state_value_set(radio, 30);
	//adding this radio to the group containing the first radio
	elm_radio_group_add(radio, rdg);
	elm_object_part_content_set(layout, "radio3", radio);
	elm_object_style_set(radio, "water");
	evas_object_smart_callback_add(radio, "changed", _radio_cb, (void *)3);

	//selecting the second radio in the group with value set to 10. This will set the 2nd radio instance
	elm_radio_value_set(rdg, 10);
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

static void _custom_style_radio_cb(void *data, Evas_Object * obj, void *event_info)
{
	Evas_Object *scroller, *layout_inner;
	struct appdata *ad = (struct appdata *)data;
	if (ad == NULL) return;

	scroller = _create_scroller(ad->nf);
	elm_naviframe_item_push(ad->nf, _("Custom Styles"), NULL, NULL, scroller, NULL);

	layout_inner = _create_custom_radios(ad->nf);
	elm_object_content_set(scroller, layout_inner);
}

static void _style_radio_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *scroller, *layout_inner;
	struct appdata *ad = (struct appdata *)data;
	if (ad == NULL) return;

	scroller = _create_scroller(ad->nf);
	elm_naviframe_item_push(ad->nf, _("Normal styles"), NULL, NULL, scroller, NULL);

	layout_inner = _create_radios(ad->nf);
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
		elm_list_item_append(li, menu_its[idx].name, NULL, NULL, menu_its[idx].func, ad); ++idx;
	}
	elm_list_go(li);

	return li;
}

static Eina_Bool _pop_cb(void *data, Elm_Object_Item *it)
{
	elm_theme_extension_del(NULL, ELM_DEMO_EDJ);
	return EINA_TRUE;
}

void radio_cb(void *data, Evas_Object * obj, void *event_info)
{
	struct appdata *ad;
	Evas_Object *list;
	Elm_Object_Item *it;
	ad = (struct appdata *)data;
	if (ad == NULL) return;

	elm_theme_extension_add(NULL, ELM_DEMO_EDJ);

	list = _create_list_winset(ad);
	it=elm_naviframe_item_push(ad->nf, _("Radio"), NULL, NULL, list, NULL);
	elm_naviframe_item_pop_cb_set(it, _pop_cb, NULL);
}
