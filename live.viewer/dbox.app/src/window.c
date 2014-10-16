#include <stdio.h>
#include <unistd.h>

#include <Evas.h>
#include <Ecore_X.h>
#include <Ecore.h>
#include <X11/Xlib.h>

#include <dynamicbox_errno.h>

#include <dlog.h>
#include "debug.h"
#include "util.h"
#include "window.h"

static struct info {
	Eina_List *win_list;
	Ecore_Event_Handler *damage_handler;
} s_info = {
	.win_list = NULL,
	.damage_handler = NULL,
};

struct window_data {
	Evas_Object *image;
	Ecore_X_Damage damage;
	Ecore_X_Window pixmap;
	struct touch_info {
		int x;
		int y;
		int down;
	} touch_info[2];
	Eina_Bool is_selected;
};

static inline void clear_win_list(void)
{
	struct window_data *item;

	EINA_LIST_FREE(s_info.win_list, item) {
		ecore_x_damage_free(item->damage);
		evas_object_del(item->image);
		free(item);
	}
}

static inline Eina_Bool damage_cb(void *data, int type, void *event)
{
	Ecore_X_Event_Damage *e = (Ecore_X_Event_Damage *)event;
	struct window_data *item;
	Eina_List *l;
	int w;
	int h;

	EINA_LIST_FOREACH(s_info.win_list, l, item) {
		if ((unsigned int)item->pixmap != (unsigned int)e->drawable) {
			continue;
		}

		ecore_x_window_size_get(item->pixmap, &w, &h);

		evas_object_image_pixels_dirty_set(item->image, EINA_TRUE);
		evas_object_image_data_update_add(item->image, 0, 0, w, h);
		evas_object_show(item->image);
		break;
	}

	ecore_x_damage_subtract(e->damage, None, None);
	return ECORE_CALLBACK_PASS_ON;
}

static void item_in_cb(void *cbdata, Evas *e, Evas_Object *obj, void *event_info)
{
	struct window_data *item = cbdata;
	Evas_Event_Mouse_In *in = event_info;

	if (item->is_selected) {
		DbgPrint("%dx%d\n", in->canvas.x, in->canvas.y);
		ecore_x_mouse_in_send(item->pixmap, in->canvas.x, in->canvas.y);
		return;
	}
}

static void item_out_cb(void *cbdata, Evas *e, Evas_Object *obj, void *event_info)
{
	struct window_data *item = cbdata;
	Evas_Event_Mouse_Out *out = event_info;

	if (item->is_selected) {
		DbgPrint("%dx%d\n", out->canvas.x, out->canvas.y);
		ecore_x_mouse_out_send(item->pixmap, out->canvas.x, out->canvas.y);
		return;
	}
}

static void item_down_cb(void *cbdata, Evas *e, Evas_Object *obj, void *event_info)
{
	struct window_data *item = cbdata;
	Evas_Event_Mouse_Down *down = event_info;

	item->touch_info[0].down = 1;

	if (item->is_selected) {
		DbgPrint("%dx%d\n", down->canvas.x, down->canvas.y);
		ecore_x_mouse_down_send(item->pixmap, down->canvas.x, down->canvas.y, 1);
		return;
	}

	item->touch_info[0].x = down->canvas.x;
	item->touch_info[0].y = down->canvas.y;
}

static void item_move_cb(void *cbdata, Evas *e, Evas_Object *obj, void *event_info)
{
	struct window_data *item = cbdata;
	Evas_Event_Mouse_Move *move = event_info;

	if (item->is_selected) {
		if (!item->touch_info[0].down) {
			return;
		}

		DbgPrint("%dx%d\n", move->cur.canvas.x, move->cur.canvas.y);
		ecore_x_mouse_move_send(item->pixmap, move->cur.canvas.x, move->cur.canvas.y);
		return;
	}

	if (abs(item->touch_info[0].x - move->cur.canvas.x) > 10) {
		item->touch_info[0].down = 0;
		return;
	}

	if (!item->touch_info[0].down) {
		return;
	}
}

static void item_up_cb(void *cbdata, Evas *e, Evas_Object *obj, void *event_info)
{
	struct window_data *item = cbdata;
	Evas_Event_Mouse_Up *up = event_info;
	Eina_List *l;
	Eina_List *n;

	if (item->is_selected) {
		if (!item->touch_info[0].down) {
			return;
		}

		ecore_x_mouse_up_send(item->pixmap, up->canvas.x, up->canvas.y, 1);
		DbgPrint("%dx%d\n", up->canvas.x, up->canvas.y);

		item->touch_info[0].down = 0;
		return;
	}

	if (abs(item->touch_info[0].x - up->canvas.x) > 10) {
		item->touch_info[0].down = 0;
		return;
	}

	if (!item->touch_info[0].down) {
		return;
	}

	item->touch_info[0].down = 0;

	evas_object_resize(item->image, 720, 1280);
	evas_object_move(item->image, 0, 0);
	item->is_selected = 1;

	EINA_LIST_FOREACH_SAFE(s_info.win_list, l, n, item) {
		if (item->is_selected) {
			continue;
		}

		s_info.win_list = eina_list_remove(s_info.win_list, item);
		ecore_x_damage_free(item->damage);
		evas_object_del(item->image);
		free(item);
	}
}

static inline int update_win_list_cb(int pid, Ecore_X_Window win, const char *cmd, void *data)
{
	struct window_data *item;
	Evas_Native_Surface surface;
	int w;
	int h;
	Evas *e;

	item = calloc(1, sizeof(*item));
	if (!item) {
		return EINA_FALSE;
	}

	e = evas_object_evas_get(data);
	item->image = evas_object_image_filled_add(e);
	if (!item->image) {
		free(item);
		return EINA_FALSE;
	}

	ecore_x_window_size_get(win, &w, &h);

	evas_object_image_size_set(item->image, w, h);
	evas_object_image_fill_set(item->image, 0, 0, w, h);

	surface.version = EVAS_NATIVE_SURFACE_VERSION;
	surface.type = EVAS_NATIVE_SURFACE_X11;
	surface.data.x11.visual = ecore_x_default_visual_get(ecore_x_display_get(), ecore_x_default_screen_get());
	surface.data.x11.pixmap = (unsigned int)win;

	evas_object_image_native_surface_set(item->image, &surface);
	evas_object_image_pixels_dirty_set(item->image, EINA_TRUE);
	evas_object_image_data_update_add(item->image, 0, 0, w, h);

	h = eina_list_count(s_info.win_list);

	w = h % 3;
	h = h / 3;

	// 720
	// 30 200 30 200 30 200 30
	// 1280
	// 200 * 5
	// top pad : 40 (280 - 40)
	// 240 / 6 = 40
	evas_object_resize(item->image, 200, 200);
	w = (w * 200) + (w + 1) * 30;
	h = 40 + (h * 200) + (h + 1) * 40;
	evas_object_move(item->image, w, h);
	evas_object_show(item->image);

	item->damage = ecore_x_damage_new(win, ECORE_X_DAMAGE_REPORT_RAW_RECTANGLES);
	if (!item->damage) {
		LOGE("Failed to create a damage handler\n");
		evas_object_del(item->image);
		free(item);
		return EINA_FALSE;
	}

	item->pixmap = win;

	evas_object_event_callback_add(item->image, EVAS_CALLBACK_MOUSE_IN, item_in_cb, item);
	evas_object_event_callback_add(item->image, EVAS_CALLBACK_MOUSE_OUT, item_out_cb, item);
	evas_object_event_callback_add(item->image, EVAS_CALLBACK_MOUSE_DOWN, item_down_cb, item);
	evas_object_event_callback_add(item->image, EVAS_CALLBACK_MOUSE_MOVE, item_move_cb, item);
	evas_object_event_callback_add(item->image, EVAS_CALLBACK_MOUSE_UP, item_up_cb, item);

	s_info.win_list = eina_list_append(s_info.win_list, item);
	return EINA_TRUE;
}

int window_list_create(Evas_Object *win)
{
	if (!s_info.damage_handler) {
		s_info.damage_handler = ecore_event_handler_add(ECORE_X_EVENT_DAMAGE_NOTIFY, damage_cb, NULL);
		if (!s_info.damage_handler) {
			ErrPrint("Failed to add damage notifier\n");
		}
	}

	window_list_update(win);
	return 0;
}

int window_list_update(Evas_Object *win)
{
	clear_win_list();

	if (util_win_list_get(update_win_list_cb, win) <= 0) {
		DbgPrint("Windows are not detected\n");
	}

	return 0;
}

int window_list_destroy(void)
{
	clear_win_list();

	if (s_info.damage_handler) {
		ecore_event_handler_del(s_info.damage_handler);
		s_info.damage_handler = NULL;
	}

	return 0;
}

/* End of a file */
