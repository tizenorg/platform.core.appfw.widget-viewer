/*
 * Copyright (c) 2011 Samsung Electronics Co., Lnd All Rights Reserved
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
#include "tickernoti.h"
#include <Ecore_X.h>

#ifndef __UNUSED__
#define __UNUSED__ __attribute__((unused))
#endif


static Evas_Object *tickernoti_email = NULL;
static Evas_Object *tickernoti_textonly = NULL;
static void _rotate_noti_win_cb(void *data, Evas_Object *obj, void *event_info);

enum Noti_Orient {
	TICKERNOTI_ORIENT_TOP = 0,
	TICKERNOTI_ORIENT_BOTTOM,
	TICKERNOTI_ORIENT_LAST
} ;

struct Tickernoti_Data {
	Evas_Object *content;
	Evas_Coord w;
	Evas_Coord h;
	int angle;
	enum Noti_Orient orient;
	Eina_Bool is_show;
} ;

static const char *data_key = "_data";

static Eina_Bool
_pop_cb(void *data, Elm_Object_Item *it)
{
	free(evas_object_data_del(tickernoti_email, data_key));
	free(evas_object_data_del(tickernoti_textonly, data_key));
	evas_object_del(tickernoti_email);
	evas_object_del(tickernoti_textonly);
	evas_object_smart_callback_del(data, "rotation,changed", _rotate_noti_win_cb);

	return EINA_TRUE;
}

static void
_tickernoti_update_geometry(Evas_Object *noti_win, Evas_Coord x, Evas_Coord y,
		Evas_Coord w, Evas_Coord h, int angle)
{
	if (!noti_win)
		return;
	elm_win_rotation_set(noti_win, angle);
	//printf("Rotate notification window to %d\n", angle);
	evas_object_move(noti_win, x, y);
	//printf("Move notification window to %d %d\n", x, y);
	evas_object_resize(noti_win, w, h);
	//printf("Resize notification window to %d %d\n", w, h);
}

static void
_tickernoti_recalc_position(Evas_Object *noti_win)
{
	struct Tickernoti_Data *td;
	Evas_Coord x = 0, y = 0, w = 0, h = 0;

	if (!noti_win) {
		printf("noti_win is NULL\n");
		return;
	}
	td = evas_object_data_get(noti_win, data_key);
	if (!td) {
		printf("noti_win data is NULL\n");
		return;
	}
	evas_object_geometry_get(td->content, NULL, NULL, &w, &h);
	ecore_x_window_size_get(ecore_x_window_root_first_get(), &x, &y);
	switch (td->angle) {
	case 90:
		if (td->orient == TICKERNOTI_ORIENT_TOP) {
			_tickernoti_update_geometry(noti_win, 0, 0, y, h, 90);
		} else {
			_tickernoti_update_geometry(noti_win, x - h, 0, y, h, 90);
		}
		break;
	case 180:
		if (td->orient == TICKERNOTI_ORIENT_TOP) {
			_tickernoti_update_geometry(noti_win, 0, y - h, x, h, 180);
		} else {
			_tickernoti_update_geometry(noti_win, 0, 0, x, h, 180);
		}
		break;
	case -90:
	case 270:
		if (td->orient == TICKERNOTI_ORIENT_TOP) {
			_tickernoti_update_geometry(noti_win, x - h, 0, y, h, 270);
		} else {
			_tickernoti_update_geometry(noti_win, 0, 0, y, h, 270);
		}
		break;
	case 0:
	default:
		if (td->orient == TICKERNOTI_ORIENT_TOP) {
			_tickernoti_update_geometry(noti_win, 0, 0, x, h, 0);
		} else {
			_tickernoti_update_geometry(noti_win, 0, y - h, x, h, 0);
		}
	}
}

static void
_rotate_noti_win_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct Tickernoti_Data *td_email, *td_textonly;
	int angle = 0;
	td_email = evas_object_data_get(tickernoti_email, data_key);
	td_textonly = evas_object_data_get(tickernoti_textonly, data_key);

	angle = elm_win_rotation_get(data);
	td_email->angle = angle;
	td_textonly->angle = angle;

	if(td_email->is_show)
		_tickernoti_recalc_position(tickernoti_email);
	if(td_textonly->is_show)
		_tickernoti_recalc_position(tickernoti_textonly);
}

static void
_hide_another_tickernoti(void *data __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj,
		void *event_info __UNUSED__)
{
	struct Tickernoti_Data *td;

	if (obj == tickernoti_email) {
		td = evas_object_data_get(tickernoti_textonly, data_key);
		if (td->is_show) {
			evas_object_hide(tickernoti_textonly);
			td->is_show = !td->is_show;
		}
	}
	else {
		td = evas_object_data_get(tickernoti_email, data_key);
		if (td->is_show) {
			evas_object_hide(tickernoti_email);
			td->is_show = !td->is_show;
		}
	}
}

static void
_tickernoti_show_hide(void *data, Evas_Object *obj __UNUSED__,
		void *event_info __UNUSED__)
{
	struct Tickernoti_Data *td;

	if (!data)
		return;
	td = evas_object_data_get(data, data_key);
	if (!td)
		return;
	if (td->is_show) {
		evas_object_hide(data);
		if (td->orient == TICKERNOTI_ORIENT_TOP)
			elm_win_indicator_mode_set(data, ELM_WIN_INDICATOR_SHOW);
	} else {
		_tickernoti_recalc_position(data);
		evas_object_show(data);
		if (td->orient == TICKERNOTI_ORIENT_TOP)
			elm_win_indicator_mode_set(data, ELM_WIN_INDICATOR_HIDE);

		/* access */
		Evas_Object *ao;
		ao = elm_access_object_get(td->content);
		elm_access_highlight_set(ao);
	}
	td->is_show = !td->is_show;
}

Evas_Object *
tickernoti_win_add(Evas_Object *parent, char *win_name)
{
	Evas_Object *noti_win;
	struct Tickernoti_Data *td;

	noti_win = elm_win_add(parent, win_name, ELM_WIN_NOTIFICATION);
	elm_win_borderless_set(noti_win, EINA_TRUE);
	elm_win_autodel_set(noti_win, EINA_TRUE);
	elm_win_alpha_set(noti_win, EINA_TRUE);
	evas_object_size_hint_weight_set(noti_win, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	td = (struct Tickernoti_Data *) calloc(1, sizeof(struct Tickernoti_Data));
	if (!td)
		return NULL;
	evas_object_data_set(noti_win, data_key, td);
	td->angle = 0;
	td->w = 0;
	td->h = 0;
	td->orient = TICKERNOTI_ORIENT_TOP;
	td->is_show = EINA_FALSE;

	return noti_win;
}

void
tickernoti_content_set(Evas_Object *noti_win, Evas_Object *content,
		enum Noti_Orient orient)
{
	struct Tickernoti_Data *td;

	if (!noti_win)
		return;
	td = evas_object_data_get(noti_win, data_key);
	if (!td)
		return;
	if (content) {
		td->content = content;
		evas_object_size_hint_weight_set(td->content, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_win_resize_object_add(noti_win, content);
		td->orient = orient;
	}
}

void
tickernoti_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad;
	Evas_Object *layout;
	Evas_Object *btn;
	Elm_Object_Item *navi_it;
	Evas_Object *content;
	Evas_Object *icon;
	char buf[255];

	ad = (struct appdata*) data;
	if (!ad)
		return;

	//layout
	layout = elm_layout_add(ad->nf);
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "tickernoti_layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	navi_it = elm_naviframe_item_push(ad->nf, _("Tickernoti"), NULL, NULL, layout, NULL);
	elm_naviframe_item_pop_cb_set(navi_it, _pop_cb, ad->win_main);
	evas_object_show(layout);

	//content layout for tickernoti_email
	tickernoti_email = tickernoti_win_add(ad->win_main, "tickernoti_email");
	content = elm_layout_add(tickernoti_email);
	elm_layout_theme_set(content, "tickernoti", "base", "default");
	elm_object_part_text_set(content, "elm.text.sub", "Check this one");
	elm_object_text_set(content, "Edward Mikeal");
	evas_object_show(content);

	/* access */
	Evas_Object *ao;
	char *content_text;
	ao = elm_access_object_register(content, tickernoti_email);
	elm_access_info_set(ao, ELM_ACCESS_TYPE, "notification");
	content_text = evas_textblock_text_markup_to_utf8(NULL, elm_object_text_get(content));
	elm_access_info_set(ao, ELM_ACCESS_INFO, content_text);

	btn = elm_button_add(content);
	elm_object_text_set(btn, "Close");
	elm_object_part_content_set(content, "button", btn);
	evas_object_smart_callback_add(btn, "clicked", _tickernoti_show_hide, tickernoti_email);
	evas_object_show(btn);

	icon = elm_image_add(content);
	snprintf(buf, sizeof(buf), "%s/g0.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	elm_object_part_content_set(content, "icon", icon);
	evas_object_show(icon);

	//tickernoti_email
	tickernoti_content_set(tickernoti_email, content, TICKERNOTI_ORIENT_TOP);

	//content layout for tickernoti_textonly
	tickernoti_textonly = tickernoti_win_add(ad->win_main, "tickernoti_textonly");
	content = elm_layout_add(tickernoti_textonly);
	elm_layout_theme_set(content, "tickernoti", "base", "textonly");
	elm_object_text_set(content, "Text text text text text text text text text text text text text text text text text text text");
	evas_object_show(content);

	//tickernoti_textonly
	tickernoti_content_set(tickernoti_textonly, content, TICKERNOTI_ORIENT_TOP);

	//rotate notification windows
	evas_object_smart_callback_add(ad->win_main, "rotation,changed", _rotate_noti_win_cb, ad->win_main);
	_rotate_noti_win_cb(ad->win_main, ad->win_main, NULL);

	//show button
	btn = elm_button_add(layout);
	elm_object_text_set(btn, _("Show/Hide Email Notification") );
	evas_object_smart_callback_add(btn, "clicked", _tickernoti_show_hide, tickernoti_email);
	evas_object_event_callback_add(tickernoti_email, EVAS_CALLBACK_SHOW, _hide_another_tickernoti, NULL);
	elm_object_part_content_set(layout, "btn1", btn);

	btn = elm_button_add(layout);
	elm_object_text_set(btn, _("Show/Hide TextOnly Notification") );
	evas_object_smart_callback_add(btn, "clicked", _tickernoti_show_hide, tickernoti_textonly);
	evas_object_event_callback_add(tickernoti_textonly, EVAS_CALLBACK_SHOW, _hide_another_tickernoti, NULL);
	elm_object_part_content_set(layout, "btn2", btn);
}
