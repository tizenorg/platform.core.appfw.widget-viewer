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
#include "popup.h"

/*********************************************************
  popup
 ********************************************************/
static Evas_Object *popup_global = NULL;
static Evas_Object *popup_global_2 = NULL;
static Elm_Genlist_Item_Class itc;
static Ecore_Timer *pb_timer = NULL;
static Ecore_Timer *del_timer = NULL;
static Evas_Object *list = NULL;
static Eina_Bool rotcb_set = EINA_FALSE;
static Eina_Bool rotation_cb = EINA_FALSE;
static Evas_Object *inner_layout = NULL;
static Evas_Object *label1 = NULL;
static Evas_Object *label2 = NULL;
static int state_index = 0; //selected radio index
static Evas_Object *gen_popup = NULL;
Evas_Object *win = NULL;

#define IMAGE_MAX 20

typedef struct _Item_Data
{
	int index;
	Evas_Object *genlist;
	Elm_Object_Item *item;
} Item_Data;

static void _on_rotation_changed(void *data, Evas_Object *obj, void *event_info );
static void _on_rotation_gengrid_1line_change(void *data, Evas_Object *obj, void *event_info );
static void _on_rotation_gengrid_2line_change(void *data, Evas_Object *obj, void *event_info );
static void _on_rotation_gengrid_3line_change(void *data, Evas_Object *obj, void *event_info );
static void _text_rotation_changed(void *data, Evas_Object *obj, void *event_info );

typedef struct _Colorplane_Data Colorplane_Data;
struct _Colorplane_Data
{
	Evas_Object *layout;
	Evas_Object *colorselector;
	Elm_Object_Item *it_last;
	Elm_Object_Item *sel_it;
	struct appdata *ad;
	Eina_Bool changed;
	int r, g, b, a;
};

typedef struct _Testitem
{
	Elm_Object_Item *item;
	const char *text;
	const char *path;
} Testitem;

typedef struct _grid_data
{
	Elm_Object_Item *selected_item;
	Eina_Bool selected;
} grid_data;

static char *Items[] = {
	"Benny Benassi", "Main Item 2", "Main Item 3",
	"Main Item 4", "Main Item 5", "Main Item 6",
	"Main Item 7", "Main Item 8"
};
static char *Sub_Items[] = {
	"08282828282", "Sub Item 2", "Sub Item 3",
	"Sub Item 4", "Sub Item 5", "Sub Item 6",
	"Sub Item 7", "Sub Item 8"
};

static void _popup_del_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	if (elm_object_scroll_freeze_get(list)) elm_object_scroll_freeze_pop(list);
}
static Eina_Bool _pop_cb(void *data, Elm_Object_Item *it)
{
	if (pb_timer) {
		ecore_timer_del(pb_timer);
		pb_timer = NULL;
	}
	if (del_timer) {
		ecore_timer_del(del_timer);
		del_timer = NULL;
	}
	if (popup_global) {
		evas_object_del(popup_global);
		popup_global = NULL;
	}
	elm_theme_extension_del(NULL, ELM_DEMO_EDJ);

	return EINA_TRUE;
}

static void _genlist_click(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *it = (Elm_Object_Item *) elm_genlist_selected_item_get(obj);
	if (it == NULL) {
		fprintf(stderr, "list item is NULL\n");
		return;
	}

	elm_genlist_item_selected_set(it, EINA_FALSE);
}

static void _list_click(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *it = (Elm_Object_Item *) elm_list_selected_item_get(obj);
	if (!it) return;
	elm_list_item_selected_set(it, EINA_FALSE);
}

static void _show_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *icon;
	char buf[1024];
	icon = elm_image_add(inner_layout);
	snprintf(buf, sizeof(buf), "%s/00_img_smart_stay.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	elm_image_aspect_fixed_set(icon, EINA_FALSE);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	evas_object_show(label1);
	evas_object_show(label2);
	elm_object_part_content_set(inner_layout, "elm.swallow.content1", label1);
	elm_object_part_content_set(inner_layout, "elm.swallow.content3", label2);
	elm_object_part_content_set(inner_layout, "elm.swallow.content2", icon);
}


static void _block_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (del_timer) {
		ecore_timer_del(del_timer);
		del_timer = NULL;
	}
	if (rotation_cb) {
		evas_object_smart_callback_del(win, "rotation,changed", _text_rotation_changed);
		rotation_cb = EINA_FALSE;
	}
	evas_object_del(obj);
}

static void _timeout_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (del_timer) {
		ecore_timer_del(del_timer);
		del_timer = NULL;
	}
	evas_object_del(obj);
}

static void _response_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup = data;
	if (pb_timer) {
		ecore_timer_del(pb_timer);
		pb_timer = NULL;
	}
	if (del_timer) {
		ecore_timer_del(del_timer);
		del_timer = NULL;
	}

	if (rotcb_set) {
		evas_object_smart_callback_del(win, "rotation,changed", _on_rotation_changed);
		rotcb_set = EINA_FALSE;
	}

	evas_object_del(data);
}

static void _password_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (elm_object_part_content_get(obj, "elm.swallow.clear")) {
		if (elm_object_focus_get(obj)) {
			if (elm_entry_is_empty(obj))
				elm_object_signal_emit(obj, "elm,state,clear,hidden", "");
			else
				elm_object_signal_emit(obj, "elm,state,clear,visible", "");
		}
	}
}

static void _password_focused_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (elm_object_part_content_get(obj, "elm.swallow.clear")) {
		if (!elm_entry_is_empty(obj))
			elm_object_signal_emit(obj, "elm,state,clear,visible", "");
		else
			elm_object_signal_emit(obj, "elm,state,clear,hidden", "");
	}
	elm_object_signal_emit(obj, "elm,state,focus,on", "");
}

static void _password_unfocused_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (elm_object_part_content_get(obj, "elm.swallow.clear"))
		elm_object_signal_emit(obj, "elm,state,clear,hidden", "");
	elm_object_signal_emit(obj, "elm,state,focus,off", "");
}

static void _eraser_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_entry_entry_set(data, "");
}

static void _password_check_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *layout = data;
	Evas_Object *entry = elm_object_part_content_get(layout, "elm.swallow.content");
	Eina_Bool state;
	state = elm_check_state_get(obj);

	if (state)
		elm_entry_password_set(entry, EINA_FALSE);
	else
		elm_entry_password_set(entry, EINA_TRUE);
}

static void _response_global_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_object_scroll_freeze_pop(list);
	if (pb_timer) {
		ecore_timer_del(pb_timer);
		pb_timer = NULL;
	}
	if (del_timer) {
		ecore_timer_del(del_timer);
		del_timer = NULL;
	}
	evas_object_hide(data);
}

static void _list_item_click(Evas_Object *obj, int index)
{
	printf("Index of the list=%d\n", index);
	evas_object_del(obj);
}

static void _test(void *data, Evas_Object *obj, void *event_info)
{
	if (!data) return;
	Evas_Object *e_n = (Evas_Object *)data;
	if (!elm_entry_is_empty(obj) && elm_entry_is_empty(e_n)) {
		elm_object_focus_set(e_n, EINA_TRUE);
		elm_entry_cursor_end_set(e_n);
	}
}

static Eina_Bool _timer_cb(void *data)
{
	Evas_Object *popup = (Evas_Object *)data;
	_timeout_cb(NULL, popup, NULL);
	del_timer = NULL;
	return ECORE_CALLBACK_CANCEL;
}

static void _item_selected(void *data, Evas_Object *obj, void *event_info)
{
	printf("item selected: %p\n", event_info);
}

static char * grid_text_get(void *data, Evas_Object *obj, const char *part)
{
	Testitem *ti = (Testitem *)data;

	if (!strcmp(part, "elm.text"))
		return strdup(ti->text);

	return NULL;
}

static Evas_Object * grid_content_get(void *data, Evas_Object *obj, const char *part)
{
	Testitem *ti = (Testitem *)data;

	if(!strcmp(part, "elm.swallow.icon")) {
		Evas_Object *icon = elm_image_add(obj);
		elm_image_file_set(icon, ti->path, NULL);
		elm_image_aspect_fixed_set(icon, EINA_FALSE);
		elm_image_preload_disabled_set(icon, EINA_FALSE);
		evas_object_show(icon);
		return icon;
	}

	return NULL;
}

static void
_gengrid_sel(void *data, Evas_Object *obj, void *ei)
{
	evas_object_smart_callback_del(win, "rotation,changed", _on_rotation_gengrid_1line_change);
	evas_object_smart_callback_del(win, "rotation,changed", _on_rotation_gengrid_2line_change);
	evas_object_smart_callback_del(win, "rotation,changed", _on_rotation_gengrid_3line_change);
	evas_object_del(data);
}

static void
_gengrid_sel_mode(void *data, Evas_Object *obj, void *ei)
{
	grid_data *gd = data;
	elm_gengrid_item_selected_set(ei, EINA_FALSE);
	printf("sel | item:%p, sel_it:%p, sel:%d\n",ei, gd->selected_item, gd->selected);
	if(gd->selected && gd->selected_item == ei){
		gd->selected = EINA_FALSE;
	    gd->selected_item = NULL;
	}
	else{
		elm_object_item_signal_emit(ei, "elm,select,enabled", "");
	    gd->selected = EINA_TRUE;
	    gd->selected_item = ei;
	}
}

static void
_gengrid_unsel_mode(void *data, Evas_Object *obj, void *ei)
{
	grid_data *gd = data;
	printf("unsel | item:%p, sel_it:%p, sel:%d\n",ei, gd->selected_item, gd->selected);
	if(gd->selected_item)
		elm_object_item_signal_emit(gd->selected_item, "elm,select,disabled", "");
}

static Evas_Object *
_create_gengrid(void *data, Evas_Object *popup, int row, int col, void *gdata)
{
	Evas_Object *parent = (Evas_Object *)data;
	Evas_Object *gengrid;
	Elm_Gengrid_Item_Class *gic;
	int i, j, n, w, h;
	char buf[PATH_MAX];
	static Testitem ti[IMAGE_MAX];
	grid_data *gd = gdata;

	gengrid = elm_gengrid_add(parent);
	elm_object_style_set(gengrid, "popup");
	evas_object_size_hint_weight_set(gengrid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_gengrid_align_set(gengrid, 0.5, 0.5);

	double scale = elm_config_scale_get();
	w = (int)((190 + (14 * 2)) * scale); // Gengrid item width (image area width + left/right padding) from Winset UX Guide
	h = (int)((150 + (37 * 2)) * scale); // Gengrid item height (image area height + top/bottom padding) from Winset UX Guide

	elm_gengrid_item_size_set(gengrid, w, h);
	if (gd)	{
		gd->selected = EINA_FALSE;
		gd->selected_item = NULL;
		evas_object_smart_callback_add(gengrid, "selected", _gengrid_sel_mode, gd);
		evas_object_smart_callback_add(gengrid, "unselected", _gengrid_unsel_mode, gd);
	} else evas_object_smart_callback_add(gengrid, "selected", _gengrid_sel, popup);

	gic = elm_gengrid_item_class_new();
	gic->item_style = "default";
	gic->func.text_get = grid_text_get;
	gic->func.content_get = grid_content_get;
	gic->func.state_get = NULL;
	gic->func.del = NULL;

	for (j = 0; j < row; j++) {
		for (i = 0; i < col; i++) {
			n = i + (j * IMAGE_MAX);
			snprintf(buf, sizeof(buf), "%s/grid_image/%d_raw.jpg", ICON_DIR, i + 1);
			ti[n].path = eina_stringshare_add(buf);

			ti[n].item = elm_gengrid_item_append(gengrid, gic, &(ti[n]), _item_selected,
					&(ti[i]));
			if (n % 4 == 0)
				ti[n].text = strdup("yyyjjjyyyABCDEFG");
			else if (n % 4 == 1)
				ti[n].text = strdup("ABCDEFGBiiijjjhhk");
			else if (n % 4 == 2)
				ti[n].text = strdup("LMNOPLKDKlkixjjjii");
			else
			ti[n].text = strdup("I have long name.jpg");
		}
	}
	elm_gengrid_item_class_free(gic);

	elm_scroller_policy_set(gengrid, ELM_SCROLLER_POLICY_OFF,ELM_SCROLLER_POLICY_OFF);
	evas_object_show(gengrid);
	return gengrid;
}

static void _back_text_popup_cb(void *data, Evas_Object *obj, void *event_info )
{
	if (rotation_cb) {
		evas_object_smart_callback_del(win, "rotation,changed", _text_rotation_changed);
		rotation_cb = EINA_FALSE;
	}
	evas_object_del(obj);
}


static void _popup_label_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup;
	Evas_Object *scroller;
	struct appdata *ad;
	Evas_Object *layout;
	Evas_Object *label;
	int rotation;
	char str[1000]= {'\0'};

	ad = (struct appdata *) data;
	popup = elm_popup_add(ad->win_main);
	elm_object_scroll_freeze_push(list);

	evas_object_event_callback_add
	     (popup, EVAS_CALLBACK_DEL, _popup_del_cb, NULL);
	ea_object_event_callback_add(popup, EA_CALLBACK_BACK, _back_text_popup_cb, NULL);

	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	strcpy(str, "This popup has only text which can be set by two ways, "
		"if text is small set is via description text. If text is more than a "
		"limit,create a layout in popup, this layout will have a scroller which "
		"minimum value will be decided by application from layout and then "
		"label will have text.Add this label to scroller and scroller is added "
		"to layout, then set layout as content of popup.This way to be used when "
		" application has text will does not fit in screen and popup needs to "
		" be restricted in size after a limit. ");

	if (strlen(str) < 500) {
		elm_object_text_set(popup, str);
	}
	else {
		layout = elm_layout_add(popup);
		rotation = elm_win_rotation_get(ad->win_main);
		if (rotation == 90 || rotation == 270)
			elm_layout_file_set(layout, ELM_DEMO_EDJ, "label_layout_landscape");
		else
			elm_layout_file_set(layout, ELM_DEMO_EDJ, "label_layout");
		evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

		label = elm_label_add(popup);
		elm_object_style_set(label, "popup/default");
		elm_label_line_wrap_set(label, ELM_WRAP_MIXED);
		elm_object_text_set(label, str);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, 0.0);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_show(label);

		scroller = elm_scroller_add(layout);
		elm_scroller_bounce_set(scroller, EINA_TRUE, EINA_TRUE);
		elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
		evas_object_show(scroller);
		elm_object_content_set(scroller, label);

		elm_object_part_content_set(layout, "elm.swallow.content", scroller);

		elm_object_content_set(popup, layout);
		evas_object_smart_callback_add(ad->win_main, "rotation,changed", _text_rotation_changed, layout);
		rotation_cb = EINA_TRUE;
	}

	evas_object_smart_callback_add(popup, "block,clicked", _block_clicked_cb,
		NULL);
	evas_object_show(popup);
}

static void _popup_center_info_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup;
	struct appdata *ad;

	ad = (struct appdata *) data;
	popup = elm_popup_add(ad->win_main);
	elm_object_scroll_freeze_push(list);

	evas_object_event_callback_add
		(popup, EVAS_CALLBACK_DEL, _popup_del_cb, NULL);
	ea_object_event_callback_add(popup, EA_CALLBACK_BACK, _back_text_popup_cb, NULL);

	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_text_set(popup, "This popup has only text which is set via desc set"
			" function, (This popup gets hidden when user "
			" clicks outside) here timeout of 3 sec is set.");
	evas_object_smart_callback_add(popup, "block,clicked", _block_clicked_cb,
		NULL);
	if (!elm_config_access_get())
	{
		elm_popup_timeout_set(popup, 3.0);
		evas_object_smart_callback_add(popup, "timeout", _timeout_cb, NULL);
	}
	else
	{
		evas_object_smart_callback_add(popup, "access,read,stop", _timeout_cb, NULL);
	}
	evas_object_show(popup);

}

static void _popup_center_title_info_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup;
	struct appdata *ad;

	ad = (struct appdata *) data;
	popup = elm_popup_add(ad->win_main);
	elm_object_scroll_freeze_push(list);
	evas_object_event_callback_add
	     (popup, EVAS_CALLBACK_DEL, _popup_del_cb, NULL);
	ea_object_event_callback_add(popup, EA_CALLBACK_BACK, ea_popup_back_cb, NULL);

	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_part_text_set(popup, "title,text", "Title");
	evas_object_smart_callback_add(popup, "block,clicked", _block_clicked_cb,
		NULL);
	elm_popup_timeout_set(popup, 3.0);
	elm_object_text_set(popup,"This Popup has title area and description area "
		"testing wrapping ABCDE-FGHIJ-KLMNO-PQRST-UVWXYZ (This popup gets "
		"hidden when user clicks outside and has a timeout of 3 seconds)");
	evas_object_smart_callback_add(popup, "timeout", _timeout_cb, NULL);
	evas_object_show(popup);
}

static void _popup_center_basic_2button_cb(void *data, Evas_Object *obj,
	void *event_info)
{
	Evas_Object *popup;
	struct appdata *ad;
	Evas_Object *btn1;
	Evas_Object *btn2;

	ad = (struct appdata *) data;
	popup = elm_popup_add(ad->win_main);
	elm_object_scroll_freeze_push(list);
	evas_object_event_callback_add
	     (popup, EVAS_CALLBACK_DEL, _popup_del_cb, NULL);
	ea_object_event_callback_add(popup, EA_CALLBACK_BACK, ea_popup_back_cb, NULL);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_text_set(popup,"This Popup has content area and "
		"action area, action area has two buttons OK and Cancel.");
	btn1 = elm_button_add(popup);
	elm_object_style_set(btn1, "popup");
	elm_object_text_set(btn1, "OK");
	elm_object_part_content_set(popup, "button1", btn1);
	evas_object_smart_callback_add(btn1, "clicked", _response_cb, popup);
	btn2 = elm_button_add(popup);
	elm_object_style_set(btn2, "popup");
	elm_object_text_set(btn2, "Cancel");
	elm_object_part_content_set(popup, "button2", btn2);
	evas_object_smart_callback_add(btn2, "clicked", _response_cb, popup);
	evas_object_show(popup);
}

static void _popup_center_basic_3button_cb(void *data, Evas_Object *obj,
	void *event_info)
{
	Evas_Object *popup;
	struct appdata *ad;
	Evas_Object *btn1;
	Evas_Object *btn2;
	Evas_Object *btn3;

	ad = (struct appdata *) data;
	popup = elm_popup_add(ad->win_main);
	elm_object_scroll_freeze_push(list);
	evas_object_event_callback_add
	     (popup, EVAS_CALLBACK_DEL, _popup_del_cb, NULL);
	ea_object_event_callback_add(popup, EA_CALLBACK_BACK, ea_popup_back_cb, NULL);

	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_text_set(popup,"This Popup has content area and "
		"action area, action area has two buttons OK, Cancel and Close.");
	btn1 = elm_button_add(popup);
	elm_object_style_set(btn1, "popup");
	elm_object_text_set(btn1, "OK");
	elm_object_part_content_set(popup, "button1", btn1);
	evas_object_smart_callback_add(btn1, "clicked", _response_cb, popup);
	btn2 = elm_button_add(popup);
	elm_object_style_set(btn2, "popup");
	elm_object_text_set(btn2, "Cancel");
	elm_object_part_content_set(popup, "button2", btn2);
	evas_object_smart_callback_add(btn2, "clicked", _response_cb, popup);
	btn3 = elm_button_add(popup);
	elm_object_style_set(btn3, "popup");
	elm_object_text_set(btn3, "Close");
	elm_object_part_content_set(popup, "button3", btn3);
	evas_object_smart_callback_add(btn3, "clicked", _response_cb, popup);
	evas_object_show(popup);
}

static void _popup_center_vertical_2button_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup;
	struct appdata *ad;
	Evas_Object *btn1;
	Evas_Object *btn2;

	ad = (struct appdata *) data;
	popup = elm_popup_add(ad->win_main);
	elm_object_scroll_freeze_push(list);
	evas_object_event_callback_add
		(popup, EVAS_CALLBACK_DEL, _popup_del_cb, NULL);
	ea_object_event_callback_add(popup, EA_CALLBACK_BACK, ea_popup_back_cb, NULL);
	elm_object_style_set(popup, "verticalbuttonstyle");
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_text_set(popup,"This popup has content area and two buttons,"
		" aligned vertically.");
	elm_layout_signal_emit(popup, "elm,state,2button", "elm");
	btn1 = elm_button_add(popup);
	elm_object_style_set(btn1, "popup");
	elm_object_text_set(btn1, "OK");
	elm_object_part_content_set(popup, "button1", btn1);
	evas_object_smart_callback_add(btn1, "clicked", _response_cb, popup);
	btn2 = elm_button_add(popup);
	elm_object_style_set(btn2, "popup");
	elm_object_text_set(btn2, "Cancel");
	elm_object_part_content_set(popup, "button2", btn2);
	evas_object_smart_callback_add(btn2, "clicked", _response_cb, popup);
	evas_object_show(popup);
}

static void _popup_center_vertical_3button_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup;
	struct appdata *ad;
	Evas_Object *btn1;
	Evas_Object *btn2;
	Evas_Object *btn3;

	ad = (struct appdata *) data;
	popup = elm_popup_add(ad->win_main);
	elm_object_scroll_freeze_push(list);
	evas_object_event_callback_add
		(popup, EVAS_CALLBACK_DEL, _popup_del_cb, NULL);
	ea_object_event_callback_add(popup, EA_CALLBACK_BACK, ea_popup_back_cb, NULL);
	elm_object_style_set(popup, "verticalbuttonstyle");
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_text_set(popup,"This popup has content area and three buttons,"
		" aligned vertically.");
	btn1 = elm_button_add(popup);
	elm_object_style_set(btn1, "popup");
	elm_object_text_set(btn1, "OK");
	elm_object_part_content_set(popup, "button1", btn1);
	evas_object_smart_callback_add(btn1, "clicked", _response_cb, popup);
	btn2 = elm_button_add(popup);
	elm_object_style_set(btn2, "popup");
	elm_object_text_set(btn2, "Cancel");
	elm_object_part_content_set(popup, "button2", btn2);
	evas_object_smart_callback_add(btn2, "clicked", _response_cb, popup);
	btn3 = elm_button_add(popup);
	elm_object_style_set(btn3, "popup");
	elm_object_text_set(btn3, "Close");
	elm_object_part_content_set(popup, "button3", btn3);
	evas_object_smart_callback_add(btn3, "clicked", _response_cb, popup);
	evas_object_show(popup);
}

static void _popup_center_title_2button_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup;
	Evas_Object *btn1;
	Evas_Object *btn2;
	struct appdata *ad = (struct appdata *) data;

	popup = elm_popup_add(ad->win_main);
	elm_object_scroll_freeze_push(list);
	evas_object_event_callback_add
	     (popup, EVAS_CALLBACK_DEL, _popup_del_cb, NULL);
	ea_object_event_callback_add(popup, EA_CALLBACK_BACK, ea_popup_back_cb, NULL);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_text_set(popup,"This Popup has title area, content area and action"
		" area set, action area has two buttons OK and Cancel.");
	elm_object_part_text_set(popup, "title,text", "Title");
	btn1 = elm_button_add(popup);
	elm_object_style_set(btn1, "popup");
	elm_object_text_set(btn1, "OK");
	elm_object_part_content_set(popup, "button1", btn1);
	evas_object_smart_callback_add(btn1, "clicked", _response_cb, popup);
	btn2 = elm_button_add(popup);
	elm_object_style_set(btn2, "popup");
	elm_object_text_set(btn2, "Cancel");
	elm_object_part_content_set(popup, "button2", btn2);
	evas_object_smart_callback_add(btn2, "clicked", _response_cb, popup);
	evas_object_show(popup);
}

static void _popup_center_basic_1button_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup;
	Evas_Object *btn1;
	struct appdata *ad = (struct appdata *) data;

	popup = elm_popup_add(ad->win_main);
	elm_object_scroll_freeze_push(list);
	evas_object_event_callback_add
	     (popup, EVAS_CALLBACK_DEL, _popup_del_cb, NULL);
	ea_object_event_callback_add(popup, EA_CALLBACK_BACK, ea_popup_back_cb, NULL);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_text_set(popup,"This Popup has content area and "
		"action area, action area has one button OK.");
	btn1 = elm_button_add(popup);
	elm_object_style_set(btn1, "popup");
	elm_object_text_set(btn1, "OK");
	elm_object_part_content_set(popup, "button1", btn1);
	evas_object_smart_callback_add(btn1, "clicked", _response_cb, popup);
	evas_object_show(popup);
}

static void _popup_center_title_1_button_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	evas_object_del(obj);
	popup_global = NULL;
}

static void _popup_center_title_1button_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *btn1;
	struct appdata *ad = (struct appdata *) data;

	elm_object_scroll_freeze_push(list);
	if (popup_global) {
		evas_object_show(popup_global);
	} else {
		popup_global = elm_popup_add(ad->win_main);
		evas_object_event_callback_add
		     (popup_global, EVAS_CALLBACK_DEL, _popup_del_cb, NULL);
		ea_object_event_callback_add(popup_global, EA_CALLBACK_BACK,
				_popup_center_title_1_button_back_cb, NULL);

		elm_object_part_text_set(popup_global, "title,text", "Title");
		elm_object_text_set(popup_global, "This Popup has title area,"
			" content area and action area,"
			" action area has one button OK");
		btn1 = elm_button_add(popup_global);
		elm_object_style_set(btn1, "popup");
		elm_object_text_set(btn1, "OK");
		elm_object_part_content_set(popup_global, "button1", btn1);
		evas_object_smart_callback_add(btn1, "clicked", _response_global_cb, popup_global);
		evas_object_show(popup_global);
	}
}

static Eina_Bool _fn_pb_timer(void *data)
{
	double value=0.0;
	Evas_Object *progressbar = (Evas_Object*) data;

	value = elm_progressbar_value_get(progressbar);
	if (value == 1.0) {
		_timeout_cb(NULL, popup_global_2, NULL);
		pb_timer = NULL;
		return ECORE_CALLBACK_CANCEL;
	}
	value = value + 0.01;
	elm_progressbar_value_set(progressbar, value);

	return ECORE_CALLBACK_RENEW;
}

static void _popup_center_processing_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup;
	Evas_Object *progressbar;
	struct appdata *ad;
	Evas_Object *layout;
	Evas_Object *label;

	ad = (struct appdata *) data;
	popup = elm_popup_add(ad->win_main);
	elm_object_scroll_freeze_push(list);
	evas_object_event_callback_add
	     (popup, EVAS_CALLBACK_DEL, _popup_del_cb, NULL);
	ea_object_event_callback_add(popup, EA_CALLBACK_BACK, ea_popup_back_cb, NULL);
	label = elm_label_add(popup);
	elm_label_line_wrap_set(label, ELM_WRAP_MIXED);
	elm_object_text_set(label, "pop-up dialog box, a child window that blocks user interact to the parent");
	evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(label);

	layout = elm_layout_add(popup);
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "popup_processingview");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	progressbar = elm_progressbar_add(popup);
	elm_progressbar_pulse(progressbar, EINA_TRUE);
	elm_object_style_set(progressbar, "process_large");
	elm_progressbar_horizontal_set(progressbar, EINA_TRUE);
	evas_object_size_hint_align_set(progressbar, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(progressbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(progressbar);

	elm_object_part_content_set(layout, "elm.swallow.content", progressbar);
	elm_object_part_content_set(layout, "elm.swallow.text", label);

	elm_object_content_set(popup, layout);
	elm_popup_timeout_set(popup, 3.0);
	evas_object_smart_callback_add(popup, "timeout", _timeout_cb, NULL);
	popup_global_2 = popup;
	evas_object_show(popup);
}

static void _popup_center_processing_1button_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	evas_object_del(obj);
	popup_global_2 = NULL;
}

static void _popup_center_processing_1button_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup;
	Evas_Object *progressbar;
	struct appdata *ad;
	Evas_Object *layout;
	Evas_Object *btn1;

	ad = (struct appdata *) data;
	popup = elm_popup_add(ad->win_main);
	elm_object_scroll_freeze_push(list);
	evas_object_event_callback_add
	     (popup, EVAS_CALLBACK_DEL, _popup_del_cb, NULL);
	ea_object_event_callback_add(popup, EA_CALLBACK_BACK, _popup_center_processing_1button_back_cb, NULL);

	layout = elm_layout_add(popup);
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "popup_processingview_1button");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	progressbar = elm_progressbar_add(popup);
	elm_progressbar_pulse(progressbar, EINA_TRUE);
	elm_object_style_set(progressbar, "process_large");
	elm_progressbar_horizontal_set(progressbar, EINA_TRUE);
	evas_object_size_hint_align_set(progressbar, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(progressbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(progressbar);

	elm_object_part_content_set(layout, "elm.swallow.content", progressbar);
	elm_object_part_text_set(layout, "elm.text", "processing ");

	elm_object_content_set(popup, layout);
	btn1 = elm_button_add(popup);
	elm_object_style_set(btn1, "popup");
	elm_object_text_set(btn1, "Cancel");
	elm_object_part_content_set(popup, "button1", btn1);
	evas_object_smart_callback_add(btn1, "clicked", _response_cb, popup);
	popup_global_2 = popup;
	evas_object_show(popup);
}

static void _popup_center_progressbar_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	evas_object_del(obj);
	popup_global_2 = NULL;

	if (pb_timer) {
		ecore_timer_del(pb_timer);
		pb_timer = NULL;
	}
}

static void _popup_center_progressbar_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup;
	Evas_Object *progressbar;
	struct appdata *ad;
	Evas_Object *layout;
	Evas_Object *btn1;
	Evas_Object *btn2;
	Evas_Object *btn3;

	ad = (struct appdata *) data;
	popup = elm_popup_add(ad->win_main);
	elm_object_scroll_freeze_push(list);
	evas_object_event_callback_add
	     (popup, EVAS_CALLBACK_DEL, _popup_del_cb, NULL);
	ea_object_event_callback_add(popup, EA_CALLBACK_BACK, _popup_center_progressbar_back_cb, NULL);

	layout = elm_layout_add(popup);
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "popup_center_progressview");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	progressbar = elm_progressbar_add(popup);
	elm_object_style_set(progressbar, "list_progress");
	elm_progressbar_horizontal_set(progressbar, EINA_TRUE);
	evas_object_size_hint_align_set(progressbar, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(progressbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_progressbar_value_set(progressbar, 0.0);
	pb_timer = ecore_timer_add(0.1, _fn_pb_timer, progressbar);
	evas_object_show(progressbar);

	elm_object_part_content_set(layout, "elm.swallow.content", progressbar);
	elm_object_part_text_set(layout, "elm.title", "Text Text");
	elm_object_part_text_set(layout, "elm.text.left", "Text");
	elm_object_part_text_set(layout, "elm.text.right", "TextTextTextText");

	elm_object_content_set(popup, layout);
	btn1 = elm_button_add(popup);
	elm_object_style_set(btn1, "popup");
	elm_object_text_set(btn1, "Ok");
	elm_object_part_content_set(popup, "button1", btn1);
	evas_object_smart_callback_add(btn1, "clicked", _response_cb, popup);
	btn2 = elm_button_add(popup);
	elm_object_style_set(btn2, "popup");
	elm_object_text_set(btn2, "Cancel");
	elm_object_part_content_set(popup, "button2", btn2);
	evas_object_smart_callback_add(btn2, "clicked", _response_cb, popup);
	btn3 = elm_button_add(popup);
	elm_object_style_set(btn3, "popup");
	elm_object_text_set(btn3, "Cancel");
	elm_object_part_content_set(popup, "button3", btn3);
	evas_object_smart_callback_add(btn3, "clicked", _response_cb, popup);

	popup_global_2 = popup;
	evas_object_show(popup);
}

static void _popup_center_text_progressbar_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	evas_object_del(obj);
	popup_global_2 = NULL;

	if (pb_timer) {
		ecore_timer_del(pb_timer);
		pb_timer = NULL;
	}
}

static void _popup_center_text_progressbar_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup,*progressbar;
	struct appdata *ad;
	Evas_Object *layout, *label;
	Evas_Object *btn1;
	Evas_Object *btn2;
	Evas_Object *btn3;

	ad = (struct appdata *) data;
	popup = elm_popup_add(ad->win_main);
	elm_object_scroll_freeze_push(list);
	evas_object_event_callback_add
	     (popup, EVAS_CALLBACK_DEL, _popup_del_cb, NULL);
	ea_object_event_callback_add(popup, EA_CALLBACK_BACK, _popup_center_text_progressbar_back_cb, NULL);

	label = elm_label_add(popup);
	elm_object_style_set(label, "popup/progressview");
	elm_label_line_wrap_set(label, ELM_WRAP_MIXED);
	elm_object_text_set(label, "Push the WPS button on your wireless access point");
	evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(label);

	layout = elm_layout_add(popup);
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "popup_center_text_progressview");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	progressbar = elm_progressbar_add(popup);
	elm_object_style_set(progressbar, "list_progress");
	elm_progressbar_horizontal_set(progressbar, EINA_TRUE);
	evas_object_size_hint_align_set(progressbar, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(progressbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_progressbar_value_set(progressbar, 0.0);
	pb_timer = ecore_timer_add(0.1, _fn_pb_timer, progressbar);
	evas_object_show(progressbar);

	elm_object_part_content_set(layout, "elm.swallow.content", label);
	elm_object_part_content_set(layout, "elm.swallow.end", progressbar);
	elm_object_part_text_set(layout, "elm.text.subtext1", "Left-Text");
	elm_object_part_text_set(layout, "elm.text.subtext2", "Right-Text");

	evas_object_show(layout);
	elm_object_content_set(popup, layout);
	btn1 = elm_button_add(popup);
	elm_object_style_set(btn1, "popup");
	elm_object_text_set(btn1, "Ok");
	elm_object_part_content_set(popup, "button1", btn1);
	evas_object_smart_callback_add(btn1, "clicked", _response_cb, popup);
	btn2 = elm_button_add(popup);
	elm_object_style_set(btn2, "popup");
	elm_object_text_set(btn2, "Cancel");
	elm_object_part_content_set(popup, "button2", btn2);
	evas_object_smart_callback_add(btn2, "clicked", _response_cb, popup);
	btn3 = elm_button_add(popup);
	elm_object_style_set(btn3, "popup");
	elm_object_text_set(btn3, "Cancel");
	elm_object_part_content_set(popup, "button3", btn3);
	evas_object_smart_callback_add(btn3, "clicked", _response_cb, popup);
	popup_global_2 = popup;
	evas_object_show(popup);
}

static void _popup_center_check_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup;
	Evas_Object *check;
	Evas_Object *layout;
	Evas_Object *label;
	Evas_Object *btn1;
	Evas_Object *btn2;
	struct appdata *ad;

	ad = (struct appdata *) data;
	popup = elm_popup_add(ad->win_main);
	elm_object_scroll_freeze_push(list);
	evas_object_event_callback_add
	     (popup, EVAS_CALLBACK_DEL, _popup_del_cb, NULL);
	ea_object_event_callback_add(popup, EA_CALLBACK_BACK, ea_popup_back_cb, NULL);

	label = elm_label_add(popup);
	elm_label_line_wrap_set(label, ELM_WRAP_MIXED);
	elm_object_text_set(label, "Use packet data must be enabled to access data service. change settings?");
	evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(label);

	layout = elm_layout_add(popup);
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "popup_checkview");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	check = elm_check_add(popup);
	elm_object_style_set(check, "multiline");
	elm_object_text_set(check, "Don't ask again Don't ask again  Don't ask again  Don't ask again  Don't ask again  Don't ask again  ");
	evas_object_size_hint_align_set(check, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(check, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(check);

	elm_object_part_content_set(layout, "elm.swallow.content", label);
	elm_object_part_content_set(layout, "elm.swallow.end", check);

	evas_object_show(layout);
	elm_object_content_set(popup, layout);
	btn1 = elm_button_add(popup);
	elm_object_style_set(btn1, "popup");
	elm_object_text_set(btn1, "OK");
	elm_object_part_content_set(popup, "button1", btn1);
	evas_object_smart_callback_add(btn1, "clicked", _response_cb, popup);
	btn2 = elm_button_add(popup);
	elm_object_style_set(btn2, "popup");
	elm_object_text_set(btn2, "Cancel");
	elm_object_part_content_set(popup, "button2", btn2);
	evas_object_smart_callback_add(btn2, "clicked", _response_cb, popup);
	evas_object_show(popup);
}

static char *_access_info_cb(void *data, Evas_Object *obj)
{
	char * text = (char *)data;
	return strdup(text);
}

static void _popup_center_check_image_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup;
	Evas_Object *check;
	Evas_Object *layout;
	Evas_Object *btn1;
	Evas_Object *btn2;
	Evas_Object *label;
	Evas_Object *icon;
	struct appdata *ad;
	char *text = NULL;
	char buf[1024];

	ad = (struct appdata *) data;
	popup = elm_popup_add(ad->win_main);
	elm_object_scroll_freeze_push(list);
	evas_object_event_callback_add
	     (popup, EVAS_CALLBACK_DEL, _popup_del_cb, NULL);
	ea_object_event_callback_add(popup, EA_CALLBACK_BACK, ea_popup_back_cb, NULL);
	elm_object_part_text_set(popup, "title,text", "Title");
	label = elm_label_add(popup);
	elm_label_line_wrap_set(label, ELM_WRAP_MIXED);
	elm_object_text_set(label, "Use packet data must be enabled to access data service. change settings?");
	evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(label);

	text = (char*)elm_object_text_get(label);

	layout = elm_layout_add(popup);
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "popup_checkview_image");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	check = elm_check_add(popup);
	elm_object_text_set(check, "Don't ask again");
	evas_object_size_hint_align_set(check, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(check, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(check);

	icon = elm_image_add(layout);
	snprintf(buf, sizeof(buf), "%s/wood_01.jpg", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	elm_image_aspect_fixed_set(icon, EINA_FALSE);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(layout, "elm.swallow.content", label);
	elm_object_part_content_set(layout, "elm.swallow.icon", icon);
	elm_object_part_content_set(layout, "elm.swallow.end", check);

	elm_access_object_unregister(label);

	Evas_Object * body = (Evas_Object*)edje_object_part_object_get(_EDJ(popup), "access.body");
	Evas_Object *ao = elm_access_object_register(body, popup);
	elm_access_info_cb_set(ao, ELM_ACCESS_INFO, _access_info_cb, text);

	evas_object_show(layout);
	elm_object_content_set(popup, layout);
	btn1 = elm_button_add(popup);
	elm_object_style_set(btn1, "popup");
	elm_object_text_set(btn1, "OK");
	elm_object_part_content_set(popup, "button1", btn1);
	evas_object_smart_callback_add(btn1, "clicked", _response_cb, popup);
	btn2 = elm_button_add(popup);
	elm_object_style_set(btn2, "popup");
	elm_object_text_set(btn2, "Cancel");
	elm_object_part_content_set(popup, "button2", btn2);
	evas_object_smart_callback_add(btn2, "clicked", _response_cb, popup);
	evas_object_show(popup);
}

static Evas_Object * item_provider(void *images, Evas_Object *en, const char *item)
{
	Evas_Object *obj = NULL;
	char buf[1024];

	if (!strcmp(item, "itemprovider"))
	{
		snprintf(buf, sizeof(buf), "%s/00_button_radio_press1.png", ICON_DIR);
		obj = evas_object_image_filled_add(evas_object_evas_get(en));
		evas_object_image_file_set(obj, buf, NULL);
	}

	return obj;
}

static void _text_rotation_changed(void *data, Evas_Object *obj, void *event_info )
{
	int rot = -1;
	Evas_Object *win = obj;
	Evas_Object *layout = data;

	rot = elm_win_rotation_get(win);
	if (rot == 90 || rot == 270)
		elm_layout_file_set(layout, ELM_DEMO_EDJ, "label_layout_landscape");
	else
		elm_layout_file_set(layout, ELM_DEMO_EDJ, "label_layout");
}

static void _on_rotation_changed(void *data, Evas_Object *obj, void *event_info )
{
	int rot = -1;
	Evas_Object *win = obj;
	Evas_Object *layout = data;

	rot = elm_win_rotation_get(win);
	if (rot == 90 || rot == 270)
		elm_layout_file_set(layout, ELM_DEMO_EDJ, "popup_smartstay1_landscape");
	else
		elm_layout_file_set(layout, ELM_DEMO_EDJ, "popup_smartstay1");
}

static void _popup_center_check_text_image_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	evas_object_del(obj);

	if (rotcb_set) {
		evas_object_smart_callback_del(win, "rotation,changed", _on_rotation_changed);
		rotcb_set = EINA_FALSE;
	}
}

static void _popup_center_check_text_image_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup;
	Evas_Object *check;
	Evas_Object *layout;
	Evas_Object *scroller;
	Evas_Object *btn;

	struct appdata *ad;
	int rotation;

	ad = (struct appdata *) data;
	popup = elm_popup_add(ad->win_main);
	elm_object_scroll_freeze_push(list);
	evas_object_event_callback_add
	     (popup, EVAS_CALLBACK_DEL, _popup_del_cb, NULL);
	ea_object_event_callback_add(popup, EA_CALLBACK_BACK, _popup_center_check_text_image_back_cb, NULL);

	elm_object_part_text_set(popup, "title,text", "Smart stay");

	label1 = elm_entry_add(popup);
	ea_entry_selection_back_event_allow_set(label1, EINA_TRUE);
	elm_entry_editable_set(label1, EINA_FALSE);
	elm_object_text_set(label1, "<align=left>Smart stay disables screen timeout if device detects that your face is watching the screen.</align>");
	evas_object_size_hint_weight_set(label1, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(label1, EVAS_HINT_FILL, EVAS_HINT_FILL);

	label2 = elm_entry_add(popup);
	ea_entry_selection_back_event_allow_set(label2, EINA_TRUE);
	elm_entry_editable_set(label2, EINA_FALSE);
	elm_entry_item_provider_append(label2, item_provider, NULL);
	elm_object_text_set(label2, "<align=left>Smart stay may not work in these situations.<br>"
		"<item size=42x42 vsize=full href=itemprovider></item> when front camera fails to detect face.<br>"
		"<item size=42x42 vsize=full href=itemprovider></item> when using device in the dark.<br>"
		"<item size=42x42 vsize=full href=itemprovider></item> when front camera is used for the application.</align>");
	evas_object_size_hint_weight_set(label2, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(label2, EVAS_HINT_FILL, EVAS_HINT_FILL);

	layout = elm_layout_add(popup);
	rotation = elm_win_rotation_get(ad->win_main);
	if (rotation == 90 || rotation == 270)
		elm_layout_file_set(layout, ELM_DEMO_EDJ, "popup_smartstay1_landscape");
	else
		elm_layout_file_set(layout, ELM_DEMO_EDJ, "popup_smartstay1");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	inner_layout = elm_layout_add(popup);
	elm_layout_file_set(inner_layout, ELM_DEMO_EDJ, "popup_smartstay1_internal");
	evas_object_size_hint_weight_set(inner_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	scroller = elm_scroller_add(layout);
	elm_scroller_bounce_set(scroller, EINA_TRUE, EINA_TRUE);
	elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	evas_object_show(scroller);
	elm_object_content_set(scroller, inner_layout);

	elm_object_part_content_set(layout, "elm.swallow.layout", scroller);

	check = elm_check_add(popup);
	elm_object_text_set(check, "Don't ask again");
	evas_object_size_hint_align_set(check, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(check, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	elm_object_part_content_set(layout, "elm.swallow.end", check);

	evas_object_smart_callback_add(popup, "show,finished", _show_clicked_cb,
		NULL);
	elm_object_content_set(popup, layout);
	btn = elm_button_add(popup);
	elm_object_style_set(btn, "popup");
	elm_object_text_set(btn, "OK");
	elm_object_part_content_set(popup, "button1", btn);
	evas_object_smart_callback_add(btn, "clicked", _response_cb, popup);

	evas_object_show(popup);
	evas_object_smart_callback_add(ad->win_main, "rotation,changed", _on_rotation_changed, layout);
	rotcb_set = EINA_TRUE;
}

static void _popup_center_image_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup;
	struct appdata *ad;
	Evas_Object *layout;
	Evas_Object *icon;
	char buf[4096];

	ad = (struct appdata *) data;
	popup = elm_popup_add(ad->win_main);
	elm_object_scroll_freeze_push(list);
	evas_object_event_callback_add
	     (popup, EVAS_CALLBACK_DEL, _popup_del_cb, NULL);
	ea_object_event_callback_add(popup, EA_CALLBACK_BACK, ea_popup_back_cb, NULL);

	elm_object_style_set(popup, "content_expand");

	elm_object_part_text_set(popup, "title,text", "Title");
	layout = elm_layout_add(popup);
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "popup_center_image");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	icon = elm_image_add(layout);
	snprintf(buf, sizeof(buf), "%s/00_popup_icon_video call.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	elm_image_resizable_set(icon, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(layout, "elm.swallow.content", icon);

	evas_object_show(layout);
	elm_object_content_set(popup, layout);
	evas_object_smart_callback_add(popup, "block,clicked", _response_cb, popup);
	evas_object_show(popup);
}

static void __popup_keypad_on(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object * popup = data;
	int rotation = elm_win_rotation_get(win);
	if (rotation == 90 || rotation == 270)
		elm_popup_orient_set(popup, ELM_POPUP_ORIENT_BOTTOM);
	else
		elm_popup_orient_set(popup, ELM_POPUP_ORIENT_CENTER);
}

static void __popup_keypad_off(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object * popup = data;
	elm_popup_orient_set(popup, ELM_POPUP_ORIENT_CENTER);
}

static void _entry_popup_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad;
	ad = (struct appdata *) data;
	evas_object_del(obj);
	evas_object_smart_callback_del(ad->conform, "virtualkeypad,state,on", __popup_keypad_on);
	evas_object_smart_callback_del(ad->conform, "virtualkeypad,state,off", __popup_keypad_off);
}

static void _popup_entry_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad;
	Evas_Object *popup;
	Evas_Object *layout;
	Evas_Object *entry;
	Evas_Object *btn1;

	ad = (struct appdata *) data;

	popup = elm_popup_add(ad->nf);
	elm_object_style_set(popup, "no_effect");
	elm_object_scroll_freeze_push(list);
	evas_object_event_callback_add
	     (popup, EVAS_CALLBACK_DEL, _popup_del_cb, NULL);
	ea_object_event_callback_add(popup, EA_CALLBACK_BACK, _entry_popup_back_cb, ad);

	layout = elm_layout_add(popup);
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "popup_entryview");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	entry = elm_entry_add(popup);
	ea_entry_selection_back_event_allow_set(entry, EINA_TRUE);
	elm_object_text_set(entry, "This is a scrolled entry hence has a default size");
	elm_object_part_content_set(layout, "elm.swallow.content", entry);

	elm_object_content_set(popup, layout);
	btn1 = elm_button_add(popup);
	elm_object_style_set(btn1, "popup");
	elm_object_text_set(btn1, "OK");
	elm_object_part_content_set(popup, "button1", btn1);
	evas_object_smart_callback_add(btn1, "clicked", _response_cb, popup);
	evas_object_smart_callback_add(ad->conform, "virtualkeypad,state,on", __popup_keypad_on, popup);
	evas_object_smart_callback_add(ad->conform, "virtualkeypad,state,off", __popup_keypad_off, popup);
	evas_object_show(popup);
}

static void _popup_entry_password_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad;
	Evas_Object *popup;
	Evas_Object *layout;
	Evas_Object *entry;
	Evas_Object *entry2;
	Evas_Object *entry3;
	Evas_Object *entry4;
	static Elm_Entry_Filter_Limit_Size limit_filter_data;

	ad = (struct appdata *) data;
	popup = elm_popup_add(ad->nf);
	elm_object_style_set(popup, "no_effect");
	elm_object_scroll_freeze_push(list);
	evas_object_event_callback_add
	     (popup, EVAS_CALLBACK_DEL, _popup_del_cb, NULL);
	ea_object_event_callback_add(popup, EA_CALLBACK_BACK, ea_popup_back_cb, NULL);

	elm_object_part_text_set(popup, "title,text", "Title");

	layout = elm_layout_add(popup);
	elm_layout_theme_set(layout, "layout", "popup", "entrypasswordview");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	entry = elm_entry_add(layout);
	ea_entry_selection_back_event_allow_set(entry, EINA_TRUE);
	elm_entry_password_set(entry, EINA_TRUE);
	limit_filter_data.max_byte_count = 0;
	limit_filter_data.max_char_count = 1;
	elm_entry_markup_filter_append(entry, elm_entry_filter_limit_size, &limit_filter_data);
	elm_object_style_set(entry, "popup");
	elm_object_part_content_set(layout, "elm.entry1", entry);

	entry2 = elm_entry_add(layout);
	ea_entry_selection_back_event_allow_set(entry2, EINA_TRUE);
	elm_entry_password_set(entry2, EINA_TRUE);
	elm_entry_markup_filter_append(entry2, elm_entry_filter_limit_size, &limit_filter_data);
	elm_object_style_set(entry2, "popup");
	elm_object_part_content_set(layout, "elm.entry2", entry2);

	entry3 = elm_entry_add(layout);
	ea_entry_selection_back_event_allow_set(entry3, EINA_TRUE);
	elm_entry_password_set(entry3, EINA_TRUE);
	elm_entry_markup_filter_append(entry3, elm_entry_filter_limit_size,
		&limit_filter_data);
	elm_object_style_set(entry3, "popup");
	elm_object_part_content_set(layout, "elm.entry3", entry3);

	entry4 = elm_entry_add(layout);
	ea_entry_selection_back_event_allow_set(entry4, EINA_TRUE);
	elm_entry_password_set(entry4, EINA_TRUE);
	elm_entry_markup_filter_append(entry4, elm_entry_filter_limit_size,
		&limit_filter_data);
	elm_object_style_set(entry4, "popup");
	elm_object_part_content_set(layout, "elm.entry4", entry4);

	elm_object_content_set(popup, layout);
	evas_object_smart_callback_add(popup, "block,clicked", _block_clicked_cb,
		NULL);
	evas_object_smart_callback_add(entry, "changed", _test, entry2);
	evas_object_smart_callback_add(entry2, "changed", _test, entry3);
	evas_object_smart_callback_add(entry3, "changed", _test, entry4);
	evas_object_smart_callback_add(entry4, "changed", _test, NULL);
	evas_object_show(popup);
}

static void _popup_entry_password_2button_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup;
	Evas_Object *layout;
	Evas_Object *entry;
	Evas_Object *check;
	Evas_Object *button;
	struct appdata *ad;

	ad = (struct appdata *) data;
	popup = elm_popup_add(ad->nf);
	elm_object_style_set(popup, "no_effect");

	elm_object_scroll_freeze_push(list);
	evas_object_event_callback_add
	     (popup, EVAS_CALLBACK_DEL, _popup_del_cb, NULL);
	ea_object_event_callback_add(popup, EA_CALLBACK_BACK, ea_popup_back_cb, NULL);

	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	layout = elm_layout_add(popup);
	elm_layout_theme_set(layout, "layout", "popup", "entrypasswordview_2button");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_part_text_set(popup, "title,text", "Enter password");

	entry = elm_entry_add(layout);
	ea_entry_selection_back_event_allow_set(entry, EINA_TRUE);
	elm_object_style_set(entry, "editfield/password/popup");
	elm_entry_password_set(entry, EINA_TRUE);
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_entry_prediction_allow_set(entry, EINA_FALSE);
	elm_object_signal_emit(entry, "elm,action,hide,search_icon", "");
	elm_object_part_text_set(entry, "elm.guide", "editfield/password/popup");
	evas_object_smart_callback_add(entry, "changed", _password_changed_cb, NULL);
	evas_object_smart_callback_add(entry, "preedit,changed", _password_changed_cb, NULL);
	evas_object_smart_callback_add(entry, "focused", _password_focused_cb, NULL);
	evas_object_smart_callback_add(entry, "unfocused", _password_unfocused_cb, NULL);
	elm_object_part_content_set(layout, "elm.swallow.content", entry);

	button = elm_button_add(layout);
	elm_object_style_set(button, "search_clear");
	elm_object_focus_allow_set(button, EINA_FALSE);
	elm_object_part_content_set(entry, "elm.swallow.clear", button);
	evas_object_smart_callback_add(button, "clicked", _eraser_btn_clicked_cb, entry);

	check = elm_check_add(popup);
	elm_object_text_set(check, "Show Password");
	elm_object_focus_allow_set(check, EINA_FALSE);
	evas_object_smart_callback_add(check, "changed", _password_check_cb, layout);
	elm_object_part_content_set(layout, "elm.swallow.end", check);

	elm_object_content_set(popup, layout);

	button = elm_button_add(popup);
	elm_object_style_set(button, "popup");
	elm_object_text_set(button, "OK");
	elm_object_part_content_set(popup, "button1", button);
	evas_object_smart_callback_add(button, "clicked", _response_cb, popup);

	button = elm_button_add(popup);
	elm_object_style_set(button, "popup");
	elm_object_text_set(button, "Cancel");
	elm_object_part_content_set(popup, "button2", button);
	evas_object_smart_callback_add(button, "clicked", _response_cb, popup);

	evas_object_show(popup);
}

static void _volume_ver_change_cb(void *data, Evas_Object *obj, void *event_info)
{
	double value;
	Evas_Object *icon;
	char buf[4096];
	static double old_value = 10.0;

	icon = elm_object_part_content_get(obj, "icon");
	value = elm_slider_value_get(obj);

	if (value < 1.0 && old_value >= 1.0) {
		snprintf(buf, sizeof(buf), "%s/00_volume_icon_mute.png", ICON_DIR);
		elm_image_file_set(icon, buf, NULL);
	}
	else if (value >= 1.0 && old_value < 1.0){
		snprintf(buf, sizeof(buf), "%s/00_volume_icon.png", ICON_DIR);
		elm_image_file_set(icon, buf, NULL);
	}
	old_value = value;
	if (del_timer) {
		ecore_timer_del(del_timer);
		del_timer = NULL;
	}
	del_timer = ecore_timer_add(3.0, _timer_cb, data);
}

static void _popup_volumepopup1_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup;
	Evas_Object *vbar;
	Evas_Object *icon;
	Evas_Object *layout;
	Evas_Object *setting_icon;
	Evas_Object *ao;
	Eina_List *l = NULL;
	char buf[4096];
	char buf1[4096];

	popup = elm_popup_add(list);
	elm_object_scroll_freeze_push(list);
	evas_object_event_callback_add
	     (popup, EVAS_CALLBACK_DEL, _popup_del_cb, NULL);
	ea_object_event_callback_add(popup, EA_CALLBACK_BACK, ea_popup_back_cb, NULL);

	elm_popup_orient_set(popup, ELM_POPUP_ORIENT_TOP);

	evas_object_smart_callback_add(popup, "block,clicked", _block_clicked_cb, NULL);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	layout = elm_layout_add(popup);
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "popup_volumebar");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	vbar = elm_slider_add(popup);
	elm_slider_min_max_set(vbar, 0.0, 15.0);
	elm_slider_value_set(vbar, 10.0);
	icon = elm_image_add(popup);
	snprintf(buf, sizeof(buf), "%s/00_volume_icon.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_FALSE, EINA_FALSE);
	elm_object_part_content_set(vbar, "icon", icon);

	setting_icon = elm_image_add(layout);
	snprintf(buf1, sizeof(buf1), "%s/00_volume_icon_settings.png", ICON_DIR);
	elm_image_file_set(setting_icon, buf1, NULL);
	elm_image_resizable_set(setting_icon, EINA_TRUE, EINA_TRUE);

	elm_object_part_content_set(layout, "elm.swallow.content", vbar);
	elm_object_part_content_set(layout, "elm.swallow.icon", setting_icon);

	ao = elm_access_object_register(setting_icon, layout);
	elm_access_info_set(ao, ELM_ACCESS_INFO, "sound setting");

	l = eina_list_append(l, vbar);
	l = eina_list_append(l, ao);

	elm_object_focus_custom_chain_set(layout, l);

	elm_object_content_set(popup, layout);
	evas_object_show(popup);
}

static void _popup_volumepopup1_text_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup;
	Evas_Object *vbar;
	Evas_Object *setting_icon;
	Evas_Object *icon;
	Evas_Object *layout;
	Evas_Object *layout1;
	char buf[4096];
	char buf1[4096];

	popup = elm_popup_add(list);
	elm_object_scroll_freeze_push(list);
	evas_object_event_callback_add
	     (popup, EVAS_CALLBACK_DEL, _popup_del_cb, NULL);
	ea_object_event_callback_add(popup, EA_CALLBACK_BACK, ea_popup_back_cb, NULL);

	elm_popup_orient_set(popup, ELM_POPUP_ORIENT_TOP);

	evas_object_smart_callback_add(popup, "block,clicked", _block_clicked_cb, NULL);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	layout = elm_layout_add(popup);
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "popup_volumebar_text");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	layout1 = elm_layout_add(popup);
	elm_layout_file_set(layout1, ELM_DEMO_EDJ, "popup_volumebar");
	evas_object_size_hint_weight_set(layout1, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	vbar = elm_slider_add(layout1);
	elm_slider_min_max_set(vbar, 0.0, 15.0);
	elm_slider_value_set(vbar, 10.0);
	icon = elm_image_add(layout1);
	snprintf(buf, sizeof(buf), "%s/00_volume_icon.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_FALSE, EINA_FALSE);
	elm_object_part_content_set(vbar, "icon", icon);

	setting_icon = elm_image_add(layout1);
	snprintf(buf1, sizeof(buf1), "%s/00_volume_icon_settings.png", ICON_DIR);
	elm_image_file_set(setting_icon, buf1, NULL);
	elm_image_resizable_set(setting_icon, EINA_TRUE, EINA_TRUE);

	elm_object_part_content_set(layout1, "elm.swallow.content", vbar);
	elm_object_part_content_set(layout1, "elm.swallow.icon", setting_icon);
	elm_object_part_content_set(layout, "elm.swallow.content1", layout1);
	elm_object_part_text_set(layout, "elm.swallow.text", "High volumes may harm your hearing if you listen for a long time");

	evas_object_show(layout1);
	evas_object_show(layout);
	elm_object_content_set(popup, layout);
	evas_object_smart_callback_add(vbar, "changed", _volume_ver_change_cb,popup);
	del_timer = ecore_timer_add(3.0, _timer_cb, popup);
	evas_object_show(popup);
}

static void _popup_volumepopup_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	evas_object_del(obj);
	if (del_timer) {
		ecore_timer_del(del_timer);
		del_timer = NULL;
	}
}

static void _popup_volumepopup_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup;
	Evas_Object *vbar;
	Evas_Object *icon;
	Evas_Object *layout;
	char buf[4096];

	popup = elm_popup_add(list);
	elm_object_scroll_freeze_push(list);
	evas_object_event_callback_add
	     (popup, EVAS_CALLBACK_DEL, _popup_del_cb, NULL);
	ea_object_event_callback_add(popup, EA_CALLBACK_BACK, _popup_volumepopup_back_cb, NULL);

	elm_popup_orient_set(popup, ELM_POPUP_ORIENT_TOP);

	evas_object_smart_callback_add(popup, "block,clicked", _block_clicked_cb, NULL);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	layout = elm_layout_add(popup);
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "popup_volumestyle");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	vbar = elm_slider_add(popup);
	elm_slider_min_max_set(vbar, 0.0, 15.0);
	elm_slider_value_set(vbar, 10.0);
	icon = elm_image_add(popup);
	snprintf(buf, sizeof(buf), "%s/00_volume_icon.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, EINA_FALSE, EINA_FALSE);
	elm_object_part_content_set(vbar, "icon", icon);

	elm_object_part_content_set(layout, "elm.swallow.content", vbar);
	elm_object_content_set(popup, layout);
	evas_object_smart_callback_add(vbar, "changed", _volume_ver_change_cb,popup);
	del_timer = ecore_timer_add(3.0, _timer_cb, popup);
	evas_object_show(popup);
}

static void _popup_center_title_3button_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup;
	Evas_Object *btn1;
	Evas_Object *btn2;
	Evas_Object *btn3;
	struct appdata *ad = data;

	popup = elm_popup_add(ad->win_main);
	elm_object_scroll_freeze_push(list);
	evas_object_event_callback_add
	     (popup, EVAS_CALLBACK_DEL, _popup_del_cb, NULL);
	ea_object_event_callback_add(popup, EA_CALLBACK_BACK, ea_popup_back_cb, NULL);

	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_text_set(popup,"This Popup has title area, content area and "
			"action area set, action area has three buttons OK, Cancel and "
			"Close.");
	elm_object_part_text_set(popup, "title,text", "Title");
	btn1 = elm_button_add(popup);
	elm_object_style_set(btn1, "popup");
	elm_object_text_set(btn1, "OK");
	elm_object_part_content_set(popup, "button1", btn1);
	evas_object_smart_callback_add(btn1, "clicked", _response_cb, popup);
	btn2 = elm_button_add(popup);
	elm_object_style_set(btn2, "popup");
	elm_object_text_set(btn2, "Cancel");
	elm_object_part_content_set(popup, "button2", btn2);
	evas_object_smart_callback_add(btn2, "clicked", _response_cb, popup);
	btn3 = elm_button_add(popup);
	elm_object_style_set(btn3, "popup");
	elm_object_text_set(btn3, "Close");
	elm_object_part_content_set(popup, "button3", btn3);
	evas_object_smart_callback_add(btn3, "clicked", _response_cb, popup);
	evas_object_show(popup);
}

static void _gl_sel(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup = data;
	int index = 0;
	Elm_Object_Item *item = (Elm_Object_Item *)event_info;
	if (item) {
		index = (int)elm_object_item_data_get(item);
		fprintf(stdout, "selected text %s\n",Items[index]);
	}
	_list_item_click(popup, index);
}

static void _gl_radio_sel(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *item = (Elm_Object_Item *) event_info;

	if (item) {
		Item_Data *id = elm_object_item_data_get(item);
		int index = id->index;
		Evas_Object *radio = elm_object_item_part_content_get(item, "elm.icon");
		state_index = index;
		if (elm_radio_value_get(radio) == index) {
			elm_genlist_item_selected_set(item, EINA_FALSE);
		}
		elm_radio_value_set(radio, state_index);
	}
}

static void
_radio_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	evas_object_del(gen_popup);
}

static Evas_Object *
_gl_radio_content_get(void *data, Evas_Object *obj, const char *part)
{

	Item_Data *id = data;
	int index = id->index;
	Evas_Object *radio;
	Evas_Object *radio_main = evas_object_data_get(obj, "radio_main");

	if (!strcmp(part, "elm.icon")) {
		radio = elm_radio_add(obj);
		elm_radio_group_add(radio, radio_main);
		elm_radio_state_value_set(radio, index);
		if (index == state_index) elm_radio_value_set(radio, state_index);
		evas_object_size_hint_weight_set(radio, EVAS_HINT_EXPAND,EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(radio, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_propagate_events_set(radio, EINA_FALSE);
		elm_object_signal_callback_add(radio, "elm,action,show,finished", "elm",
				_radio_cb, NULL);
		return radio;
	}
	return NULL;
}

static char *_gl_radio_text_get(void *data, Evas_Object *obj, const char *part)
{
	Item_Data *id = (Item_Data *) data;

	if (!strcmp(part, "elm.text.sub"))
		return strdup(Sub_Items[id->index]);
	else
		return strdup(Items[id->index]);
}

static char *_gl_text_get(void *data, Evas_Object *obj, const char *part)
{
	int index = (int) data;

	if (!strcmp(part, "elm.text.sub"))
		return strdup(Sub_Items[index]);
	else
		return strdup(Items[index]);
}

static Evas_Object *_gl_content_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *icon;
	if (!strcmp(part, "elm.icon")) {
		icon = elm_image_add(obj);
		elm_image_file_set(icon, ICON_DIR"/genlist/00_brightness_right.png", NULL);
		evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
		return icon;
	}
	else {
		return NULL;
	}
}

static void _gl_realized(void *data, Evas_Object *obj, void *event_info)
{
	if (!event_info) return;
	int index = (int)elm_object_item_data_get(event_info);

	if (index == 7) {
		elm_object_item_signal_emit(event_info, "elm,state,bottomline,hide", "");
	}
}

static void _track_resize(void *data, Evas *e, Evas_Object *obj, void *ei)
{
	char buf[256];
	int idx;
	Evas_Object *genlist;
	Evas_Coord gh, h[4];
	genlist = elm_object_item_widget_get(data);
	idx = elm_genlist_item_index_get(data);
	if (idx < 0) return;

	evas_object_geometry_get(obj, NULL, NULL, NULL, &h[idx]);
	snprintf(buf, 255, "item_h%d", idx);
	evas_object_data_set(genlist, buf, (void *)h[idx]);

	h[0] = (int)evas_object_data_get(genlist, "item_h0");
	h[1] = (int)evas_object_data_get(genlist, "item_h1");
	h[2] = (int)evas_object_data_get(genlist, "item_h2");
	h[3] = (int)evas_object_data_get(genlist, "item_h3");

	gh = h[0] + h[1] + h[2] + h[3];
	evas_object_size_hint_min_set(genlist, 0, gh);
	printf("track resize: %d: h:%d = %d + %d + %d + %d\n", idx, gh,
			h[0], h[1], h[2], h[3]);
}

static void _gl_realized_track(void *data, Evas_Object *obj, void *ei)
{
	if (!ei) return;
	int index = (int)elm_object_item_data_get(ei);

	// track usage
	Evas_Object *track = elm_object_item_track(ei);
	evas_object_event_callback_add(track, EVAS_CALLBACK_RESIZE, _track_resize, ei);

	if (index == 3) {
		elm_object_item_signal_emit(ei, "elm,state,bottomline,hide", "");
	}
}

static void _popup_center_liststyle_1button_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup;
	int index;
	Evas_Object *genlist;
	Evas_Object *btn1;
	Evas_Object *box;
	struct appdata *ad = data;

	popup = elm_popup_add(ad->win_main);
	elm_object_scroll_freeze_push(list);
	evas_object_event_callback_add
	     (popup, EVAS_CALLBACK_DEL, _popup_del_cb, NULL);
	ea_object_event_callback_add(popup, EA_CALLBACK_BACK, ea_popup_back_cb, NULL);

	elm_object_part_text_set(popup, "title,text", "Title");
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	btn1 = elm_button_add(popup);
	elm_object_style_set(btn1, "popup");
	elm_object_text_set(btn1, "OK");
	elm_object_part_content_set(popup, "button1", btn1);
	evas_object_smart_callback_add(btn1, "clicked", _response_cb, popup);

	itc.item_style = "1text/popup";
	itc.func.text_get = _gl_text_get;
	itc.func.content_get = NULL;
	itc.func.state_get = NULL;
	itc.func.del = NULL;
	box = elm_box_add(popup);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	genlist = elm_genlist_add(box);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND,
			EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_smart_callback_add(genlist, "realized", _gl_realized, NULL);
	for (index = 0; index < 8; index++) {
		elm_genlist_item_append(genlist, &itc, (void *) index, NULL, ELM_GENLIST_ITEM_NONE, _gl_sel, popup);
	}
	elm_box_pack_end(box, genlist);
	evas_object_show(genlist);
	/* The height of popup being adjusted by application here based on app requirement */
	evas_object_size_hint_min_set(box, 618, 436);
	evas_object_show(box);

	elm_object_content_set(popup, box);
	evas_object_show(popup);
}

static void _popup_center_liststyle_3button_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup;
	int index;
	Evas_Object *lst;
	Evas_Object *layout;
	Evas_Object *btn1;
	Evas_Object *btn2;
	Evas_Object *btn3;
	struct appdata *ad = data;

	popup = elm_popup_add(ad->win_main);
	elm_object_scroll_freeze_push(list);
	evas_object_event_callback_add
	     (popup, EVAS_CALLBACK_DEL, _popup_del_cb, NULL);
	ea_object_event_callback_add(popup, EA_CALLBACK_BACK, ea_popup_back_cb, NULL);

	elm_object_part_text_set(popup, "title,text", "Title");
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	layout = elm_layout_add(popup);
	elm_layout_theme_set(layout, "layout", "content", "menustyle");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	btn1 = elm_button_add(popup);
	elm_object_style_set(btn1, "popup");
	elm_object_text_set(btn1, "OK");
	elm_object_part_content_set(popup, "button1", btn1);
	evas_object_smart_callback_add(btn1, "clicked", _response_cb, popup);
	btn2 = elm_button_add(popup);
	elm_object_style_set(btn2, "popup");
	elm_object_text_set(btn2, "Cancel");
	elm_object_part_content_set(popup, "button2", btn2);
	evas_object_smart_callback_add(btn2, "clicked", _response_cb, popup);
	btn3 = elm_button_add(popup);
	elm_object_style_set(btn3, "popup");
	elm_object_text_set(btn3, "Close");
	elm_object_part_content_set(popup, "button3", btn3);
	evas_object_smart_callback_add(btn3, "clicked", _response_cb, popup);

	lst = elm_list_add(layout);
	elm_object_style_set(lst, "popup");
	evas_object_size_hint_weight_set(lst, EVAS_HINT_EXPAND,
			EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(lst, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_smart_callback_add(lst, "realized", _gl_realized, NULL);
	for (index = 0; index < 8; index++) {
		elm_list_item_append(lst, Items[index], NULL, NULL, _gl_sel, data);
	}
	elm_object_part_content_set(layout, "elm.swallow.content" , lst);
	elm_object_content_set(popup, layout);
	evas_object_show(popup);
}

static void _popup_1linetext_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup;
	int index;
	Evas_Object *genlist;
	Evas_Object *layout;
	struct appdata *ad = data;

	popup = elm_popup_add(ad->win_main);
	elm_object_scroll_freeze_push(list);
	evas_object_event_callback_add
	     (popup, EVAS_CALLBACK_DEL, _popup_del_cb, NULL);
	ea_object_event_callback_add(popup, EA_CALLBACK_BACK, ea_popup_back_cb, NULL);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	layout = elm_layout_add(popup);
	elm_layout_theme_set(layout, "layout", "content", "liststyle");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	itc.item_style = "1text/popup";
	itc.func.text_get = _gl_text_get;
	itc.func.content_get = NULL;
	itc.func.state_get = NULL;
	itc.func.del = NULL;
	genlist = elm_genlist_add(layout);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_smart_callback_add(genlist, "realized", _gl_realized_track, NULL);
	for (index = 0; index < 4; index++) {
		elm_genlist_item_append(genlist, &itc, (void *) index, NULL, ELM_GENLIST_ITEM_NONE, _gl_sel, popup);
	}

	elm_object_part_content_set(layout, "elm.swallow.content" , genlist);
	elm_object_content_set(popup, layout);

	Evas_Object *base = (Evas_Object*)edje_object_part_object_get(_EDJ(popup), "access.base");
	elm_access_object_unregister(base);

	evas_object_show(popup);
}

static void _popup_2linetext_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup;
	int index;
	Evas_Object *genlist;
	Evas_Object *layout;
	struct appdata *ad = data;

	popup = elm_popup_add(ad->win_main);
	elm_object_scroll_freeze_push(list);
	evas_object_event_callback_add
	     (popup, EVAS_CALLBACK_DEL, _popup_del_cb, NULL);
	ea_object_event_callback_add(popup, EA_CALLBACK_BACK, ea_popup_back_cb, NULL);

	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	layout = elm_layout_add(popup);
	elm_layout_theme_set(layout, "layout", "content", "liststyle");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	itc.item_style = "2text.2/popup";
	itc.func.text_get = _gl_text_get;
	itc.func.content_get = NULL;
	itc.func.state_get = NULL;
	itc.func.del = NULL;
	genlist = elm_genlist_add(layout);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
	for (index = 0; index < 4; index++) {
		elm_genlist_item_append(genlist, &itc, (void *) index, NULL, ELM_GENLIST_ITEM_NONE, _gl_sel, popup);
	}
	evas_object_smart_callback_add(genlist, "realized", _gl_realized_track, NULL);

	elm_object_part_content_set(layout, "elm.swallow.content" , genlist);
	elm_object_content_set(popup, layout);
	evas_object_show(popup);
}

static void _popup_radio_cb(void *data, Evas_Object *obj, void *event_info)
{
	int index;
	Evas_Object *genlist, *radio_main;
	Evas_Object *layout;
	struct appdata *ad = data;
	gen_popup = elm_popup_add(ad->win_main);
	elm_object_scroll_freeze_push(list);
	evas_object_event_callback_add
	     (gen_popup, EVAS_CALLBACK_DEL, _popup_del_cb, NULL);
	ea_object_event_callback_add(gen_popup, EA_CALLBACK_BACK, ea_popup_back_cb, NULL);
	evas_object_size_hint_weight_set(gen_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	layout = elm_layout_add(gen_popup);
	elm_layout_theme_set(layout, "layout", "content", "liststyle");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	genlist = elm_genlist_add(layout);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_smart_callback_add(genlist, "realized", _gl_realized_track, NULL);

	radio_main = elm_radio_add(genlist);
	elm_radio_state_value_set(radio_main, 0);
	elm_radio_value_set(radio_main, 0);
	evas_object_data_set(genlist, "radio_main", radio_main);

	itc.item_style = "1text.1icon.3/popup";
	itc.func.text_get = _gl_radio_text_get;
	itc.func.content_get = _gl_radio_content_get;
	itc.func.state_get = NULL;
	itc.func.del = NULL;

	for (index = 0; index < 4; index++) {
		Item_Data *id = calloc(sizeof(Item_Data), 1);
		id->index  = index;
		Elm_Object_Item *item = elm_genlist_item_append(genlist, &itc, id, NULL,
				ELM_GENLIST_ITEM_NONE, _gl_radio_sel, NULL);
		id->item = item;
		id->genlist = genlist;
	}

	elm_object_part_content_set(layout, "elm.swallow.content" , genlist);
	elm_object_content_set(gen_popup, layout);
	evas_object_show(gen_popup);
}

static void _item_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	printf(" \n popup item selected: %s\n", elm_object_item_text_get(event_info));
}

static void _popup_itemlist_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup;
	unsigned int index;
	Elm_Object_Item *item;
	char buf[4096];
	struct appdata *ad = data;

	popup = elm_popup_add(ad->win_main);
	elm_object_scroll_freeze_push(list);
	evas_object_event_callback_add
	     (popup, EVAS_CALLBACK_DEL, _popup_del_cb, NULL);
	ea_object_event_callback_add(popup, EA_CALLBACK_BACK, ea_popup_back_cb, NULL);

	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	for (index = 0; index < 7; index++) {
		snprintf(buf, sizeof(buf), "Item%u", index+1);
		item = elm_popup_item_append(popup, buf, NULL, _item_selected_cb, NULL);
		if (index == 1)
			elm_object_item_disabled_set(item ,EINA_TRUE);
		if (index == 6)
			elm_object_item_signal_emit(item, "elm,state,bottomline,hide", "");
	}
	evas_object_smart_callback_add(popup, "block,clicked", _block_clicked_cb, NULL);
	evas_object_show(popup);
}

static void _popup_center_liststyle_2button_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup;
	int index;
	Evas_Object *genlist;
	Evas_Object *layout;
	Evas_Object *btn1;
	Evas_Object *btn2;
	struct appdata *ad = data;

	popup = elm_popup_add(ad->win_main);
	elm_object_scroll_freeze_push(list);
	evas_object_event_callback_add
	     (popup, EVAS_CALLBACK_DEL, _popup_del_cb, NULL);
	ea_object_event_callback_add(popup, EA_CALLBACK_BACK, ea_popup_back_cb, NULL);

	elm_object_part_text_set(popup, "title,text", "Title");
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	layout = elm_layout_add(popup);
	elm_layout_theme_set(layout, "layout", "content", "menustyle");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	btn1 = elm_button_add(popup);
	elm_object_style_set(btn1, "popup");
	elm_object_text_set(btn1, "OK");
	elm_object_part_content_set(popup, "button1", btn1);
	evas_object_smart_callback_add(btn1, "clicked", _response_cb, popup);
	btn2 = elm_button_add(popup);
	elm_object_style_set(btn2, "popup");
	elm_object_text_set(btn2, "Cancel");
	elm_object_part_content_set(popup, "button2", btn2);
	evas_object_smart_callback_add(btn2, "clicked", _response_cb, popup);

	itc.item_style = "1text.1icon.2/popup";
	itc.func.text_get = _gl_text_get;
	itc.func.content_get = _gl_content_get;
	itc.func.state_get = NULL;
	itc.func.del = NULL;
	genlist = elm_genlist_add(layout);
	evas_object_smart_callback_add(genlist, "selected", _genlist_click, genlist);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_smart_callback_add(genlist, "realized", _gl_realized, NULL);
	for (index = 0; index < 8; index++) {
		elm_genlist_item_append(genlist, &itc, (void *) index, NULL, ELM_GENLIST_ITEM_NONE, _gl_sel, popup);
	}

	elm_object_part_content_set(layout, "elm.swallow.content" , genlist);
	elm_object_content_set(popup, layout);
	evas_object_show(popup);
}

static void _popup_center_buttonstyle_3button_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup;
	Evas_Object *btn;
	Evas_Object *ic;
	Evas_Object *btn1;
	Evas_Object *layout;
	char buf[4096];
	struct appdata *ad = data;

	popup = elm_popup_add(ad->win_main);
	elm_object_scroll_freeze_push(list);
	evas_object_event_callback_add
	     (popup, EVAS_CALLBACK_DEL, _popup_del_cb, NULL);
	ea_object_event_callback_add(popup, EA_CALLBACK_BACK, ea_popup_back_cb, NULL);

	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	btn1 = elm_button_add(popup);
	elm_object_style_set(btn1, "popup");
	elm_object_text_set(btn1, "OK");
	elm_object_part_content_set(popup, "button1", btn1);
	evas_object_smart_callback_add(btn1, "clicked", _response_cb, popup);

	layout = elm_layout_add(popup);
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "nbpopup_3button_view");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	elm_object_part_text_set(layout, "contact_label", "Text Text Text Text Text");

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_default", btn);
	ic = elm_image_add(layout);
	snprintf(buf, sizeof(buf), "%s/00_popup_icon_call.png", ICON_DIR);
	elm_image_file_set(ic, buf, NULL);
	evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(ic, EINA_TRUE, EINA_TRUE);
	elm_object_content_set(btn, ic);
	elm_object_text_set(btn, _("Call"));

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_center", btn);
	ic = elm_image_add(layout);
	snprintf(buf, sizeof(buf), "%s/00_popup_icon_message.png", ICON_DIR);
	elm_image_file_set(ic, buf, NULL);
	evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(ic, EINA_TRUE, EINA_TRUE);
	elm_object_content_set(btn, ic);
	elm_object_text_set(btn, _("Message"));

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_text_only_style1", btn);
	ic = elm_image_add(layout);
	snprintf(buf, sizeof(buf), "%s/00_popup_icon_video call.png", ICON_DIR);
	elm_image_file_set(ic, buf, NULL);
	evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(ic, EINA_TRUE, EINA_TRUE);
	elm_object_content_set(btn, ic);
	elm_object_text_set(btn, _("Video call"));

	elm_object_content_set(popup, layout);
	evas_object_show(popup);
}

static void _popup_center_buttonstyle_2button_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup;
	Evas_Object *layout;
	Evas_Object *btn1;
	struct appdata *ad = data;

	popup = elm_popup_add(ad->win_main);
	elm_object_scroll_freeze_push(list);
	evas_object_event_callback_add
	     (popup, EVAS_CALLBACK_DEL, _popup_del_cb, NULL);
	ea_object_event_callback_add(popup, EA_CALLBACK_BACK, ea_popup_back_cb, NULL);

	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	layout = elm_layout_add(popup);
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "nbpopup_2button_view");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	elm_object_part_text_set(layout, "contact_label", "Text Text Text Text Text");

	btn1 = elm_button_add(popup);
	elm_object_style_set(btn1, "popup");
	elm_object_text_set(btn1, "OK");
	elm_object_part_content_set(popup, "button1", btn1);
	evas_object_smart_callback_add(btn1, "clicked", _response_cb, popup);

	Evas_Object *btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_default", btn);
	elm_object_text_set(btn, _("Call"));

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_center", btn);
	elm_object_text_set(btn, _("Message"));

	elm_object_content_set(popup, layout);
	evas_object_show(popup);
}

static void _popup_colorpalette_cb(void *data, Evas_Object *obj, void *event_info)
{
	int r = 0, g = 0, b = 0 ,a = 0;
	Colorplane_Data *cp = (Colorplane_Data *)data;
	Elm_Object_Item *color_it = (Elm_Object_Item *) event_info;
	cp->sel_it = color_it;
	elm_colorselector_palette_item_color_get(color_it, &r, &g, &b, &a);
	elm_colorselector_color_set(cp->colorselector, r, g, b, a);
}


static void _popup_colorplane_cb(void *data, Evas_Object *obj, void *event_info)
{
	int r, g, b, a;
	Colorplane_Data *cp = (Colorplane_Data*)data;
	if (!cp->changed) {
		elm_object_item_signal_emit(cp->it_last, "elm,state,custom,hide", "");
		cp->changed = EINA_TRUE;
	}
	elm_colorselector_color_get(cp->colorselector, &r, &g, &b, &a);
	cp->r = r;
	cp->g = g;
	cp->b = b;
	cp->a = a;
	elm_colorselector_palette_item_color_set(cp->it_last, r, g, b, a);
	if (cp->sel_it != cp->it_last)
		elm_object_item_signal_emit(cp->it_last, "elm,state,selected", "elm");
}

static void _popup_colorpalette(Evas_Object *layout, Colorplane_Data *cp)
{
	/* add color palette widget */
	Eina_List *color_list, *last_list;

	cp->colorselector = elm_colorselector_add(layout);
	elm_object_style_set(cp->colorselector, "colorplane");
	elm_colorselector_mode_set(cp->colorselector, ELM_COLORSELECTOR_PALETTE_PLANE);
	evas_object_size_hint_fill_set(cp->colorselector, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(cp->colorselector, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_part_content_set(layout, "colorpalette", cp->colorselector);
	evas_object_smart_callback_add(cp->colorselector, "color,item,selected", _popup_colorpalette_cb, cp);

	color_list = elm_colorselector_palette_items_get(cp->colorselector);
	last_list = eina_list_last(color_list);
	cp->it_last = eina_list_data_get(last_list);
	elm_object_item_signal_emit(cp->it_last, "elm,state,custom,show", "");
	cp->changed = EINA_FALSE;
	evas_object_smart_callback_add(cp->colorselector, "changed", _popup_colorplane_cb, cp);
	cp->sel_it = eina_list_nth(color_list, 3);
	elm_object_item_signal_emit(cp->sel_it, "elm,state,selected", "elm");
}

static Evas_Object *_popup_create_colorplane(Evas_Object *parent, Colorplane_Data *cp)
{
	cp->layout = elm_layout_add(parent);
	elm_layout_file_set(cp->layout, ELM_DEMO_EDJ, "colorplane_popup");
	evas_object_size_hint_weight_set(cp->layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(cp->layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(cp->layout);
	_popup_colorpalette(cp->layout, cp);
	return cp->layout;
}

static void _popup_colorpicker_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *content,*popup,*btn1,*btn2;
	static Colorplane_Data cp;
	if (data == NULL) return;
	struct appdata *ad = data;
	cp.ad = data;

	popup = elm_popup_add(ad->nf);
	elm_object_scroll_freeze_push(list);
	evas_object_event_callback_add
	     (popup, EVAS_CALLBACK_DEL, _popup_del_cb, NULL);
	ea_object_event_callback_add(popup, EA_CALLBACK_BACK, ea_popup_back_cb, NULL);

	content = _popup_create_colorplane(popup, &cp);
	elm_object_content_set(popup, content);

	btn1 = elm_button_add(popup);
	elm_object_style_set(btn1, "popup");
	elm_object_text_set(btn1, "Ok");
	elm_object_part_content_set(popup, "button1", btn1);
	evas_object_smart_callback_add(btn1, "clicked", _response_cb, popup);

	btn2 = elm_button_add(popup);
	elm_object_style_set(btn2, "popup");
	elm_object_text_set(btn2, "Cancel");
	elm_object_part_content_set(popup, "button2", btn2);
	evas_object_smart_callback_add(btn2, "clicked", _response_cb, popup);

	evas_object_show(popup);
}

static void _back_rotate_gengrid_cb(void *data, Evas_Object *obj, void *event_info)
{
	grid_data *gd = NULL;
	if (data) gd = data;
	if (gd) free(gd);
	evas_object_smart_callback_del(win, "rotation,changed", _on_rotation_gengrid_1line_change);
	evas_object_smart_callback_del(win, "rotation,changed", _on_rotation_gengrid_2line_change);
	evas_object_smart_callback_del(win, "rotation,changed", _on_rotation_gengrid_3line_change);
	evas_object_del(obj);
}

static void _on_rotation_gengrid_1line_change(void *data, Evas_Object *obj, void *event_info )
{
	int rot = -1;
	Evas_Object *win = obj;
	Evas_Object *layout = data;

	rot = elm_win_rotation_get(win);
	if (rot == 90 || rot == 270)
		elm_layout_file_set(layout, ELM_DEMO_EDJ, "popup_gengrid_custom_rotate_1line");
	else
		elm_layout_file_set(layout, ELM_DEMO_EDJ, "popup_gengrid_custom_1line");
}

static void _popup_gengrid_1line_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup;
	Evas_Object *layout;
	Evas_Object *gengrid;
	int rot = -1;
	struct appdata *ad = data;

	popup = elm_popup_add(ad->win_main);
	elm_object_scroll_freeze_push(list);
	evas_object_event_callback_add
	     (popup, EVAS_CALLBACK_DEL, _popup_del_cb, NULL);
	ea_object_event_callback_add(popup, EA_CALLBACK_BACK, _back_rotate_gengrid_cb, NULL);

	elm_object_part_text_set(popup, "title,text", "Title");
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	layout = elm_layout_add(popup);
	rot = elm_win_rotation_get(ad->win_main);
	if (rot == 90 || rot == 270)
		elm_layout_file_set(layout, ELM_DEMO_EDJ, "popup_gengrid_custom_rotate_1line");
	else
		elm_layout_file_set(layout, ELM_DEMO_EDJ, "popup_gengrid_custom_1line");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	gengrid = _create_gengrid(layout, popup, 1, 3, NULL);

	elm_gengrid_item_show(elm_gengrid_first_item_get(gengrid), ELM_GENGRID_ITEM_SCROLLTO_TOP);
	elm_object_part_content_set(layout, "elm.swallow.content" , gengrid);
	elm_object_content_set(popup, layout);

	evas_object_smart_callback_add(ad->win_main, "rotation,changed", _on_rotation_gengrid_1line_change, layout);
	evas_object_smart_callback_add(popup, "block,clicked", _block_clicked_cb, NULL);
	evas_object_show(popup);
}

static void _popup_gengrid_1line_select_mode_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup;
	Evas_Object *layout;
	Evas_Object *gengrid;
	int rot = -1;
	struct appdata *ad = data;
	grid_data *gd;
    gd = calloc(1, sizeof(grid_data));

	popup = elm_popup_add(ad->win_main);
	elm_object_scroll_freeze_push(list);
	evas_object_event_callback_add
	     (popup, EVAS_CALLBACK_DEL, _popup_del_cb, NULL);
	ea_object_event_callback_add(popup, EA_CALLBACK_BACK, _back_rotate_gengrid_cb, gd);

	elm_object_part_text_set(popup, "title,text", "Title");
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	layout = elm_layout_add(popup);
	rot = elm_win_rotation_get(ad->win_main);
	if (rot == 90 || rot == 270)
		elm_layout_file_set(layout, ELM_DEMO_EDJ, "popup_gengrid_custom_rotate_1line");
	else
		elm_layout_file_set(layout, ELM_DEMO_EDJ, "popup_gengrid_custom_1line");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	gengrid = _create_gengrid(layout, popup, 1, 3, gd);

	elm_gengrid_item_show(elm_gengrid_first_item_get(gengrid), ELM_GENGRID_ITEM_SCROLLTO_TOP);
	elm_object_part_content_set(layout, "elm.swallow.content" , gengrid);
	elm_object_content_set(popup, layout);

	evas_object_smart_callback_add(ad->win_main, "rotation,changed", _on_rotation_gengrid_1line_change, layout);
	evas_object_smart_callback_add(popup, "block,clicked", _block_clicked_cb, NULL);
	evas_object_show(popup);
}
static void _on_rotation_gengrid_2line_change(void *data, Evas_Object *obj, void *event_info )
{
	int rot = -1;
	Evas_Object *win = obj;
	Evas_Object *layout = data;

	rot = elm_win_rotation_get(win);
	if (rot == 90 || rot == 270)
		elm_layout_file_set(layout, ELM_DEMO_EDJ, "popup_gengrid_custom_rotate_2line");
	else
		elm_layout_file_set(layout, ELM_DEMO_EDJ, "popup_gengrid_custom_2line");
}

static void _popup_gengrid_2line_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup;
	Evas_Object *layout;
	Evas_Object *gengrid;
	int rot = -1;
	struct appdata *ad = data;

	popup = elm_popup_add(ad->win_main);
	elm_object_scroll_freeze_push(list);
	evas_object_event_callback_add
	     (popup, EVAS_CALLBACK_DEL, _popup_del_cb, NULL);
	ea_object_event_callback_add(popup, EA_CALLBACK_BACK, _back_rotate_gengrid_cb, NULL);

	elm_object_part_text_set(popup, "title,text", "Title");
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	layout = elm_layout_add(popup);
	rot = elm_win_rotation_get(ad->win_main);
	if (rot == 90 || rot == 270)
		elm_layout_file_set(layout, ELM_DEMO_EDJ, "popup_gengrid_custom_rotate_2line");
	else
		elm_layout_file_set(layout, ELM_DEMO_EDJ, "popup_gengrid_custom_2line");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	gengrid = _create_gengrid(layout, popup, 2, 3, NULL);

	elm_gengrid_item_show(elm_gengrid_first_item_get(gengrid), ELM_GENGRID_ITEM_SCROLLTO_TOP);
	elm_object_part_content_set(layout, "elm.swallow.content" , gengrid);
	elm_object_content_set(popup, layout);

	evas_object_smart_callback_add(ad->win_main, "rotation,changed", _on_rotation_gengrid_2line_change, layout);
	evas_object_smart_callback_add(popup, "block,clicked", _block_clicked_cb, NULL);
	evas_object_show(popup);
}

static void _on_rotation_gengrid_3line_change(void *data, Evas_Object *obj, void *event_info )
{
	int rot = -1;
	Evas_Object *win = obj;
	Evas_Object *layout = data;

	rot = elm_win_rotation_get(win);
	if (rot == 90 || rot == 270)
		elm_layout_file_set(layout, ELM_DEMO_EDJ, "popup_gengrid_custom_rotate_3line");
	else
		elm_layout_file_set(layout, ELM_DEMO_EDJ, "popup_gengrid_custom_3line");
}

static void _popup_gengrid_3line_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup;
	Evas_Object *layout;
	Evas_Object *gengrid;
	int rot = -1;
	struct appdata *ad = data;

	popup = elm_popup_add(ad->win_main);
	elm_object_scroll_freeze_push(list);
	evas_object_event_callback_add
	     (popup, EVAS_CALLBACK_DEL, _popup_del_cb, NULL);
	ea_object_event_callback_add(popup, EA_CALLBACK_BACK, _back_rotate_gengrid_cb, NULL);

	elm_object_part_text_set(popup, "title,text", "Title");
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	layout = elm_layout_add(popup);
	rot = elm_win_rotation_get(ad->win_main);
	if (rot == 90 || rot == 270)
		elm_layout_file_set(layout, ELM_DEMO_EDJ, "popup_gengrid_custom_rotate_3line");
	else
		elm_layout_file_set(layout, ELM_DEMO_EDJ, "popup_gengrid_custom_3line");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	gengrid = _create_gengrid(layout, popup, 3, 3, NULL);

	elm_object_part_content_set(layout, "elm.swallow.content" , gengrid);
	elm_object_content_set(popup, layout);

	evas_object_smart_callback_add(ad->win_main, "rotation,changed", _on_rotation_gengrid_3line_change, layout);
	evas_object_smart_callback_add(popup, "block,clicked", _block_clicked_cb, NULL);
	evas_object_show(popup);
}

static void _popup_gengrid_3line_scroll_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup;
	Evas_Object *layout;
	Evas_Object *gengrid;
	int rot = -1;
	struct appdata *ad = data;


	popup = elm_popup_add(ad->win_main);
	elm_object_scroll_freeze_push(list);
	evas_object_event_callback_add
	     (popup, EVAS_CALLBACK_DEL, _popup_del_cb, NULL);
	ea_object_event_callback_add(popup, EA_CALLBACK_BACK, _back_rotate_gengrid_cb, NULL);

	elm_object_part_text_set(popup, "title,text", "Title");
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	layout = elm_layout_add(popup);
	rot = elm_win_rotation_get(ad->win_main);
	if (rot == 90 || rot == 270)
		elm_layout_file_set(layout, ELM_DEMO_EDJ, "popup_gengrid_custom_rotate_3line");
	else
		elm_layout_file_set(layout, ELM_DEMO_EDJ, "popup_gengrid_custom_3line");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	gengrid = _create_gengrid(layout, popup, 6, 3, NULL);

	elm_object_part_content_set(layout, "elm.swallow.content" , gengrid);
	elm_object_content_set(popup, layout);

	evas_object_smart_callback_add(ad->win_main, "rotation,changed", _on_rotation_gengrid_3line_change, layout);
	evas_object_smart_callback_add(popup, "block,clicked", _block_clicked_cb, NULL);
	evas_object_show(popup);
}

void popup_cb(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *it;
	struct appdata *ad = data;
	if (ad == NULL) return;

	elm_theme_extension_add(NULL, ELM_DEMO_EDJ);
	win = ad->win_main;
	list = elm_list_add(ad->nf);
	elm_list_mode_set(list, ELM_LIST_COMPRESS);
	evas_object_smart_callback_add(list, "selected", _list_click, NULL);
	it = elm_naviframe_item_push(ad->nf, _("Popup"), NULL, NULL, list, NULL);
	elm_naviframe_item_pop_cb_set(it, _pop_cb, NULL);
	elm_list_item_append(list, "popup- text", NULL, NULL,
			_popup_center_info_cb, ad);
	elm_list_item_append(list, "popup- text+1 button", NULL, NULL,
			_popup_center_basic_1button_cb, ad);
	elm_list_item_append(list, "popup- text+2 buttons", NULL, NULL,
			_popup_center_basic_2button_cb, ad);
	elm_list_item_append(list, "popup- text+3 buttons", NULL, NULL,
			_popup_center_basic_3button_cb, ad);
	elm_list_item_append(list, "popup- title+text", NULL, NULL,
			_popup_center_title_info_cb, ad);
	elm_list_item_append(list, "popup- title+text+1 button", NULL, NULL,
			_popup_center_title_1button_cb, ad);
	elm_list_item_append(list, "popup- title+text+2 buttons", NULL, NULL,
			_popup_center_title_2button_cb, ad);
	elm_list_item_append(list, "popup- title+text+3 buttons", NULL, NULL,
			_popup_center_title_3button_cb, ad);
	elm_list_item_append(list, "popup- text+2 vertical buttons", NULL, NULL,
			_popup_center_vertical_2button_cb, ad);
	elm_list_item_append(list, "popup- text+3 vertical buttons", NULL, NULL,
			_popup_center_vertical_3button_cb, ad);
	elm_list_item_append(list, "popup- text+check", NULL, NULL,
			_popup_center_check_cb, ad);
	elm_list_item_append(list, "popup- text+check+image", NULL, NULL,
			_popup_center_check_image_cb, ad);
	elm_list_item_append(list, "popup- text+image+text+check", NULL, NULL,
			_popup_center_check_text_image_cb, ad);
	elm_list_item_append(list, "popup- big-icon", NULL, NULL,
			_popup_center_image_cb, ad);
	elm_list_item_append(list, "popup-list -1btn", NULL, NULL,
			_popup_center_liststyle_1button_cb, ad);
	elm_list_item_append(list, "popup-list -2btns", NULL, NULL,
			_popup_center_liststyle_2button_cb, ad);
	elm_list_item_append(list, "popup-list -3btns", NULL, NULL,
			_popup_center_liststyle_3button_cb, ad);
	elm_list_item_append(list, "popup-buttonstyle-3btns", NULL, NULL,
			_popup_center_buttonstyle_3button_cb, ad);
	elm_list_item_append(list, "popup-buttonstyle-2btns", NULL, NULL,
			_popup_center_buttonstyle_2button_cb, ad);
	elm_list_item_append(list, "popup-volumebar", NULL, NULL,
			_popup_volumepopup_cb, ad);
	elm_list_item_append(list, "popup-volumebar1", NULL, NULL,
				_popup_volumepopup1_cb, ad);
	elm_list_item_append(list, "popup-volumebar1_text", NULL, NULL,
			_popup_volumepopup1_text_cb, ad);
	elm_list_item_append(list, "popup-center-progressbar", NULL, NULL,
			_popup_center_progressbar_cb, ad);
	elm_list_item_append(list, "popup-center-processing", NULL, NULL,
			_popup_center_processing_cb, ad);
	elm_list_item_append(list, "popup-center-processing_1button", NULL, NULL,
			_popup_center_processing_1button_cb, ad);
	elm_list_item_append(list, "popup-entry", NULL, NULL,
			_popup_entry_cb, ad);
	elm_list_item_append(list, "popup-entry-password", NULL, NULL,
			_popup_entry_password_cb, ad);
	elm_list_item_append(list, "popup-entry-password-2-button", NULL, NULL,
			_popup_entry_password_2button_cb, ad);
	elm_list_item_append(list, "popup-center_text-progressbar", NULL, NULL,
			_popup_center_text_progressbar_cb, ad);
	elm_list_item_append(list, "popup-1 line", NULL, NULL,
			_popup_1linetext_cb, ad);
	elm_list_item_append(list, "popup-2 line", NULL, NULL,
			_popup_2linetext_cb, ad);
	elm_list_item_append(list, "popup-radio", NULL, NULL,
			_popup_radio_cb, ad);
	elm_list_item_append(list, "popup-item list", NULL, NULL,
			_popup_itemlist_cb, ad);
	elm_list_item_append(list, "popup-color picker", NULL, NULL,
			_popup_colorpicker_cb, ad);
	elm_list_item_append(list, "popup-item 1line grid", NULL, NULL,
			_popup_gengrid_1line_cb, ad);
	elm_list_item_append(list, "popup-item 2line grid", NULL, NULL,
			_popup_gengrid_2line_cb, ad);
	elm_list_item_append(list, "popup-item 3line grid", NULL, NULL,
			_popup_gengrid_3line_cb, ad);
	elm_list_item_append(list, "popup-item 3line scroll grid", NULL, NULL,
			_popup_gengrid_3line_scroll_cb, ad);
	elm_list_item_append(list, "popup-item 1line grid select mode", NULL, NULL,
			_popup_gengrid_1line_select_mode_cb, ad);
	elm_list_item_append(list, "popup- label+scroller", NULL, NULL,
			_popup_label_cb, ad);
	elm_list_go(list);
}
