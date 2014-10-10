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

#include "elmdemo_util.h"
#include "elmdemo_test.h"
#include "genlist.h"

#define NUM_OF_ITEMS 2000
static char **genlist_demo_names = NULL;

static void
_button_jumpto_top_cb(void *data, Evas_Object * obj, void *event_info)
{
	Evas_Object *genlist;
	Elm_Object_Item *it;

	if(!data)
		return;
	genlist = (Evas_Object *) data;
	it = elm_genlist_first_item_get(genlist);
	elm_genlist_item_bring_in(it, ELM_GENLIST_ITEM_SCROLLTO_TOP);
}

static char *
_gl_text_get(void* data, Evas_Object *obj, const char* part)
{
	int index = ((int) data) % NUM_OF_GENLIST_DEMO_NAMES;

	if (!strcmp(part, "elm.text")) {
		if (index)
			return strdup(genlist_demo_names[index%NUM_OF_GENLIST_DEMO_NAMES]);
	}
	return NULL;
}

static Evas_Object *
_create_genlist(Evas_Object *parent)
{
	int index;
	Evas_Object *genlist, *button;

	genlist_demo_names = genlist_get_demo_names();

	if (!parent)
		return NULL;

	Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();

	genlist = elm_genlist_add(parent);
	// HOMOGENEOUS MODE
	// If item height is same when each style name is same,
	// Use homogeneous mode.
	printf("Homogeneous mode enabled\n");
	elm_genlist_homogeneous_set(genlist, EINA_TRUE);

	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);

	itc->item_style = "1text";
	itc->func.text_get = _gl_text_get;

	for (index = 0; index < NUM_OF_ITEMS; index++)
	{
		elm_genlist_item_append(
				genlist,
				itc,
				(void *) index,
				NULL,
				ELM_GENLIST_ITEM_NONE,
				NULL,
				NULL);
	}
	elm_genlist_item_class_free(itc);

	button = elm_button_add(genlist);
	evas_object_smart_callback_add(button, "clicked", _button_jumpto_top_cb, (void *) genlist);
	elm_object_part_content_set(genlist, "elm.swallow.jump_to_top", button);
	elm_object_style_set(button, "jumpto_top");
	evas_object_show(button);

	return genlist;
}

void
genlist_scroll_jump_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad;
	Evas_Object *genlist;

	if (!data)
		return;
	ad = (struct appdata *) data;

	genlist = _create_genlist(ad->nf);
	evas_object_show(genlist);

	elm_naviframe_item_push(ad->nf, _(("Genlist Scroll Jump")), NULL, NULL, genlist, NULL);
}
