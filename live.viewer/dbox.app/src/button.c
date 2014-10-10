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
#include "button.h"
#include "style_button.h"

/*********************************************************
  button
 ********************************************************/
static Evas_Object *_create_scroller(Evas_Object * parent);
static Evas_Object *_create_buttons(Evas_Object * parent);
static void _style_normal_cb(void *data, Evas_Object * obj, void *event_info);
static void _style_extended_cb(void *data, Evas_Object * obj, void *event_info);

static struct _menu_item _menu_its[] = {
	{"Normal Styles", _style_normal_cb},
	{"Extended Styles", _style_extended_cb},
	{"Custom Style", style_button_cb},
	/* do not delete below */
	{NULL, NULL}
};

enum {
	EXPAND_OPENED_STYLE               = 1001, /* Normal Expand styles */
	EXPAND_CLOSED_STYLE               = 1002,
	EXPAND_OPENED_EXTEND_STYLE        = 1003, /* Extended Expand styles */
	EXPAND_CLOSED_EXTEND_STYLE        = 1004,
	EXPAND_ICON_OPENED_STYLE          = 1005,
	EXPAND_ICON_CLOSED_STYLE          = 1006,
	EXPAND_ICON_OPENED_EXTEND_STYLE   = 1007,
	EXPAND_ICON_CLOSED_EXTEND_STYLE   = 1008
}expand_styles;

static int toggle = 0;
static int toggle1 = 0;

static void _list_click(void *data, Evas_Object * obj, void *event_info)
{
	Elm_Object_Item *it = (Elm_Object_Item *) elm_list_selected_item_get(obj);

	if (!it) {
		fprintf(stderr, "list item is NULL\n");
		return;
	}

	elm_list_item_selected_set(it, EINA_FALSE);
}

static void _go_button_clicked_cb(void *data, Evas_Object * obj, void *event_info)
{
	int param = (int)(long)(data);
	char buf[PATH_MAX];
	Evas_Object *btn = obj;

	switch (param) {
	case EXPAND_OPENED_STYLE:
	case EXPAND_OPENED_EXTEND_STYLE:
		if (toggle == 0) {
			/* expand button closed style */
			if (param == EXPAND_OPENED_STYLE) snprintf(buf, sizeof(buf), "expand/closed");
			/* expand button closed extended style */
			else if (param == EXPAND_OPENED_EXTEND_STYLE) snprintf(buf, sizeof(buf), "expand/closed/extended");

			toggle = 1;
		} else {
			/* expand button opened style */
			if (param == EXPAND_OPENED_STYLE) snprintf(buf, sizeof(buf), "expand/opened");
			/* expand button opened extended style */
			else if (param == EXPAND_OPENED_EXTEND_STYLE) snprintf(buf, sizeof(buf), "expand/opened/extended");

			toggle = 0;
		}
		elm_object_style_set(btn, buf);
		printf("clicked event on Button:%d\n", param);
		break;
	case EXPAND_ICON_OPENED_STYLE:
	case EXPAND_ICON_OPENED_EXTEND_STYLE:
		if (toggle1 == 0) {
			/* icon button closed style*/
			if (param == EXPAND_ICON_OPENED_STYLE) snprintf(buf, sizeof(buf), "icon_expand_opened");
			/* icon button closed extended style*/
			else if (param == EXPAND_ICON_OPENED_EXTEND_STYLE) snprintf(buf, sizeof(buf), "icon_expand_opened/extended");

			toggle1 = 1;
		} else {
			/* icon button opened style*/
			if (param == EXPAND_ICON_OPENED_STYLE) snprintf(buf, sizeof(buf), "icon_expand_closed");
			/* icon button opened extended style*/
			else if (param == EXPAND_ICON_OPENED_EXTEND_STYLE) snprintf(buf, sizeof(buf), "icon_expand_closed/extended");

			toggle1 = 0;
		}
		elm_object_style_set(btn, buf);
		printf("clicked event on Button:%d\n", param);
		break;
	default:
		printf("clicked event on Button:%d\n", param);
		break;
	}
}

static void _go_button_unpressed_cb(void *data, Evas_Object * obj, void *event_info)
{
	int param = (int)(long)(data);

	printf("unpressed event on Button:%d\n", param);
}

static char *_access_info_cb(void *data, Evas_Object *obj)
{
	char *ret;
	int param = (int)(long)(data);
	Eina_Strbuf *buf;

	buf = eina_strbuf_new();
	switch (param) {
	case EXPAND_OPENED_STYLE:
	case EXPAND_OPENED_EXTEND_STYLE:
		eina_strbuf_append_printf(buf, "expand style for %d", param);
		break;
	case EXPAND_ICON_OPENED_STYLE:
	case EXPAND_ICON_OPENED_EXTEND_STYLE:
		eina_strbuf_append_printf(buf, "expand icon style for %d", param);
		break;
	default:
		eina_strbuf_append_printf(buf, "there is no text to read on %d", param);
		break;
	}

	ret = eina_strbuf_string_steal(buf);
	eina_strbuf_free(buf);
	return ret;
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

static Evas_Object *_create_buttons(Evas_Object * parent)
{
	Evas_Object *btn;
	Evas_Object *layout;
	Evas_Object *ic;
	char buf[PATH_MAX];
	toggle = 0;
	toggle1 = 0;

	layout = elm_layout_add(parent);
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "elmdemo-test/button");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_reveal", btn);
	elm_object_style_set(btn, "reveal");
	evas_object_smart_callback_add(btn, "clicked", _go_button_clicked_cb, (void *)1);
	evas_object_smart_callback_add(btn, "unpressed", _go_button_unpressed_cb, (void *)1);
	elm_access_info_cb_set(btn, ELM_ACCESS_INFO, _access_info_cb, (void *)1);

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_expand", btn);
	elm_object_style_set(btn, "expand/opened");
	evas_object_smart_callback_add(btn, "clicked", _go_button_clicked_cb, (void *)EXPAND_OPENED_STYLE);
	evas_object_smart_callback_add(btn, "unpressed", _go_button_unpressed_cb, (void *)EXPAND_OPENED_STYLE);
	elm_access_info_cb_set(btn, ELM_ACCESS_INFO, _access_info_cb, (void *)EXPAND_OPENED_STYLE);

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_info", btn);
	elm_object_style_set(btn, "info");
	evas_object_smart_callback_add(btn, "clicked", _go_button_clicked_cb, (void *)3);
	evas_object_smart_callback_add(btn, "unpressed", _go_button_unpressed_cb, (void *)3);
	elm_access_info_cb_set(btn, ELM_ACCESS_INFO, _access_info_cb, (void *)3);

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_rename", btn);
	elm_object_style_set(btn, "rename");
	evas_object_smart_callback_add(btn, "clicked", _go_button_clicked_cb, (void *)4);
	evas_object_smart_callback_add(btn, "unpressed", _go_button_unpressed_cb, (void *)4);
	elm_access_info_cb_set(btn, ELM_ACCESS_INFO, _access_info_cb, (void *)4);

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_call", btn);
	elm_object_style_set(btn, "call");
	evas_object_smart_callback_add(btn, "clicked", _go_button_clicked_cb, (void *)5);
	evas_object_smart_callback_add(btn, "unpressed", _go_button_unpressed_cb, (void *)5);
	elm_access_info_cb_set(btn, ELM_ACCESS_INFO, _access_info_cb, (void *)5);

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_warning", btn);
	elm_object_style_set(btn, "warning");
	evas_object_smart_callback_add(btn, "clicked", _go_button_clicked_cb, (void *)6);
	evas_object_smart_callback_add(btn, "unpressed", _go_button_unpressed_cb, (void *)6);
	elm_access_info_cb_set(btn, ELM_ACCESS_INFO, _access_info_cb, (void *)6);

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_plus", btn);
	elm_object_style_set(btn, "plus");
	evas_object_smart_callback_add(btn, "clicked", _go_button_clicked_cb, (void *)7);
	evas_object_smart_callback_add(btn, "unpressed", _go_button_unpressed_cb, (void *)7);
	elm_access_info_cb_set(btn, ELM_ACCESS_INFO, _access_info_cb, (void *)7);

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_minus", btn);
	elm_object_style_set(btn, "minus");
	evas_object_smart_callback_add(btn, "clicked", _go_button_clicked_cb, (void *)8);
	evas_object_smart_callback_add(btn, "unpressed", _go_button_unpressed_cb, (void *)8);
	elm_access_info_cb_set(btn, ELM_ACCESS_INFO, _access_info_cb, (void *)8);

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_cancel", btn);
	elm_object_style_set(btn, "cancel");
	evas_object_smart_callback_add(btn, "clicked", _go_button_clicked_cb, (void *)9);
	evas_object_smart_callback_add(btn, "unpressed", _go_button_unpressed_cb, (void *)9);
	elm_access_info_cb_set(btn, ELM_ACCESS_INFO, _access_info_cb, (void *)9);

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_send", btn);
	elm_object_style_set(btn, "send");
	evas_object_smart_callback_add(btn, "clicked", _go_button_clicked_cb, (void *)10);
	evas_object_smart_callback_add(btn, "unpressed", _go_button_unpressed_cb, (void *)10);
	elm_access_info_cb_set(btn, ELM_ACCESS_INFO, _access_info_cb, (void *)10);

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_contact", btn);
	elm_object_style_set(btn, "contact");
	evas_object_smart_callback_add(btn, "clicked", _go_button_clicked_cb, (void *)11);
	evas_object_smart_callback_add(btn, "unpressed", _go_button_unpressed_cb, (void *)11);
	elm_access_info_cb_set(btn, ELM_ACCESS_INFO, _access_info_cb, (void *)11);

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_circle", btn);
	ic = elm_image_add(layout);
	snprintf(buf, sizeof(buf), "%s/00_controlbar_icon_logs.png", ICON_DIR);
	elm_image_file_set(ic, buf, NULL);
	elm_image_resizable_set(ic, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(btn, "icon", ic);
	elm_object_style_set(btn, "circle/empty");
	evas_object_smart_callback_add(btn, "clicked", _go_button_clicked_cb, (void *)12);
	evas_object_smart_callback_add(btn, "unpressed", _go_button_unpressed_cb, (void *)12);
	elm_access_info_cb_set(btn, ELM_ACCESS_INFO, _access_info_cb, (void *)12);

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_icon_plus", btn);
	elm_object_style_set(btn, "icon_plus");
	evas_object_smart_callback_add(btn, "clicked", _go_button_clicked_cb, (void *)13);
	evas_object_smart_callback_add(btn, "unpressed", _go_button_unpressed_cb, (void *)13);
	elm_access_info_cb_set(btn, ELM_ACCESS_INFO, _access_info_cb, (void *)13);

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_icon_minus", btn);
	elm_object_style_set(btn, "icon_minus");
	evas_object_smart_callback_add(btn, "clicked", _go_button_clicked_cb, (void *)14);
	evas_object_smart_callback_add(btn, "unpressed", _go_button_unpressed_cb, (void *)14);
	elm_access_info_cb_set(btn, ELM_ACCESS_INFO, _access_info_cb, (void *)14);

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_icon_expand", btn);
	elm_object_style_set(btn, "icon_expand_closed");
	evas_object_smart_callback_add(btn, "clicked", _go_button_clicked_cb, (void *)EXPAND_ICON_OPENED_STYLE);
	evas_object_smart_callback_add(btn, "unpressed", _go_button_unpressed_cb, (void *)EXPAND_ICON_OPENED_STYLE);
	elm_access_info_cb_set(btn, ELM_ACCESS_INFO, _access_info_cb, (void *)EXPAND_ICON_OPENED_STYLE);

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_style1", btn);
	elm_object_style_set(btn, "style1");
	ic = elm_image_add(layout);
	snprintf(buf, sizeof(buf), "%s/00_button_call.png", ICON_DIR);
	elm_image_file_set(ic, buf, NULL);
	evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(ic, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(btn, "icon", ic);
	elm_object_text_set(btn, _("style1"));
	evas_object_smart_callback_add(btn, "clicked", _go_button_clicked_cb, (void *)15);
	evas_object_smart_callback_add(btn, "unpressed", _go_button_unpressed_cb, (void *)15);
	elm_access_info_cb_set(btn, ELM_ACCESS_INFO, _access_info_cb, (void *)15);

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_expandable_number", btn);
	elm_object_style_set(btn, "expandable_number");
	elm_object_text_set(btn, _("1"));
	evas_object_smart_callback_add(btn, "clicked", _go_button_clicked_cb, (void *)16);
	evas_object_smart_callback_add(btn, "unpressed", _go_button_unpressed_cb, (void *)16);
	elm_access_info_cb_set(btn, ELM_ACCESS_INFO, _access_info_cb, (void *)16);

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_sweep", btn);
	elm_object_style_set(btn, "sweep");
	elm_object_text_set(btn, _("sweep"));
	evas_object_smart_callback_add(btn, "clicked", _go_button_clicked_cb, (void *)17);
	evas_object_smart_callback_add(btn, "unpressed", _go_button_unpressed_cb, (void *)17);
	elm_access_info_cb_set(btn, ELM_ACCESS_INFO, _access_info_cb, (void *)17);

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_multibuttonentry", btn);
	elm_object_style_set(btn, "multibuttonentry");
	elm_object_text_set(btn, _("MBE"));
	evas_object_smart_callback_add(btn, "clicked", _go_button_clicked_cb, (void *)18);
	evas_object_smart_callback_add(btn, "unpressed", _go_button_unpressed_cb, (void *)18);
	elm_access_info_cb_set(btn, ELM_ACCESS_INFO, _access_info_cb, (void *)18);

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_naviframe_title_text", btn);
	elm_object_style_set(btn, "naviframe/title_text");
	elm_object_text_set(btn, _("Create"));
	evas_object_smart_callback_add(btn, "clicked", _go_button_clicked_cb, (void *)19);
	evas_object_smart_callback_add(btn, "unpressed", _go_button_unpressed_cb, (void *)19);
	elm_access_info_cb_set(btn, ELM_ACCESS_INFO, _access_info_cb, (void *)19);

	return layout;
}

static Evas_Object *_create_extended_style_buttons(Evas_Object * parent)
{
	Evas_Object *btn;
	Evas_Object *layout;
	toggle = 0;
	toggle1 = 0;

	layout = elm_layout_add(parent);
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "elmdemo-test/button-extended");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_reveal", btn);
	elm_object_style_set(btn, "reveal/extended");
	evas_object_smart_callback_add(btn, "clicked", _go_button_clicked_cb, (void *)1);
	evas_object_smart_callback_add(btn, "unpressed", _go_button_unpressed_cb, (void *)1);

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_expand", btn);
	elm_object_style_set(btn, "expand/opened/extended");
	evas_object_smart_callback_add(btn, "clicked", _go_button_clicked_cb, (void *)EXPAND_OPENED_EXTEND_STYLE);
	evas_object_smart_callback_add(btn, "unpressed", _go_button_unpressed_cb, (void *)EXPAND_OPENED_EXTEND_STYLE);

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_info", btn);
	elm_object_style_set(btn, "info/extended");
	evas_object_smart_callback_add(btn, "clicked", _go_button_clicked_cb, (void *)3);
	evas_object_smart_callback_add(btn, "unpressed", _go_button_unpressed_cb, (void *)3);

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_rename", btn);
	elm_object_style_set(btn, "rename/extended");
	evas_object_smart_callback_add(btn, "clicked", _go_button_clicked_cb, (void *)4);
	evas_object_smart_callback_add(btn, "unpressed", _go_button_unpressed_cb, (void *)4);

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_call", btn);
	elm_object_style_set(btn, "call/extended");
	evas_object_smart_callback_add(btn, "clicked", _go_button_clicked_cb, (void *)5);
	evas_object_smart_callback_add(btn, "unpressed", _go_button_unpressed_cb, (void *)5);

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_warning", btn);
	elm_object_style_set(btn, "warning/extended");
	evas_object_smart_callback_add(btn, "clicked", _go_button_clicked_cb, (void *)6);
	evas_object_smart_callback_add(btn, "unpressed", _go_button_unpressed_cb, (void *)6);

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_plus", btn);
	elm_object_style_set(btn, "plus/extended");
	evas_object_smart_callback_add(btn, "clicked", _go_button_clicked_cb, (void *)7);
	evas_object_smart_callback_add(btn, "unpressed", _go_button_unpressed_cb, (void *)7);

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_minus", btn);
	elm_object_style_set(btn, "minus/extended");
	evas_object_smart_callback_add(btn, "clicked", _go_button_clicked_cb, (void *)8);
	evas_object_smart_callback_add(btn, "unpressed", _go_button_unpressed_cb, (void *)8);

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_cancel", btn);
	elm_object_style_set(btn, "cancel/extended");
	evas_object_smart_callback_add(btn, "clicked", _go_button_clicked_cb, (void *)9);
	evas_object_smart_callback_add(btn, "unpressed", _go_button_unpressed_cb, (void *)9);

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_send", btn);
	elm_object_style_set(btn, "send/extended");
	evas_object_smart_callback_add(btn, "clicked", _go_button_clicked_cb, (void *)10);
	evas_object_smart_callback_add(btn, "unpressed", _go_button_unpressed_cb, (void *)10);

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_icon_plus", btn);
	elm_object_style_set(btn, "icon_plus/extended");
	evas_object_smart_callback_add(btn, "clicked", _go_button_clicked_cb, (void *)11);
	evas_object_smart_callback_add(btn, "unpressed", _go_button_unpressed_cb, (void *)11);

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_icon_minus", btn);
	elm_object_style_set(btn, "icon_minus/extended");
	evas_object_smart_callback_add(btn, "clicked", _go_button_clicked_cb, (void *)12);
	evas_object_smart_callback_add(btn, "unpressed", _go_button_unpressed_cb, (void *)12);

	return layout;
}

static Evas_Object *_create_scroller(Evas_Object * parent)
{
	Evas_Object *scroller = elm_scroller_add(parent);

	elm_scroller_bounce_set(scroller, EINA_FALSE, EINA_TRUE);
	elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	evas_object_show(scroller);

	return scroller;
}

static void _style_normal_cb(void *data, Evas_Object * obj, void *event_info)
{
	Evas_Object *scroller, *layout_inner;

	struct appdata *ad = (struct appdata *)data;

	if (!ad) return;

	scroller = _create_scroller(ad->nf);
	elm_naviframe_item_push(ad->nf, _("Normal Styles"), NULL, NULL, scroller, NULL);

	layout_inner = _create_buttons(ad->nf);
	elm_object_content_set(scroller, layout_inner);
}

static void _style_extended_cb(void *data, Evas_Object * obj, void *event_info)
{
	Evas_Object *scroller, *layout_inner;

	struct appdata *ad = (struct appdata *)data;

	if (!ad) return;

	scroller = _create_scroller(ad->nf);
	elm_naviframe_item_push(ad->nf, _("Extended Styles"), NULL, NULL, scroller, NULL);

	layout_inner = _create_extended_style_buttons(ad->nf);
	elm_object_content_set(scroller, layout_inner);
}

void button_cb(void *data, Evas_Object * obj, void *event_info)
{
	struct appdata *ad;

	Evas_Object *list;

	ad = (struct appdata *)data;
	if (!ad) return;

	list = _create_list_winset(ad);
	elm_naviframe_item_push(ad->nf, _("Button"), NULL, NULL, list, NULL);
}
