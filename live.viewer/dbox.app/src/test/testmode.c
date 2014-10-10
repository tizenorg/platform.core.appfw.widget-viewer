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
#include "testmode.h"
#include "test_genlist.h"
#include "test_fontchange.h"

/*********************************************************
  Test Mode
 ********************************************************/
static Evas_Object* _create_multitouch(Evas_Object* parent);
static void multitouch_cb(void *data, Evas_Object *obj, void *event_info);

static struct _menu_item menu_its[] = {
	{ "Multitouch Test", multitouch_cb },
	{ "Genlist All styles (take screenshots)", genlist_test_all_styles_cb },
	{ "Genlist Sweep with Edit mode", genlist_test_sweep_edit_cb },
	{ "Font Change", test_fontchange_cb },

	/* do not delete below */
	{ NULL, NULL }
};

static void _list_click(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *it = (Elm_Object_Item *) elm_list_selected_item_get(obj);

	if (it == NULL)
	{
		fprintf((LOG_PRI(LOG_ERR) == LOG_ERR?stderr:stdout), "List item is NULL.\n");
		return;
	}

	elm_list_item_selected_set(it, EINA_FALSE);
}


static Evas_Object* _create_list_winset(struct appdata* ad)
{
	Evas_Object *li;

	if (ad == NULL) return NULL;

	li = elm_list_add(ad->nf);
	elm_list_mode_set(li, ELM_LIST_COMPRESS);
	evas_object_smart_callback_add(li, "selected", _list_click, NULL);

	int idx = 0;

	while(menu_its[ idx ].name != NULL) {
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

static Evas_Object* _create_multitouch(Evas_Object* parent)
{
	Evas_Object *layout;

	layout = elm_layout_add(parent);
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "elmdemo-test/multi-touch");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	return layout;
}

static void
_detect_touch_type(void *data, Eina_Bool is_horizontal)
{
	struct appdata *ad = (struct appdata *) data;
	Touch_Info *ti = &(ad->ti);
	Elm_Object_Item *it = elm_naviframe_top_item_get(ad->nf);
	Evas_Object *layout = elm_object_item_content_get(it);

	if(ti->multitouch_detected && !ti->touchtype_detected)
		return;

   	elm_object_signal_emit(layout, "all,off", "elm");

	if(is_horizontal)
	{
		if(!ti->multitouch_detected)
		{
			if(ti->cur_x > ti->prev_x)
			{
				// sweep L -> R
				elm_object_signal_emit(layout, "01,on", "elm");
			}
			else if (ti->cur_x < ti->prev_x)
			{
				// sweep R -> L
				elm_object_signal_emit(layout, "02,on", "elm");
			}
		}
		else
		{
			if(ti->cur_x > ti->prev_x && ti->cur_mx > ti->prev_mx)
			{
				// multi sweep L -> R
				elm_object_signal_emit(layout, "03,on", "elm");
			}
			else if (ti->cur_x < ti->prev_x && ti->cur_mx < ti->prev_mx)
			{
				// multi sweep R -> L
			}
			else
			{
				if(((ti->cur_x - ti->cur_mx) * (ti->cur_x - ti->cur_mx) + (ti->cur_y - ti->cur_my) * (ti->cur_y - ti->cur_my))
					> ((ti->prev_x - ti->prev_mx) * (ti->prev_x - ti->prev_mx) + (ti->prev_y - ti->prev_my) * (ti->prev_y - ti->prev_my)))
				{
					// pinch out
					elm_object_signal_emit(layout, "06,on", "elm");
				}
				else
				{
					// pinch in
					elm_object_signal_emit(layout, "05,on", "elm");
				}
			}
		}
	}
	else
	{
		if(!ti->multitouch_detected)
		{
			if(ti->cur_y > ti->prev_y)
			{
				// sweep U -> D
			}
			else if (ti->cur_y < ti->prev_y)
			{
				// sweep D -> U
			}
		}
		else
		{
			if(ti->cur_y > ti->prev_y && ti->cur_my > ti->prev_my)
			{
				// multi sweep U -> D
				elm_object_signal_emit(layout, "04,on", "elm");
			}
			else if (ti->cur_y < ti->prev_y && ti->cur_my < ti->prev_my)
			{
				// multi sweep D -> U
				elm_object_signal_emit(layout, "05,on", "elm");
			}
			else
			{
				if(((ti->cur_x - ti->cur_mx) * (ti->cur_x - ti->cur_mx) + (ti->cur_y - ti->cur_my) * (ti->cur_y - ti->cur_my))
					> ((ti->prev_x - ti->prev_mx) * (ti->prev_x - ti->prev_mx) + (ti->prev_y - ti->prev_my) * (ti->prev_y - ti->prev_my)))
				{
					// pinch out
					elm_object_signal_emit(layout, "06,on", "elm");
				}
				else
				{
					// pinch in
					elm_object_signal_emit(layout, "05,on", "elm");
				}
			}
		}
	}

	ti->prev_x = ti->cur_x;
	ti->prev_y = ti->cur_y;
	ti->prev_mx = ti->cur_mx;
	ti->prev_my = ti->cur_my;
}

static void
_mouse_down_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *) data;
	Touch_Info *ti = &(ad->ti);
	Evas_Event_Mouse_Down *ev = event_info;
	Evas_Coord x, y;

	if (ev->button != 1) return;

	evas_object_geometry_get(obj, &x, &y, NULL, NULL);
	ti->prev_x = ev->canvas.x;
	ti->prev_y = ev->canvas.y;
}

static void
_mouse_up_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *) data;
	Elm_Object_Item *it = elm_naviframe_top_item_get(ad->nf);
	Evas_Object *layout = elm_object_item_content_get(it);
	Touch_Info *ti = &(ad->ti);
	ti->multitouch_detected = 0;

	ti->prev_x = 0;
	ti->prev_y = 0;
	elm_object_signal_emit(layout, "all,off", "elm");
}

static void
_mouse_move_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *) data;
	Touch_Info *ti = &(ad->ti);
	Evas_Event_Mouse_Move *ev = event_info;
	Evas_Coord minw = 0, minh = 0;

	elm_coords_finger_size_adjust(1, &minw, 1, &minh);
	Evas_Coord off_x = 0, off_y = 0;

	ti->cur_x = ev->cur.canvas.x;
	ti->cur_y = ev->cur.canvas.y;

	if(ti->prev_x * ti->prev_y == 0)
		return;

	off_x = ti->cur_x - ti->prev_x;
	off_y = ti->cur_y - ti->prev_y;

	if(off_x > minw || off_x < -minw)
	{
		_detect_touch_type(data, 1);
	}
	else if (off_y > minh || off_y < -minh)
	{
		_detect_touch_type(data, 0);
	}
}

static void
_multi_down_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *) data;
	Touch_Info *ti = &(ad->ti);
	Evas_Event_Multi_Down *ev = event_info;
	Evas_Coord x, y;

	ti->multitouch_detected = 1;

	evas_object_geometry_get(obj, &x, &y, NULL, NULL);
	ti->prev_mx = ev->canvas.x;
	ti->prev_my = ev->canvas.y;
}

static void
_multi_up_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *) data;
	Elm_Object_Item *it = elm_naviframe_top_item_get(ad->nf);
	Evas_Object *layout = elm_object_item_content_get(it);
	Touch_Info *ti = &(ad->ti);
	ti->touchtype_detected = 0;
	ti->multitouch_detected = 0;

	ti->prev_mx = 0;
	ti->prev_my = 0;
	elm_object_signal_emit(layout, "all,off", "elm");
}

static void
_multi_move_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *) data;
	Touch_Info *ti = &(ad->ti);
	Evas_Event_Multi_Move *ev = event_info;
	Evas_Coord minw = 0, minh = 0;

	elm_coords_finger_size_adjust(1, &minw, 1, &minh);
	Evas_Coord off_mx = 0, off_my = 0;

	ti->cur_mx = ev->cur.canvas.x;
	ti->cur_my = ev->cur.canvas.y;

	if(ti->prev_mx * ti->prev_my == 0)
		return;

	off_mx = ti->cur_mx - ti->prev_mx;
	off_my = ti->cur_my - ti->prev_my;


	if(off_mx > minw || off_mx < -minw)
	{
		ti->touchtype_detected = 1;
	}
	else if (off_my > minh || off_my < -minh)
	{
		ti->touchtype_detected = 1;
	}
	else
		ti->touchtype_detected = 0;

}



static void
multitouch_cb(void *data, Evas_Object *obj, void *event_info)
{
	//Evas_Object *scroller, *layout_inner;
	Evas_Object *layout_inner;
	struct appdata *ad = (struct appdata *) data;
	if(ad == NULL) return;

	layout_inner = _create_multitouch(ad->nf);
	elm_naviframe_item_push(ad->nf, _("Multitouch"), NULL,NULL, layout_inner, NULL);

	Evas_Object* rect_obj;
	rect_obj = evas_object_rectangle_add(evas_object_evas_get(ad->win_main));
	edje_object_part_swallow(elm_layout_edje_get(layout_inner), "contents_bottom", rect_obj);
	evas_object_show(rect_obj);

	evas_object_event_callback_add(rect_obj, EVAS_CALLBACK_MOUSE_DOWN, _mouse_down_cb, ad);
	evas_object_event_callback_add(rect_obj, EVAS_CALLBACK_MOUSE_UP, _mouse_up_cb, ad);
	evas_object_event_callback_add(rect_obj, EVAS_CALLBACK_MOUSE_MOVE, _mouse_move_cb, ad);
	evas_object_event_callback_add(rect_obj, EVAS_CALLBACK_MULTI_DOWN, _multi_down_cb, ad);
	evas_object_event_callback_add(rect_obj, EVAS_CALLBACK_MULTI_UP, _multi_up_cb, ad);
	evas_object_event_callback_add(rect_obj, EVAS_CALLBACK_MULTI_MOVE, _multi_move_cb, ad);

}

void
testmode_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad;
	Evas_Object *list;

	ad = (struct appdata *) data;
	if (ad == NULL) return;

	list = _create_list_winset(ad);
	elm_naviframe_item_push(ad->nf, _("Test Mode"), NULL, NULL, list, NULL);
}
