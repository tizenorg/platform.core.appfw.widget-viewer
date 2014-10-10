/*
 * Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *		http://www.apache.org/licenses/LICENSE-2.0
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
#include "floating.h"

#include <Ecore_X.h>

/*********************************************************
 floating test
 ********************************************************/

/* variables */
typedef struct _Floating_Info Floating_Info;
struct _Floating_Info
{
	Ecore_X_Window xwin;

	Evas_Object *tb;
	Evas_Object *rect_tl;
	Evas_Object *rect_t;
	Evas_Object *rect_tr;
	Evas_Object *rect_l;
	Evas_Object *rect_r;
	Evas_Object *rect_bl;
	Evas_Object *rect_b;
	Evas_Object *rect_br;

	Eina_Bool is_shown;
};

Floating_Info *g_floating_info;


/* functions */
 static void _show_move_resize_rect(void)
{
	if (g_floating_info)
	{
		evas_object_show(g_floating_info->tb);
	}
}

static void _hide_move_resize_rect(void)
{
	if (g_floating_info)
	{
		evas_object_hide(g_floating_info->tb);
	}
}

static void _button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!data) return;
	Evas_Object *btn = (Evas_Object*)data;

	if (g_floating_info->is_shown)
	{
		// Hide Rect
		_hide_move_resize_rect();

		//change text to SHOW
		elm_object_text_set(btn, _("Enable Move/Resize") );
	}
	else
	{
		// Show Rect
		_show_move_resize_rect();

		//change text to Hide
		elm_object_text_set(btn, _("Disable Move/Resize") );
	}

	g_floating_info->is_shown = !g_floating_info->is_shown;
}

static Evas_Object *_create_scroller(Evas_Object * parent)
{
	Evas_Object *scroller = elm_scroller_add(parent);

	elm_scroller_bounce_set(scroller, EINA_FALSE, EINA_TRUE);
	elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	evas_object_show(scroller);

	return scroller;
}

static Evas_Object *_create_button(void *data, Evas_Object *layout)
{
	struct appdata *ad;
	Evas_Object *btn;

	ad = (struct appdata *) data;
	if (ad == NULL) return NULL;

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_default", btn);
	elm_object_style_set(btn, "sweep");

	if (g_floating_info->is_shown)
	{
		elm_object_text_set(btn, _("Disable Move/Resize") );
		_show_move_resize_rect();
	}
	else
	{
		elm_object_text_set(btn, _("Enable Move/Resize") );
		_hide_move_resize_rect();
	}

	evas_object_smart_callback_add(btn, "clicked", _button_clicked_cb, btn);
	evas_object_show(btn);

	return layout;
}

static void _iconic_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!data) return;

	struct appdata *ad;
	ad = (struct appdata *) data;
	elm_win_iconified_set(ad->win_main, EINA_TRUE);
}

static Evas_Object *_create_button2(void *data, Evas_Object *layout)
{
	struct appdata *ad;
	Evas_Object *btn;

	ad = (struct appdata *) data;
	if (ad == NULL) return NULL;

	btn = elm_button_add(layout);
	elm_object_part_content_set(layout, "btn_default2", btn);
	elm_object_style_set(btn, "sweep");

	elm_object_text_set(btn, _("Change to ICONIC") );

	evas_object_smart_callback_add(btn, "clicked", _iconic_button_clicked_cb, ad);
	evas_object_show(btn);

	return layout;
}

static void _begin_act(void *data, Evas_Event_Mouse_Down *ev, int act)
{
	struct appdata *ad;
	Ecore_X_Window xwin;
	int x = 0, y = 0;

	ad = (struct appdata *) data;
	if (ad == NULL) return;

	xwin = elm_win_xwindow_get(ad->win_main);

	ecore_x_pointer_last_xy_get(&x, &y);
	ecore_x_mouse_up_send(xwin, x, y, ev->button);
	ecore_x_pointer_ungrab();

	// request to resize window
	ecore_x_netwm_moveresize_request_send(xwin, x, y, act, ev->button);
}

static void _tl_down(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	_begin_act(data, event_info, ECORE_X_NETWM_DIRECTION_SIZE_TL);
}

static void _t_down(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	_begin_act(data, event_info, ECORE_X_NETWM_DIRECTION_MOVE);
}

static void _tr_down(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	_begin_act(data, event_info, ECORE_X_NETWM_DIRECTION_SIZE_TR);
}

static void _l_down(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	_begin_act(data, event_info, ECORE_X_NETWM_DIRECTION_SIZE_L);
}

static void _r_down(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	_begin_act(data, event_info, ECORE_X_NETWM_DIRECTION_SIZE_R);
}

static void _bl_down(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	_begin_act(data, event_info, ECORE_X_NETWM_DIRECTION_SIZE_BL);
}

static void _b_down(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	_begin_act(data, event_info, ECORE_X_NETWM_DIRECTION_SIZE_B);
}

static void _br_down(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	_begin_act(data, event_info, ECORE_X_NETWM_DIRECTION_SIZE_BR);
}

static Eina_Bool _create_floating_mode_info(void *data)
{
	struct appdata  *ad;
	Evas_Object *parent;

	ad = (struct appdata *) data;
	if (ad == NULL) return EINA_FALSE;

	if (!g_floating_info)
	{
		g_floating_info = calloc(sizeof(Floating_Info), 1);
		if (!g_floating_info) return EINA_FALSE;

		g_floating_info->xwin = elm_win_xwindow_get(ad->win_main);
		g_floating_info->is_shown = EINA_TRUE;

		parent = ad->win_main;

		g_floating_info->tb = elm_table_add(parent);
		elm_win_resize_object_add(parent, g_floating_info->tb);
		evas_object_size_hint_weight_set(g_floating_info->tb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_fill_set(g_floating_info->tb, EVAS_HINT_FILL, EVAS_HINT_FILL);

		g_floating_info->rect_tl = evas_object_rectangle_add(evas_object_evas_get(parent));
		evas_object_size_hint_align_set(g_floating_info->rect_tl, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(g_floating_info->rect_tl, 0.0, 0.0);
		evas_object_size_hint_min_set(g_floating_info->rect_tl, 32, 32);
		evas_object_color_set(g_floating_info->rect_tl, 128, 0, 0, 128);
		elm_table_pack(g_floating_info->tb, g_floating_info->rect_tl, 0, 0, 1, 1);
		evas_object_event_callback_add(g_floating_info->rect_tl, EVAS_CALLBACK_MOUSE_DOWN, _tl_down, ad);
		evas_object_show(g_floating_info->rect_tl);

		g_floating_info->rect_t = evas_object_rectangle_add(evas_object_evas_get(parent));
		evas_object_size_hint_align_set(g_floating_info->rect_t, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(g_floating_info->rect_t, EVAS_HINT_EXPAND, 0.0);
		evas_object_size_hint_min_set(g_floating_info->rect_t, 32, 32);
		evas_object_color_set(g_floating_info->rect_t, 0, 64, 128, 128);
		elm_table_pack(g_floating_info->tb, g_floating_info->rect_t, 1, 0, 3, 1);
		evas_object_event_callback_add(g_floating_info->rect_t, EVAS_CALLBACK_MOUSE_DOWN, _t_down, ad);
		evas_object_show(g_floating_info->rect_t);

		g_floating_info->rect_tr = evas_object_rectangle_add(evas_object_evas_get(parent));
		evas_object_size_hint_align_set(g_floating_info->rect_tr, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(g_floating_info->rect_tr, 0.0, 0.0);
		evas_object_size_hint_min_set(g_floating_info->rect_tr, 32, 32);
		evas_object_color_set(g_floating_info->rect_tr, 128, 0, 0, 128);
		elm_table_pack(g_floating_info->tb, g_floating_info->rect_tr, 4, 0, 1, 1);
		evas_object_event_callback_add(g_floating_info->rect_tr, EVAS_CALLBACK_MOUSE_DOWN, _tr_down, ad);
		evas_object_show(g_floating_info->rect_tr);

		g_floating_info->rect_l = evas_object_rectangle_add(evas_object_evas_get(parent));
		evas_object_size_hint_align_set(g_floating_info->rect_l, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(g_floating_info->rect_l, 0.0, EVAS_HINT_EXPAND);
		evas_object_size_hint_min_set(g_floating_info->rect_l, 32, 32);
		evas_object_color_set(g_floating_info->rect_l, 128, 64, 0, 128);
		elm_table_pack(g_floating_info->tb, g_floating_info->rect_l, 0, 1, 1, 2);
		evas_object_event_callback_add(g_floating_info->rect_l, EVAS_CALLBACK_MOUSE_DOWN, _l_down, ad);
		evas_object_show(g_floating_info->rect_l);

		g_floating_info->rect_r = evas_object_rectangle_add(evas_object_evas_get(parent));
		evas_object_size_hint_align_set(g_floating_info->rect_r, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(g_floating_info->rect_r, 0.0, EVAS_HINT_EXPAND);
		evas_object_size_hint_min_set(g_floating_info->rect_r, 32, 32);
		evas_object_color_set(g_floating_info->rect_r, 128, 64, 0, 128);
		elm_table_pack(g_floating_info->tb, g_floating_info->rect_r, 4, 1, 1, 2);
		evas_object_event_callback_add(g_floating_info->rect_r, EVAS_CALLBACK_MOUSE_DOWN, _r_down, ad);
		evas_object_show(g_floating_info->rect_r);

		g_floating_info->rect_bl = evas_object_rectangle_add(evas_object_evas_get(parent));
		evas_object_size_hint_align_set(g_floating_info->rect_bl, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(g_floating_info->rect_bl, 0.0, 0.0);
		evas_object_size_hint_min_set(g_floating_info->rect_bl, 32, 32);
		evas_object_color_set(g_floating_info->rect_bl, 128, 0, 0, 128);
		elm_table_pack(g_floating_info->tb, g_floating_info->rect_bl, 0, 3, 1, 1);
		evas_object_event_callback_add(g_floating_info->rect_bl, EVAS_CALLBACK_MOUSE_DOWN, _bl_down, ad);
		evas_object_show(g_floating_info->rect_bl);

		g_floating_info->rect_b = evas_object_rectangle_add(evas_object_evas_get(parent));
		evas_object_size_hint_align_set(g_floating_info->rect_b, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(g_floating_info->rect_b, EVAS_HINT_EXPAND, 0.0);
		evas_object_size_hint_min_set(g_floating_info->rect_b, 32, 32);
		evas_object_color_set(g_floating_info->rect_b, 128, 64, 0, 128);
		elm_table_pack(g_floating_info->tb, g_floating_info->rect_b, 1, 3, 3, 1);
		evas_object_event_callback_add(g_floating_info->rect_b, EVAS_CALLBACK_MOUSE_DOWN, _b_down, ad);
		evas_object_show(g_floating_info->rect_b);

		g_floating_info->rect_br = evas_object_rectangle_add(evas_object_evas_get(parent));
		evas_object_size_hint_align_set(g_floating_info->rect_br, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(g_floating_info->rect_br, 0.0, 0.0);
		evas_object_size_hint_min_set(g_floating_info->rect_br, 32, 32);
		evas_object_color_set(g_floating_info->rect_br, 128, 0, 0, 128);
		elm_table_pack(g_floating_info->tb, g_floating_info->rect_br, 4, 3, 1, 1);
		evas_object_event_callback_add(g_floating_info->rect_br, EVAS_CALLBACK_MOUSE_DOWN, _br_down, ad);
		evas_object_show(g_floating_info->rect_br);

	}

	return EINA_TRUE;
}


static void _list_click(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *it = (Elm_Object_Item *) elm_list_selected_item_get(obj);
	if (it == NULL)
	{
		printf("list item is NULL\n");
		return;
	}

	elm_list_item_selected_set(it, EINA_FALSE);
}

static Eina_Bool _create_floating_ui(void *data)
{
	struct appdata *ad;
	Eina_Bool ret;
	Evas_Object *scroller;
	Evas_Object *layout_inner;

	ad = (struct appdata *) data;
	if (ad == NULL) return EINA_FALSE;

	ret = _create_floating_mode_info(ad);
	if (!ret) return EINA_FALSE;

	scroller = _create_scroller(ad->nf);
	elm_naviframe_item_push(ad->nf, _("Floating Mode"), NULL, NULL, scroller, NULL);

	Evas_Object *layout;

	layout = elm_layout_add(ad->nf);
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "elmdemo-test/floating");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	layout_inner = _create_button(ad, layout);
	elm_object_content_set(scroller, layout_inner);

	layout_inner = _create_button2(ad, layout);
	elm_object_content_set(scroller, layout_inner);

	return EINA_TRUE;
}

static void _use_case_floating_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad;
	Eina_Bool is_floating;
	Eina_Bool ret;

	ad = (struct appdata *) data;
	if (ad == NULL) return;

	ret = _create_floating_ui(ad);
	if (!ret) return;

	is_floating = elm_win_floating_mode_get(ad->win_main);
	if (!is_floating)
	{
		elm_win_floating_mode_set(ad->win_main, EINA_TRUE);
		evas_object_resize(ad->win_main, 500, 600);
	}
}

static void _use_case_floating_move_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad;
	Eina_Bool is_floating;
	Eina_Bool ret;

	ad = (struct appdata *) data;
	if (ad == NULL) return;

	ret = _create_floating_ui(ad);
	if (!ret) return;

	is_floating = elm_win_floating_mode_get(ad->win_main);
	if (!is_floating)
	{
		elm_win_floating_mode_set(ad->win_main, EINA_TRUE);
		evas_object_move(ad->win_main, 100, 100);
		evas_object_resize(ad->win_main, 500, 600);
	}
	else
	{
		evas_object_move(ad->win_main, 100, 100);
	}
}

static void _use_case_normal_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad;
	Eina_Bool is_floating;

	ad = (struct appdata *) data;
	if (ad == NULL) return;

	is_floating = elm_win_floating_mode_get(ad->win_main);
	if (is_floating)
	{
		elm_win_floating_mode_set(ad->win_main, EINA_FALSE);
	}
}

void floating_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *) data;
	if (ad == NULL) return;

	Evas_Object  *list;

	list = elm_list_add(ad->nf);
	elm_list_mode_set(list, ELM_LIST_COMPRESS);
	evas_object_smart_callback_add(list, "selected", _list_click, NULL);

	elm_list_item_append(list, "Floating Mode", NULL, NULL,
			_use_case_floating_cb, ad);
	elm_list_item_append(list, "Floating Mode with Move", NULL, NULL,
			_use_case_floating_move_cb, ad);
	elm_list_item_append(list, "Normal Mode", NULL, NULL,
			_use_case_normal_cb, ad);
	elm_list_go(list);

	elm_naviframe_item_push(ad->nf, _("Floating Test"), NULL, NULL, list, NULL);
}
