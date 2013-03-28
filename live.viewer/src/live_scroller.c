/*
 * Copyright 2013  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.tizenopensource.org/license
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <Elementary.h>

#include <dlog.h>

#include "live_scroller.h"
#include "util.h"
#include "debug.h"

#define SAMPLE_MAX 10
#define EVT_PERIOD 0.016 /* 60 fps */
#define EVT_SAMPLE_PERIOD 9
#define DRAG_SENS 100
#define ANIM_MIN 40
#define ANIM_UNIT 15

struct item_list_entry {
	struct item_list_entry *prev;
	struct item_list_entry *next;

	Evas_Object *data;
	Evas_Coord x;
	Evas_Coord y;
	Evas_Coord w;
	Evas_Coord h;
};

struct evt_info {
	Evas_Coord x;
	unsigned int timestamp;
};

struct evt_queue {
	struct evt_info ei[SAMPLE_MAX];
	int front;
	int rear;
	int cnt;
	unsigned int old_timestamp;
};

struct widget_data {
	Eina_Bool is_loop;
	Eina_Bool is_freezed;

	struct item_list_entry *item_list;

	int item_cnt;
	struct item_list_entry *curlist;
	struct item_list_entry *tolist;

	Eina_Bool drag_started;
	Eina_Bool is_pressed;
	Evas_Coord press_x;
	Evas_Coord press_y;

	Ecore_Timer *sc_anim_timer;
	int sc_anim_dx;

	Evas_Object *clip;
	Evas_Object *evt_layer;
	Evas_Object *scroller;

	Evas_Coord clip_bx;
	Evas_Coord clip_bw;

	struct evt_queue evtq;
	Ecore_Timer *evt_emulator;
	Evas_Coord old_x;
	unsigned int prev_timestamp;
};

#ifdef PROFILE
#define PROFILE_START() \
static int _exec_cnt = 0; \
struct timeval _stv, _etv; \
long _elapsed; \
gettimeofday(&_stv, NULL);

#define PROFILE_END() \
do { \
	_exec_cnt++; \
	gettimeofday(&_etv, NULL); \
	_elapsed = (_etv.tv_sec - _stv.tv_sec) * 1000000l + (_etv.tv_usec - _stv.tv_usec); \
	DbgPrint("[%d] Elapsed time: %lu\n", _exec_cnt, _elapsed); \
} while (0)
#else
#define PROFILE_START()
#define PROFILE_END()
#endif

#define LIST_NEXT(list)	((list)->next)
#define LIST_PREV(list)	((list)->prev)
#define LIST_DATA(list)	((list) ? (list)->data : NULL)

static inline void LIST_ITEM_GEO_SET(struct item_list_entry *list, int x, int y, int w, int h)
{
	list->x = x;
	list->y = y;
	list->w = w;
	list->h = h;
}

static inline void LIST_ITEM_GEO_GET(struct item_list_entry *list, int *x, int *y, int *w, int *h)
{
	if (x)
		*x = list->x;
	if (y)
		*y = list->y;
	if (w)
		*w = list->w;
	if (h)
		*h = list->h;
}

static inline struct item_list_entry *list_item_append(struct item_list_entry *list, void *obj)
{
	struct item_list_entry *item;

	item = malloc(sizeof(*item));
	if (!item)
		return NULL;

	item->data = obj;

	if (list) {
		list->prev->next = item;
		item->prev = list->prev;

		item->next = list;
		list->prev = item;
	} else{
		item->prev = item;
		item->next = item;
		list = item;
	}

	return list;
}

static inline void *list_item_nth(struct item_list_entry *list, int idx)
{
	if (!list)
		return NULL;

	while (--idx >= 0)
		list = list->next;

	return list->data;
}

static inline struct item_list_entry *list_item_nth_list(struct item_list_entry *list, int idx)
{
	if (!list)
		return NULL;

	while (--idx >= 0)
		list = list->next;

	return list;
}

static inline struct item_list_entry *list_item_find(struct item_list_entry *list, void *data)
{
	struct item_list_entry *item;

	if (!list)
		return NULL;

	item = list;
	do {
		if (LIST_DATA(item) == data)
			return item;

		item = LIST_NEXT(item);
	} while (item != list);

	return NULL;
}

static inline struct item_list_entry *list_item_remove(struct item_list_entry *list, void *data)
{
	struct item_list_entry *item;

	if (!list) {
		ErrPrint("List is not valid\n");
		return NULL;
	}

	DbgPrint("Start\n");
	item = list;
	do {
		if (LIST_DATA(item) == data) {
			DbgPrint("ITEM is removed\n");
			if (item == list) {
				if (list == LIST_NEXT(list))
					list = NULL;
				else
					list = LIST_NEXT(list);
			}

			item->prev->next = item->next;
			item->next->prev = item->prev;
			free(item);
			break;
		}

		item = LIST_NEXT(item);
	} while (item != list);
	DbgPrint("End\n");

	return list;
}

static inline void *list_item_last(struct item_list_entry *list)
{
	if (!list)
		return NULL;

	return list->prev->data;
}

static inline struct item_list_entry *list_item_last_list(struct item_list_entry *list)
{
	if (!list)
		return NULL;

	return list->prev;
}

static inline int list_item_count(struct item_list_entry *list)
{
	struct item_list_entry *n;
	int cnt;

	if (!list)
		return 0;

	cnt = 0;
	n = list;
	do {
		cnt++;
		n = LIST_NEXT(n);
	} while (n != list);

	return cnt;
}

static inline int list_item_idx(struct widget_data *sc_data, struct item_list_entry *ilist)
{
	int idx;
	idx = 0;

	while (ilist != sc_data->item_list) {
		idx++;
		ilist = LIST_PREV(ilist);
	}

	return idx;
}

static inline void init_evtq(struct evt_queue *evtq)
{
	evtq->front = 0;
	evtq->rear = 0;
	evtq->cnt = 0;
	evtq->old_timestamp = 0;
}

static inline void dq_evt(struct evt_queue *evtq)
{
	if (evtq->cnt <= 0)
		return;
	evtq->front++;
	if (evtq->front >= SAMPLE_MAX)
		evtq->front -= SAMPLE_MAX;
	evtq->cnt--;
}

static inline void enq_evt(struct evt_queue *evtq, Evas_Coord x, unsigned int timestamp)
{
	unsigned int t_diff;
	int replace;

	replace = 0;
	t_diff = timestamp - evtq->old_timestamp;

	if (evtq->cnt <= 0)
		evtq->old_timestamp = timestamp;
	else if (t_diff > EVT_SAMPLE_PERIOD)
		evtq->old_timestamp += EVT_SAMPLE_PERIOD * (t_diff / EVT_SAMPLE_PERIOD);
	else
		replace = 1;

	if (!replace) {
		if (evtq->cnt >= SAMPLE_MAX)
			dq_evt(evtq);
		evtq->rear++;
		if (evtq->rear >= SAMPLE_MAX)
			evtq->rear -= SAMPLE_MAX;
		evtq->cnt++;
	}

	evtq->ei[evtq->rear].x = x;
	evtq->ei[evtq->rear].timestamp = evtq->old_timestamp;
}

static inline Evas_Coord get_evt_avg(struct evt_queue *evtq)
{
	int i;
	int rear;
	Evas_Coord x;
	int weight;
	int t;

	t = (int)(ecore_time_get() * 1000);
	rear = evtq->rear;

	x = 0;
	for (i = 0; i < evtq->cnt; i += weight) {
		weight = (t - evtq->ei[rear].timestamp) / EVT_SAMPLE_PERIOD;
		if (weight > (evtq->cnt - i))
			weight = evtq->cnt - i;
		else
			weight = 1;

		x += evtq->ei[rear].x * weight;
		t = evtq->ei[rear].timestamp;
		rear--;
		if (rear < 0)
			rear += SAMPLE_MAX;
	}

	x /= evtq->cnt;
	return x;
}

/* Move the item to given direction to fit its coordinates to border */
static inline int calc_anim_dx_with_dir(struct widget_data *sc_data, int *dir)
{
	Evas_Coord x, w;

	LIST_ITEM_GEO_GET(sc_data->curlist, &x, NULL, &w, NULL);
	sc_data->sc_anim_dx = 0;

	if (*dir < 0) {
		DbgPrint("MOVE to LEFT\n");
		if (x < sc_data->clip_bx) {
			(*dir)++;

			if (sc_data->tolist == sc_data->item_list) {
				if (!sc_data->is_loop) {
					*dir = 0;
					DbgPrint("Looping is disabled\n");
					return -EINVAL;
				}
			}

			sc_data->tolist = LIST_PREV(sc_data->tolist);
			sc_data->sc_anim_dx = sc_data->clip_bx - x /*- w*/;
		} else {
			sc_data->sc_anim_dx = sc_data->clip_bx - x;
		}
	} else if (*dir > 0) {
		DbgPrint("MOVE to RIGHT\n");
		if (x < sc_data->clip_bx) {
			sc_data->sc_anim_dx = sc_data->clip_bx - x;
		} else if (x > sc_data->clip_bx) {
			struct item_list_entry *newlist;

			(*dir)--;
			newlist = LIST_NEXT(sc_data->tolist);
			if (newlist == sc_data->item_list) {
				if (!sc_data->is_loop) {
					*dir = 0;
					DbgPrint("Looping is disabled\n");
					return -EINVAL;
				}
			}
			sc_data->tolist = newlist;
			sc_data->sc_anim_dx = sc_data->clip_bx - x; /*(sc_data->clip_bx + sc_data->clip_bw) - x;*/
		}
	}

	return 0;
}

static inline void move_item(struct widget_data *sc_data, struct item_list_entry *ilist, int x, int y, int w, int h)
{
	struct live_sc_move_info info;

	info.item = LIST_DATA(ilist);

	info.relx = ((double)x - (double)sc_data->clip_bx) / (double)sc_data->clip_bw;

	LIST_ITEM_GEO_SET(ilist, x, y, w, h);
	info.x = x;
	info.y = y;
	info.w = w;
	info.h = h;

	evas_object_smart_callback_call(sc_data->scroller, "item,moved", &info);
}

static struct item_list_entry *update_items_geo(struct widget_data *sc_data, int dx)
{
	Evas_Coord sx, sw;
	Evas_Coord y, w, h;
	struct item_list_entry *ilist;
	struct item_list_entry *newlist;
	struct item_list_entry *boundary;
	int bx_bw;
	register int x;

	LIST_ITEM_GEO_GET(sc_data->curlist, &sx, &y, &sw, &h);

	bx_bw = sc_data->clip_bx + sc_data->clip_bw;

	sx += dx;
	move_item(sc_data, sc_data->curlist, sx, y, sw, h);

	newlist = NULL;

	if (sc_data->item_cnt < 3) {
		ilist = LIST_NEXT(sc_data->curlist);
		LIST_ITEM_GEO_GET(ilist, NULL, &y, &w, &h);

		if (sx + sw < bx_bw) {
			x = sx + sw;
			move_item(sc_data, ilist, x, y, w, h);
			if (x == sc_data->clip_bx || (x < sc_data->clip_bx && (x + w) > sc_data->clip_bx))
				newlist = ilist;
		} else if (sx > 0) {
			x = sx - w;
			move_item(sc_data, ilist, x, y, w, h);
			if (x == sc_data->clip_bx || (x > sc_data->clip_bx && x < bx_bw)) {
				newlist = ilist;
			}
		}

		goto out;
	}

	x = sx;
	boundary = NULL;
	ilist = sc_data->curlist;
	do {
		if (!sc_data->is_loop && ilist == sc_data->item_list)
			break;

		ilist = LIST_PREV(ilist);
		if (!ilist)
			break;

		LIST_ITEM_GEO_GET(ilist, NULL, &y, &w, &h);
		x -= w;
		move_item(sc_data, ilist, x, y, w, h);

		if (dx > 0 && !newlist) {
			if ((x == sc_data->clip_bx) || (x > sc_data->clip_bx && x < bx_bw))
				newlist = ilist;
		}

		boundary = ilist;
	} while (x > sc_data->clip_bx);

	x = sx;
	w = sw;
	ilist = sc_data->curlist;
	do {
		ilist = LIST_NEXT(ilist);
		if (!ilist || (!sc_data->is_loop && ilist == sc_data->item_list) || ilist == boundary)
			break;

		x += w;
		LIST_ITEM_GEO_GET(ilist, NULL, &y, &w, &h);
		move_item(sc_data, ilist, x, y, w, h);

		if (dx < 0 && !newlist) {
			if ((x == sc_data->clip_bx) || (x < sc_data->clip_bx && (x + w) > sc_data->clip_bx))
				newlist = ilist;
		}
	} while (x < bx_bw);

out:
	if (newlist)
		sc_data->curlist = newlist;
		
	return newlist;
}

static Eina_Bool emulate_evt(void *data)
{
	PROFILE_START();
	struct widget_data *sc_data;
	Evas_Coord x;
	Evas_Coord dx;
	struct item_list_entry *newlist;

	sc_data = data;

	x = get_evt_avg(&sc_data->evtq);
	if (x == sc_data->old_x) {
		PROFILE_END();
		return ECORE_CALLBACK_RENEW;
	}
	
	dx = x - sc_data->old_x;
	sc_data->old_x = x;

	newlist = update_items_geo(sc_data, dx);
	if (newlist) {
		int idx;

		idx = list_item_idx(sc_data, newlist);
		evas_object_smart_callback_call(sc_data->scroller, "page,changed", (void *)idx);
	}
	PROFILE_END();
	return ECORE_CALLBACK_RENEW;
}

static void evt_mouse_down_cb(void *data, Evas *e, Evas_Object *evt_layer, void *event_info)
{
	Evas_Event_Mouse_Down *down;
	struct widget_data *sc_data;

	sc_data = data;
	down = event_info;

	sc_data->is_pressed = EINA_TRUE;
	sc_data->press_x = down->canvas.x;
	sc_data->press_y = down->canvas.y;
	sc_data->old_x = down->canvas.x;

	if (!sc_data->is_freezed) {
		init_evtq(&sc_data->evtq);
		enq_evt(&sc_data->evtq, down->canvas.x, down->timestamp);
	}
}

static void evt_mouse_up_cb(void *data, Evas *e, Evas_Object *evt_layer, void *event_info)
{
	Evas_Event_Mouse_Up *up;
	struct widget_data *sc_data;
	struct live_sc_drag_info info;

	sc_data = data;

	if (!sc_data->is_pressed)
		return;

	sc_data->is_pressed = EINA_FALSE;

	if (sc_data->evt_emulator) {
		ecore_timer_del(sc_data->evt_emulator);
		sc_data->evt_emulator = NULL;
	}

	if (sc_data->drag_started == EINA_FALSE) {
		DbgPrint("drag is not started\n");
		return;
	}

	up = event_info;

	info.dx = up->canvas.x - sc_data->press_x;
	info.dy = up->canvas.y - sc_data->press_y;

	sc_data->drag_started = EINA_FALSE;

	evas_object_smart_callback_call(sc_data->scroller, "drag,stop", &info);
}

static void evt_mouse_move_cb(void *data, Evas *e, Evas_Object *evt_layer, void *event_info)
{
	struct widget_data *sc_data;
	Evas_Event_Mouse_Move *move;

	sc_data = data;
	
	if (sc_data->is_pressed == EINA_FALSE)
		return;

	if (sc_data->item_cnt <= 1)
		return;

	if (sc_data->is_freezed)
		return;

	move = event_info;

	if (sc_data->drag_started == EINA_FALSE) {
		if (abs(move->cur.canvas.x - sc_data->press_x) < DRAG_SENS)
			return;

		if (sc_data->sc_anim_timer) {
			ecore_timer_del(sc_data->sc_anim_timer);
			sc_data->sc_anim_timer = NULL;
		}

		evas_object_smart_callback_call(sc_data->scroller, "drag,start", NULL);
		sc_data->drag_started = EINA_TRUE;
	}

	sc_data->prev_timestamp = move->timestamp;
	enq_evt(&sc_data->evtq, move->cur.canvas.x, move->timestamp);
	if (!sc_data->evt_emulator) {
		if (!sc_data->curlist)
			sc_data->curlist = sc_data->item_list;

		sc_data->evt_emulator = ecore_timer_add(EVT_PERIOD, emulate_evt, sc_data);
	}
}

static inline int prepare_evt_layer(struct widget_data *sc_data)
{
	Evas *e;

	e = evas_object_evas_get(sc_data->scroller);
	if (!e)
		return -EFAULT;

	sc_data->evt_layer = evas_object_rectangle_add(e);
	if (!sc_data->evt_layer)
		return -EFAULT;

	evas_object_smart_member_add(sc_data->evt_layer, sc_data->scroller);

	evas_object_color_set(sc_data->evt_layer, 255, 255, 255, 0);
	evas_object_show(sc_data->evt_layer);
	evas_object_repeat_events_set(sc_data->evt_layer, EINA_TRUE);

	evas_object_event_callback_add(sc_data->evt_layer,
			EVAS_CALLBACK_MOUSE_DOWN, evt_mouse_down_cb, sc_data);

	evas_object_event_callback_add(sc_data->evt_layer,
			EVAS_CALLBACK_MOUSE_UP, evt_mouse_up_cb, sc_data);

	evas_object_event_callback_add(sc_data->evt_layer,
			EVAS_CALLBACK_MOUSE_MOVE, evt_mouse_move_cb, sc_data);

	evas_object_clip_set(sc_data->evt_layer, sc_data->clip);
	return 0;
}

static void live_add(Evas_Object *scroller)
{
	struct widget_data *sc_data;
	Evas *e;
	int ret;

	sc_data = calloc(1, sizeof(*sc_data));
	if (!sc_data)
		return;

	e = evas_object_evas_get(scroller);
	if (!e) {
		free(sc_data);
		return;
	}

	evas_object_smart_data_set(scroller, sc_data);

	sc_data->clip = evas_object_rectangle_add(e);
	if (!sc_data->clip) {
		free(sc_data);
		return;
	}

	sc_data->is_pressed = EINA_FALSE;
	sc_data->drag_started = EINA_FALSE;
	sc_data->tolist = NULL;
	sc_data->curlist = NULL;
	sc_data->item_list = NULL;
	sc_data->scroller = scroller;

	evas_object_smart_member_add(sc_data->clip, sc_data->scroller);

	ret = prepare_evt_layer(sc_data);
	if (ret < 0) {
		evas_object_del(sc_data->clip);
		free(sc_data);
	}

	return;
}

static void live_del(Evas_Object *scroller)
{
	struct widget_data *sc_data;
	Evas_Object *item;
	struct item_list_entry *ilist;
	struct item_list_entry *next;

	sc_data = evas_object_smart_data_get(scroller);
	if (!sc_data) {
		ErrPrint("sc_data is not valid\n");
		return;
	}

	ilist = sc_data->item_list;
	if (ilist) {
		do {
			next = LIST_NEXT(ilist);
			item = LIST_DATA(ilist);
			evas_object_clip_unset(item);
			evas_object_smart_member_del(item);
			free(ilist);
			ilist = next;
		} while (ilist != sc_data->item_list);
	}

	evas_object_del(sc_data->evt_layer);
	evas_object_del(sc_data->clip);
	free(sc_data);
}

static void live_move(Evas_Object *scroller, Evas_Coord bx, Evas_Coord by)
{
	struct widget_data *sc_data;
	Evas_Coord x, y, w, h;
	Evas_Coord bw;

	Evas_Coord dx;
	Evas_Coord dy;

	struct item_list_entry *n;

	sc_data = evas_object_smart_data_get(scroller);
	if (!sc_data) {
		ErrPrint("sc_data is not valid\n");
		return;
	}

	evas_object_geometry_get(sc_data->clip, &x, &y, &bw, NULL);

	evas_object_move(sc_data->evt_layer, bx, by);
	evas_object_move(sc_data->clip, bx, by);
	sc_data->clip_bx = bx;
	sc_data->clip_bw = bw;

	dx = bx - x;
	dy = by - y;

	if (sc_data->item_list) {
		n = sc_data->item_list;
		do {
			evas_object_move(LIST_DATA(n), bx, by);

			LIST_ITEM_GEO_GET(n, &x, &y, &w, &h);
			x += dx;
			y += dy;
			move_item(sc_data, n, x, y, w, h);
			n = LIST_NEXT(n);
		} while (n != sc_data->item_list);
	}
}

static void live_resize(Evas_Object *scroller, Evas_Coord w, Evas_Coord h)
{
	struct widget_data *sc_data;

	sc_data = evas_object_smart_data_get(scroller);
	if (!sc_data) {
		ErrPrint("sc_data is not valid\n");
		return;
	}

	evas_object_resize(sc_data->clip, w, h);
	evas_object_resize(sc_data->evt_layer, w, h);

	sc_data->clip_bw = w;
}

static void live_show(Evas_Object *scroller)
{
	struct widget_data *sc_data;

	sc_data = evas_object_smart_data_get(scroller);
	if (!sc_data) {
		ErrPrint("sc_data is not valid\n");
		return;
	}

	evas_object_show(sc_data->clip);
}

static void live_hide(Evas_Object *scroller)
{
	struct widget_data *sc_data;

	sc_data = evas_object_smart_data_get(scroller);
	if (!sc_data) {
		ErrPrint("sc_data is not valid\n");
		return;
	}

	evas_object_hide(sc_data->clip);
}

static void live_set_color(Evas_Object *scroller, int r, int g, int b, int a)
{
	struct widget_data *sc_data;

	sc_data = evas_object_smart_data_get(scroller);
	if (!sc_data) {
		ErrPrint("sc_data is not valid\n");
		return;
	}

	evas_object_color_set(sc_data->clip, r, g, b, a);
}

static void live_set_clip(Evas_Object *scroller, Evas_Object *clip)
{
	struct widget_data *sc_data;

	sc_data = evas_object_smart_data_get(scroller);
	if (!sc_data) {
		ErrPrint("sc_data is not valid\n");
		return;
	}

	evas_object_clip_set(sc_data->clip, clip);
}

static void live_unset_clip(Evas_Object *scroller)
{
	struct widget_data *sc_data;

	sc_data = evas_object_smart_data_get(scroller);
	if (!sc_data) {
		ErrPrint("sc_data is not valid\n");
		return;
	}

	evas_object_clip_unset(sc_data->clip);
}

static inline void rearrange_items(struct widget_data *sc_data)
{
	struct item_list_entry *ilist;
	Evas_Coord x, y, w, h;
	Evas_Coord sw;

	LIST_ITEM_GEO_GET(sc_data->curlist, NULL, &y, &sw, &h);
	move_item(sc_data, sc_data->curlist, sc_data->clip_bx, y, sw, h);

	x = sc_data->clip_bx;
	ilist = sc_data->curlist;
	while (ilist != sc_data->item_list) {
		ilist = LIST_PREV(ilist);
		LIST_ITEM_GEO_GET(ilist, NULL, &y, &w, &h);
		x -= w;
		move_item(sc_data, ilist, x, y, w, h);
	}

	w = sw;
	x = sc_data->clip_bx;
	ilist = LIST_NEXT(sc_data->curlist);
	while (ilist != sc_data->item_list) {
		x += w;
		LIST_ITEM_GEO_GET(ilist, NULL, &y, &w, &h);
		move_item(sc_data, ilist, x, y, w, h);
		ilist = LIST_NEXT(ilist);
	}
}

static Eina_Bool sc_anim_cb(void *data)
{
	PROFILE_START();
	struct widget_data *sc_data;
	Evas_Coord sx, sw;
	Evas_Coord y;
	Evas_Coord dx;
	struct item_list_entry *ilist;

	sc_data = data;

	if (!sc_data->curlist || !sc_data->tolist) {
		DbgPrint("cur_list: %p, tolist: %p\n", sc_data->curlist, sc_data->tolist);
		goto clean_out;
	}

	ilist = sc_data->curlist;
	if (sc_data->curlist != sc_data->tolist) {
		if (sc_data->sc_anim_dx > 0)
			ilist = LIST_PREV(ilist);
		else
			ilist = LIST_NEXT(ilist);
	}

	LIST_ITEM_GEO_GET(ilist, &sx, &y, &sw, NULL);
	if (ilist == sc_data->tolist) {
		DbgPrint("next list == tolist\n");
		dx = abs(sx - sc_data->clip_bx);
		DbgPrint("sx: %d, clip_bx: %d --> %d\n", sx, sc_data->clip_bx, dx);
		if (dx < abs(sc_data->sc_anim_dx)) {
			if (sc_data->sc_anim_dx < 0)
				dx = -dx;
		} else {
			dx = sc_data->sc_anim_dx;
		}
	} else {
		dx = sc_data->sc_anim_dx;
		DbgPrint("dx: %d\n", dx);
	}

	if (!dx) {
		DbgPrint("dx is 0\n");
		goto clean_out;
	}

	ilist = update_items_geo(sc_data, dx);
	if (ilist) {
		int idx;

		idx = list_item_idx(sc_data, ilist);
		evas_object_smart_callback_call(sc_data->scroller,"page,changed", (void *)idx);
	}
	PROFILE_END();
	return ECORE_CALLBACK_RENEW;

clean_out:
	PROFILE_END();
	evas_object_smart_callback_call(sc_data->scroller, "anim,stop", NULL);
	sc_data->sc_anim_timer = NULL;
	return ECORE_CALLBACK_CANCEL;
}

Evas_Object *live_scroller_add(Evas_Object *parent)
{
	static Evas_Smart_Class sc = EVAS_SMART_CLASS_INIT_NAME_VERSION("live,scroller");
	static Evas_Smart *smart = NULL;
	Evas_Object *scroller;
	Evas *e;

	if (!parent)
		return NULL;

	e = evas_object_evas_get(parent);
	if (!e)
		return NULL;

	if (!smart) {
		sc.add = live_add;
		sc.del = live_del;
		sc.move = live_move;
		sc.resize = live_resize;
		sc.show = live_show;
		sc.hide = live_hide;
		sc.color_set = live_set_color;
		sc.clip_set = live_set_clip;
		sc.clip_unset = live_unset_clip;

		smart = evas_smart_class_new(&sc);
	}

	scroller = evas_object_smart_add(e, smart);

	return scroller;
}

int live_scroller_append(Evas_Object *scroller, Evas_Object *item)
{
	Evas_Coord x, y, w, h;
	Evas_Coord bx, by, bw, bh;
	struct widget_data *sc_data;
	struct item_list_entry *tmplist;
	Evas_Coord start_x = 0;
	Evas_Coord start_w = 0;

	sc_data = evas_object_smart_data_get(scroller);
	if (!sc_data) {
		ErrPrint("sc_data is not valid\n");
		return -EINVAL;
	}

	evas_object_geometry_get(sc_data->clip, &bx, &by, &bw, &bh);

	if (sc_data->item_list)
		LIST_ITEM_GEO_GET(sc_data->item_list, &start_x, NULL, &start_w, NULL);

	tmplist = list_item_last_list(sc_data->item_list);
	if (tmplist) {
		LIST_ITEM_GEO_GET(tmplist, &x, NULL, &w, NULL);
		if (x + w == start_x) {
			x = start_x - w;
		} else {
			x += w;
		}
	} else {
		x = bx;
	}
	
	evas_object_geometry_get(item, NULL, NULL, &w, &h);
	evas_object_smart_member_add(item, sc_data->scroller);

	y = by + ((bh - h) >> 1);

	tmplist = list_item_append(sc_data->item_list, item);
	if (sc_data->item_list == sc_data->curlist)
		sc_data->curlist = tmplist;
	if (sc_data->item_list == sc_data->tolist)
		sc_data->tolist = tmplist;
	sc_data->item_list = tmplist;

	sc_data->item_cnt++;
	evas_object_clip_set(item, sc_data->clip);
	evas_object_stack_below(item, sc_data->clip);

	evas_object_move(item, bx, by);
	move_item(sc_data, list_item_find(sc_data->item_list, item), x, y, w, h);

	return 0;
}

int live_scroller_remove_by_obj(Evas_Object *scroller, Evas_Object *obj)
{
	struct widget_data *sc_data;
	struct item_list_entry *tmplist;
	Evas_Object *item;

	sc_data = evas_object_smart_data_get(scroller);
	if (!sc_data) {
		ErrPrint("sc_data is not valid\n");
		return -EINVAL;
	}

	if (sc_data->curlist) {
		if (LIST_DATA(sc_data->curlist) == obj) {
			sc_data->curlist = sc_data->item_list;
			DbgPrint("Reset curlist\n");
		}
	}

	if (sc_data->tolist) {
		if (LIST_DATA(sc_data->tolist) == obj) {
			sc_data->tolist = sc_data->item_list;
			DbgPrint("Reset tolist\n");
		}
	}

	tmplist = list_item_remove(sc_data->item_list, obj);
	if (sc_data->item_list == sc_data->curlist) {
		sc_data->curlist = tmplist;
		DbgPrint("Update curlist\n");
	}
	if (sc_data->item_list == sc_data->tolist) {
		sc_data->tolist = tmplist;
		DbgPrint("Update tolist\n");
	}
	sc_data->item_list = tmplist;

	sc_data->item_cnt--;
	evas_object_clip_unset(obj);
	evas_object_smart_member_del(obj);
	DbgPrint("Count of items: %d\n", sc_data->item_cnt);

	item = LIST_DATA(sc_data->curlist);
	if (item) {
		Evas_Coord y, w, h;
		int idx;

		LIST_ITEM_GEO_GET(sc_data->curlist, NULL, &y, &w, &h);
		DbgPrint("Current GEO: %d, %dx%d\n", y, w, h);
		LIST_ITEM_GEO_SET(sc_data->curlist, sc_data->clip_bx, y, w, h);
		DbgPrint("sc_data->curlist : %p\n", sc_data->curlist);
		update_items_geo(sc_data, 0);
		DbgPrint("sc_data->curlist : %p\n", sc_data->curlist);

		idx = list_item_idx(sc_data, sc_data->curlist);
		evas_object_smart_callback_call(sc_data->scroller, "page,changed", (void *)idx);
	}

	return 0;
}

Evas_Object *live_scroller_remove(Evas_Object *scroller, int idx)
{
	struct widget_data *sc_data;
	struct item_list_entry *tmplist;
	Evas_Object *ret;
	Evas_Object *item;

	sc_data = evas_object_smart_data_get(scroller);
	if (!sc_data) {
		ErrPrint("sc_data is not valid\n");
		return NULL;
	}

	if (idx < 0 || idx >= sc_data->item_cnt)
		return NULL;

	ret = list_item_nth(sc_data->item_list, idx);
	if (!ret)
		return NULL;

	if (list_item_nth_list(sc_data->item_list, idx) == sc_data->curlist) {
		DbgPrint("Reset curlist\n");
		sc_data->curlist = sc_data->item_list;
	}

	if (list_item_nth_list(sc_data->item_list, idx) == sc_data->tolist) {
		DbgPrint("Reset tolist\n");
		sc_data->tolist = sc_data->item_list;
	}

	tmplist = list_item_remove(sc_data->item_list, ret);
	if (sc_data->item_list == sc_data->curlist)
		sc_data->curlist = tmplist;
	if (sc_data->item_list == sc_data->tolist)
		sc_data->tolist = tmplist;
	sc_data->item_list = tmplist;

	sc_data->item_cnt--;
	evas_object_clip_unset(ret);
	evas_object_smart_member_del(ret);

	item = LIST_DATA(sc_data->curlist);
	if (item) {
		Evas_Coord y, w, h;
		int idx;
		LIST_ITEM_GEO_GET(sc_data->curlist, NULL, &y, &w, &h);
		LIST_ITEM_GEO_SET(sc_data->curlist, sc_data->clip_bx, y, w, h);
		update_items_geo(sc_data, 0);
		idx = list_item_idx(sc_data, sc_data->curlist);
		evas_object_smart_callback_call(sc_data->scroller, "page,changed", (void *)idx);
	}
	return ret;
}

Evas_Object *live_scroller_get_item(Evas_Object *scroller, int idx)
{
	struct widget_data *sc_data;

	sc_data = evas_object_smart_data_get(scroller);
	if (!sc_data) {
		ErrPrint("sc_data is not valid\n");
		return NULL;
	}

	if (idx < 0 || idx >= sc_data->item_cnt)
		return NULL;

	return list_item_nth(sc_data->item_list, idx);
}

int live_scroller_get_current(Evas_Object *scroller)
{
	struct widget_data *sc_data;
	struct item_list_entry *ilist;
	int idx;

	sc_data = evas_object_smart_data_get(scroller);
	if (!sc_data) {
		ErrPrint("sc_data is not valid\n");
		return -EINVAL;
	}

	ilist = sc_data->curlist;
	idx = 0;
	if (ilist) {
		while (ilist != sc_data->item_list) {
			idx++;
			ilist = LIST_PREV(ilist);
		}
	}

	return idx;
}

int live_scroller_loop_set(Evas_Object *scroller, int is_loop)
{
	struct widget_data *sc_data;

	sc_data = evas_object_smart_data_get(scroller);
	if (!sc_data) {
		ErrPrint("sc_data is not valid\n");
		return -EINVAL;
	}

	if (is_loop == EINA_FALSE && sc_data->is_loop == EINA_TRUE)
		rearrange_items(sc_data);

	sc_data->is_loop = is_loop;
	return 0;
}

int live_scroller_freeze(Evas_Object *scroller)
{
	struct widget_data *sc_data;

	sc_data = evas_object_smart_data_get(scroller);
	if (!sc_data) {
		ErrPrint("sc_data is not valid\n");
		return -EINVAL;
	}

	sc_data->is_freezed = EINA_TRUE;
	return 0;
}

int live_scroller_thaw(Evas_Object *scroller)
{
	struct widget_data *sc_data;

	sc_data = evas_object_smart_data_get(scroller);
	if (!sc_data) {
		ErrPrint("sc_data is not valid\n");
		return -EINVAL;
	}

	sc_data->is_freezed = EINA_FALSE;
	return 0;
}

int live_scroller_anim_to(Evas_Object *scroller, double sec, int offset)
{
	PROFILE_START();
	struct widget_data *sc_data;
	Evas_Coord sx, sw;
	Evas_Coord y;
	struct live_sc_event_info info;
	struct item_list_entry *ilist;
	double ftmp;
	int ret;

	sc_data = evas_object_smart_data_get(scroller);
	if (!sc_data) {
		ErrPrint("sc_data is not valid\n");
		ret = -EINVAL;
		goto out;
	}
	
	if (sc_data->is_freezed) {
		ErrPrint("Scroller is freezed\n");
		ret = -EBUSY;
		goto out;
	}

	if (!sc_data->curlist)
		sc_data->curlist = sc_data->item_list;

	if (!sc_data->curlist) {
		ErrPrint("List is empty\n");
		ret = -ENOENT;
		goto out;
	}

	if (sc_data->sc_anim_timer) {
		ecore_timer_del(sc_data->sc_anim_timer);
		sc_data->sc_anim_timer = NULL;
		evas_object_smart_callback_call(sc_data->scroller, "anim,stop", NULL);
	} else {
		sc_data->tolist = sc_data->curlist;
	}

	LIST_ITEM_GEO_GET(sc_data->curlist, &sx, &y, &sw, NULL);

	if (!offset) {
		sc_data->sc_anim_dx = sc_data->clip_bx - sx;
		DbgPrint("offset==0, dx: %d\n", sc_data->sc_anim_dx);
	} else {
		Evas_Coord tw;
		struct item_list_entry *tmplist;

		calc_anim_dx_with_dir(sc_data, &offset);
		DbgPrint("Offset: %d\n", offset);

		ilist = sc_data->curlist;
		while (offset < 0) {
			if (!sc_data->is_loop && ilist == sc_data->item_list) {
				DbgPrint("Loop is disabled\n");
				break;
			}

			LIST_ITEM_GEO_GET(ilist, NULL, NULL, &tw, NULL);
			ilist = LIST_PREV(ilist);

			sc_data->sc_anim_dx += tw;
			DbgPrint("tw: %d (%d)\n", tw, sc_data->sc_anim_dx);

			offset++;
			if (sc_data->tolist == sc_data->item_list) {
				if (!sc_data->is_loop) {
					DbgPrint("Looping disabled\n");
					break;
				}
			}
			sc_data->tolist = LIST_PREV(sc_data->tolist);
		}
		
		while (offset > 0) {
			LIST_ITEM_GEO_GET(ilist, NULL, NULL, &tw, NULL);
			ilist = LIST_NEXT(ilist);

			sc_data->sc_anim_dx -= tw;
			DbgPrint("tw: %d (%d)\n", tw, sc_data->sc_anim_dx);

			offset--;
			tmplist = LIST_NEXT(sc_data->tolist);
			if (tmplist == sc_data->item_list) {
				if (!sc_data->is_loop) {
					DbgPrint("Looping disabled\n");
					break;
				}
			}
			sc_data->tolist = tmplist;

			if (!sc_data->is_loop && ilist == sc_data->item_list) {
				DbgPrint("Destination arrived or loop is disabled");
				break;
			}
		}
	}

	if (abs(sc_data->sc_anim_dx) > ANIM_MIN) {
		ftmp = (double)sc_data->sc_anim_dx / ANIM_UNIT;
		DbgPrint("ftmp: %lf\n", ftmp);
		if (fabs(ftmp) < ANIM_MIN || fabs(ftmp) > abs(sc_data->sc_anim_dx))
			sc_data->sc_anim_dx = ftmp < 0 ? -ANIM_MIN : ANIM_MIN;
		else
			sc_data->sc_anim_dx = (int)ftmp;
		DbgPrint("Result: %d\n", sc_data->sc_anim_dx);
	}

	sc_data->sc_anim_timer = ecore_timer_add(sec, sc_anim_cb, sc_data);
	if (!sc_data->sc_anim_timer) {
		ErrPrint("Failed to add a animator\n");
		ret = -EFAULT;
		goto out;
	}

	info.curidx = list_item_idx(sc_data, sc_data->curlist);
	info.toidx = list_item_idx(sc_data, sc_data->tolist);
	DbgPrint("Current index: %d, To index: %d\n", info.curidx, info.toidx);

	evas_object_smart_callback_call(sc_data->scroller, "anim,start", &info);
	ret = 0;

out:
	PROFILE_END();
	return ret;
}

int live_scroller_go_to(Evas_Object *scroller, int idx)
{
	struct widget_data *sc_data;

	sc_data = evas_object_smart_data_get(scroller);
	if (!sc_data) {
		ErrPrint("sc_data is not valid\n");
		return -EINVAL;
	}

	if (sc_data->is_freezed)
		return -EBUSY;

	if (idx < 0 || idx >= sc_data->item_cnt)
		return -EINVAL;

	sc_data->curlist = list_item_nth_list(sc_data->item_list, idx);
	if (!sc_data->curlist)
		return -EFAULT;

	rearrange_items(sc_data);
	evas_object_smart_callback_call(sc_data->scroller, "page,changed", (void *)idx);

	return 0;
}

int live_scroller_update(Evas_Object *scroller)
{
	struct widget_data *sc_data;
	struct item_list_entry *n;
	Evas_Object *item;
	Evas_Coord x, y, w, h;

	sc_data = evas_object_smart_data_get(scroller);
	if (!sc_data) {
		ErrPrint("sc_data is not valid\n");
		return -EINVAL;
	}

	if (sc_data->item_list) {
		n = sc_data->item_list;
		do {
			item = LIST_DATA(n);
			LIST_ITEM_GEO_GET(n, &x, &y, &w, &h);
			move_item(sc_data, n, x, y, w, h);
			n = LIST_NEXT(n);
		} while (n != sc_data->item_list);
	}

	return 0;
}

int live_scroller_get_item_count(Evas_Object *scroller)
{
	struct widget_data *sc_data;

	sc_data = evas_object_smart_data_get(scroller);
	if (!sc_data) {
		ErrPrint("sc_data is not valid\n");
		return 0;
	}

	return list_item_count(sc_data->item_list);
}

int live_scroller_get_item_index(Evas_Object *scroller, Evas_Object *item)
{
	struct widget_data *sc_data;
	struct item_list_entry *n;
	Evas_Object *tmp;
	int idx;

	sc_data = evas_object_smart_data_get(scroller);
	if (!sc_data) {
		ErrPrint("sc_data is not valid\n");
		return -EINVAL;
	}

	if (!sc_data->item_list)
		return -ENOENT;

	idx = 0;
	n = sc_data->item_list;
	do {
		tmp = LIST_DATA(n);
		n = LIST_NEXT(n);

		if (tmp == item)
			return idx;

		idx++;
	} while (n != sc_data->item_list);

	return -ENOENT;
}

/* End of a file */
