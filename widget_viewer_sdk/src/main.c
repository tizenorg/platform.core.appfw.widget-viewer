/*
 * Copyright 2014  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.1 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://floralicense.org/license/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <Elementary.h>
#include <app.h>
#include <unistd.h>
#include <errno.h>
#include <widget_errno.h>
#include <widget_service.h>
#include <widget_viewer_evas.h>
#include <vconf.h>
#include <bundle.h>
#include <dlog.h>
#include <string.h>
#include <app_control.h>
#include <app_control_internal.h>

int errno;

#include "main.h"
#include "debug.h"

static struct info {
	int w;
	int h;
	Evas_Object *win;
	Evas_Object *bg;
	Evas_Object *box;
	Evas_Object *layout;
	Ecore_Timer *long_press;

	struct ctx_item {
		char *content_info;
		char *title;
		double period;
		Evas_Object *widget;
		int *size_types;
		int count_of_size_type;
	} ctx;
} s_info = {
	.w = 0,
	.h = 0,
	.win = NULL,
	.layout = NULL,
	.long_press = NULL,

	.ctx = {
		.content_info = NULL,
		.title = NULL,
		.period = WIDGET_VIEWER_EVAS_DEFAULT_PERIOD, 
		.widget = NULL,
		.size_types = NULL,
		.count_of_size_type = 20,
	},
};

#define LONG_PRESS 1.0f

static void hide_widget_info_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	DbgPrint("Hide info panel\n");
	elm_object_signal_emit(s_info.layout, "hide", "info");
}

static Eina_Bool show_widget_info_cb(void *data)
{
	DbgPrint("Show info panel\n");
	elm_object_signal_emit(s_info.layout, "show", "info");

	s_info.long_press = NULL;
	return ECORE_CALLBACK_CANCEL;
}

static void layout_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	if (s_info.long_press) {
		ecore_timer_del(s_info.long_press);
		s_info.long_press = NULL;
	}

	s_info.long_press = ecore_timer_add(LONG_PRESS, show_widget_info_cb, NULL);
	if (!s_info.long_press) {
		ErrPrint("Failed to add a timer\n");
	}
}

static void layout_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	if (s_info.long_press) {
		ecore_timer_del(s_info.long_press);
		s_info.long_press = NULL;
	}
}

static bool app_create(void *data)
{
	s_info.win = elm_win_add(NULL, "Widget Viewer (SDK)", ELM_WIN_BASIC);
	if (!s_info.win) {
		LOGD("Failed to create a window\n");
		return false;
	}

	evas_object_show(s_info.win);

	elm_win_screen_size_get(s_info.win, NULL, NULL, &s_info.w, &s_info.h);
	LOGD("Window: %dx%d\n", s_info.w, s_info.h);

	s_info.bg = elm_bg_add(s_info.win);
	if (!s_info.bg) {
		ErrPrint("Failed to add a BG\n");
		evas_object_del(s_info.win);
		s_info.win = NULL;
		return false;
	}

	elm_win_resize_object_add(s_info.win, s_info.bg);
	elm_bg_color_set(s_info.bg, 0, 0, 0);
	evas_object_show(s_info.bg);

	s_info.box = elm_box_add(s_info.win);
	if (!s_info.box) {
		evas_object_del(s_info.bg);
		evas_object_del(s_info.win);
		s_info.bg = NULL;
		s_info.win = NULL;
		return false;
	}
	elm_win_resize_object_add(s_info.win, s_info.box);
	evas_object_show(s_info.box);

	s_info.layout = elm_layout_add(s_info.win);
	if (!s_info.layout) {
		evas_object_del(s_info.box);
		evas_object_del(s_info.bg);
		evas_object_del(s_info.win);
		s_info.box = NULL;
		s_info.win = NULL;
		s_info.bg = NULL;
		return false;
	}

	elm_box_align_set(s_info.box, 0.5, 0.5);
	evas_object_size_hint_expand_set(s_info.box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_fill_set(s_info.box, EVAS_HINT_FILL, EVAS_HINT_FILL);

	if (elm_layout_file_set(s_info.layout, LAYOUT_EDJ, "layout") != EINA_TRUE) {
		LOGE("Failed to load an edje\n");
		evas_object_del(s_info.bg);
		evas_object_del(s_info.layout);
		evas_object_del(s_info.win);
		s_info.bg = NULL;
		s_info.layout = NULL;
		s_info.win = NULL;
		return false;
	}
	evas_object_show(s_info.layout);

	elm_object_signal_callback_add(s_info.layout, "mouse,clicked,1", "widget,info,bg", hide_widget_info_cb, NULL);
	evas_object_event_callback_add(s_info.layout, EVAS_CALLBACK_MOUSE_DOWN, layout_down_cb, NULL);
	evas_object_event_callback_add(s_info.layout, EVAS_CALLBACK_MOUSE_UP, layout_up_cb, NULL);

	elm_box_pack_end(s_info.box, s_info.layout);

	/* WIDGET init */
	widget_viewer_evas_set_option(WIDGET_VIEWER_EVAS_DIRECT_UPDATE, 1);
	widget_viewer_evas_set_option(WIDGET_VIEWER_EVAS_EVENT_AUTO_FEED, 1);

	widget_viewer_evas_init(s_info.win);

	return true;
}

static int unload_widget(void)
{
	const char *tmp;

	if (!s_info.ctx.widget) {
		return WIDGET_ERROR_NOT_EXIST;
	}

	elm_box_unpack(s_info.box, s_info.ctx.widget);

	tmp = widget_viewer_evas_get_widget_id(s_info.ctx.widget);
	DbgPrint("Unload previous widget: %s\n", tmp);

	widget_viewer_evas_set_permanent_delete(s_info.ctx.widget, EINA_TRUE);
	evas_object_del(s_info.ctx.widget);
	free(s_info.ctx.title);
	free(s_info.ctx.content_info);
	free(s_info.ctx.size_types);

	s_info.ctx.widget = NULL;
	s_info.ctx.title = NULL;
	s_info.ctx.content_info = NULL;
	s_info.ctx.size_types = NULL;
	s_info.ctx.period = WIDGET_VIEWER_EVAS_DEFAULT_PERIOD;
	s_info.ctx.count_of_size_type = 20;

	elm_object_part_text_set(s_info.layout, "widget,id", "");
	elm_object_part_text_set(s_info.layout, "widget,content,info", "");
	elm_object_part_text_set(s_info.layout, "widget,title", "");
	elm_object_part_text_set(s_info.layout, "widget,period", "");

	return WIDGET_ERROR_NONE;
}

static void app_terminate(void *data)
{
	unload_widget();

	/* WIDGET fini */
	widget_viewer_evas_fini();

	if (s_info.win) {
		evas_object_del(s_info.win);
		s_info.win = NULL;
	}
}

static void app_pause(void *data)
{
	/* WIDGET pause */
	widget_viewer_evas_notify_paused_status_of_viewer();
}

static void app_resume(void *data)
{
	/* WIDGET resume */
	widget_viewer_evas_notify_resumed_status_of_viewer();
}

static void updated_cb(void *data, Evas_Object *obj, void *event_info)
{
	/* Updated */
	DbgPrint("Widget updated\n");
}

static void list_item_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	int w;
	int h;

	if (widget_service_get_size(s_info.ctx.size_types[(long)data], &w, &h) != WIDGET_ERROR_NONE) {
		return;
	}

	evas_object_resize(s_info.ctx.widget, w, h);
	evas_object_size_hint_min_set(s_info.ctx.widget, w, h);
	evas_object_size_hint_max_set(s_info.ctx.widget, w, h);

	evas_object_resize(s_info.layout, w, h);
	evas_object_size_hint_min_set(s_info.layout, w, h);
	evas_object_size_hint_max_set(s_info.layout, w, h);
}

static void extra_updated_cb(void *data, Evas_Object *obj, void *event_info)
{
	const char *string;
	char *tmp;

	string = widget_viewer_evas_get_title_string(obj);
	if (string) {
		tmp = strdup(string);
		if (!tmp) {
			return;
		}

		free(s_info.ctx.title);
		s_info.ctx.title = tmp;

		elm_object_part_text_set(s_info.layout, "widget,title", s_info.ctx.title);
	}

	string = widget_viewer_evas_get_content_info(obj);
	if (string) {
		tmp = strdup(string);
		if (!tmp) {
			return;
		}

		free(s_info.ctx.content_info);
		s_info.ctx.content_info = tmp;

		elm_object_part_text_set(s_info.layout, "widget,content,info", s_info.ctx.content_info);
		DbgPrint("Content updated: [%s]\n", s_info.ctx.content_info);
	}
}

static void period_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	char buffer[96];

	s_info.ctx.period = widget_viewer_evas_get_period(obj);
	snprintf(buffer, sizeof(buffer), "%lf", s_info.ctx.period);
	elm_object_part_text_set(s_info.layout, "widget,period", buffer);
	DbgPrint("Period updated: %s\n", buffer);
}

static void widget_created_cb(void *data, Evas_Object *obj, void *event_info)
{
	period_changed_cb(data, obj, NULL);
	extra_updated_cb(data, obj, NULL);
}

static int load_widget(const char *widget_id)
{
	int w = 0;
	int h = 0;
	int i;

	for (i = 0; i < s_info.ctx.count_of_size_type; i++) {
		if (widget_service_get_size(s_info.ctx.size_types[i], &w, &h) != WIDGET_ERROR_NONE || w == 0 || h == 0) {
			continue;
		}

		break;
	}

	if (i == s_info.ctx.count_of_size_type) {
		ErrPrint("Supported size is not found\n");
		evas_object_resize(s_info.layout, s_info.w, s_info.h);
		evas_object_size_hint_min_set(s_info.layout, s_info.w, s_info.h);
		evas_object_size_hint_max_set(s_info.layout, s_info.w, s_info.h);
		evas_object_show(s_info.layout);

		elm_object_part_text_set(s_info.layout, "message", "Supported size is not found");
		elm_object_signal_emit(s_info.layout, "show", "message");
		return WIDGET_ERROR_NOT_SUPPORTED;
	}
	elm_object_signal_emit(s_info.layout, "hide", "message");

	DbgPrint("Found valid size[%X]: %dx%d\n", s_info.ctx.size_types[i], w, h);

	s_info.ctx.widget = widget_viewer_evas_add_widget(s_info.win, widget_id, s_info.ctx.content_info, s_info.ctx.period);
	if (!s_info.ctx.widget) {
		ErrPrint("Failed to create a new widget\n");
		return WIDGET_ERROR_FAULT;
	}

	DbgPrint("Resize the widget(%s) to [%X] %dx%d\n", widget_id, s_info.ctx.size_types[0], w, h);

	evas_object_smart_callback_add(s_info.ctx.widget, WIDGET_SMART_SIGNAL_UPDATED, updated_cb, NULL);
	evas_object_smart_callback_add(s_info.ctx.widget, WIDGET_SMART_SIGNAL_EXTRA_INFO_UPDATED, extra_updated_cb, NULL);
	evas_object_smart_callback_add(s_info.ctx.widget, WIDGET_SMART_SIGNAL_PERIOD_CHANGED, period_changed_cb, NULL);
	evas_object_smart_callback_add(s_info.ctx.widget, WIDGET_SMART_SIGNAL_WIDGET_CREATED, widget_created_cb, NULL);

	elm_object_part_text_set(s_info.layout, "widget,id", widget_id);
	elm_object_part_content_set(s_info.layout, "widget", s_info.ctx.widget);

	evas_object_resize(s_info.ctx.widget, w, h);
	evas_object_size_hint_min_set(s_info.ctx.widget, w, h);
	evas_object_size_hint_max_set(s_info.ctx.widget, w, h);

	evas_object_resize(s_info.layout, w, h);
	evas_object_size_hint_min_set(s_info.layout, w, h);
	evas_object_size_hint_max_set(s_info.layout, w, h);

	return WIDGET_ERROR_NONE;
}

static char *list_item_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
	char buffer[256];
	int ret;
	int w;
	int h;

	ret = widget_service_get_size(s_info.ctx.size_types[(long)data], &w, &h);
	if (ret != WIDGET_ERROR_NONE) {
		return strdup("Invalid");
	}

	snprintf(buffer, sizeof(buffer), "%dx%d", w, h);
	DbgPrint("Size: [%s]\n", buffer);
	return strdup(buffer);
}

static Evas_Object *list_item_content_get_cb(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *icon = NULL;
	char *icon_filename;
	const char *widget_id;

	widget_id = widget_viewer_evas_get_widget_id(s_info.ctx.widget);

	icon_filename = widget_service_get_preview_image_path(widget_id, s_info.ctx.size_types[(long)data]);
	if (icon_filename) {
		icon = elm_icon_add(s_info.win);
		if (icon) {
			elm_image_file_set(icon, icon_filename, NULL);
			elm_image_resizable_set(icon, EINA_FALSE, EINA_FALSE);
			DbgPrint("Icon: %s\n", icon_filename);
		}
		free(icon_filename);
	} else {
		icon = NULL;
	}

	return icon;
}

static int prepare_widget(const char *widget_id, app_control_h control)
{
	int ret;
	Evas_Object *size_list;
	Elm_Object_Item *item;
	long i;
	int w;
	int h;
	bundle *b;
	static Elm_Genlist_Item_Class class = {
		.func = {
			.text_get = list_item_text_get_cb,
			.content_get = list_item_content_get_cb,
			.state_get = NULL,
			.del = NULL,
		}
	};

	if (app_control_export_as_bundle(control, &b) == APP_CONTROL_ERROR_NONE) {
		bundle_raw *r;
		int len;

		if (bundle_encode(b, &r, &len) == BUNDLE_ERROR_NONE) {
			s_info.ctx.content_info = malloc(len + 8);
			if (!s_info.ctx.content_info) {
				ErrPrint("malloc: %d\n", errno);
			} else {
				snprintf(s_info.ctx.content_info, len + 8, "%d:%s", len, (char *)r);
				DbgPrint("Encoded content_info: [%s]\n", s_info.ctx.content_info);
			}

			free((char *)r); /* Do I have to use the g_free? */
		} else {
			ErrPrint("Failed to encode a bundle\n");
		}

		bundle_free(b);
	}

	s_info.ctx.count_of_size_type = 20;
	ret = widget_service_get_supported_size_types(widget_id, &s_info.ctx.count_of_size_type, &s_info.ctx.size_types);
	if (ret != WIDGET_ERROR_NONE) {
		ErrPrint("Failed to load an widget\n");
	}

	DbgPrint("[%s] %d\n", widget_id, s_info.ctx.count_of_size_type);

	if (s_info.ctx.count_of_size_type <= 1) {
		elm_object_signal_emit(s_info.layout, "hide", "size,list");
		return WIDGET_ERROR_NONE;
	} else {
		elm_object_signal_emit(s_info.layout, "show", "size,list");
	}

	size_list = elm_object_part_content_get(s_info.layout, "widget,size,list");
	if (size_list) {
		elm_genlist_clear(size_list);
	} else {
		size_list = elm_genlist_add(s_info.win);
		if (!size_list) {
			ErrPrint("Failed to create a genlist\n");
			return WIDGET_ERROR_FAULT;
		}

		elm_object_part_content_set(s_info.layout, "widget,size,list", size_list);
	}

	DbgPrint("========\nDump supported size types [%s]\n", widget_id);
	for (i = 0; i < s_info.ctx.count_of_size_type; i++) {
		if (widget_service_get_size(s_info.ctx.size_types[i], &w, &h) != WIDGET_ERROR_NONE || w == 0 || h == 0) {
			DbgPrint("[%X] is not supported (%dx%d)\n", s_info.ctx.size_types[i], w, h);
			continue;
		}

		DbgPrint("Size[%X]\n", s_info.ctx.size_types[i]);
		item = elm_genlist_item_append(size_list, &class, (void *)i, NULL, ELM_GENLIST_ITEM_NONE, list_item_clicked_cb, (void *)i);
		if (!item) {
			ErrPrint("Failed to add a new size item\n");
		}
	}
	DbgPrint("========\nEnd of a dump\n");

	return ret;
}

static void _app_control(app_control_h service, void *data)
{
	char *widget_id = NULL;
	int ret;

	ret = app_control_get_extra_data(service, WIDGET_APPID, &widget_id);
	if (ret == APP_CONTROL_ERROR_NONE) {
		DbgPrint("Loading a widget: [%s]\n", widget_id);
		if (!widget_id) {
			return;
		}

		ret = unload_widget();
		ret = prepare_widget(widget_id, service);
		ret = load_widget(widget_id);
		free(widget_id);
	} else {
		/**
		 * @note
		 * Just raise up the window and notify to resumed state
		 */
		widget_viewer_evas_notify_resumed_status_of_viewer();
		elm_win_activate(s_info.win);
	}
}

int main(int argc, char *argv[])
{
	ui_app_lifecycle_callback_s event_callback;

	event_callback.create = app_create;
	event_callback.terminate = app_terminate;
	event_callback.pause = app_pause;
	event_callback.resume = app_resume;
	event_callback.app_control = _app_control;

	if (setenv("BUFMGR_LOCK_TYPE", "once", 0) < 0) {
		LOGE("setenv(BUFMGR_LOCK_TYPE) is failed: %s", strerror(errno));
	}

	if (setenv("BUFMGR_MAP_CACHE", "true", 0) < 0) {
		LOGE("setenv(BUFMGR_MAP_CACHE) is failed: %s", strerror(errno));
	}

	return ui_app_main(argc, argv, &event_callback, NULL);
}

/* End of a file */
