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
  Genlist Fast Scroll
 ********************************************************/
#define NUM_OF_ITEMS 204
#define NUM_OF_TITLES 204

#define ITEM_STYLE_CHECK "1text.1icon.2"
#define ITEM_STYLE_THUMBNAIL "1text.1icon.5.thumb.square"

typedef struct _Item_Data Item_Data;
static Eina_Bool state_pointer[2*NUM_OF_ITEMS] = {0};//check states
static Eina_Bool state_pointer_title[2*NUM_OF_TITLES] = {0};//check states of titles

static struct _menu_item menu_its[] = {
	{ "FastScroll", genlist_index_fastscroll_cb },
	{ "Check", genlist_index_fastscroll_cb },
	/* do not delete below */
	{ NULL, NULL }
};

static char *photo_path[] = {
	"00_list_photo_default.png", "iu.jpg", "iu2.jpg", "koo.jpg", "top.jpg", "boa.jpg",
	"kimtaehee.jpg", "moon.jpg", "taeyon.jpg"
};

static Evas_Object *
_gl_icon_photo_get(void *data, Evas_Object *obj, const char *part)
{
	int index = (int)data;
	index = index % (sizeof(photo_path)/sizeof(*photo_path));
	char buf[PATH_MAX];
	Evas_Object *icon = elm_image_add(obj);
	sprintf(buf, ICON_DIR"/genlist/%s", photo_path[index]);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_align_set(icon, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	return icon;
}

static Evas_Object *_gli_content_get(void *data, Evas_Object *obj, const char *part)
{
	int index = (int)data;
	Evas_Object *check;

	if (!strcmp(part, "elm.icon") || !strcmp(part, "elm.swallow.icon")) {
		check = elm_check_add(obj);
		elm_object_style_set(check, "default/genlist");
		//set the State pointer to keep the current UI state of Checkbox.
		elm_check_state_pointer_set(check, &(state_pointer[index]));
		evas_object_repeat_events_set(check, EINA_TRUE);
		evas_object_propagate_events_set(check, EINA_FALSE);
		evas_object_size_hint_weight_set(check, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(check, EVAS_HINT_FILL, EVAS_HINT_FILL);

		return check;
	}

	return NULL;
}

static Evas_Object *_glh_content_get(void *data, Evas_Object *obj, const char *part)
{
	int index = (int)data;
	Evas_Object *check;

	if (!strcmp(part, "elm.icon") || !strcmp(part, "elm.swallow.icon")) {
		check = elm_check_add(obj);
		elm_object_style_set(check, "default/genlist");
		//set the State pointer to keep the current UI state of Checkbox.
		elm_check_state_pointer_set(check, &(state_pointer_title[index]));
		evas_object_repeat_events_set(check, EINA_TRUE);
		evas_object_propagate_events_set(check, EINA_FALSE);
		evas_object_size_hint_weight_set(check, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(check, EVAS_HINT_FILL, EVAS_HINT_FILL);

		return check;
	}

	return NULL;
}

static Eina_Bool _gli_state_get(void *data, Evas_Object *obj, const char *part)
{
	return EINA_FALSE;
}

static void _list_selected(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *it = (Elm_Object_Item *) elm_list_selected_item_get(obj);

	if (it == NULL)
	{
		fprintf((LOG_PRI(LOG_ERR) == LOG_ERR?stderr:stdout), "List item is NULL.\n");
		return;
	}

	elm_list_item_selected_set(it, EINA_FALSE);
}

static char *_gli_text_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024];
	int index = (int) data;

	if ((index>>4) <= 15) {
		snprintf(buf, sizeof(buf), "%c%c", 'A' + ((index >> 4) & 0xf), 'a'
				+ ((index) & 0xf));
	}
	else {
		snprintf(buf, sizeof(buf), "%c%c", 'Q' + ((index >> 4) & 0xf), 'a'
				+ ((index) & 0xf));
	}

	return strdup(buf);
}

static char *_gli_title_text_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024];
	int index = (int) data;

	if ((index>>4) <= 15) {
		snprintf(buf, sizeof(buf), "%c", 'A' + ((index >> 4) & 0xf));
	}
	else {
		snprintf(buf, sizeof(buf), "%c", 'Q' + ((index >> 4) & 0xf));
	}

	return strdup(buf);
}

static void _index_delayed_changed(void *data, Evas_Object *obj, void *event_info)
{
	// called on a change but delayed in case multiple changes happen in a
	// short timespan
	elm_genlist_item_bring_in(elm_object_item_data_get(event_info), ELM_GENLIST_ITEM_SCROLLTO_TOP);
}

static void _index_changed(void *data, Evas_Object *obj, void *event_info)
{
	// this is calld on every change, no matter how often
	return;
}

static void _index_selected(void *data, Evas_Object *obj, void *event_info)
{
	// called on final select
	elm_genlist_item_bring_in(elm_object_item_data_get(event_info), ELM_GENLIST_ITEM_SCROLLTO_TOP);
	elm_index_item_selected_set(event_info, EINA_FALSE);
}

static Evas_Object* _create_layout_inner(Evas_Object* parent)
{
	Evas_Object* layout_inner;

	layout_inner = elm_layout_add(parent);
	elm_layout_theme_set(layout_inner, "layout", "application", "fastscroll");
	evas_object_size_hint_weight_set(layout_inner, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	return layout_inner;
}

static void _gl_sel(void *data, Evas_Object *obj, void *ei)
{
	if (!ei) return;
	elm_genlist_item_selected_set(ei, EINA_FALSE);
	int idx = (int)elm_object_item_data_get(ei);

	const Elm_Genlist_Item_Class *class = elm_genlist_item_item_class_get(ei);
	// Update check button
	Evas_Object *ck = elm_object_item_part_content_get(ei, "elm.icon");
	if (ck) {
		printf("check changed\n");
		Eina_Bool state = !elm_check_state_get(ck);
		elm_check_state_set(ck, state);
		state_pointer_title[idx] = state;

		// if group index is checked/unchecked, then all items in group must be check/uncheck
		if (!strcmp(class->item_style, "groupindex.icon")) {

			printf("update child items\n");
			Elm_Object_Item *it = elm_genlist_item_next_get(ei);
			while(it)  {
				const Elm_Genlist_Item_Class *child_class =
					elm_genlist_item_item_class_get(it);
				if (!child_class ||
					!strcmp(child_class->item_style, "groupindex.icon"))
					break;

				ck = elm_object_item_part_content_get(it, "elm.icon");
				if (ck) elm_check_state_set(ck, state);
				idx = (int)elm_object_item_data_get(it);
				state_pointer[idx] = state;

				it = elm_genlist_item_next_get(it);
			}
		}

	}


	/*
	_chk_changed_cb(data, ck, NULL);

	int index;
	Eina_Bool gb;

	index = (int)elm_object_item_data_get(item);
	elm_genlist_item_selected_set(item, EINA_FALSE);
	// if group index is checked/unchecked, then all items in group must be check/uncheck
	if (!strcmp(class->item_style, "groupindex.icon")) {
		state_pointer_title[index] = !state_pointer_title[index];
		elm_genlist_item_fields_update(item, "elm.icon", ELM_GENLIST_ITEM_FIELD_CONTENT);

		Elm_Object_Item *it = elm_genlist_item_next_get(item);
		while(it)  {
			const Elm_Genlist_Item_Class *class = elm_genlist_item_item_class_get(it);
			if (!class || strcmp(class->item_style, ITEM_STYLE_CHECK)) break;

			// Update child items
			state_pointer[(int)elm_object_item_data_get(it)] = state_pointer_title[index];
			elm_genlist_item_fields_update(it, "elm.icon", ELM_GENLIST_ITEM_FIELD_CONTENT);
			it = elm_genlist_item_next_get(it);
		}
	}
	*/
}

static Evas_Object* _create_fastscroll_genlist(void *data, Evas_Object* parent, const char *style)
{
	Elm_Object_Item *git=NULL;
	Evas_Object *genlist, *index;
	int index1, index2;
	char buf[32];

	Elm_Genlist_Item_Class *itci, *itch;
	itci = elm_genlist_item_class_new();
	itch = elm_genlist_item_class_new();

	// Create genlist
	genlist = elm_genlist_add(parent);
	elm_scroller_policy_set(genlist, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);

	// HOMOGENEOUS MODE
	// If item height is same when each style name is same,
	// Use homogeneous mode.
	elm_genlist_homogeneous_set(genlist, EINA_TRUE);
	printf("Homogeneous mode enabled\n");

	// Create index
	index = elm_index_add(parent);
	elm_object_part_content_set(parent, "elm.swallow.fastscroll", index);
	elm_index_autohide_disabled_set(index, EINA_TRUE);

	// Set item class
	if (!strcmp(style,"Check")) {
		itci->item_style = ITEM_STYLE_CHECK;
		itci->func.text_get = _gli_text_get;
		itci->func.content_get = _gli_content_get;
		itci->func.state_get = _gli_state_get;

		itch->item_style = "groupindex.icon";
		itch->func.content_get = _glh_content_get;
		itch->func.state_get = _gli_state_get;
		itch->func.text_get = _gli_title_text_get;
	}
	else {
		itci->item_style = ITEM_STYLE_THUMBNAIL;
		itci->func.text_get = _gli_text_get;
		itci->func.content_get = _gl_icon_photo_get;
		itch->item_style = "groupindex";
		itch->func.text_get = _gli_title_text_get;
	}

	index2 = 0;
	for (index1 = 0; index1 < NUM_OF_ITEMS; index1++) {
		if ((index2 & 0xf) == 0) {
			if ((index2>>4) <= 15)
				snprintf(buf, sizeof(buf), "%c", 'A' + ((index2 >> 4) & 0xf));
			else if ((index2>>4) <= 25)
				snprintf(buf, sizeof(buf), "%c", 'Q' + ((index2 >> 4) & 0xf));

			git = elm_genlist_item_append(genlist, itch, (void *) index2,
					NULL, ELM_GENLIST_ITEM_GROUP, _gl_sel, NULL);
			elm_index_item_append(index, buf, NULL, git);
		}
		elm_genlist_item_append(genlist, itci, (void *) index2,
					   git, ELM_GENLIST_ITEM_NONE, _gl_sel,
					   NULL);
		index2 += 2;
	}

	elm_genlist_item_class_free(itci);
	elm_genlist_item_class_free(itch);

	evas_object_smart_callback_add(index, "delay,changed", _index_delayed_changed, index);
	evas_object_smart_callback_add(index, "changed", _index_changed, NULL);
	evas_object_smart_callback_add(index, "selected", _index_selected, NULL);
	elm_index_level_go(index, 0);

	return genlist;
}

void genlist_index_fastscroll_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad;
	Evas_Object *genlist;
	Evas_Object *layout_inner;
	const char *menu_name = elm_object_item_text_get((Elm_Object_Item *)event_info);
	ad = (struct appdata *) data;
	if (ad == NULL) return;

	layout_inner = _create_layout_inner(ad->nf);
	genlist = _create_fastscroll_genlist(ad, layout_inner, menu_name);
	if (genlist == NULL) return;

	elm_object_part_content_set(layout_inner, "elm.swallow.content", genlist);

	elm_naviframe_item_push(ad->nf, _(menu_name), NULL, NULL, layout_inner, NULL);
}

void genlist_fastscroll_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad;
	Evas_Object *list;
	int i;

	ad = (struct appdata *) data;
	if (ad == NULL) return;

	list = elm_list_add(ad->nf);
	elm_list_mode_set(list, ELM_LIST_COMPRESS);
	evas_object_smart_callback_add(list, "selected", _list_selected, NULL);

	for (i = 0; menu_its[i].name; i++) {
		elm_list_item_append(list, menu_its[i].name, NULL, NULL,
				menu_its[i].func, ad);
	}
	elm_list_go(list);

	elm_naviframe_item_push(ad->nf, _("Indexed Lists"), NULL, NULL, list, NULL);
}
