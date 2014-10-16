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
#include "genlist.h"
#include <stdlib.h>
#include <time.h>

/*********************************************************
  Genlist Color
 ********************************************************/

#define NUM_OF_ITEMS 2000
typedef struct _Item_Data {
	Elm_Object_Item *item;
	int bg_r, bg_g, bg_b, bg_a;
	char *name;
	char *country;
	char *photo_path;
} Item_Data;

#define NUM_OF_PHOTO 9
static char *photo_path[] = {
	"00_list_photo_default.png", "iu100.jpg", "iu2-100.jpg", "koo100.jpg", "top100.jpg",
	"boa100.jpg", "kimtaehee100.jpg", "moon100.jpg", "taeyon100.jpg",
	NULL
};

static char **genlist_demo_names = NULL;
static char **genlist_demo_country_names = NULL;

static void
_view_free_cb(void *data, Evas *e, Evas_Object *obj, void *ei)
{
	Elm_Theme *th = data;
	// Free customized theme
	elm_theme_free(th);
}

static void _change_color_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	Item_Data *item_data = (Item_Data *)elm_object_item_data_get(data);
	item_data->bg_r = rand() % 255;
	item_data->bg_g = rand() % 255;
	item_data->bg_b = rand() % 255;
	item_data->bg_a = 255;
	elm_genlist_item_update(data);
}

static char *_gl_text_get(void *data, Evas_Object *obj, const char *part)
{
	Item_Data *item_data = (Item_Data *)data;

	if (item_data) {
		if (!strcmp(part, "elm.text")) {
			return strdup(item_data->name);
		} else if (!strcmp(part, "elm.text.sub")) {
			return strdup(item_data->country);
		}
	}

	return NULL;
}

static Evas_Object *_gl_content_get(void *data, Evas_Object *obj, const char *part)
{
	Item_Data *item_data = (Item_Data *)data;
	Evas_Object *image, *rect, *button;

        if (item_data) {
             if (!strcmp(part, "elm.thumbnail")) {
                  image = evas_object_image_add(evas_object_evas_get(obj));
                  evas_object_image_load_size_set(image, 100 * elm_config_scale_get(),
                                                  100 * elm_config_scale_get());
                  evas_object_image_fill_set(image, 0, 0, 100 * elm_config_scale_get(),
                                             100 * elm_config_scale_get());
                  evas_object_image_file_set(image, item_data->photo_path, NULL);
                  return image;
             } else if (!strcmp(part, "elm.swallow.color.bg")) {
                  rect = evas_object_rectangle_add(evas_object_evas_get(obj));
                  evas_object_color_set(rect, item_data->bg_r, item_data->bg_g, item_data->bg_b, item_data->bg_a);
                  return rect;
             } else if (!strcmp(part, "elm.swallow.end")) {
                  button = elm_button_add(obj);
                  elm_object_text_set(button, _("Change Color"));
                  evas_object_propagate_events_set(button, EINA_FALSE);
                  evas_object_smart_callback_add(button, "clicked", _change_color_button_cb, item_data->item);
                  return button;
             }
        }

	return NULL;
}

static Eina_Bool _gl_state_get(void *data, Evas_Object *obj, const char *part)
{
	return EINA_FALSE;
}

static void _gl_del(void *data, Evas_Object *obj)
{
	Item_Data *item_data = (Item_Data *)data;
	if (item_data) {
		if (item_data->photo_path)
			free(item_data->photo_path);
		free(item_data);
	}
}

static char *_get_name(int index)
{
	int num = index;
	if (num < 0)
		num = 0;
	return genlist_demo_names[(num % NUM_OF_GENLIST_DEMO_NAMES)];
}

static char *_get_country(int index)
{
	int num = index;
	if (num < 0)
		num = 0;
	return genlist_demo_country_names[(num % NUM_OF_GENLIST_DEMO_COUNTRY_NAMES)];
}

static char *_get_photo_path(int index)
{
	char *path = NULL;
	path = calloc(1, PATH_MAX);
	sprintf(path, ICON_DIR"/genlist/%s", photo_path[(index % NUM_OF_PHOTO)]);
	return path;
}

static void _gl_sel(void *data, Evas_Object *obj, void *event_info)
{
	if (event_info)
		elm_genlist_item_selected_set(event_info, EINA_FALSE);
}

static Evas_Object *_create_genlist(Evas_Object *parent, Elm_Theme *th)
{
	int index = 0;
	Evas_Object *genlist;
	Elm_Object_Item *item;
	Item_Data *item_data;

	genlist_demo_names = genlist_get_demo_names();
	genlist_demo_country_names = genlist_get_demo_country_names();

	// Create genlist
	genlist = elm_genlist_add(parent);
	// Use the new theme for genlist
	elm_object_theme_set(genlist, th);

	// HOMOGENEOUS MODE
	// If item height is same when each style name is same,
	// Use homogeneous mode.
	printf("Homogeneous mode enabled\n");
	elm_genlist_homogeneous_set(genlist, EINA_TRUE);

	Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
	// Set genlist item class
	itc->item_style = "elm_demo_tizen/dynamic_text_style_change";
	itc->func.text_get = _gl_text_get;
	itc->func.content_get = _gl_content_get;
	itc->func.state_get = _gl_state_get;
	itc->func.del = _gl_del;

	// Genlist item append
	for (index = 0; index < NUM_OF_ITEMS; index++) {
		// Create a user data for each item
		// This should be freed in _gl_del
		item_data = calloc(1, sizeof(Item_Data));

		// Set item's user data
		item_data->name = _get_name(index);
		item_data->country = _get_country(index);
		item_data->photo_path = _get_photo_path(index);
		item_data->bg_r = rand() % 255;
		item_data->bg_g = rand() % 255;
		item_data->bg_b = rand() % 255;
		item_data->bg_a = 255;

		// Append a genlist item
		item = elm_genlist_item_append(genlist, // Genlist
				itc,                           // Item Class
				item_data,                      // User data
				NULL,                           // Parent
				ELM_GENLIST_ITEM_NONE,          // Item flags
				_gl_sel,                        // Selected Callback
				NULL                            // Selected Callback Parameter
				);

		// Save item's pointer to use it later
		item_data->item = item;
	}
	elm_genlist_item_class_free(itc);

	return genlist;
}

void genlist_color_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *genlist;
	struct appdata *ad;
	Elm_Theme *th;

	ad = (struct appdata *) data;
	if (ad == NULL) return;

	// Set random seed
	srand(time(NULL));

	/* Use of Elm_Theme *th
		This is for performance enhancement. Genlist in this sample uses
		customized styles, so we need to use customized edc. If we just add/del
		extension theme, all created widgets will be refreshed
		(_theme_hook call). But using a separate Elm_Theme, only genlist will be
		refreshed. This enhances performance dramatically.
	 **/

	// Create a new theme for customized genlist
	th = elm_theme_new();
	// New theme will refer the default theme
	elm_theme_ref_set(th, NULL);
	// Extend the new theme to use customized styles as well
	elm_theme_extension_add(th, ELM_DEMO_EDJ);

	// Create Genlist
	genlist = _create_genlist(ad->nf, th);

	elm_naviframe_item_push(ad->nf, "Dynamic Color Change", NULL, NULL, genlist, NULL);
	evas_object_event_callback_add(genlist, EVAS_CALLBACK_FREE, _view_free_cb, th);
}
