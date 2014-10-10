/*
 * Copyright 2012  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.tizenopensource.org/license
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#if defined(LOG_TAG)
#undef LOG_TAG
#endif

#define LOG_TAG "ADD_VIEWER"

#include <Elementary.h>
//#include <appcore-efl.h>
#include <app.h>
#include <unistd.h>
#include <errno.h>
#include <dynamicbox.h>
#include <dynamicbox_service.h>
#include <dynamicbox_errno.h>
#include <vconf.h>
#include <bundle.h>
#include <dlog.h>
#include <string.h>
#include <utilX.h>
#include <Ecore_X.h>

int errno;

#include "main.h"
#include "dynamicbox_evas.h"

#define HOME_SERVICE_KEY "home_op"
#define HOME_SERVICE_VALUE_POWERKEY "powerkey"
#define PREVIEW_DEFAULT RESDIR"/images/unknown.png"
#define VCONF_KEY_VIEWER_AUTO_FEED "memory/dbox-viewer/auto_feed"
#define LAYOUT_EDJ RESDIR"/edje/w_add_viewer.edj"

static struct info {
	int resize_test;
	Ecore_Timer *resize_timer;
	int w;
	int h;
} s_info = {
	.resize_test = 0,
	.resize_timer = NULL,
	.w = 0,
	.h = 0,
};

struct item {
	struct appdata *ad;
	char *dbox_id;
	int size;
	struct {
		int pressed;
		int x;
		int y;
	} down;
};

static void move_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	struct item *item = data;
	Evas_Event_Mouse_Move *move = event_info;

	if (abs(item->down.x - move->cur.canvas.x) > 7 || abs(item->down.y - move->cur.canvas.y) > 7) {
		item->down.pressed = 0;
	}
}

static void down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	struct item *item = data;
	Evas_Event_Mouse_Down *down = event_info;

	item->down.pressed = 1;
	item->down.x = down->canvas.x;
	item->down.y = down->canvas.y;
	LOGD("Down\n");
}

static Eina_Bool resize_test_cb(void *data)
{
	struct item *item = data;
	Evas_Coord w, h;
	Evas_Coord x, y;

	evas_object_geometry_get(item->ad->layout, &x, &y, &w, &h);
	if (w < 10 || h < 10) {
		s_info.resize_timer = NULL;
		return ECORE_CALLBACK_CANCEL;
	}

	evas_object_move(item->ad->layout, x + 10, y + 10);
	evas_object_resize(item->ad->layout, w - 10, h - 10);
	LOGD("%dx%d - %dx%d\n", x, y, w, h);
	return ECORE_CALLBACK_RENEW;
}

static Eina_Bool resize_test_fire_cb(void *data)
{
	s_info.resize_timer = ecore_timer_add(0.5f, resize_test_cb, data);
	if (!s_info.resize_timer) {
		LOGE("Failed to add a test timer\n");
	}
	return ECORE_CALLBACK_CANCEL;
}

static void up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	struct item *item = data;
	Evas_Object *box;
	Evas_Object *bg;
	int w, h;

	if (!item->down.pressed) {
		return;
	}

	box = elm_object_part_content_unset(item->ad->layout, "dynamicbox");
	if (box) {
		evas_object_del(box);
		elm_object_signal_emit(item->ad->layout, "hide", "dynamicbox");
		item->ad->box_created = 0;
	}

	box = evas_object_dynamicbox_add(item->ad->win, item->dbox_id, NULL, NULL, NULL, DYNAMICBOX_EVAS_DEFAULT_PERIOD);
	if (!box) {
		return;
	}

	bg = elm_object_part_content_get(item->ad->layout, "bg");
	if (!bg) {
		bg = elm_bg_add(item->ad->layout);
		elm_bg_color_set(bg, 0, 0, 0);
		evas_object_show(bg);
		elm_object_part_content_set(item->ad->layout, "bg", bg);
	}

	dynamicbox_service_get_size(item->size, &w, &h);
	evas_object_size_hint_min_set(bg, w, h);
	evas_object_size_hint_max_set(bg, w, h);
	evas_object_resize(bg, w, h);

	elm_object_part_content_set(item->ad->layout, "dynamicbox", box);
	item->down.pressed = 0;
	LOGD("Box loaded: %s %x (%dx%d)\n", item->dbox_id, item->size, w, h);
	elm_object_signal_emit(item->ad->layout, "show", "dynamicbox");
	item->ad->box_created = 1;

	if (s_info.resize_test) {
		if (s_info.resize_timer) {
			ecore_timer_del(s_info.resize_timer);
			s_info.resize_timer = NULL;
		}

		s_info.resize_timer = ecore_timer_add(3.0f, resize_test_fire_cb, item);
		if (!s_info.resize_timer) {
			LOGE("Failed to add a timer\n");
		}
	}
}

Evas_Object *preview_load_image(Evas_Object *parent, const char *filename)
{
	Evas_Object *preview;
	int w;
	int h;

	preview = evas_object_image_filled_add(evas_object_evas_get(parent));
	if (!preview) {
		LOGD("Failed to create an icon\n");
		return NULL;
	}

	evas_object_image_file_set(preview, filename, NULL);
	if (evas_object_image_load_error_get(preview) != EVAS_LOAD_ERROR_NONE) {
		int ret;
		evas_object_image_file_set(preview, PREVIEW_DEFAULT, NULL);
		ret = evas_object_image_load_error_get(preview);
		if (ret != EVAS_LOAD_ERROR_NONE) {
			LOGE("Failed to load an image file [%d]\n", ret);
		}
	}

	evas_object_image_size_get(preview, &w, &h);
	evas_object_image_fill_set(preview, 0, 0, w, h);
	evas_object_resize(preview, w, h);
	evas_object_show(preview);
	LOGD("Image size: %dx%d\n", w, h);

	return preview;
}

Evas_Object *preview_create_layout(struct appdata *ad, Evas_Object *parent, const char *filename, const char *name, int type)
{
	Evas_Object *layout;
	Evas_Object *preview;
	Evas_Object *box;
	Evas_Coord w, h;

	layout = elm_layout_add(parent);
	if (!layout) {
		return NULL;
	}

	preview = preview_load_image(layout, filename);
	if (!preview) {
		evas_object_del(layout);
		return NULL;
	}

	if (elm_layout_file_set(layout, LAYOUT_EDJ, "preview") != EINA_TRUE) {
		LOGE("Failed to load EDJE\n");
	}

	dynamicbox_service_get_size(type, &w, &h);
	evas_object_size_hint_min_set(preview, w, h);

	elm_object_part_content_set(layout, "preview", preview);
	elm_object_part_text_set(layout, "name", name);

	box = elm_box_add(ad->scroller_box);
	evas_object_show(layout);
	evas_object_size_hint_min_set(box, s_info.w, 0);
	elm_object_part_content_set(layout, "bg", box);

	evas_object_size_hint_min_set(layout, s_info.w, h);
	evas_object_show(layout);

	LOGD("PREVIEW: %s name(%s) %X (%dx%d)\n", filename, name, type, w, h);

	elm_object_signal_emit(layout, "show", "preview");

	return layout;
}

static int pkglist_cb(const char *pkgid, const char *dbox_id, int is_prime, void *data)
{
	struct appdata *ad = data;
	Evas_Object *preview;
	struct item *item;
	char *filename;
	int cnt = DBOX_NR_OF_SIZE_LIST;
	int type[DBOX_NR_OF_SIZE_LIST];
	int i;

	if (dynamicbox_service_nodisplay(dbox_id)) {
		LOGD("NODISPLAY: %s\n", dbox_id);
		return DBOX_STATUS_ERROR_NONE;
	}

	dynamicbox_service_get_supported_size_types(dbox_id, &cnt, type);
	for (i = 0; i < cnt; i++) {
		filename = dynamicbox_service_preview(dbox_id, type[i]);
		if (!filename || access(filename, F_OK) != 0) {
			filename = strdup(PREVIEW_DEFAULT);
			if (!filename) {
				LOGE("Heap: %s\n", strerror(errno));
				return DBOX_STATUS_ERROR_OUT_OF_MEMORY;
			}
		}
		preview = preview_create_layout(ad, ad->win, filename, dbox_id, type[i]);
		if (!preview) {
			free(filename);
			return DBOX_STATUS_ERROR_FAULT;
		}
		LOGD("preview: %p\n", preview);

		item = calloc(1, sizeof(*item));
		if (!item) {
			free(filename);
			evas_object_del(preview);
			return DBOX_STATUS_ERROR_OUT_OF_MEMORY;
		}
		item->ad = ad;
		item->size = type[i];
		item->dbox_id = strdup(dbox_id);
		if (!item->dbox_id) {
			free(filename);
			free(item);
			evas_object_del(preview);
			return DBOX_STATUS_ERROR_OUT_OF_MEMORY;
		}

		elm_box_pack_end(ad->scroller_box, preview);

		evas_object_event_callback_add(preview, EVAS_CALLBACK_MOUSE_DOWN, down_cb, item);
		evas_object_event_callback_add(preview, EVAS_CALLBACK_MOUSE_MOVE, move_cb, item);
		evas_object_event_callback_add(preview, EVAS_CALLBACK_MOUSE_UP, up_cb, item);

		free(filename);
	}
	return DBOX_STATUS_ERROR_NONE;
}

static void load_widgets(struct appdata *ad)
{
	int cnt;
	cnt = dynamicbox_service_get_pkglist(pkglist_cb, ad);
	LOGD("Package list: %d\n", cnt);
	if (cnt == 0) {
		elm_object_signal_emit(ad->layout, "empty", "content");
	}
}

Eina_Bool key_up_event_cb(void *data, int type, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	Ecore_Event_Key *ev = event_info;

	if(!strcmp(ev->keyname, KEY_BACK)) {
		if (ad->box_created) {
			Evas_Object *box;
			box = elm_object_part_content_unset(ad->layout, "dynamicbox");
			if (box) {
				evas_object_del(box);
				elm_object_signal_emit(ad->layout, "hide", "dynamicbox");
				ad->box_created = 0;
			}
		} else {
			elm_exit();
		}
	}

	return ECORE_CALLBACK_PASS_ON;
}

static bool app_create(void *data)
{
	struct appdata *ad = data;
	Evas_Object *bg;
	Evas_Object *scroller;
	int auto_feed;

	ad->win = elm_win_add(NULL, "DBoxViewer", ELM_WIN_BASIC);
	if (!ad->win) {
		LOGD("Failed to create a window\n");
		return false;
	}

	evas_object_show(ad->win);

	bg = elm_bg_add(ad->win);
	elm_win_resize_object_add(ad->win, bg);
	elm_bg_color_set(bg, 0, 0, 0);
	evas_object_show(bg);

	ad->layout = elm_layout_add(ad->win);
	if (!ad->layout) {
		evas_object_del(bg);
		evas_object_del(ad->win);
		return false;
	}
	elm_win_resize_object_add(ad->win, ad->layout);
	if (elm_layout_file_set(ad->layout, LAYOUT_EDJ, "layout") != EINA_TRUE) {
		LOGE("Failed to load an edje\n");
		evas_object_del(bg);
		evas_object_del(ad->layout);
		evas_object_del(ad->win);
		return false;
	}
	evas_object_show(ad->layout);

	scroller = elm_scroller_add(ad->win);
	if (!scroller) {
		evas_object_del(ad->win);
		ad->win = NULL;
		LOGD("Failed to create a scroller\n");
		return false;
	}
	evas_object_geometry_get(ad->win, NULL, NULL, &s_info.w, &s_info.h);
	LOGD("Window: %dx%d\n", s_info.w, s_info.h);

	ecore_x_window_size_get(0, &s_info.w, &s_info.h);
	LOGD("Window: %dx%d\n", s_info.w, s_info.h);

	elm_scroller_page_size_set(scroller, s_info.w, s_info.h);
	elm_object_part_content_set(ad->layout, "scroller", scroller);
	elm_scroller_bounce_set(scroller, EINA_TRUE, EINA_FALSE);

	ad->scroller_box = elm_box_add(scroller);
	if (!ad->scroller_box) {
		evas_object_del(scroller);
		evas_object_del(ad->win);

		ad->win = NULL;
		LOGD("Failed to create a box\n");
		return false;
	}
	elm_box_homogeneous_set(ad->scroller_box, EINA_TRUE);
	elm_box_horizontal_set(ad->scroller_box, EINA_TRUE);
	evas_object_size_hint_weight_set(ad->scroller_box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(ad->scroller_box);

	elm_object_content_set(scroller, ad->scroller_box);

	/* DYNAMICBOX init */
	evas_object_dynamicbox_init(ad->win, 0);

	/* DYNAMICBOX init */
	evas_object_dynamicbox_conf_set(DYNAMICBOX_EVAS_EVENT_AUTO_FEED, 1);
	if (vconf_get_int(VCONF_KEY_VIEWER_AUTO_FEED, &auto_feed) >= 0) {
		LOGD("========> Enable autofeed? %d\n", auto_feed);
	}

	load_widgets(ad);

	ad->handler = ecore_event_handler_add(ECORE_EVENT_KEY_UP, (Ecore_Event_Handler_Cb)key_up_event_cb, ad);
	return true;
}

static void app_terminate(void *data)
{
	struct appdata *ad = data;

	/* DYNAMICBOX fini */
	evas_object_dynamicbox_fini();

	if (ad->win) {
		evas_object_del(ad->win);
		ad->win = NULL;
	}
}

static void app_pause(void *data)
{
	/* DYNAMICBOX pause */
	evas_object_dynamicbox_paused();
}

static void app_resume(void *data)
{
	/* DYNAMICBOX resume */
	evas_object_dynamicbox_resumed();
}

static void _app_control(app_control_h service, void *data)
{
	struct appdata *ad = data;
	evas_object_dynamicbox_resumed();
	elm_win_activate(ad->win);
}

int main(int argc, char *argv[])
{
	struct appdata ad;

	app_event_callback_s event_callback;

	setenv("ELM_ENGINE", "gl", 0);

	event_callback.create = app_create;
	event_callback.terminate = app_terminate;
	event_callback.pause = app_pause;
	event_callback.resume = app_resume;
	event_callback.app_control = _app_control;
	event_callback.low_memory = NULL;
	event_callback.low_battery = NULL;
	event_callback.device_orientation = NULL;
	event_callback.language_changed = NULL;
	event_callback.region_format_changed = NULL;

	memset(&ad, 0x0, sizeof(struct appdata));

	if (setenv("BUFMGR_LOCK_TYPE", "once", 0) < 0) {
		LOGE("setenv(BUFMGR_LOCK_TYPE) is failed: %s", strerror(errno));
	}

	if (setenv("BUFMGR_MAP_CACHE", "true", 0) < 0) {
		LOGE("setenv(BUFMGR_MAP_CACHE) is failed: %s", strerror(errno));
	}

	return app_efl_main(&argc, &argv, &event_callback, &ad);
}
