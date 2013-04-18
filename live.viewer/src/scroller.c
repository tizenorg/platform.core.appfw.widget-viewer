/*
 * Copyright 2013  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://floralicense.org
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <Elementary.h>

#include <dlog.h>

#include "util.h"
#include "live_scroller.h"
#include "scroller.h"
#include "debug.h"

#define FOCAL_DIST 800
#define FLICK_COND 100

struct cb_item {
	int (*cb)(Evas_Object *sc, void *data);
	void *data;
};

struct scroll_info {
	int locked;
	Eina_Bool scrolling;
	int focal;
	Eina_Bool quick;

	Evas_Map *map;

	Ecore_Idler *bg_changer;
	Eina_List *cb_list;
};

void scroller_lock(Evas_Object *sc)
{
	struct scroll_info *scinfo;

	scinfo = evas_object_data_get(sc, "scinfo");
	if (!scinfo) {
		ErrPrint("scinfo is not valid\n");
		return;
	}

	if (!scinfo->locked)
		live_scroller_freeze(sc);

	scinfo->locked++;
}

void scroller_unlock(Evas_Object *sc)
{
	struct scroll_info *scinfo;

	scinfo = evas_object_data_get(sc, "scinfo");
	if (!scinfo) {
		ErrPrint("scinfo is not valid\n");
		return;
	}

	if (scinfo->locked == 0)
		return;

	scinfo->locked--;

	if (scinfo->locked == 0)
		live_scroller_thaw(sc);
}

static void sc_anim_stop(void *data, Evas_Object *obj, void *event_info)
{
	Eina_List *l;
	Eina_List *tmp;
	struct cb_item *item;
	struct scroll_info *scinfo;

	scinfo = evas_object_data_get(obj, "scinfo");
	if (!scinfo) {
		ErrPrint("scinfo is not valid\n");
		return;
	}
	/*!
	 * \TODO
	 * Do what you want at here when the scroller is stopped
	 */

	scinfo->scrolling = EINA_FALSE;
	EINA_LIST_FOREACH_SAFE(scinfo->cb_list, l, tmp, item) {
		if (item->cb(obj, item->data) == ECORE_CALLBACK_CANCEL) {
			if (eina_list_data_find(scinfo->cb_list, item)) {
				scinfo->cb_list = eina_list_remove(scinfo->cb_list, item);
				free(item);
			}
		}
	}
}

static inline void sc_drag_start(void *data, Evas_Object *obj, void *event_info)
{
	struct scroll_info *scinfo;

	scinfo = evas_object_data_get(obj, "scinfo");
	if (!scinfo) {
		ErrPrint("scinfo is not valid\n");
		return;
	}

	scinfo->scrolling = EINA_TRUE;
}

static inline void sc_drag_stop(void *data, Evas_Object *scroller, void *event_info)
{
	struct live_sc_drag_info *info;
	int offset = 0;
	int ret;

	info = event_info;

	if (info->dx > FLICK_COND)
		offset = -1;
	else if (info->dx < -FLICK_COND)
		offset = 1;

	ret = live_scroller_anim_to(scroller, 0.016f, offset);
	if (ret < 0) {
		struct scroll_info *scinfo;
		scinfo = evas_object_data_get(scroller, "scinfo");
		if (scinfo)
			scinfo->scrolling = EINA_FALSE;
	}
}

static Eina_Bool bg_change_cb(void *data)
{
	Evas_Object *sc = data;
	struct scroll_info *scinfo;

	scinfo = evas_object_data_get(sc, "scinfo");
	if (scinfo)
		scinfo->bg_changer = NULL;

	/*!
	 * \note:
	 *  Here,
	 *  Filename of background image handling code is only
	 *  used to demonstrates UX concept and estimates its perfomance.
	 *  So, I'll change this if it should be appled to
	 *  main branch.
	 */
	DbgPrint("Change the background image (%p)\n", sc);
	return ECORE_CALLBACK_CANCEL;
}

static void sc_anim_start(void *data, Evas_Object *obj, void *event_info)
{
	struct scroll_info *scinfo;

	scinfo = evas_object_data_get(obj, "scinfo");
	if (!scinfo) {
		ErrPrint("scinfo is not valid\n");
		return;
	}

	/*!
	 * \note
	 * without drag,start
	 * anim start can be invoked by the scroller_anim_to
	 */
	scinfo->scrolling = EINA_TRUE;

	if (scinfo->bg_changer)
		ecore_idler_del(scinfo->bg_changer);

	scinfo->bg_changer = ecore_idler_add(bg_change_cb, obj);
	if (!scinfo->bg_changer)
		DbgPrint("Failed to add an idler\n");
}

static void sc_item_moved(void *data, Evas_Object *obj, void *event_info)
{
	struct live_sc_move_info *evt = event_info;
	int color;
	int focal;
	Evas_Coord y, sx, sw;
	double ftmp;
	struct scroll_info *scinfo;

	scinfo = evas_object_data_get(obj, "scinfo");
	if (!scinfo) {
		ErrPrint("Has no scinfo\n");
		return;
	}

	ftmp = fabsl(evt->relx);
	if (ftmp >= 1.0f) {
		evas_object_map_enable_set(evt->item, EINA_FALSE);
		evas_object_hide(evt->item);
		return;
	}

	color = 255 * (1.0f - ftmp);
	if (scinfo->quick) {
		if (color < 100)
			color = 100;

		focal = scinfo->focal;
	} else {
		if (color == 0) {
			evas_object_map_enable_set(evt->item, EINA_FALSE);
			evas_object_hide(evt->item);
			return;
		}

		focal = -ftmp * 200.0f + scinfo->focal;
	}

	evas_object_geometry_get(data, &sx, NULL, &sw, NULL);
	
	/* LEFT */
	evas_map_point_coord_set(scinfo->map, 0, evt->x, evt->y, 0);
	evas_map_point_image_uv_set(scinfo->map, 0, 0, 0);
	evas_map_point_color_set(scinfo->map, 0, color, color, color, color);

	/* RIGHT */
	evas_map_point_coord_set(scinfo->map, 1, evt->x + evt->w, evt->y, 0);
	evas_map_point_image_uv_set(scinfo->map, 1, evt->w, 0);
	evas_map_point_color_set(scinfo->map, 1, color, color, color, color);

	/* BOTTOM-RIGHT */
	evas_map_point_coord_set(scinfo->map, 2, evt->x + evt->w, evt->y + evt->h, 0);
	evas_map_point_image_uv_set(scinfo->map, 2, evt->w, evt->h);
	evas_map_point_color_set(scinfo->map, 2, color, color, color, color);

	/* BOTTOM-LEFT */
	evas_map_point_coord_set(scinfo->map, 3, evt->x, evt->y + evt->h, 0);
	evas_map_point_image_uv_set(scinfo->map, 3, 0, evt->h);
	evas_map_point_color_set(scinfo->map, 3, color, color, color, color);

	y = evt->y + (evt->h >> 1);
	evas_map_util_3d_rotate(scinfo->map, 0.0f, -30.0f * evt->relx, 0.0f, evt->x + (evt->w >> 1), y, 0);
	evas_map_util_3d_perspective(scinfo->map, sx + (sw >> 1), y, focal, FOCAL_DIST);
	evas_object_map_set(evt->item, scinfo->map);
	evas_object_map_enable_set(evt->item, EINA_TRUE);
	evas_object_show(evt->item);
	return;
}

static void sc_page_changed(void *data, Evas_Object *obj, void *event_info)
{
	DbgPrint("Page is changed %d\n", (int)event_info);
}

int scroller_add_stop_cb(Evas_Object *scroller,
			int (*cb)(Evas_Object *sc, void *data), void *data)
{
	struct cb_item *item;
	struct scroll_info *scinfo;

	scinfo = evas_object_data_get(scroller, "scinfo");
	if (!scinfo) {
		ErrPrint("scinfo is not valid\n");
		return -EINVAL;
	}

	item = calloc(1, sizeof(*item));
	if (!item) {
		ErrPrint("Error: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	item->cb = cb;
	item->data = data;

	scinfo->cb_list = eina_list_append(scinfo->cb_list, item);
	return EXIT_SUCCESS;
}

void scroller_del_stop_cb(Evas_Object *scroller,
			int (*cb)(Evas_Object *sc, void *data), void *data)
{
	struct cb_item *item;
	Eina_List *l;
	Eina_List *tmp;
	struct scroll_info *scinfo;

	scinfo = evas_object_data_get(scroller, "scinfo");
	if (!scinfo) {
		ErrPrint("Failed to get scinfo\n");
		return;
	}

	EINA_LIST_FOREACH_SAFE(scinfo->cb_list, l, tmp, item) {
		if (item->cb == cb && item->data == data) {
			scinfo->cb_list = eina_list_remove(scinfo->cb_list, item);
			free(item);
			break;
		}
	}
}

Evas_Object *scroller_create(Evas_Object *ctrl)
{
	Evas_Object *sc;
	struct scroll_info *scinfo;

	scinfo = calloc(1, sizeof(*scinfo));
	if (!scinfo) {
		ErrPrint("Heap: %s\n", strerror(errno));
		return NULL;
	}

	sc = live_scroller_add(ctrl);
	if (!sc) {
		DbgPrint("Failed to create flip object\n");
		free(scinfo);
		return NULL;
	}

	evas_object_data_set(sc, "scinfo", scinfo);

	scinfo->map = evas_map_new(4);
	if (!scinfo->map) {
		ErrPrint("Failed to create a map object\n");
		evas_object_del(sc);
		free(scinfo);
		return NULL;
	}

	evas_map_smooth_set(scinfo->map, EINA_TRUE);
	evas_map_alpha_set(scinfo->map, EINA_TRUE);

	evas_object_size_hint_weight_set(sc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_smart_callback_add(sc, "drag,start", sc_drag_start, NULL);
	evas_object_smart_callback_add(sc, "drag,stop", sc_drag_stop, NULL);
	evas_object_smart_callback_add(sc, "anim,stop", sc_anim_stop, NULL);
	evas_object_smart_callback_add(sc, "anim,start", sc_anim_start, NULL);
	evas_object_smart_callback_add(sc, "page,changed", sc_page_changed, NULL);
	evas_object_smart_callback_add(sc, "item,moved", sc_item_moved, NULL);
	live_scroller_loop_set(sc, EINA_TRUE);
	evas_object_show(sc);

	return sc;
}

int scroller_append(Evas_Object *sc, Evas_Object *child)
{
	return live_scroller_append(sc, child);
}

int scroller_get_page_index(Evas_Object *sc, Evas_Object *page)
{
	return live_scroller_get_item_index(sc, page);
}

Evas_Object *scroller_get_page(Evas_Object *sc, int idx)
{
	return live_scroller_get_item(sc, idx);
}

Evas_Object *scroller_peek_by_idx(Evas_Object *sc, int idx)
{
	return live_scroller_remove(sc, idx);
}

int scroller_peek_by_obj(Evas_Object *sc, Evas_Object *page)
{
	return live_scroller_remove_by_obj(sc, page);
}

int scroller_get_current_idx(Evas_Object *sc)
{
	return live_scroller_get_current(sc);
}

int scroller_is_scrolling(Evas_Object *sc)
{
	struct scroll_info *scinfo;

	scinfo = evas_object_data_get(sc, "scinfo");
	if (!scinfo) {
		ErrPrint("scinfo is not valid\n");
		return -EINVAL;
	}

	return scinfo->scrolling;
}

int scroller_get_page_count(Evas_Object *sc)
{
	return live_scroller_get_item_count(sc);
}

int scroller_scroll_to(Evas_Object *sc, int idx)
{
	int curidx;
	int cnt;
	register int i;
	int next_offset;
	int prev_offset;
	struct scroll_info *scinfo;

	scinfo = evas_object_data_get(sc, "scinfo");
	if (!scinfo) {
		ErrPrint("scinfo is not valid\n");
		return -EINVAL;
	}

	if (scinfo->scrolling) {
		DbgPrint("Scroller is scrolling\n");
		return -EINVAL;
	}

	curidx = live_scroller_get_current(sc);
	cnt = live_scroller_get_item_count(sc);

	i = curidx;
	next_offset = 0;
	while (i != idx && i >= 0 && i < cnt) {
		i++;
		if (i >= cnt)
			i = 0;

		next_offset++;
	}

	i = curidx;
	prev_offset = 0;
	while (i != idx && i >= 0 && i < cnt) {
		i--;
		if (i < 0)
			i = cnt - 1;

		prev_offset--;
	}

	idx = next_offset < -prev_offset ? next_offset : prev_offset;
	live_scroller_anim_to(sc, 0.016f, idx);
	return 0;
}

int scroller_jump_to(Evas_Object *sc, int idx)
{
	live_scroller_go_to(sc, idx);
	return 0;
}

int scroller_destroy(Evas_Object *sc)
{
	int cnt;
	struct scroll_info *scinfo;
	struct cb_item *item;

	scinfo = evas_object_data_del(sc, "scinfo");
	if (!scinfo)
		return -EFAULT;

	if (scinfo->bg_changer)
		ecore_idler_del(scinfo->bg_changer);

	EINA_LIST_FREE(scinfo->cb_list, item) {
		free(item);
	}

	cnt = live_scroller_get_item_count(sc);
	if (cnt)
		DbgPrint("Children is not cleared (%d)\n", cnt);

	evas_object_del(sc);
	evas_map_free(scinfo->map);
	free(scinfo);
	return 0;
}

int scroller_update(Evas_Object *sc, void *data)
{
	struct scroll_info *scinfo;

	scinfo = evas_object_data_get(sc, "scinfo");
	if (!scinfo) {
		ErrPrint("scinfo is not valid\n");
		return -EFAULT;
	}

	scinfo->focal = (int)data;
	live_scroller_update(sc);
	return EXIT_SUCCESS;
}

int scroller_fast_scroll(Evas_Object *sc, int idx)
{
	idx -= scroller_get_current_idx(sc);
	live_scroller_anim_to(sc, 0.016f, idx);
	return 0;
}

void scroller_loop_set(Evas_Object *sc, Eina_Bool val)
{
	live_scroller_loop_set(sc, val);
}

void scroller_quick_navi(Evas_Object *sc, Eina_Bool val)
{
	struct scroll_info *scinfo;
	scinfo = evas_object_data_get(sc, "scinfo");
	if (!scinfo) {
		ErrPrint("scinfo is not valid\n");
		return;
	}

	scinfo->quick = val;
}

/* End of a file */
