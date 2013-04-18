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
#include <ail.h>
#include <app.h>
#include <bundle.h>

#include <livebox.h>
#include <livebox-service.h>

#include "main.h"
#include "util.h"
#include "debug.h"
#include "scroller.h"
#include "lb.h"

static struct info {
	Evas_Object *window;
	Evas_Object *scroller;
} s_info = {
	.window = NULL,
	.scroller = NULL,
};

Evas_Object *main_get_window(void)
{
	return s_info.window;
}

static void click_cb(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *item;
	const char *label;

	item = elm_list_selected_item_get(obj);
	if (!item)
		return;

	label = elm_object_item_part_text_get(item, NULL);
	if (!label)
		return;

	DbgPrint("Label: %s (%s)\n", label, data);
	if (lb_add(s_info.scroller, data) < 0)
		ErrPrint("Failed to add a new livebox\n");
}

static int append_livebox_cb(const char *appid, const char *lbid, int is_prime, void *data)
{
	char *name;

	DbgPrint("%s - %s\n", appid, lbid);

	name = livebox_service_i18n_name(lbid, NULL);
	if (!name) {
		name = strdup(lbid);
		if (!name) {
			ErrPrint("Heap: %s\n", strerror(errno));
			return 0;
		}
	}

	DbgPrint("Name: %s\n", name);
	elm_list_item_append(data, name, NULL, NULL, click_cb, strdup(lbid));
	free(name);
	return 0;
}

static inline void livebox_list_create(void)
{
	Evas_Object *list;

	list = elm_list_add(s_info.window);
	if (!list) {
		ErrPrint("Failed to create a list\n");
		return;
	}

	evas_object_resize(list, 720, 1280);
	evas_object_show(list);

	DbgPrint("Get Package list\n");
	livebox_service_get_pkglist(append_livebox_cb, list);
	elm_list_go(list);

	scroller_append(s_info.scroller, list);
}

static bool app_create(void *data)
{
	Evas_Object *bg;
	DbgPrint("create");
	lb_init();

	s_info.window = elm_win_add(NULL, "Box viewer", ELM_WIN_BASIC);
	if (!s_info.window) {
		ErrPrint("Failed to create a window\n");
		return false;
	}
	elm_win_autodel_set(s_info.window, EINA_TRUE);

	evas_object_resize(s_info.window, 720, 1280);
	evas_object_show(s_info.window);

	bg = elm_bg_add(s_info.window);
	elm_win_resize_object_add(s_info.window, bg);
	elm_bg_color_set(bg, 0, 0, 255);
	evas_object_show(bg);

	s_info.scroller = scroller_create(s_info.window);
	if (!s_info.scroller) {
		evas_object_del(s_info.window);
		s_info.window = NULL;
		ErrPrint("Failed to create a scroller\n");
		return false;
	}

	evas_object_resize(s_info.scroller, 720, 1280);
	evas_object_show(s_info.scroller);

	livebox_list_create();

	return true;
}

static void app_terminate(void *data)
{
	DbgPrint("terminate");
	lb_fini();
	/*!
	 * \TODO
	 * Delete all objects from the scroller.
	 */

	scroller_destroy(s_info.scroller);
	evas_object_del(s_info.window);
	s_info.window = NULL;
}

static void app_pause(void *data)
{
	DbgPrint("pause");
}

static void app_resume(void *data)
{
	DbgPrint("resume");
}

static void app_reset(service_h service, void *data)
{
	DbgPrint("reset");
}

int main(int argc, char *argv[])
{
	app_event_callback_s event_callback;

	setenv("ELM_ENGINE", "gl", 0);
	event_callback.create = app_create;
	event_callback.terminate = app_terminate;
	event_callback.pause = app_pause;
	event_callback.resume = app_resume;
	event_callback.service = app_reset;
	event_callback.low_memory = NULL;
	event_callback.low_battery = NULL;
	event_callback.device_orientation = NULL;
	event_callback.language_changed = NULL;

	return app_efl_main(&argc, &argv, &event_callback, NULL);
}

/* End of a file */
