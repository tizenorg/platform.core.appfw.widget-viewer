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
 Genlist Radio
 ********************************************************/
#define NUM_OF_ITEMS 2000
static int state_index = 0; //selected radio index
static char **genlist_demo_names = NULL;

typedef struct _Item_Data
{
	int index;
	Evas_Object *genlist;
	Elm_Object_Item *item;
} Item_Data;

static void _gl_del(void *data, Evas_Object *obj)
{
	// FIXME: Unrealized callback can be called after this.
	// Accessing Item_Data can be dangerous on unrealized callback.
	Item_Data *id = data;
	if (id) free(id);
}

static char *
_gl_text_get(void *data, Evas_Object *obj, const char *part)
{
	Item_Data *id = data;
	int index = (id->index) % NUM_OF_GENLIST_DEMO_NAMES;

	if (!strcmp(part, "elm.text")) return strdup(genlist_demo_names[index]);
	return NULL;
}

static void
_radio_cb(void *data, Evas_Object *obj, void *ei)
{
	Item_Data *id = data;
	Elm_Object_Item *it = elm_genlist_nth_item_get(id->genlist, id->index);
	elm_genlist_item_selected_set(it, EINA_TRUE);
}

static Evas_Object *
_gl_content_get(void *data, Evas_Object *obj, const char *part)
{
	Item_Data *id = data;
	int index = id->index;
	Evas_Object *radio;
	Evas_Object *radio_main = evas_object_data_get(obj, "radio_main");

	if (!strcmp(part, "elm.icon")) {
		radio = elm_radio_add(obj);
		elm_object_style_set(radio, "default/genlist");
		elm_radio_state_value_set(radio, index);
		elm_radio_group_add(radio, radio_main);
		if (index == state_index)
			elm_radio_value_set(radio, state_index);
		evas_object_size_hint_weight_set(radio, EVAS_HINT_EXPAND,
			EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(radio, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_smart_callback_add(radio, "changed", _radio_cb, id);

		// If no divider, unregister access object
		elm_access_object_unregister(radio);
		return radio;
	}

	return NULL;
}

static Eina_Bool
_gl_state_get(void *data, Evas_Object *obj, const char *part)
{
	return EINA_FALSE;
}

static void
_gl_sel(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *item = (Elm_Object_Item *) event_info;

	if (item) {
		Item_Data *id = elm_object_item_data_get(item);
		int index = id->index;
		printf("[Genlist] Selected Text : %s\n",
			genlist_demo_names[index % NUM_OF_GENLIST_DEMO_NAMES]);
		Evas_Object *radio = elm_object_item_part_content_get(item, "elm.icon");
		state_index = index;
		elm_radio_value_set(radio, state_index);
	}
}

static void _view_free_cb(void *data, Evas *e, Evas_Object *obj, void *ei)
{
	Evas_Object *radio_main = evas_object_data_get(obj, "radio_main");
	if (radio_main) evas_object_del(radio_main);
}

static Evas_Object *
_create_genlist(Evas_Object *parent)
{
	int index;
	Evas_Object *genlist, *radio_main;

	genlist_demo_names = genlist_get_demo_names();

	Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
	// Create genlist
	genlist = elm_genlist_add(parent);

	// HOMOGENEOUS MODE
	// If item height is same when each style name is same,
	// Use homogeneous mode.
	printf("Homogeneous mode enabled\n");
	elm_genlist_homogeneous_set(genlist, EINA_TRUE);

	radio_main = elm_radio_add(genlist);
	elm_radio_state_value_set(radio_main, 0);
	elm_radio_value_set(radio_main, 0);
	evas_object_data_set(genlist, "radio_main", radio_main);

	// Set genlist item class
	itc->item_style = "1text.1icon.3";
	itc->func.text_get = _gl_text_get;
	itc->func.content_get = _gl_content_get;
	itc->func.state_get = _gl_state_get;
	itc->func.del = _gl_del;

	// Append items
	for (index = 0; index < NUM_OF_ITEMS; index++) {
		Item_Data *id = calloc(sizeof(Item_Data), 1);
		id->index  = index;
		Elm_Object_Item *item =
			elm_genlist_item_append(genlist, // genlist object
				itc, // item class
				id, // data
				NULL, ELM_GENLIST_ITEM_NONE, _gl_sel, NULL);
		id->item = item;
		id->genlist = genlist;
	}
	elm_genlist_item_class_free(itc);

	return genlist;
}

void
genlist_radio_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *) data;
	Evas_Object *genlist;

	genlist = _create_genlist(ad->nf);

	elm_naviframe_item_push(ad->nf, _("Genlist with Radio"), NULL, NULL, genlist, NULL);
	evas_object_event_callback_add(genlist, EVAS_CALLBACK_FREE, _view_free_cb, NULL);
}
