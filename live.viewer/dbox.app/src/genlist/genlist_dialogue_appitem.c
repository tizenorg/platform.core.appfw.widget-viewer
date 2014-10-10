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

/*********************************************************
  Genlist Dialogue Appitem
 ********************************************************/

static void _create_genlist(void *data, Evas_Object *obj, void *event_info);

static struct _menu_item menu_its[] = {
	{ "contact - 2text.1icon", _create_genlist },
	{ "contact - 3text.2icon", _create_genlist },
	{ NULL, NULL }
};

#define NUM_OF_ITEMS 10
#define NUM_OF_TIMES 10
static Eina_Bool dialoque_bg;

#define NUM_OF_GROUPS 5
static char *title[] = {
	NULL, N_("Colleagues"), NULL, NULL, N_("Classmates")
};

#define NUM_OF_NAMES 40
static char **genlist_demo_names = NULL;

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

static void _view_free_cb(void *data, Evas *e, Evas_Object *obj, void *ei)
{
	if (data) free(data);
}

static char *_gl_text_get(void *data, Evas_Object *obj, const char *part)
{
	int index = ((int)data) % NUM_OF_NAMES;
	return strdup(genlist_demo_names[index]);
}

static char* _gl_text_get_title(void *data, Evas_Object *obj,
		const char *part)
{
	int index = ((int)data / 5) % NUM_OF_GROUPS;
	return strdup(title[index]);
}

static Evas_Object *_gl_content_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *icon;

	icon = elm_image_add(obj);
	elm_image_file_set(icon, ICON_DIR"/genlist/00_list_photo_default.png", NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	return icon;
}

static void _gl_sel(void *data, Evas_Object *obj, void *event_info)
{
	if (event_info)
		elm_genlist_item_selected_set(event_info, EINA_FALSE);
}

static char *_accinfo_cb(void *data, Evas_Object *acc)
{
	char *dup = NULL;
	Elm_Object_Item *it = data;
	const char *txt = elm_object_item_part_text_get(it, "elm.text");
	if (!txt) {
		txt = elm_object_item_part_text_get(it, "elm.text.1");
	}
	if (txt) dup = strdup(txt);
	return dup;
}

static void _gl_realized(void *data, Evas_Object *obj, void *ei)
{
	if (!ei) return;
	int index = (int)elm_object_item_data_get(ei);
	//
	// ===== Accessibility ====
	if (elm_config_access_get()) {
		// Register bg_dialogue image instead whole item
		Evas_Object *acc = elm_object_item_part_access_register(ei, "bg_dialogue");
		elm_access_info_cb_set(acc, ELM_ACCESS_INFO, _accinfo_cb, ei);
	}

	if (index == 0){
		elm_object_item_signal_emit(ei, "elm,state,top", "");
		fprintf(stderr, "top item realized\n");
	} else if (index == (NUM_OF_ITEMS-1)) {
		elm_object_item_signal_emit(ei, "elm,state,bottom", "");
		fprintf(stderr, "bottom item realized\n");
	} else	{
		elm_object_item_signal_emit(ei, "elm,state,center", "");
		fprintf(stderr, "center item realized\n");
	}
}

static void _create_genlist(void *data, Evas_Object *obj, void *event_info)
{
	if (!data) return;
	const char *style = elm_object_item_text_get((Elm_Object_Item *)event_info);
	char *str = calloc(128, sizeof(char));
	sprintf(str, "dialogue/bg/%s", style+10);
	dialoque_bg = EINA_TRUE;

	int index;
	struct appdata *ad = data;
	Elm_Object_Item *item = NULL;
	Evas_Object *genlist;
	Elm_Genlist_Item_Class *itc, *itc2, *itc3;

	itc = elm_genlist_item_class_new();
	itc2 = elm_genlist_item_class_new();
	itc3 = elm_genlist_item_class_new();

	itc->item_style = str;
	itc->func.text_get = _gl_text_get;
	itc->func.content_get = _gl_content_get;

	// Set item class for dialogue separator
	itc2->item_style = "dialogue/seperator";

	// Set item class for dialogue title
	itc3->item_style = "dialogue/grouptitle";
	itc3->func.text_get = _gl_text_get_title;

	// Create genlist
	genlist = elm_genlist_add(ad->nf);
	// HOMOGENEOUS MODE
	// If item height is same when each style name is same,
	// Use homogeneous mode.
	elm_genlist_homogeneous_set(genlist, EINA_TRUE);
	printf("Homogeneous mode enabled\n");

	evas_object_smart_callback_add(genlist, "realized", _gl_realized, NULL);

	for (index = 0; index < NUM_OF_ITEMS; index++) {
		if (index % 5 == 0 && !dialoque_bg) {
			if (title[index/5%NUM_OF_GROUPS])
				item = elm_genlist_item_append(genlist, itc3, (void *)index, NULL, ELM_GENLIST_ITEM_NONE, _gl_sel, NULL);
			else
				item = elm_genlist_item_append(genlist, itc2, (void *)index, NULL, ELM_GENLIST_ITEM_NONE, _gl_sel, NULL);
			elm_genlist_item_select_mode_set(item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		} else {
			item = elm_genlist_item_append(genlist, itc, (void *)index, NULL, ELM_GENLIST_ITEM_NONE, _gl_sel, NULL);
		}
		if (index == 0)
			elm_object_item_disabled_set(item, EINA_TRUE);
	}
	elm_genlist_item_class_free(itc);
	elm_genlist_item_class_free(itc2);
	elm_genlist_item_class_free(itc3);

	elm_naviframe_item_push(ad->nf, _(style), NULL, NULL, genlist, NULL);
	evas_object_event_callback_add(genlist, EVAS_CALLBACK_FREE, _view_free_cb, str);
}

void genlist_dialogue_appitem_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad;
	Evas_Object *list;
	int i;
	dialoque_bg = EINA_FALSE;

	ad = (struct appdata *) data;
	if (ad == NULL) return;

	list = elm_list_add(ad->nf);
	elm_list_mode_set(list, ELM_LIST_COMPRESS);
	evas_object_smart_callback_add(list, "selected", _list_selected, NULL);

	for (i = 0; menu_its[i].name; i++) {
		elm_list_item_append(list, menu_its[i].name, NULL, NULL, menu_its[i].func, ad);
	}
	elm_list_go(list);

	elm_naviframe_item_push(ad->nf, _("Application Items"), NULL, NULL, list, NULL);

	genlist_demo_names = genlist_get_demo_names();
}
