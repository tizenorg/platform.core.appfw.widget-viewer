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

#include <stdio.h>
#include <Ecore_X.h>

#include "elmdemo_util.h"
#include "elmdemo_test.h"

#include "fonttest.h"
#include "fonteffecttest.h"

#include "access.h"
#include "indicator.h"
#include "bubble.h"
#include "button.h"
#include "check.h"
#include "colorclass.h"
#include "colorselector.h"
#include "ctxpopup.h"
#include "datetime.h"
#include "dialoguegroup.h"
#include "drawer.h"
#include "entry.h"
#include "gengrid.h"
#include "genlist.h"
#include "handler.h"
#include "index.h"
#include "label.h"
#include "layout.h"
#include "map.h"
#include "multibuttonentry.h"
#include "naviframe.h"
#include "panes.h"
#include "popup.h"
#include "center_popup.h"
#include "progressbar.h"
#include "radio.h"
#include "searchbar.h"
#include "segmentcontrol.h"
#include "selectioninfo.h"
#include "slider.h"
#include "testmode.h"
#include "toolbar.h"
#include "transit.h"
#include "autoscroll.h"
#include "elmglviewgears.h"
#include "video.h"
#include "tickernoti.h"
#include "fastscroll.h"
#include "vi.h"
#include "floating.h"
#include "config.h"
#include "dnd.h"
#include "theme.h"
#include "wallpaper.h"
//#include "map.h"

#ifdef MTRACE
	#include <mcheck.h>
#endif


//static void _not_implemented_menu_cb(void *data, Evas_Object *obj, void *event_info);
static void _list_clicked_event(void *data, Evas_Object *obj, void *event_info);

static Evas_Object* _create_bg(Evas_Object *parent);
static Evas_Object* _create_conform(Evas_Object *parent);
static Evas_Object* _create_layout_main(Evas_Object* parent);
static Evas_Object* _create_list_winset(Evas_Object* parent, struct appdata* ad);

static Evas_Object* _create_naviframe_layout(Evas_Object* parent);
static void _create_view_layout(struct appdata *ad);
static void _set_rotation_mode_for_emulator(void *data);

static struct _menu_item tizen_menu_its[] = {
	{ "Accessibility", access_cb },
	{ "AutoScroll", autoscroll_cb },
	{ "Bubble", bubble_cb },
	{ "Button", button_cb },
	{ "Center Popup", center_popup_cb },
	{ "Check", check_cb },
	{ "ColorSelector", colorselector_cb },
	{ "CtxPopup", ctxpopup_cb },
	{ "Datetime", datetime_cb },
	{ "DialogueGroup", dialoguegroup_cb },
	{ "Drawer", drawer_cb },
	{ "Entry", entry_cb },
	{ "GLView", elmgears_cb },
	{ "Fastscroll", fastscroll_cb },
	{ "Font", fonttest_cb },
	{ "Font Effect", fonteffecttest_cb },
	{ "Gengrid", gengrid_cb },
	{ "Genlist", genlist_cb },
	{ "Handler", handler_cb },
	{ "Indicator", indicator_cb },
	{ "Label", label_cb },
	{ "Layout", layout_cb },
	{ "List", list_cb },
	{ "Map", map_cb },
	{ "MultiButtonEntry", multibuttonentry_cb },
	{ "Naviframe", naviframe_cb},
	{ "PageControl", index_cb },
	{ "Popup", popup_cb },
	{ "ProgressBar", progressbar_cb },
	{ "Radio", radio_cb },
	{ "SearchBar", searchbar_cb },
//	{ "SegmentControl", segmented_control_cb },
	{ "SelectionInfo", selectioninfo_cb },
	{ "Slider", slider_cb },
	{ "Split View", panes_cb },
	{ "Toolbar", toolbar_cb },
	{ "Transit", effect_cb },
	{ "Ticker Notification", tickernoti_cb },
	{ "Video", video_cb },
	{ "[VI]", vi_cb},
	{ "[Floating]", floating_cb},
	{ "[Config]", config_cb},
	{ "[Drag&Drop]", dnd_cb},
	{ "[Theme]", theme_cb},
	{ "[Theme Change Wallpaper]", wallpaper_theme_cb},
//	{ "[Color Class]", colorclass_cb},
	/* do not delete below */
	{ NULL, NULL }
};

static void _quit_cb(void *data, Evas_Object *obj, void *ei)
{
	/*
	//To make your application go to background,
	//Call the elm_win_lower() instead
	Evas_Object *win = (Evas_Object *) data;
	elm_win_lower(win);
	*/
	elm_exit();
}

static Eina_Bool _pop_cb(void *data, Elm_Object_Item *it)
{
	_quit_cb(NULL, NULL, NULL);
	return EINA_FALSE;
}

static void _list_clicked_event(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *it = (Elm_Object_Item *) elm_list_selected_item_get(obj);

	if (it == NULL)
	{
		fprintf((LOG_PRI(LOG_ERR) == LOG_ERR?stderr:stdout), "List item is NULL.\n");
		return;
	}

	elm_list_item_selected_set(it, EINA_FALSE);
}

static Evas_Object* _create_bg(Evas_Object *parent)
{
	Evas_Object *bg;

	if (parent == NULL) return NULL;

	bg = elm_bg_add(parent);
	evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(parent, bg);
	evas_object_show(bg);

	return bg;
}

static Evas_Object* _create_conform(Evas_Object *parent)
{
	Evas_Object *conform, *bg;

	if (parent == NULL) return NULL;

	conform = elm_conformant_add(parent);
	evas_object_size_hint_weight_set(conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(parent, conform);

	bg = elm_bg_add(conform);
	elm_object_style_set(bg, "indicator/headerbg");
	elm_object_part_content_set(conform, "elm.swallow.indicator_bg", bg);
	evas_object_show(bg);

	evas_object_show(conform);
	return conform;
}


static Evas_Object* _create_layout_main(Evas_Object* parent)
{
	Evas_Object *layout;

	if (parent == NULL) return NULL;

	layout = elm_layout_add(parent);

	if (layout == NULL)
	{
		fprintf((LOG_PRI(LOG_ERR) == LOG_ERR?stderr:stdout), "Failed elm_layout_add.\n");
		return NULL;
	}

	elm_layout_theme_set(layout, "layout", "application", "default");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	evas_object_show(layout);

	return layout;
}

static Evas_Object* _add_debug_mode(struct appdata* ad)
{
	static Eina_Bool is_debug_mode_enabled = 0;
	Elm_Object_Item *it;
	Evas_Object *list;

	if (ad == NULL) return NULL;

	it = elm_naviframe_bottom_item_get(ad->nf);
	list = elm_object_item_content_get(it);

	if (list == NULL) return NULL;
	if (is_debug_mode_enabled) return NULL;

	elm_list_item_prepend(
			list,
			"(Test Mode)",
			NULL,
			NULL,
			testmode_cb,
			ad);
	elm_list_go(list);

	is_debug_mode_enabled = 1;

	return list;
}

static Evas_Object* _create_list_winset(Evas_Object* parent, struct appdata* ad)
{
	Evas_Object *li;
	int idx = 0;
	struct _menu_item *menu_its;

	if (parent == NULL || ad == NULL) return NULL;

	li = elm_list_add(parent);
	elm_list_mode_set(li, ELM_LIST_COMPRESS);
	evas_object_smart_callback_add(li, "selected", _list_clicked_event, NULL);
	menu_its = tizen_menu_its;

	while (menu_its[ idx ].name != NULL) {
		elm_list_item_append(
				li,
				menu_its[ idx ].name,
				NULL,
				NULL,
				menu_its[ idx ].func,
				ad);
		++idx;
	}

	elm_list_go(li);

	return li;
}

static void _detect_touch_type(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	Touch_Info *ti = &(ad->ti);

	if ((ti->cur_x > ti->prev_x) && (ti->moving_type != FLICK_L2R))
	{
		ti->moving_type = FLICK_L2R;

		if (ti->magic_num == 0 || ti->magic_num == 0xEF) ti->magic_num += 0xF;
		else ti->magic_num += 0xEE;
	}
	else if ((ti->cur_x < ti->prev_x) && (ti->moving_type != FLICK_R2L))
	{
		ti->moving_type = FLICK_R2L;
		if (ti->magic_num == 0xF) ti->magic_num += 0xE0;
		else ti->magic_num += 0xFF;
	}

	ti->prev_x = ti->cur_x;
	ti->prev_y = ti->cur_y;
}

static void _mouse_down_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	Touch_Info *ti = &(ad->ti);

	Evas_Event_Mouse_Down *ev = event_info;
	Evas_Coord x, y;

	if (ev->button != 1) return;

	evas_object_geometry_get(obj, &x, &y, NULL, NULL);
	ti->prev_x = ev->canvas.x;
	ti->prev_y = ev->canvas.y;
	ti->moving_type = NONE;
}

static void _mouse_up_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	Touch_Info *ti = &(ad->ti);

	ti->prev_x = 0;
	ti->prev_y = 0;
	ti->moving_type = NONE;

	if (ti->magic_num == 0xFE || ti->magic_num == 0xFFE) ti->magic_num += 0xF00;
	else ti->magic_num = 0;

	if (ti->magic_num == 0x1EFE) _add_debug_mode(ad);
}

static void _mouse_move_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	Touch_Info *ti = &(ad->ti);

	Evas_Event_Mouse_Move *ev = event_info;
	Evas_Coord minw = 0, minh = 0;

	if (ev->buttons ^ 0x01)
		return;

	elm_coords_finger_size_adjust(1, &minw, 1, &minh);
	Evas_Coord off_x = 0;

	ti->cur_x = ev->cur.canvas.x;

	if (ti->prev_x == 0)
		return;

	off_x = ti->cur_x - ti->prev_x;

	if (off_x > minw || off_x < -minw)
		_detect_touch_type(ad);
}

static Evas_Object* _create_naviframe_layout(Evas_Object* parent)
{
	Evas_Object *nf;

	if (parent == NULL) return NULL;

	nf = elm_naviframe_add(parent);
	elm_object_part_content_set(parent, "elm.swallow.content", nf);

	evas_object_show(nf);

	return nf;
}

#ifdef DESKTOP
static Eina_Bool _key_cb(void *data, int type, void *ei)
{
	Ecore_Event_Key *ev = ei;
	struct appdata *ad = data;
	if (ev && ev->keyname) {
		if (!strcmp(ev->keyname, "F1")) {
			_elm_access_highlight_cycle(ad->win_main, ELM_FOCUS_PREVIOUS);
		} else if (!strcmp(ev->keyname, "F2")) {
			_elm_access_highlight_cycle(ad->win_main, ELM_FOCUS_NEXT);
		} else if (!strcmp(ev->keyname, "F3")) {
			// 0 means ELM_ACTIVATE_DEFAULT
			_elm_access_highlight_object_activate(ad->win_main, 0);
		} else if (!strcmp(ev->keyname, "F5")) {
			int rot = elm_win_rotation_get(ad->win_main);
			elm_win_rotation_set(ad->win_main, rot + 90);
			evas_object_resize(ad->win_main, 1280*elm_config_scale_get(), 720*elm_config_scale_get());
		}

	}
	return EINA_TRUE;
}
#endif

static void _create_view_layout(struct appdata *ad)
{
	Evas_Object *list;
	Evas_Object *btn;
	Elm_Object_Item *nf_it;

	if (ad == NULL) return;
	if (ad->nf == NULL) return;

	list = _create_list_winset(ad->win_main, ad);
	btn = elm_button_add(ad->nf);
	elm_object_style_set(btn, "naviframe/end_btn/default");
	ea_object_event_callback_add(ad->nf, EA_CALLBACK_BACK, ea_naviframe_back_cb, NULL);
	ea_object_event_callback_add(ad->nf, EA_CALLBACK_MORE, ea_naviframe_more_cb, NULL);
	nf_it = elm_naviframe_item_push(ad->nf, _("Tizen UI"), btn, NULL, list, NULL);
	elm_naviframe_item_pop_cb_set(nf_it, _pop_cb, ad->win_main);
}

int init_elmdemo_test(struct appdata *ad)
{
	if (ad == NULL) return -1;

	// Background Image
	ad->bg = _create_bg(ad->win_main);

	// Conformant
	ad->conform = _create_conform(ad->win_main);
	if (ad->conform == NULL) return -1;

	// Base Layout
	ad->layout_main = _create_layout_main(ad->conform);
	if (ad->layout_main == NULL) return -1;

	elm_object_content_set(ad->conform, ad->layout_main);

	elm_win_conformant_set(ad->win_main, EINA_TRUE);

	// Indicator
	elm_win_indicator_mode_set(ad->win_main, ELM_WIN_INDICATOR_SHOW);

	// Naviframe
	ad->nf = _create_naviframe_layout(ad->layout_main);

	// Naviframe Content
	_create_view_layout(ad);

#ifdef DESKTOP
	ecore_event_handler_add(ECORE_EVENT_KEY_DOWN, _key_cb, ad);
#endif
	evas_object_event_callback_add(ad->nf, EVAS_CALLBACK_MOUSE_DOWN, _mouse_down_cb, ad);
	evas_object_event_callback_add(ad->nf, EVAS_CALLBACK_MOUSE_UP, _mouse_up_cb,  ad);
	evas_object_event_callback_add(ad->nf, EVAS_CALLBACK_MOUSE_MOVE, _mouse_move_cb, ad);

	return 0;
}

int _lang_changed(void *data)
{
	char *locale = vconf_get_str(VCONFKEY_LANGSET);
	if (locale) elm_language_set(locale);
	return 0;
}

static void _window_resize_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	evas_object_geometry_get(ad->win_main, NULL, NULL, &ad->root_w, &ad->root_h);
}

// This is for a rotation test on the emulator(Xephyr)
static void _set_rotation_mode_for_emulator(void *data)
{
	struct appdata *ad = (struct appdata *)data;

	evas_object_geometry_get(ad->win_main, NULL, NULL, &ad->root_w, &ad->root_h);

	if (ad->root_w > ad->root_h) {
		set_portrait_mode(EINA_FALSE);
	} else {
		set_portrait_mode(EINA_TRUE);
	}
}

static struct appdata s_ad;

int demo_app_create(Evas_Object *win)
{
#if MTRACE
	printf("- Mtrace started -\n");
	mtrace();
#endif
	struct appdata *ad = &s_ad;
	char *locale;

	fprintf(stderr, "[TIME] 3. main -> app_create : %d msec\n", appcore_measure_time());
	fprintf(stdout, "Cpu count: %d\n", eina_cpu_count());

	ad->win_main = win;
	if (ad->win_main == NULL) return -1;

	evas_object_event_callback_add(ad->win_main, EVAS_CALLBACK_RESIZE, _window_resize_cb, ad);

	evas_object_show(ad->win_main);

	ad->evas = evas_object_evas_get(ad->win_main);

	locale = vconf_get_str(VCONFKEY_LANGSET);
	if (locale) elm_language_set(locale);

	_set_rotation_mode_for_emulator(ad);

	init_elmdemo_test(ad);

	fprintf(stderr, "theme : %s\n", (const char *)eina_list_data_get(elm_theme_list_get(NULL)));

	return 0;
}

/*
int app_terminate(void *data)
{
#if MTRACE
	printf("- Mtrace ended -\n");
	muntrace();
#endif
	//struct appdata *ad = data;

	return 0;
}

int app_pause(void *data)
{
	//struct appdata *ad = data;

	return 0;
}

int app_resume(void *data)
{
	//struct appdata *ad = data;

	return 0;
}

int app_reset(bundle *b, void *data)
{
	struct appdata *ad = data;

	fprintf(stderr, "[TIME] 5. app_create -> app_resume (first display) : %d msec\n", appcore_measure_time());
	fprintf(stderr, "[TIME] Total. aul_launch -> app_resume (first display) : %d msec\n", appcore_measure_time_from("APP_START_TIME"));

	if (ad->win_main)
		elm_win_activate(ad->win_main);

	return 0;
}

int main(int argc, char *argv[])
{
	struct appdata ad;
	struct appcore_ops ops = {
		.create = app_create,
		.terminate = app_terminate,
		.pause = app_pause,
		.resume = app_resume,
		.reset = app_reset,
	};

	fprintf(stderr, "[TIME] 1. aul_launch -> main : %d msec\n", appcore_measure_time_from("APP_START_TIME"));
	appcore_measure_start();

	memset(&ad, 0x0, sizeof(struct appdata));
	ops.data = &ad;

	fprintf(stderr, "[TIME] 2. main : %d msec\n", appcore_measure_time());
	appcore_measure_start();

	return appcore_efl_main(PACKAGE, &argc, &argv, &ops);
}
*/
