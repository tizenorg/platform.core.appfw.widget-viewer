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
 Genlist Theme
 ********************************************************/

#define NUM_OF_ITEMS 2000
static char **genlist_demo_names = NULL;

static char *
_gl_text_get(void *data, Evas_Object *obj, const char *part)
{
	int index = ((int) data) % NUM_OF_GENLIST_DEMO_NAMES;

	if (!strcmp(part, "elm.text")) return strdup(genlist_demo_names[index]);
	return NULL;
}

static Evas_Object *
_gl_content_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *icon = elm_image_add(obj);
	elm_image_file_set(icon, ICON_DIR"/genlist/00_brightness_right.png", NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	return icon;
}

static void
_gl_sel(void *data, Evas_Object *obj, void *event_info)
{
	int index;
	Elm_Object_Item *item = (Elm_Object_Item *) event_info;

	if (item) {
		index = (int) elm_object_item_data_get(item);
		printf("[Genlist] Selected Text : %s\n",
			genlist_demo_names[index % NUM_OF_GENLIST_DEMO_NAMES]);
	}
}

static Evas_Object *
_create_genlist(void *data)
{
	struct appdata *ad = data;
	int index;
	Evas_Object *genlist;

	genlist_demo_names = genlist_get_demo_names();


	Elm_Genlist_Item_Class *itc;
	itc = elm_genlist_item_class_new();
	// Create genlist
	genlist = elm_genlist_add(ad->nf);

	// HOMOGENEOUS MODE
	// If item height is same when each style name is same,
	// Use homogeneous mode.
	printf("Homogeneous mode enabled\n");
	elm_genlist_homogeneous_set(genlist, EINA_TRUE);

	// Set genlist item class
	itc->item_style = "1text.1icon.7";
	itc->func.text_get = _gl_text_get;
	itc->func.content_get = _gl_content_get;

	// Append items
	for (index = 1; index < NUM_OF_ITEMS; index++) {
		elm_genlist_item_append(genlist, // genlist object
			itc, // item class
			(void *) index, // data
			NULL, ELM_GENLIST_ITEM_NONE, _gl_sel, NULL);
	}
	elm_genlist_item_class_free(itc);

	return genlist;
}

void
genlist_theme_default_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = data;
	Evas_Object *genlist;

	genlist = _create_genlist(ad);

	elm_naviframe_item_push(ad->nf, _("Theme default"), NULL, NULL, genlist, NULL);
}


void
genlist_theme_no_effect_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = data;
	Evas_Object *genlist;

	genlist = _create_genlist(ad);
	elm_object_style_set(genlist, "no_effect");

	elm_naviframe_item_push(ad->nf, _("Theme no_effect"), NULL, NULL, genlist, NULL);
}

