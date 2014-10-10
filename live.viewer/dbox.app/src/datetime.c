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
#include "datetime.h"
#include <langinfo.h>

static void _datetime_cb(void *data, Evas_Object *obj, void *event_info);
static void _datepicker_cb(void *data, Evas_Object * obj, void *event_info);
static void _timepicker_cb(void *data, Evas_Object * obj, void *event_info);
static void _pickerpopup_cb(void *data, Evas_Object * obj, void *event_info);
static void _custom_picker_normal_popup_cb(void *data, Evas_Object * obj, void *event_info);
static void _custom_picker_center_popup_cb(void *data, Evas_Object * obj, void *event_info);
static void _edit_start_cb(void *data, Evas_Object *obj, void *event_info);
static void _edit_end_cb(void *data, Evas_Object *obj, void *event_info);
static void _picker_back_cb(void *data, Evas_Object *obj, void *event_info);

static Evas_Object *datetime_date_picker_layout = NULL;
static Evas_Object *datetime_time_picker_layout = NULL;

static Evas_Object *date_picker_layout = NULL;
static Evas_Object *time_picker_layout = NULL;

static Evas_Object *datetime_popup = NULL;
static Evas_Object *date_popup = NULL;
static Evas_Object *time_popup = NULL;
static Evas_Object *win = NULL;
static Evas_Object *naviframe = NULL;
static Evas_Object *is_normal_popup = EINA_TRUE;

static struct _menu_item _menu_its[] = {
	{"Datetime Styles", _datetime_cb},
	{"Datepicker Styles", _datepicker_cb},
	{"Timepicker Styles", _timepicker_cb},
	{"Datetime Picker popup Styles", _pickerpopup_cb},
	{"Datetime Custom Normal Popup Styles", _custom_picker_normal_popup_cb},
	{"Datetime Custom Center Popup Styles", _custom_picker_center_popup_cb},
	/* do not delete below */
	{NULL, NULL}
};

static void _edit_start_cb(void *data, Evas_Object *obj, void *event_info)
{
	ea_object_event_callback_add(obj, EA_CALLBACK_BACK, _picker_back_cb, NULL);
}

static void _edit_end_cb(void *data, Evas_Object *obj, void *event_info)
{
	ea_object_event_callback_del(obj, EA_CALLBACK_BACK, _picker_back_cb);
}

static inline void _picker_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_object_signal_emit(obj, "picker,hide", "");
}

static void _list_click(void *data, Evas_Object * obj, void *event_info)
{
	Elm_Object_Item *it = (Elm_Object_Item *) elm_list_selected_item_get(obj);

	if (!it) {
		fprintf(stderr, "list item is NULL\n");
		return;
	}

	elm_list_item_selected_set(it, EINA_FALSE);
}

static Evas_Object* _create_scroller(Evas_Object* parent)
{
	Evas_Object* scroller = elm_scroller_add(parent);
	elm_scroller_bounce_set(scroller, EINA_FALSE, EINA_TRUE);
	elm_scroller_policy_set(scroller,ELM_SCROLLER_POLICY_OFF,ELM_SCROLLER_POLICY_AUTO);
	evas_object_show(scroller);

	return scroller;
}

static void _changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	printf("Datetime value is changed\n");
}

static void _set_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	printf("Set clicked\n");
}

#ifndef DESKTOP
static char *
_datetime_format_get()
{
	char *dt_fmt, *region_fmt, *lang = NULL;
	char buf[256] = {0,};
	int time_val = 0, is_hour24 = 0, ret;

	lang = getenv("LANGUAGE");
	setenv("LANGUAGE", "en_US", 1);

	region_fmt = vconf_get_str(VCONFKEY_REGIONFORMAT);
	ret = vconf_get_int(VCONFKEY_REGIONFORMAT_TIME1224, &time_val);
	if (ret < 0)
		is_hour24 = 0;
	else if( time_val == VCONFKEY_TIME_FORMAT_12 || time_val == VCONFKEY_TIME_FORMAT_24)
		is_hour24 = time_val - 1;

	if (is_hour24)
		snprintf(buf, sizeof(buf), "%s_DTFMT_24HR", region_fmt);
	else
		snprintf(buf, sizeof(buf), "%s_DTFMT_12HR", region_fmt);

	dt_fmt = dgettext("dt_fmt", buf);

	if(!lang || !strcmp(lang, ""))
		unsetenv("LANGUAGE");
	else
		setenv("LANGUAGE", lang, 1);

	return strdup(dt_fmt);
}

static void
_vconf_datetime_format_changed_cb(keynode_t *node, void *data)
{
	Evas_Object *datetime = (Evas_Object *) data;
	char *dt_fmt = _datetime_format_get();
	elm_datetime_format_set(datetime, dt_fmt);
	free(dt_fmt);
}
#endif

static Eina_Bool _pop_cb(void *data, Elm_Object_Item *it)
{
#ifndef DESKTOP
	vconf_ignore_key_changed(VCONFKEY_REGIONFORMAT, _vconf_datetime_format_changed_cb);
	vconf_ignore_key_changed(VCONFKEY_REGIONFORMAT_TIME1224, _vconf_datetime_format_changed_cb);
#endif
	return EINA_TRUE;
}

static Evas_Object *_create_datetime_styles(Evas_Object * parent)
{
	Evas_Object *layout, *datetime;
	time_t t;
	struct tm time1;
	char *dt_fmt;

	layout = elm_layout_add(parent);
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "elmdemo-test/datetime_styles");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	datetime = elm_datetime_add(layout);
	elm_datetime_format_set(datetime, "%d/%b/%Y %I:%M %p");
	elm_object_part_content_set(layout, "picker1", datetime);
	evas_object_smart_callback_add(datetime, "edit,start", _edit_start_cb, NULL);
	evas_object_smart_callback_add(datetime, "edit,end", _edit_end_cb, NULL);

	datetime = elm_datetime_add(layout);
	elm_datetime_format_set(datetime, "%b/%d/%Y %H:%M");
	elm_object_part_content_set(layout, "picker2", datetime);
	evas_object_smart_callback_add(datetime, "edit,start", _edit_start_cb, NULL);
	evas_object_smart_callback_add(datetime, "edit,end", _edit_end_cb, NULL);

	datetime = elm_datetime_add(layout);
	elm_datetime_format_set(datetime, "%d/%b/%Y %I:%M %p");
	elm_object_part_content_set(layout, "picker3", datetime);
	evas_object_smart_callback_add(datetime, "edit,start", _edit_start_cb, NULL);
	evas_object_smart_callback_add(datetime, "edit,end", _edit_end_cb, NULL);
	elm_object_signal_emit(datetime, "weekday,show", "");

	datetime = elm_datetime_add(layout);
	elm_datetime_format_set(datetime, "%b/%d/%Y %H:%M");
	elm_object_part_content_set(layout, "picker4", datetime);
	evas_object_smart_callback_add(datetime, "edit,start", _edit_start_cb, NULL);
	evas_object_smart_callback_add(datetime, "edit,end", _edit_end_cb, NULL);
	elm_object_signal_emit(datetime, "weekday,show", "");

	datetime = elm_datetime_add(layout);
	elm_object_part_content_set(layout, "picker5", datetime);
	evas_object_smart_callback_add(datetime, "edit,start", _edit_start_cb, NULL);
	evas_object_smart_callback_add(datetime, "edit,end", _edit_end_cb, NULL);
#ifndef DESKTOP
	dt_fmt = _datetime_format_get();
	elm_datetime_format_set(datetime, dt_fmt);
	free(dt_fmt);
	vconf_notify_key_changed(VCONFKEY_REGIONFORMAT, _vconf_datetime_format_changed_cb, datetime);
	vconf_notify_key_changed(VCONFKEY_REGIONFORMAT_TIME1224, _vconf_datetime_format_changed_cb, datetime);
#endif

	datetime = elm_datetime_add(layout);
	elm_datetime_format_set(datetime, "%Y/%b/%d %I:%M %p");
	t = time(NULL);
	localtime_r(&t, &time1);
	// set the min time limit as "2000 May 10th 02:30 PM"
	time1.tm_year = 100;
	time1.tm_mon = 4;
	time1.tm_mday = 10;
	time1.tm_hour = 14;
	time1.tm_min = 30;
	elm_datetime_value_min_set(datetime, &time1);
	// date can be input only in between 10 to 20
	elm_datetime_field_limit_set(datetime, ELM_DATETIME_DATE, 10, 20);
	// minutes can be input only in between 20 and 40
	elm_datetime_field_limit_set(datetime, ELM_DATETIME_MINUTE, 20, 40);
	elm_object_part_content_set(layout, "picker6", datetime);
	evas_object_smart_callback_add(datetime, "changed", _changed_cb, datetime);
	evas_object_smart_callback_add(datetime, "picker,value,set", _set_clicked_cb, NULL);
	evas_object_smart_callback_add(datetime, "edit,start", _edit_start_cb, NULL);
	evas_object_smart_callback_add(datetime, "edit,end", _edit_end_cb, NULL);

	return layout;
}

static void _datetime_cb(void *data, Evas_Object * obj, void *event_info)
{
	Evas_Object *scroller, *layout_inner;
	Elm_Object_Item *it;

	struct appdata *ad = (struct appdata *)data;
	if (!ad) return;

	scroller = _create_scroller(ad->nf);
	it = elm_naviframe_item_push(ad->nf, _("Datetime Styles"), NULL, NULL, scroller, NULL);
	elm_naviframe_item_pop_cb_set(it, _pop_cb, NULL);

	layout_inner = _create_datetime_styles(ad->nf);
	elm_object_content_set(scroller, layout_inner);
}

static Evas_Object *_create_datepicker_styles(Evas_Object * parent)
{
	Evas_Object *layout, *datetime;

	layout = elm_layout_add(parent);
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "elmdemo-test/datepicker_styles");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	datetime = elm_datetime_add(layout);
	elm_datetime_format_set(datetime, "%d/%b/%Y");
	elm_object_part_content_set(layout, "picker1", datetime);
	evas_object_smart_callback_add(datetime, "edit,start", _edit_start_cb, NULL);
	evas_object_smart_callback_add(datetime, "edit,end", _edit_end_cb, NULL);
	elm_object_signal_emit(datetime, "weekday,show", "");

	datetime = elm_datetime_add(layout);
	elm_datetime_format_set(datetime, "%b/%d/%Y");
	elm_object_part_content_set(layout, "picker2", datetime);
	evas_object_smart_callback_add(datetime, "edit,start", _edit_start_cb, NULL);
	evas_object_smart_callback_add(datetime, "edit,end", _edit_end_cb, NULL);

	datetime = elm_datetime_add(layout);
	elm_datetime_format_set(datetime, "%Y/%d/%b");
	elm_object_part_content_set(layout, "picker3", datetime);
	evas_object_smart_callback_add(datetime, "edit,start", _edit_start_cb, NULL);
	evas_object_smart_callback_add(datetime, "edit,end", _edit_end_cb, NULL);

	datetime = elm_datetime_add(layout);
	elm_datetime_format_set(datetime, "%d,%b,%Y");
	elm_object_part_content_set(layout, "picker4", datetime);
	evas_object_smart_callback_add(datetime, "edit,start", _edit_start_cb, NULL);
	evas_object_smart_callback_add(datetime, "edit,end", _edit_end_cb, NULL);

	datetime = elm_datetime_add(layout);
	elm_datetime_format_set(datetime, "%d-%b-%Y");
	elm_object_part_content_set(layout, "picker5", datetime);
	evas_object_smart_callback_add(datetime, "edit,start", _edit_start_cb, NULL);
	evas_object_smart_callback_add(datetime, "edit,end", _edit_end_cb, NULL);

	datetime = elm_datetime_add(layout);
	elm_datetime_format_set(datetime, "%d %b %Y");
	elm_object_part_content_set(layout, "picker6", datetime);
	evas_object_smart_callback_add(datetime, "picker,value,set", _set_clicked_cb, NULL);
	evas_object_smart_callback_add(datetime, "edit,start", _edit_start_cb, NULL);
	evas_object_smart_callback_add(datetime, "edit,end", _edit_end_cb, NULL);

	return layout;
}

static void _datepicker_cb(void *data, Evas_Object * obj, void *event_info)
{
	Evas_Object *scroller, *layout_inner;

	struct appdata *ad = (struct appdata *)data;
	if (!ad) return;

	scroller = _create_scroller(ad->nf);
	elm_naviframe_item_push(ad->nf, _("Datepicker Styles"), NULL, NULL, scroller, NULL);

	layout_inner = _create_datepicker_styles(ad->nf);
	elm_object_content_set(scroller, layout_inner);
}

static Evas_Object *_create_timepicker_styles(Evas_Object * parent)
{
	Evas_Object *layout, *datetime;

	layout = elm_layout_add(parent);
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "elmdemo-test/timepicker_styles");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	datetime = elm_datetime_add(layout);
	elm_datetime_format_set(datetime, "%I:%M %p");
	elm_object_part_content_set(layout, "picker1", datetime);
	evas_object_smart_callback_add(datetime, "edit,start", _edit_start_cb, NULL);
	evas_object_smart_callback_add(datetime, "edit,end", _edit_end_cb, NULL);

	datetime = elm_datetime_add(layout);
	elm_datetime_format_set(datetime, "%H:%M");
	elm_object_part_content_set(layout, "picker2", datetime);
	evas_object_smart_callback_add(datetime, "edit,start", _edit_start_cb, NULL);
	evas_object_smart_callback_add(datetime, "edit,end", _edit_end_cb, NULL);

	datetime = elm_datetime_add(layout);
	elm_datetime_format_set(datetime, "%p %I:%M");
	elm_object_part_content_set(layout, "picker3", datetime);
	evas_object_smart_callback_add(datetime, "picker,value,set", _set_clicked_cb, NULL);
	evas_object_smart_callback_add(datetime, "edit,start", _edit_start_cb, NULL);
	evas_object_smart_callback_add(datetime, "edit,end", _edit_end_cb, NULL);

	return layout;
}

static void _timepicker_cb(void *data, Evas_Object * obj, void *event_info)
{
	Evas_Object *scroller, *layout_inner;

	struct appdata *ad = (struct appdata *)data;
	if (!ad) return;

	scroller = _create_scroller(ad->nf);
	elm_naviframe_item_push(ad->nf, _("Timepicker Styles"), NULL, NULL, scroller, NULL);

	layout_inner = _create_timepicker_styles(ad->nf);
	elm_object_content_set(scroller, layout_inner);
}

static void _launch_datepicker_popup(void *data, Evas_Object * obj, void *event_info)
{
	Evas_Object *datetime;

	datetime = (Evas_Object *)data;
	if (!datetime) return;

	elm_object_signal_emit(datetime, "datepicker,show", "");
}

static void _launch_timepicker_popup(void *data, Evas_Object * obj, void *event_info)
{
	Evas_Object *datetime;

	datetime = (Evas_Object *)data;
	if (!datetime) return;

	elm_object_signal_emit(datetime, "timepicker,show", "");
}

static Evas_Object *_create_pickerpopup_styles(Evas_Object * parent)
{
	Evas_Object *layout, *btn, *datetime;

	layout = elm_layout_add(parent);
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "elmdemo-test/datetime_pickerpopup_styles");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	datetime = elm_datetime_add(layout);
	elm_object_style_set(datetime, "pickerstyle");
	elm_datetime_format_set(datetime, "%d/ %b/ %Y");
	btn = elm_button_add(layout);
	elm_object_style_set(btn, "style1/auto_expand");
	elm_object_text_set(btn, "Launch Datepicker popup");
	elm_object_part_content_set(layout, "picker1", btn);
	evas_object_show(datetime);
	evas_object_smart_callback_add(btn, "clicked", _launch_datepicker_popup, datetime);
	evas_object_smart_callback_add(datetime, "edit,start", _edit_start_cb, NULL);
	evas_object_smart_callback_add(datetime, "edit,end", _edit_end_cb, NULL);

	datetime = elm_datetime_add(layout);
	elm_object_style_set(datetime, "pickerstyle");
	elm_datetime_format_set(datetime, "%I:%M %p");
	btn = elm_button_add(layout);
	elm_object_style_set(btn, "style1/auto_expand");
	elm_object_text_set(btn, "Launch Timepicker 12hr popup");
	elm_object_part_content_set(layout, "picker2", btn);
	evas_object_show(datetime);
	evas_object_smart_callback_add(btn, "clicked", _launch_timepicker_popup, datetime);
	evas_object_smart_callback_add(datetime, "edit,start", _edit_start_cb, NULL);
	evas_object_smart_callback_add(datetime, "edit,end", _edit_end_cb, NULL);

	datetime = elm_datetime_add(layout);
	elm_object_style_set(datetime, "pickerstyle");
	elm_datetime_format_set(datetime, "%H:%M");
	btn = elm_button_add(layout);
	elm_object_style_set(btn, "style1/auto_expand");
	elm_object_text_set(btn, "Launch Timepicker 24hr popup");
	elm_object_part_content_set(layout, "picker3", btn);
	evas_object_show(datetime);
	evas_object_smart_callback_add(btn, "clicked", _launch_timepicker_popup, datetime);
	evas_object_smart_callback_add(datetime, "picker,value,set", _set_clicked_cb, NULL);
	evas_object_smart_callback_add(datetime, "edit,start", _edit_start_cb, NULL);
	evas_object_smart_callback_add(datetime, "edit,end", _edit_end_cb, NULL);

	return layout;
}

static void _pickerpopup_cb(void *data, Evas_Object * obj, void *event_info)
{
	Evas_Object *scroller, *layout_inner;

	struct appdata *ad = (struct appdata *)data;
	if (!ad) return;

	scroller = _create_scroller(ad->nf);
	elm_naviframe_item_push(ad->nf, _("Datetime Picker popup Styles"), NULL, NULL, scroller, NULL);

	layout_inner = _create_pickerpopup_styles(ad->nf);
	elm_object_content_set(scroller, layout_inner);
}

/* Custom Datetime picker Popup implementation Starts*/
static void _custom_picker_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	ea_object_event_callback_del(obj, EA_CALLBACK_BACK, _custom_picker_back_cb);
	if (obj == datetime_popup) {
		evas_object_del(datetime_popup);
		datetime_popup = NULL;
	}
	else if (obj == date_popup) {
		evas_object_del(date_popup);
		date_popup = NULL;
	}
	else if (obj == time_popup) {
		evas_object_del(time_popup);
		time_popup = NULL;
	}
}

static void _win_rotation_changed(void *data, Evas_Object *obj, void *event_info)
{
	int rotation = -1;
	Evas_Object *layout;
	Evas_Object *window = obj;
	if (datetime_popup) {
		layout = elm_object_content_get(datetime_popup);
		rotation = elm_win_rotation_get(window);
		if (layout) {
			if (rotation == 90 || rotation == 270)
				elm_layout_file_set(layout, ELM_DEMO_EDJ, "elmdemo-test/datetime_custom_layout_landscape");
			else
				elm_layout_file_set(layout, ELM_DEMO_EDJ, "elmdemo-test/datetime_custom_layout");
		}
	}
}

static void _datetime_picker_value_set_cb(void *data, Evas_Object *obj, void *event_info)
{
	char *date, *time;
	char buf[1024];
	Evas_Object* btn = data;
	date = elm_object_text_get(elm_object_part_content_get(obj, "date.btn"));
	time = elm_object_text_get(elm_object_part_content_get(obj, "time.btn"));
	snprintf(buf, sizeof(buf), "%s  %s", date, time);
	elm_object_text_set(btn, buf);
}

static void _date_picker_value_set_cb(void *data, Evas_Object *obj, void *event_info)
{
	char *date;
	Evas_Object* btn = data;
	date = elm_object_text_get(elm_object_part_content_get(obj, "date.btn"));
	elm_object_text_set(btn, date);
}

static void _time_picker_value_set_cb(void *data, Evas_Object *obj, void *event_info)
{
	char *time;
	Evas_Object* btn = data;
	time = elm_object_text_get(elm_object_part_content_get(obj, "time.btn"));
	elm_object_text_set(btn, time);
}

static void _datetime_date_picker_popup_layout_get(void *data, Evas_Object *obj, void *event_info)
{
	datetime_date_picker_layout = (Evas_Object*)event_info;
}

static void _datetime_time_picker_popup_layout_get(void *data, Evas_Object *obj, void *event_info)
{
	datetime_time_picker_layout = (Evas_Object*)event_info;
}

static void _datepicker_popup_layout_get(void *data, Evas_Object *obj, void *event_info)
{
	date_picker_layout = (Evas_Object*)event_info;
}

static void _timepicker_popup_layout_get(void *data, Evas_Object *obj, void *event_info)
{
	time_picker_layout = (Evas_Object*)event_info;
}

static void
_popup_cancel_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (data == datetime_popup) {
		evas_object_del(datetime_popup);
		datetime_popup = NULL;
	}
	else if (data == date_popup) {
		evas_object_del(date_popup);
		date_popup = NULL;
	}
	else if (data == time_popup) {
		evas_object_del(time_popup);
		time_popup = NULL;
	}
}

static void
_popup_set_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	evas_object_smart_callback_call(data, "custom,picker,value,set", NULL);
	if (datetime_popup) {
		evas_object_del(datetime_popup);
		datetime_popup = NULL;
	}

	if (date_popup) {
		evas_object_del(date_popup);
		date_popup = NULL;
	}

	if (time_popup) {
		evas_object_del(time_popup);
		time_popup = NULL;
	}
}

static Evas_Object* _custom_datetime_picker_popup_add(Evas_Object* parent, Evas_Object* datetime)
{
	Evas_Object *popup = NULL;
	Evas_Object *set_btn, *cancel_btn;
	if (is_normal_popup)
		popup = elm_popup_add(parent);
	else
		popup = ea_center_popup_add(parent);
	cancel_btn = elm_button_add(popup);
	elm_object_style_set(cancel_btn, "popup");
	elm_object_text_set(cancel_btn, "Cancel");
	elm_object_part_content_set(popup, "button1", cancel_btn);
	evas_object_smart_callback_add(cancel_btn, "clicked", _popup_cancel_btn_clicked_cb, popup);

	set_btn = elm_button_add(popup);
	elm_object_style_set(set_btn, "popup");
	elm_object_text_set(set_btn,"Set");
	elm_object_part_content_set(popup, "button2", set_btn);
	evas_object_smart_callback_add(set_btn, "clicked", _popup_set_btn_clicked_cb, datetime);
	return popup;
}

static void _launch_custom_datetimepicker_popup(void *data, Evas_Object * obj, void *event_info)
{
	Evas_Object *layout, *scroller, *layout1, *datetime;
	datetime = (Evas_Object *)data;
	int rotation = -1;

	datetime_popup = _custom_datetime_picker_popup_add(naviframe, datetime);
	if (is_normal_popup)
		evas_object_smart_callback_call(datetime, "custom,picker,win,get", NULL);
	else
		evas_object_smart_callback_call(datetime, "custom,picker,win,get", ea_center_popup_win_get(datetime_popup));

	evas_object_smart_callback_call(data, "datetime,picker,layout,create", NULL);
	elm_object_part_text_set(datetime_popup, "title,text", "DateTime");

	layout1 = elm_layout_add(datetime_popup);
	rotation = elm_win_rotation_get(win);
	if (rotation == 90 || rotation == 270)
		elm_layout_file_set(layout1, ELM_DEMO_EDJ, "elmdemo-test/datetime_custom_layout_landscape");
	else
		elm_layout_file_set(layout1, ELM_DEMO_EDJ, "elmdemo-test/datetime_custom_layout");
	evas_object_size_hint_weight_set(layout1, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	scroller = elm_scroller_add(layout1);
	elm_scroller_bounce_set(scroller, EINA_TRUE, EINA_TRUE);
	elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	evas_object_show(scroller);

	layout = elm_layout_add(datetime_popup);
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "elmdemo-test/datetime_custom_pickerpopup_layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_part_content_set(layout, "datepicker", datetime_date_picker_layout);
	elm_object_part_content_set(layout, "timepicker", datetime_time_picker_layout);
	evas_object_show(layout);
	elm_object_content_set(scroller, layout);

	elm_object_part_content_set(layout1, "scroller", scroller);
	elm_object_content_set(datetime_popup, layout1);
	evas_object_show(datetime_popup);

	ea_object_event_callback_add(datetime_popup, EA_CALLBACK_BACK, _custom_picker_back_cb, NULL);
}

static void _launch_custom_date_picker_popup(void *data, Evas_Object * obj, void *event_info)
{
	Evas_Object *datetime;

	datetime = (Evas_Object *)data;
	date_popup = _custom_datetime_picker_popup_add(naviframe, datetime);
	if (is_normal_popup)
		evas_object_smart_callback_call(datetime, "custom,picker,win,get", NULL);
	else
		evas_object_smart_callback_call(datetime, "custom,picker,win,get", ea_center_popup_win_get(date_popup));

	evas_object_smart_callback_call(data, "datetime,picker,layout,create", NULL);
	elm_object_part_text_set(date_popup, "title,text", "Date");
	elm_object_content_set(date_popup, date_picker_layout);

	evas_object_show(date_popup);
	ea_object_event_callback_add(date_popup, EA_CALLBACK_BACK, _custom_picker_back_cb, NULL);
}

static void _launch_custom_time_picker_popup(void *data, Evas_Object * obj, void *event_info)
{
	Evas_Object *datetime;

	datetime = (Evas_Object *)data;
	time_popup = _custom_datetime_picker_popup_add(naviframe, datetime);

	if (is_normal_popup)
		evas_object_smart_callback_call(datetime, "custom,picker,win,get", NULL);
	else
		evas_object_smart_callback_call(datetime, "custom,picker,win,get", ea_center_popup_win_get(time_popup));

	evas_object_smart_callback_call(data, "datetime,picker,layout,create", NULL);
	elm_object_part_text_set(time_popup, "title,text", "Time");
	elm_object_content_set(time_popup, time_picker_layout);

	evas_object_show(time_popup);
	ea_object_event_callback_add(time_popup, EA_CALLBACK_BACK, _custom_picker_back_cb, NULL);
}

static Evas_Object *_create_custom_pickerpopup_styles(struct appdata *ad)
{
	Evas_Object *layout, *btn1, *btn2, *btn3, *datetime;
	struct tm set_time;
	char *date;
	char *time;
	char buf[1024];
	win = ad->win_main;
	layout = elm_layout_add(ad->nf);
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "elmdemo-test/datetime_custom_pickerpopup_styles");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	datetime = elm_datetime_add(layout);
	elm_object_style_set(datetime, "pickerstyle");

	btn1 = elm_button_add(layout);
	elm_object_style_set(btn1, "style1/auto_expand");
	elm_object_part_content_set(layout, "picker1", btn1);
	evas_object_smart_callback_add(btn1, "clicked", _launch_custom_datetimepicker_popup, datetime);

	evas_object_smart_callback_add(datetime, "picker,date,layout,get", _datetime_date_picker_popup_layout_get, NULL);
	evas_object_smart_callback_add(datetime, "picker,time,layout,get", _datetime_time_picker_popup_layout_get, NULL);
	evas_object_smart_callback_add(datetime, "picker,value,set", _datetime_picker_value_set_cb, btn1);

	elm_datetime_format_set(datetime, "%d/%b/%Y %I:%M %p");
	evas_object_show(datetime);

	date = elm_object_text_get(elm_object_part_content_get(datetime, "date.btn"));
	time = elm_object_text_get(elm_object_part_content_get(datetime, "time.btn"));
	snprintf(buf, sizeof(buf), "%s  %s", date, time);
	elm_object_text_set(btn1, buf);

	datetime = elm_datetime_add(layout);
	elm_object_style_set(datetime, "pickerstyle");

	btn2 = elm_button_add(layout);
	elm_object_style_set(btn2, "style1/auto_expand");
	elm_object_part_content_set(layout, "picker2", btn2);
	evas_object_smart_callback_add(btn2, "clicked", _launch_custom_date_picker_popup, datetime);

	evas_object_smart_callback_add(datetime, "picker,date,layout,get", _datepicker_popup_layout_get, NULL);
	evas_object_smart_callback_add(datetime, "picker,value,set", _date_picker_value_set_cb, btn2);

	elm_datetime_format_set(datetime, "%d/%b/%Y");
	evas_object_show(datetime);


	date = elm_object_text_get(elm_object_part_content_get(datetime, "date.btn"));
	elm_object_text_set(btn2, date);

	datetime = elm_datetime_add(layout);
	elm_object_style_set(datetime, "pickerstyle");

	btn3 = elm_button_add(layout);
	elm_object_style_set(btn3, "style1/auto_expand");
	elm_object_part_content_set(layout, "picker3", btn3);
	evas_object_smart_callback_add(btn3, "clicked", _launch_custom_time_picker_popup, datetime);

	evas_object_smart_callback_add(datetime, "picker,time,layout,get", _timepicker_popup_layout_get, NULL);
	evas_object_smart_callback_add(datetime, "picker,value,set", _time_picker_value_set_cb, btn3);

	elm_datetime_format_set(datetime, "%I:%M %p");
	evas_object_show(datetime);


	time = elm_object_text_get(elm_object_part_content_get(datetime, "time.btn"));
	elm_object_text_set(btn3, time);
	evas_object_smart_callback_add(win, "rotation,changed", _win_rotation_changed, NULL);
	return layout;
}

static void _custom_picker_normal_popup_cb(void *data, Evas_Object * obj, void *event_info)
{
	Evas_Object *scroller, *layout_inner;

	struct appdata *ad = (struct appdata *)data;
	if (!ad) return;
	is_normal_popup = EINA_TRUE;
	naviframe = ad->nf;
	scroller = _create_scroller(ad->nf);
	elm_naviframe_item_push(ad->nf, _("Datetime Custom Picker Normal popup Styles"), NULL, NULL, scroller, NULL);

	layout_inner = _create_custom_pickerpopup_styles(ad);
	elm_object_content_set(scroller, layout_inner);
}

static void _custom_picker_center_popup_cb(void *data, Evas_Object * obj, void *event_info)
{
	Evas_Object *scroller, *layout_inner;

	struct appdata *ad = (struct appdata *)data;
	if (!ad) return;
	is_normal_popup = EINA_FALSE;
	naviframe = ad->nf;
	scroller = _create_scroller(ad->nf);
	elm_naviframe_item_push(ad->nf, _("Datetime Custom Picker Center popup Styles"), NULL, NULL, scroller, NULL);

	layout_inner = _create_custom_pickerpopup_styles(ad);
	elm_object_content_set(scroller, layout_inner);
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

void datetime_cb(void *data, Evas_Object * obj, void *event_info)
{
	struct appdata *ad;

	Evas_Object *list;

	ad = (struct appdata *)data;
	if (!ad) return;

	list = _create_list_winset(ad);
	elm_naviframe_item_push(ad->nf, _("Datetime"), NULL, NULL, list, NULL);
}
