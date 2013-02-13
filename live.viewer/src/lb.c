/*
 * Copyright 2012  Samsung Electronics Co., Ltd
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
#include <Ecore_X.h>

#include <dlog.h>

#include <livebox.h>
#include <livebox-service.h>

#include "main.h"
#include "util.h"
#include "debug.h"
#include "lb.h"
#include "scroller.h"

#define FLICK_COND	100

static Evas_Object *create_canvas(Evas_Object *parent)
{
	Evas_Object *canvas;

	canvas = evas_object_image_filled_add(evas_object_evas_get(parent));
	if (!canvas)
		return NULL;

	evas_object_image_content_hint_set(canvas, EVAS_IMAGE_CONTENT_HINT_DYNAMIC);
//	evas_object_image_colorspace_set(canvas, EVAS_COLORSPACE_ARGB8888);
//	evas_object_image_alpha_set(canvas, EINA_TRUE);
	evas_object_move(canvas, 0, 0);
	return canvas;
}

static int update_pd_canvas(struct livebox *handle, Evas_Object *image)
{
	int w;
	int h;
	Evas_Native_Surface surface;

	switch (livebox_pd_type(handle)) {
	case PD_TYPE_BUFFER:
		evas_object_image_colorspace_set(image, EVAS_COLORSPACE_ARGB8888);
		evas_object_image_alpha_set(image, EINA_TRUE);

		livebox_get_pdsize(handle, &w, &h);
		if (w > 0 && h > 0) {
			void *data;
			data = livebox_acquire_pdfb(handle);
			if (data) {
				evas_object_image_size_set(image, w, h);
				evas_object_image_data_copy_set(image, data);
				livebox_release_pdfb(data);
			}
			evas_object_resize(image, w, h);
			//evas_object_size_hint_min_set(image, w, h);
			evas_object_size_hint_max_set(image, w, h);
		}
		break;
	case PD_TYPE_PIXMAP:
		h = w = 0;
		livebox_get_pdsize(handle, &w, &h);
		if (w <= 0 || h <= 0)
			break;

		DbgPrint("Update: %dx%d\n", w, h);
		surface.version = EVAS_NATIVE_SURFACE_VERSION;
		surface.type = EVAS_NATIVE_SURFACE_X11;
		surface.data.x11.pixmap = livebox_pd_pixmap(handle);
		surface.data.x11.visual = NULL; //ecore_x_default_visual_get(ecore_x_display_get(), ecore_x_default_screen_get());
		evas_object_image_native_surface_set(image, &surface);

		evas_object_image_size_set(image, w, h);
		evas_object_resize(image, w, h);
		evas_object_size_hint_max_set(image, w, h);
		evas_object_show(image);
		break;
	case PD_TYPE_TEXT:
	default:
		break;
	}

	return 0;
}

static int update_canvas(struct livebox *handle, Evas_Object *image)
{
	Evas_Native_Surface surface;
	const char *filename;
	int w;
	int h;
	int type;

	switch (livebox_lb_type(handle)) {
	case LB_TYPE_PIXMAP:
		w = h = 0;
		type = livebox_size(handle);
		livebox_service_get_size(type, &w, &h);
		if (w <= 0 || h <= 0)
			break;

		DbgPrint("Update: %dx%d\n", w, h);

		surface.version = EVAS_NATIVE_SURFACE_VERSION;
		surface.type = EVAS_NATIVE_SURFACE_X11;
		surface.data.x11.pixmap = livebox_lb_pixmap(handle);
		surface.data.x11.visual = NULL; //ecore_x_default_visual_get(ecore_x_display_get(), ecore_x_default_screen_get());
		evas_object_image_native_surface_set(image, &surface);

		evas_object_image_size_set(image, w, h);
		evas_object_resize(image, w, h);
		evas_object_size_hint_min_set(image, w, h);
		evas_object_size_hint_max_set(image, w, h);
		evas_object_show(image);
		break;
	case LB_TYPE_BUFFER:
		evas_object_image_colorspace_set(image, EVAS_COLORSPACE_ARGB8888);
		evas_object_image_alpha_set(image, EINA_TRUE);

		w = h = 0;
		type = livebox_size(handle);
		livebox_service_get_size(type, &w, &h);
		if (w > 0 && h > 0) {
			void *data;
			data = livebox_acquire_fb(handle);
			if (data) {
				evas_object_image_size_set(image, w, h);
				evas_object_image_data_copy_set(image, data);
				livebox_release_fb(data);
			}
			evas_object_resize(image, w, h);
			evas_object_size_hint_min_set(image, w, h);
			evas_object_size_hint_max_set(image, w, h);
		}
		break;
	case LB_TYPE_IMAGE:
		filename = livebox_filename(handle);
		if (filename) {
			const char *old;
			evas_object_image_file_get(image, &old, NULL);
			if (old && !strcmp(filename, old)) {
				evas_object_image_reload(image);
			} else {
				evas_object_image_file_set(image, filename, NULL);
			}

			w = h = 0;
			type = livebox_size(handle);
			livebox_service_get_size(type, &w, &h);
			if (w > 0 && h > 0) {
				evas_object_resize(image, w, h);
				evas_object_size_hint_min_set(image, w, h);
				evas_object_size_hint_max_set(image, w, h);
			}
		}
		break;
	case LB_TYPE_TEXT:
	default:
		break;
	}

	return 0;
}

static inline void prepend_log(struct livebox *handle, const char *buffer)
{
	Evas_Object *layout;
	Evas_Object *logger;

	layout = livebox_get_data(handle);
	if (!layout) {
		ErrPrint("Failed to get layout\n");
		return;
	}

	logger = elm_object_part_content_get(layout, "logger");
	if (logger)
		elm_list_item_prepend(logger, buffer, NULL, NULL, NULL, NULL);
}

static int lb_event_cb(struct livebox *handle, enum livebox_event_type event, void *data)
{
	Evas_Object *layout;
	Evas_Object *sc;
	Evas_Object *pd;
	Evas_Object *image;

	layout = (Evas_Object *)livebox_get_data(handle);
	if (!layout)
		return -EFAULT;

	switch (event) {
	case LB_EVENT_LB_UPDATED:
		DbgPrint("Contents: [%s]\n", livebox_content(handle));
		image = elm_object_part_content_get(layout, "livebox");
		if (image)
			update_canvas(handle, image);
		break;
	case LB_EVENT_PD_UPDATED:
		image = elm_object_part_content_get(layout, "pd");
		if (image)
			update_pd_canvas(handle, image);
		break;
	case LB_EVENT_DELETED:
		sc = evas_object_data_del(layout, "sc");
		if (sc)
			scroller_peek_by_obj(sc, layout);

		evas_object_del(layout);
		break;
	case LB_EVENT_PD_DESTROYED:
		pd = elm_object_part_content_unset(layout, "pd");
		if (pd)
			evas_object_del(pd);

		sc = evas_object_data_del(layout, "sc");
		if (sc)
			scroller_unlock(sc);
		break;
	default:
		break;
	}

	return 0;
}

static int lb_fault_cb(enum livebox_fault_type event,
					const char *pkgname, const char *filename,
					const char *funcname, void *data)
{
	DbgPrint("pkgname: %s\nfilename: %s\n:funcname: %s\n", pkgname, filename, funcname);
	return 0;
}

static void del_cb(struct livebox *handle, int ret, void *data)
{
	Evas_Object *layout = data;
	Evas_Object *sc;

	sc = evas_object_data_del(layout, "sc");
	if (sc) {
		DbgPrint("Scroller: %p\n", sc);
		scroller_peek_by_obj(sc, layout);
	}

	DbgPrint("Delete a layout object\n");
	evas_object_del(layout);
}

static void delete_btn_cb(void *handle, Evas_Object *obj, void *event_info)
{
	int ret;
	Evas_Object *layout;
	layout = livebox_get_data(handle);
	DbgPrint("Livebox Get Data %p - %p\n", handle, layout);
	ret = livebox_del(handle, del_cb, layout);
	if (ret < 0) {
		char buffer[256];
		snprintf(buffer, sizeof(buffer), "Delete returns: %d\n", ret);
		prepend_log(handle, buffer);
	}
}

static void error_popup(Evas_Object *parent, struct livebox *handle, int ret)
{
	ErrPrint("Failed to add a box: %d\n", ret);
	return;
}

static void resize_cb(struct livebox *handle, int ret, void *data)
{
	Evas_Object *layout;
	Evas_Object *log_list;
	char buffer[256];

	layout = livebox_get_data(handle);
	if (!layout)
		return;

	log_list = elm_object_part_content_get(layout, "logger");
	if (!log_list)
		return;

	snprintf(buffer, sizeof(buffer) - 1, "Resize: %d", ret);
	elm_list_item_prepend(log_list, buffer, NULL, NULL, NULL, NULL);
}

static void resize_click_cb(void *handle, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *item;
	const char *label;
	int w;
	int h;
	int size_type;

	item = elm_list_selected_item_get(obj);
	if (!item)
		return;

	label = elm_object_item_part_text_get(item, NULL);
	if (!label)
		return;

	sscanf(label, "%dx%d", &w, &h);
	size_type = livebox_service_size_type(w, h);

	livebox_resize(handle, size_type, resize_cb, NULL);
}

static void create_resize_controller(struct livebox *handle, Evas_Object *layout)
{
	Evas_Object *size_list;
	char buffer[256];
	int sizes[NR_OF_SIZE_LIST];
	int cnt;
	int i;
	int w;
	int h;

	size_list = elm_list_add(layout);
	cnt = sizeof(sizes) / sizeof(sizes[0]);
	livebox_get_supported_sizes(handle, &cnt, sizes);
	for (i = 0; i < cnt; i++) {
		livebox_service_get_size(sizes[i], &w, &h);
		snprintf(buffer, sizeof(buffer) - 1, "%dx%d", w, h);

		elm_list_item_append(size_list, buffer, NULL, NULL, resize_click_cb, handle);
	}
	evas_object_show(size_list);
	elm_list_go(size_list);
	evas_object_size_hint_weight_set(size_list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(size_list, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_part_content_set(layout, "controller", size_list);
}

static void create_logger(struct livebox *handle, Evas_Object *layout)
{
	Evas_Object *log_list;

	log_list = elm_list_add(layout);
	if (!log_list)
		return;

	elm_list_item_prepend(log_list, "Created", NULL, NULL, NULL, NULL);
	evas_object_show(log_list);
	elm_list_go(log_list);

	evas_object_size_hint_weight_set(log_list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(log_list, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_part_content_set(layout, "logger", log_list);
}

struct event_data {
	Evas_Coord x;
	Evas_Coord y;

	enum flick {
		FLICK_UNKNOWN = 0x00,
		FLICK_DOWN = 0x01,
		FLICK_UP = 0x02,
	} flick;
};

static void pd_closed_cb(struct livebox *handle, int ret, void *data)
{
	evas_object_del(data);
}

static void pd_hide_done_cb(void *data, Evas_Object *obj, const char *emission, const char *source) 
{
	Evas_Object *pd;
	Evas_Object *sc;

	sc = evas_object_data_get(obj, "sc");
	scroller_unlock(sc);

	elm_object_signal_callback_del(obj, emission, source, pd_hide_done_cb);
	pd = elm_object_part_content_unset(obj, "pd");
	livebox_destroy_pd(data, pd_closed_cb, pd);
}

static void pd_mouse_up_cb(void *handle, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Event_Mouse_Up *up = event_info;
	Evas_Coord x, y, w, h;
	double rx, ry;

	evas_object_geometry_get(obj, &x, &y, &w, &h);

	rx = (double)(up->canvas.x - x) / (double)w;
	ry = (double)(up->canvas.y - y) / (double)h;
	DbgPrint("%dx%d - %dx%d, %lfx%lf\n", x, y, w, h, rx, ry);
	livebox_content_event(handle, PD_MOUSE_UP, rx, ry);
}

static void pd_mouse_down_cb(void *handle, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Event_Mouse_Down *down = event_info;
	Evas_Coord x, y, w, h;
	double rx, ry;

	evas_object_geometry_get(obj, &x, &y, &w, &h);
	rx = (double)(down->canvas.x - x) / (double)w;
	ry = (double)(down->canvas.y - y) / (double)h;
	DbgPrint("%dx%d - %dx%d, %lfx%lf\n", x, y, w, h, rx, ry);
	livebox_content_event(handle, PD_MOUSE_DOWN, rx, ry);
}

static void pd_mouse_move_cb(void *handle, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Event_Mouse_Move *move = event_info;
	Evas_Coord x, y, w, h;
	double rx, ry;

	evas_object_geometry_get(obj, &x, &y, &w, &h);
	rx = (double)(move->cur.canvas.x - x) / (double)w;
	ry = (double)(move->cur.canvas.y - y) / (double)h;
	DbgPrint("%dx%d - %dx%d, %lfx%lf\n", x, y, w, h, rx, ry);
	livebox_content_event(handle, PD_MOUSE_MOVE, rx, ry);
}

static void pd_created_cb(struct livebox *handle, int ret, void *data)
{
	Evas_Object *layout;
	Evas_Object *pd_image;
	Evas_Object *sc;

	layout = (Evas_Object *)livebox_get_data(handle);
	if (!layout)
		return;

	sc = evas_object_data_get(layout, "sc");

	pd_image = create_canvas(layout);
	if (!pd_image)
		return;

	evas_object_event_callback_add(pd_image, EVAS_CALLBACK_MOUSE_UP, pd_mouse_up_cb, handle);
	evas_object_event_callback_add(pd_image, EVAS_CALLBACK_MOUSE_DOWN, pd_mouse_down_cb, handle);
	evas_object_event_callback_add(pd_image, EVAS_CALLBACK_MOUSE_MOVE, pd_mouse_move_cb, handle);

	update_pd_canvas(handle, pd_image);

	elm_object_signal_callback_add(layout, "hide,done", "pd", pd_hide_done_cb, handle);
	elm_object_part_content_set(layout, "pd", pd_image);
	elm_object_signal_emit(layout, "open", "pd");
	scroller_lock(sc);
}

static void lb_mouse_up_cb(void *handle, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Event_Mouse_Up *up = event_info;
	Evas_Coord x, y, w, h;
	struct event_data *evt;

	evt = evas_object_data_del(obj, "evt");
	if (!evt)
		return;

	evas_object_geometry_get(obj, &x, &y, &w, &h);

	if (livebox_lb_type(handle) == LB_TYPE_PIXMAP || livebox_lb_type(handle) == LB_TYPE_BUFFER) {
		double rx, ry;
		rx = (double)(up->canvas.x - x) / (double)w;
		ry = (double)(up->canvas.y - y) / (double)h;
		livebox_content_event(handle, LB_MOUSE_UP, rx, ry);
	}

	if (x < up->canvas.x && up->canvas.x < x + w) {
		if (y < up->canvas.y && up->canvas.y < y + h) {
			livebox_click(handle, (double)x / (double)w, (double)y / (double)h);
		}
	}

	if (evt->flick == FLICK_DOWN && (up->canvas.y - evt->y) > (FLICK_COND>>1)) {
		int ret;
		/* Open PD */
		ret = livebox_create_pd_with_position(handle, 0.5, 0.0, pd_created_cb, NULL);
	}

	free(evt);
}

static void lb_mouse_down_cb(void *handle, Evas *e, Evas_Object *obj, void *event_info)
{
	struct event_data *evt;
	Evas_Event_Mouse_Down *down = event_info;
	Evas_Object *layout;
	Evas_Object *sc;

	layout = livebox_get_data(handle);
	if (!layout)
		return;

	sc = evas_object_data_get(layout, "sc");
	if (!sc)
		return;

	if (scroller_is_scrolling(sc))
		return;

	if (livebox_lb_type(handle) == LB_TYPE_PIXMAP || livebox_lb_type(handle) == LB_TYPE_BUFFER) {
		Evas_Coord x, y, w, h;
		double rx, ry;

		evas_object_geometry_get(obj, &x, &y, &w, &h);
		rx = (double)(down->canvas.x - x) / (double)w;
		ry = (double)(down->canvas.y - y) / (double)h;
		livebox_content_event(handle, LB_MOUSE_DOWN, rx, ry);
	}

	evt = evas_object_data_get(obj, "evt");
	if (evt) {
		ErrPrint("Huh?");
	} else {
		evt = malloc(sizeof(*evt));
		if (!evt) {
			ErrPrint("Heap: %s\n", strerror(errno));
			return;
		}
	}

	evas_object_data_set(obj, "evt", evt);

	evt->x = down->canvas.x;
	evt->y = down->canvas.y;
	evt->flick = FLICK_UNKNOWN;
}

static void lb_mouse_move_cb(void *handle, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Event_Mouse_Move *move = event_info;
	struct event_data *evt;

	evt = evas_object_data_get(obj, "evt");
	if (!evt)
		return;

	if (livebox_lb_type(handle) == LB_TYPE_PIXMAP || livebox_lb_type(handle) == LB_TYPE_BUFFER) {
		Evas_Coord x, y, w, h;
		double rx, ry;

		evas_object_geometry_get(obj, &x, &y, &w, &h);
		rx = (double)(move->cur.canvas.x - x) / (double)w;
		ry = (double)(move->cur.canvas.y - y) / (double)h;
		livebox_content_event(handle, LB_MOUSE_MOVE, rx, ry);
	}

	if ((move->cur.canvas.x - move->prev.canvas.x) > FLICK_COND) {
		evt->flick = FLICK_UNKNOWN;
		return;
	} else if ((move->cur.canvas.x - move->prev.canvas.x) < -FLICK_COND) {
		evt->flick = FLICK_UNKNOWN;
		return;
	}

	if ((move->cur.canvas.y - move->prev.canvas.y) > 0) {
		if (evt->flick != FLICK_DOWN)
			evt->flick = FLICK_DOWN;
	} else if ((move->cur.canvas.y - move->prev.canvas.y) < 0) {
		if (evt->flick != FLICK_UP)
			evt->flick = FLICK_UP;
	}
}

static int lb_update_begin(struct livebox *handle)
{
	DbgPrint("%p\n", handle);

	Evas_Object *layout;
	Evas_Object *list;
	char buffer[80];

	layout = (Evas_Object *)livebox_get_data(handle);
	if (!layout) {
		ErrPrint("Failed to get layout object\n");
		return 0;
	}

	list = elm_object_part_content_get(layout, "livebox");
	if (!list) {
		ErrPrint("Failed to get list\n");
		return 0;
	}

	snprintf(buffer, sizeof(buffer), "begin");
	elm_list_item_prepend(list, buffer, NULL, NULL, NULL, NULL);

	return 0;
}

static int lb_update_end(struct livebox *handle)
{
	DbgPrint("%p\n", handle);

	Evas_Object *layout;
	Evas_Object *list;
	char buffer[80];

	layout = (Evas_Object *)livebox_get_data(handle);
	if (!layout) {
		ErrPrint("Failed to get layout object\n");
		return 0;
	}

	list = elm_object_part_content_get(layout, "livebox");
	if (!list) {
		ErrPrint("Failed to get list\n");
		return 0;
	}

	snprintf(buffer, sizeof(buffer), "end");
	elm_list_item_prepend(list, buffer, NULL, NULL, NULL, NULL);

	return 0;
}

static int lb_update_text(struct livebox *handle, const char *id, const char *part, const char *data)
{
	DbgPrint("%p\n", handle);
	DbgPrint("id: [%s], part[%s], data[%s]\n", id, part, data);

	Evas_Object *layout;
	Evas_Object *list;
	char buffer[80];

	layout = (Evas_Object *)livebox_get_data(handle);
	if (!layout) {
		ErrPrint("Failed to get layout object\n");
		return 0;
	}

	list = elm_object_part_content_get(layout, "livebox");
	if (!list) {
		ErrPrint("Failed to get list\n");
		return 0;
	}

	snprintf(buffer, sizeof(buffer), "%s=%s", part, data);
	elm_list_item_prepend(list, buffer, NULL, NULL, NULL, NULL);

	return 0;
}

static int lb_update_image(struct livebox *handle, const char *id, const char *part, const char *data)
{
	DbgPrint("%p\n", handle);
	DbgPrint("id: [%s], part[%s], data[%s]\n", id, part, data);

	Evas_Object *layout;
	Evas_Object *list;
	char buffer[80];

	layout = (Evas_Object *)livebox_get_data(handle);
	if (!layout) {
		ErrPrint("Failed to get layout object\n");
		return 0;
	}

	list = elm_object_part_content_get(layout, "livebox");
	if (!list) {
		ErrPrint("Failed to get list\n");
		return 0;
	}

	snprintf(buffer, sizeof(buffer), "%s=%s", part, data);
	elm_list_item_prepend(list, buffer, NULL, NULL, NULL, NULL);

	return 0;
}

static int lb_update_script(struct livebox *handle, const char *id, const char *part, const char *file, const char *group)
{
	DbgPrint("%p\n", handle);
	DbgPrint("id: [%s], part[%s], file[%s], group[%s]\n", id, part, file, group);

	Evas_Object *layout;
	Evas_Object *list;
	char buffer[80];

	layout = (Evas_Object *)livebox_get_data(handle);
	if (!layout) {
		ErrPrint("Failed to get layout object\n");
		return 0;
	}

	list = elm_object_part_content_get(layout, "livebox");
	if (!list) {
		ErrPrint("Failed to get list\n");
		return 0;
	}

	snprintf(buffer, sizeof(buffer), "%s=%s, %s", part, file, group);
	elm_list_item_prepend(list, buffer, NULL, NULL, NULL, NULL);

	return 0;
}

static int lb_update_signal(struct livebox *handle, const char *id, const char *emission, const char *signal)
{
	DbgPrint("%p\n", handle);
	DbgPrint("id: [%s], emission[%s], signal[%s]\n", id, emission, signal);

	Evas_Object *layout;
	Evas_Object *list;
	char buffer[80];

	layout = (Evas_Object *)livebox_get_data(handle);
	if (!layout) {
		ErrPrint("Failed to get layout object\n");
		return 0;
	}

	list = elm_object_part_content_get(layout, "livebox");
	if (!list) {
		ErrPrint("Failed to get list\n");
		return 0;
	}

	snprintf(buffer, sizeof(buffer), "%s,%s", emission, signal);
	elm_list_item_prepend(list, buffer, NULL, NULL, NULL, NULL);
	return 0;
}

static int lb_update_drag(struct livebox *handle, const char *id, const char *part, double dx, double dy)
{
	DbgPrint("%p\n", handle);
	DbgPrint("id: [%s], part[%s], pos[%lfx%lf]\n", id, part, dx, dy);

	Evas_Object *layout;
	Evas_Object *list;
	char buffer[80];

	layout = (Evas_Object *)livebox_get_data(handle);
	if (!layout) {
		ErrPrint("Failed to get layout object\n");
		return 0;
	}

	list = elm_object_part_content_get(layout, "livebox");
	if (!list) {
		ErrPrint("Failed to get list\n");
		return 0;
	}

	snprintf(buffer, sizeof(buffer), "%s=%lfx%lf", part, dx, dy);
	elm_list_item_prepend(list, buffer, NULL, NULL, NULL, NULL);
	return 0;
}

static int lb_update_info_size(struct livebox *handle, const char *id, int w, int h)
{
	DbgPrint("%p\n", handle);
	DbgPrint("id: [%s], size[%dx%d]\n", id, w, h);

	Evas_Object *layout;
	Evas_Object *list;
	char buffer[80];

	layout = (Evas_Object *)livebox_get_data(handle);
	if (!layout) {
		ErrPrint("Failed to get layout object\n");
		return 0;
	}

	list = elm_object_part_content_get(layout, "livebox");
	if (!list) {
		ErrPrint("Failed to get list\n");
		return 0;
	}

	snprintf(buffer, sizeof(buffer), "%dx%d", w, h);
	elm_list_item_prepend(list, buffer, NULL, NULL, NULL, NULL);
	return 0;
}

static int lb_update_info_category(struct livebox *handle, const char *id, const char *category)
{
	DbgPrint("%p\n", handle);
	DbgPrint("id: [%s], category: [%s]\n", id, category);

	Evas_Object *layout;
	Evas_Object *list;
	char buffer[80];

	layout = (Evas_Object *)livebox_get_data(handle);
	if (!layout) {
		ErrPrint("Failed to get layout object\n");
		return 0;
	}

	list = elm_object_part_content_get(layout, "livebox");
	if (!list) {
		ErrPrint("Failed to get list\n");
		return 0;
	}

	snprintf(buffer, sizeof(buffer), "%s=%s", id, category);
	elm_list_item_prepend(list, buffer, NULL, NULL, NULL, NULL);
	return 0;
}

static void livebox_added_cb(struct livebox *handle, int ret, void *data)
{
	Evas_Object *layout;
	Evas_Object *btn;
	int w;
	int h;
	int type;
	int idx;

	DbgPrint("%s - %d\n", livebox_pkgname(handle), ret);

	if (ret != 0) {
		error_popup(main_get_window(), handle, ret);
		return;
	}

	layout = elm_layout_add(main_get_window());
	if (!layout) {
		ErrPrint("Failed to add a layout\n");
		return;
	}

	if (elm_layout_file_set(layout, PKGROOT "/res/edje/live-viewer.edj", "layout") == EINA_FALSE) {
		DbgPrint("Failed to add a layout\n");
		evas_object_del(layout);
		return;
	}

	if (livebox_lb_type(handle) == LB_TYPE_TEXT) {
		Evas_Object *list;
		static struct livebox_script_operators ops = {
			.update_begin = lb_update_begin,
			.update_end = lb_update_end,
			.update_text = lb_update_text,
			.update_image = lb_update_image,
			.update_script = lb_update_script,
			.update_signal = lb_update_signal,
			.update_drag = lb_update_drag,
			.update_info_size = lb_update_info_size,
			.update_info_category = lb_update_info_category,
		};

		list = elm_list_add(layout);
		if (!list) {
			evas_object_del(layout);
			return;
		}

		(void)livebox_set_text_handler(handle, &ops);
		elm_object_part_content_set(layout, "livebox", list);
		elm_list_go(list);
	} else {
		Evas_Object *lb_image;
		lb_image = create_canvas(layout);
		if (!lb_image) {
			ErrPrint("Failed to create a canvas\n");
			evas_object_del(layout);
			return;
		}

		evas_object_event_callback_add(lb_image, EVAS_CALLBACK_MOUSE_UP, lb_mouse_up_cb, handle);
		evas_object_event_callback_add(lb_image, EVAS_CALLBACK_MOUSE_DOWN, lb_mouse_down_cb, handle);
		evas_object_event_callback_add(lb_image, EVAS_CALLBACK_MOUSE_MOVE, lb_mouse_move_cb, handle);

		w = 0;
		h = 0;
		type = livebox_size(handle);
		if (type != LB_SIZE_TYPE_UNKNOWN) {
			livebox_service_get_size(type, &w, &h);
			DbgPrint("%dx%d\n", w, h);
		}
		evas_object_resize(lb_image, w, h);
		evas_object_show(lb_image);

		update_canvas(handle, lb_image);

		btn = elm_button_add(main_get_window());
		if (btn) {
			elm_object_text_set(btn, "Delete");
			evas_object_smart_callback_add(btn, "clicked", delete_btn_cb, handle);
			elm_object_part_content_set(layout, "delete,btn", btn);
		}

		elm_object_part_content_set(layout, "livebox", lb_image);
	}
	evas_object_resize(layout, 720, 1280);
	evas_object_show(layout);

	create_resize_controller(handle, layout);
	create_logger(handle, layout);

	livebox_set_data(handle, layout);
	DbgPrint("Livebox Set Data: %p - %p\n", handle, layout);
	evas_object_data_set(layout, "sc", data);

	scroller_append(data, layout);

	idx = scroller_get_page_index(data, layout);
	DbgPrint("Scroll to %d\n", idx);
	scroller_scroll_to(data, idx);
}

int lb_add(Evas_Object *sc, const char *pkgname)
{
	int w, h;
	struct livebox *handle;

	evas_object_geometry_get(sc, NULL, NULL, &w, &h);

	DbgPrint("sc: %dx%d, package: %s\n", w, h, pkgname);
	livebox_activate(pkgname, NULL, NULL);

	handle = livebox_add(pkgname, "default", "user,created", "default",
						DEFAULT_PERIOD, livebox_added_cb, sc);
	if (!handle) {
		ErrPrint("Failed to add a new livebox\n");
		return -EFAULT;
	}

	return 0;
}

int lb_init(void)
{
	livebox_init(ecore_x_display_get());
	livebox_set_event_handler(lb_event_cb, NULL);
	livebox_set_fault_handler(lb_fault_cb, NULL);
	return 0;
}

int lb_fini(void)
{
	livebox_fini();
	return 0;
}

/* End of a file */
