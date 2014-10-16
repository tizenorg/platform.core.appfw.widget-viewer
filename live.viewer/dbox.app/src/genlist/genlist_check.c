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
  Genlist Check
 ********************************************************/
#define NUM_OF_ITEMS 2000
Evas_Object *box, *select_all_layout, *genlist;
static int checked_count = 0;
static Eina_Bool state_pointer[NUM_OF_ITEMS] = {0};//check states
static char **genlist_demo_names = NULL;

static void _select_all_chk_changed_cb(void *data, Evas_Object *obj, void *ei)
{
	Eina_Bool state = elm_check_state_get(obj);

	if (state) checked_count = elm_genlist_items_count(genlist);
	else checked_count = 0;

	Elm_Object_Item *it = elm_genlist_first_item_get(genlist);
	while(it) {
		int index = (int)elm_object_item_data_get(it);
		// For realized items, set state of real check object
		Evas_Object *ck = elm_object_item_part_content_get(it, "elm.icon");
		if (ck) elm_check_state_set(ck, state);
		// For all items (include unrealized), just set pointer state
		state_pointer[index] = state;
		it = elm_genlist_item_next_get(it);
	}
}

static void _select_all_layout_down_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	printf("select all layout clicked\n");
	Evas_Object *check = elm_object_part_content_get(select_all_layout, "elm.icon");
	Eina_Bool state = elm_check_state_get(check);
	elm_check_state_set(check, !state);
	_select_all_chk_changed_cb(data, check, NULL);
}

static Evas_Object *
_create_select_all_layout(Evas_Object *parent)
{
	Evas_Object *layout = elm_layout_add(parent);
	elm_layout_theme_set(layout, "genlist", "item", "select_all/default");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_FILL);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_event_callback_add(layout, EVAS_CALLBACK_MOUSE_DOWN, _select_all_layout_down_cb, NULL);
	evas_object_show(layout);

	Evas_Object *check = elm_check_add(layout);
	evas_object_propagate_events_set(check, EINA_FALSE);
	evas_object_smart_callback_add(check, "changed", _select_all_chk_changed_cb, NULL);
	elm_object_part_content_set(layout, "elm.icon", check);
	elm_object_part_text_set(layout, "elm.text", "Select All");
	return layout;
}

static char *_gl_text_get(void *data, Evas_Object *obj, const char *part)
{
	int index = ((int) data) % NUM_OF_GENLIST_DEMO_NAMES;

	if (!strcmp(part, "elm.text")) {
		if (!index) {
			return strdup(_("Select All"));
		} else {
			return strdup(genlist_demo_names[index%NUM_OF_GENLIST_DEMO_NAMES]);
		}
	}
	return NULL;
}

static void _chk_changed_cb(void *data, Evas_Object *obj, void *ei)
{
	Eina_Bool state;

	state = elm_check_state_get(obj);
	if (state) checked_count++;
	else checked_count--;
	printf("check changed, count: %d\n", checked_count);

	Evas_Object *check = elm_object_part_content_get(select_all_layout, "elm.icon");
	if (elm_genlist_items_count(genlist) == checked_count)
		elm_check_state_set(check, EINA_TRUE);
	else elm_check_state_set(check, EINA_FALSE);
}

static void _gl_sel(void *data, Evas_Object *obj, void *ei)
{
	Elm_Object_Item *item = ei;

	printf("item selected\n");
	elm_genlist_item_selected_set(item, EINA_FALSE);

	// Update check button
	Evas_Object *ck = elm_object_item_part_content_get(ei, "elm.icon");
	Eina_Bool state = elm_check_state_get(ck);
	elm_check_state_set(ck, !state);

	_chk_changed_cb(data, ck, NULL);
}

static void
_check_cb(void *data, Evas_Object *obj, const char *emission, const char* src)
{
	int index = (int)data;
	printf("Check show finished callback Item: %d \n",index);
}
static Evas_Object *_gl_content_get(void *data, Evas_Object *obj, const char *part)
{
	int index = (int)data;
	Evas_Object *check;

	if (!strcmp(part, "elm.icon") || !strcmp(part, "elm.swallow.icon")) {
		check = elm_check_add(obj);
		elm_object_style_set(check, "default/genlist");
		//set the State pointer to keep the current UI state of Checkbox.
		elm_check_state_pointer_set(check, &(state_pointer[index]));
		// Repeat events to below object (genlist)
		// So that if check is clicked, genlist can be clicked.
		evas_object_repeat_events_set(check, EINA_TRUE);
		evas_object_propagate_events_set(check, EINA_FALSE);
		evas_object_size_hint_weight_set(check, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(check, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_object_signal_callback_add(check, "elm,action,show,finished", "elm",
				_check_cb, (void *)index);

		// If no divider, unregister access object
		elm_access_object_unregister(check);
		return check;
	}
	return NULL;
}

static Evas_Object * _create_genlist(Evas_Object *parent)
{
	int index;
	Evas_Object *genlist;
	Elm_Object_Item *git;

	genlist_demo_names = genlist_get_demo_names();

	// Create genlist
	genlist = elm_genlist_add(parent);
	// HOMOGENEOUS MODE
	// If item height is same when each style name is same,
	// Use homogeneous mode.
	elm_genlist_homogeneous_set(genlist, EINA_TRUE);
	printf("Homogeneous mode enabled\n");
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);

	Elm_Genlist_Item_Class *itc_sa = elm_genlist_item_class_new();
	Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
	// Set genlist item class for "select all"
	//itc_sa.item_style = "selectall_check";
	itc_sa->item_style = "elm_demo_tizen/select_all";
	itc_sa->func.text_get = _gl_text_get;
	itc_sa->func.content_get = _gl_content_get;

	// Set genlist item class
	itc->item_style = "1text.1icon.3";
	itc->func.text_get = _gl_text_get;
	itc->func.content_get = _gl_content_get;

	git = elm_genlist_item_append(genlist, itc_sa, (void *) 0, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(git, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);


	// Append items
	for (index = 1; index < NUM_OF_ITEMS; index++) {
		elm_genlist_item_append(
				genlist,			// genlist object
				itc,				// item class
				(void *) index,		// data
				NULL,
				ELM_GENLIST_ITEM_NONE,
				_gl_sel,
				NULL
		);
	}
	elm_genlist_item_class_free(itc);
	elm_genlist_item_class_free(itc_sa);

	return genlist;
}

void genlist_check_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;

	box = elm_box_add(ad->nf);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
	select_all_layout =_create_select_all_layout(box);
	evas_object_show(select_all_layout);
	elm_box_pack_end(box, select_all_layout);

	genlist = _create_genlist(box);
	evas_object_show(genlist);
	elm_box_pack_end(box, genlist);
	evas_object_show(box);

	elm_naviframe_item_push(ad->nf, _("Genlist with Check"), NULL, NULL, box, NULL);
}
