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

/*********************************************************
  Genlist Externals
 ********************************************************/

#define NUM_OF_ITEMS 40
static Elm_Genlist_Item_Class itc;
static char **genlist_demo_names = NULL;

static void _genlist_item_class_set(void);
static void _genlist_item_append(Evas_Object *genlist);

static char *_gl_text_get(void *data, Evas_Object *obj, const char *part)
{
	int index = (int) data % NUM_OF_ITEMS;

	if (!strcmp(part, "elm.text")) {
		return strdup(genlist_demo_names[index]);
	}
	return NULL;
}

static Evas_Object *_gl_content_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *icon = NULL;

	if (!strcmp(part, "elm.icon")) {
		icon = elm_image_add(obj);
		elm_image_file_set(icon, ICON_DIR"/genlist/00_brightness_right.png", NULL);
		evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
		return icon;
	}

	return NULL;
}

static void _gl_sel(void *data, Evas_Object *obj, void *event_info)
{
	int index;
	Elm_Object_Item *item = (Elm_Object_Item *)event_info;

	if (item != NULL) {
		elm_genlist_item_selected_set(item, EINA_FALSE);
		index = (int)elm_object_item_data_get(item);
		printf("[Genlist] Selected Text : %s\n", genlist_demo_names[index]);
	}
}

static void _genlist_item_append(Evas_Object *genlist)
{
	int index;
	Elm_Object_Item *item;

	// Set genlist block size.
	// Optimize your application with appropriate genlist block size.
	elm_genlist_block_count_set(genlist, 14);

	// Append items
	for (index = 0; index < 4*NUM_OF_ITEMS; index++) {
		item = elm_genlist_item_append(
				genlist,			// genlist object
				&itc,				// item class
				(void *) index,		// data
				NULL,
				ELM_GENLIST_ITEM_NONE,
				_gl_sel,
				NULL
		);
	}
}

static void _genlist_item_class_set(void)
{
	// Set genlist item class
	itc.item_style = "1line_icontext";
	itc.func.text_get = _gl_text_get;
	itc.func.content_get = _gl_content_get;
	itc.func.state_get = NULL;
	itc.func.del = NULL;
}

void genlist_externals_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad;
	Evas_Object *layout = NULL, *genlist = NULL;
	ad = (struct appdata *) data;
	if (ad == NULL) return;

	genlist_demo_names = genlist_get_demo_names();

	// Create a layout which contains genlist object as edje_external.
	ad->layout_main = layout = elm_layout_add(ad->nf);
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "elm_demo_tizen/layout/genlist_externals");

	// Get genlist object from the layout
	genlist = edje_object_part_external_object_get(_EDJ(ad->layout_main), "genlist");
	_genlist_item_class_set();
	_genlist_item_append(genlist);

	elm_naviframe_item_push(ad->nf, _("Genlist Externals"), NULL, NULL, layout, NULL);
}
