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
  Genlist Dialogue Editfield
 ********************************************************/

typedef struct _Item_Data
{
	Elm_Object_Item *item;
	int dial;
	int title;
} Item_Data;

static void _gl_del(void *data, Evas_Object *obj)
{
	// FIXME: Unrealized callback can be called after this.
	// Accessing Item_Data can be dangerous on unrealized callback.
	Item_Data *id = data;
	if (id) free(id);
}

static char* _gl_text_get(void *data, Evas_Object *obj, const char *part)
{
	if (!strcmp(part, "elm.text")) return strdup("Title");
	return NULL;
}

static char* _gl_title_get(void *data, Evas_Object *obj, const char *part)
{
	Item_Data *id = data;
	if (!strcmp(part, "elm.text")) {
		if (id->title == 1)
			return strdup("Single Line");
		else if (id->title == 2)
			return strdup("Multi Line");
		else if (id->title == 3)
			return strdup("With transparent background");
	}
	return NULL;
}

static Evas_Object *_gl_minus_btn_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object * icon = elm_button_add(obj);
	elm_object_style_set(icon, "minus");
	evas_object_propagate_events_set(icon, EINA_FALSE);
	return icon;
}

static Evas_Object *_gl_content_get(void *data, Evas_Object *obj, const char *part)
{
	if (!strcmp(part, "elm.icon.edit"))
		return _gl_minus_btn_get(data, obj, part);
	else if (!strcmp(part, "elm.icon.entry")) { // Add elm_entry to current editfield genlist item.
		Evas_Object *entry = ea_editfield_add(obj, EA_EDITFIELD_SCROLL_SINGLELINE);
		elm_object_domain_translatable_part_text_set(entry, "elm.guide", "sys_string", "IDS_COM_BODY_EDIT"); // Add guide text to elm_entry.

		return entry;
	}
	return NULL;
}

static Evas_Object *_gl_multiline_content_get(void *data, Evas_Object *obj, const char *part)
{
	if (!strcmp(part, "elm.icon.edit"))
		return _gl_minus_btn_get(data, obj, part);
	else if (!strcmp(part, "elm.icon.entry")) { // Add elm_entry to current editfield genlist item.
		Evas_Object *entry = ea_editfield_add(obj, EA_EDITFIELD_MULTILINE);
		ea_editfield_clear_button_disabled_set(entry, EINA_TRUE);
		elm_object_domain_translatable_part_text_set(entry, "elm.guide", "sys_string", "IDS_COM_BODY_EDIT"); // Add guide text to elm_entry.

		return entry;
	}
	return NULL;
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

static void _realized_cb(void *data, Evas_Object *obj, void *ei)
{
	Item_Data *id = elm_object_item_data_get(ei);

	printf("Realized: ");
	if (!id) {
		printf("Separator\n");
		return;
	}
    else if (id->title > 0) printf("Title[%d]\n", id->title);
	else printf("Editfield item[%d]\n", id->dial);

	// if dialogue styles
	if (id->dial == 1) elm_object_item_signal_emit(ei, "elm,state,top", "");
	else if (id->dial == 2) elm_object_item_signal_emit(ei, "elm,state,center", "");
	else if (id->dial == 3) elm_object_item_signal_emit(ei, "elm,state,center", "");
	else if (id->dial == 4) elm_object_item_signal_emit(ei, "elm,state,bottom", "");
	if (id->dial == 3 || id->dial == 4)
		elm_object_item_signal_emit(ei, "elm,state,edit,enabled", "");

	if (elm_config_access_get()) {
		// ===== Accessibility ====
		const Elm_Genlist_Item_Class *itc = elm_genlist_item_item_class_get(ei);
		if (strstr(itc->item_style, "dialogue/") &&
			strcmp(itc->item_style, "dialogue/grouptitle")) {
			// Register bg_dialogue image instead whole item
			Evas_Object *acc = elm_object_item_part_access_register(ei, "bg_dialogue");
			elm_access_info_cb_set(acc, ELM_ACCESS_INFO, _accinfo_cb, ei);
		}
	}
}


static void _unrealized_cb(void *data, Evas_Object *obj, void *ei)
{
	Item_Data *id = elm_object_item_data_get(ei);
	printf("Unrealized: ");
	if (!id) {
		printf("Separator\n");
		return;
	}
    else if (id->title > 0) printf("Title[%d]\n", id->title);
	else printf("Editfield item\n");
}

void genlist_editfield_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!data) return;
	struct appdata *ad = data;
	Item_Data *id = NULL;
	Elm_Object_Item *item = NULL;
	Evas_Object *genlist;

	Elm_Genlist_Item_Class *itc, *itc0, *itc1, *itc2, *itc3, *itc4, *itc5, *itc6, *itc7, *itc8, *itca, *itcb, *itcc;
	itc = elm_genlist_item_class_new();
	itc0 = elm_genlist_item_class_new();
	itc1 = elm_genlist_item_class_new();
	itc2 = elm_genlist_item_class_new();
	itc3 = elm_genlist_item_class_new();
	itc4 = elm_genlist_item_class_new();
	itc5 = elm_genlist_item_class_new();
	itc6 = elm_genlist_item_class_new();
	itc7 = elm_genlist_item_class_new();
	itc8 = elm_genlist_item_class_new();
	itca = elm_genlist_item_class_new();
	itcb = elm_genlist_item_class_new();
	itcc = elm_genlist_item_class_new();

	// Create genlist
	genlist = elm_genlist_add(ad->nf);
	elm_genlist_realization_mode_set(genlist, EINA_TRUE);

	// COMPRESS MODE
	// If multiline text (multiline entry or multiline textblock or sliding mode)
	// is used, use compress mode for compressing width to fit the viewport width.
	// So genlist can calculate item's height correctly.
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	printf("Compress mode enabled\n");

	evas_object_smart_callback_add(genlist, "realized", _realized_cb, NULL);
	evas_object_smart_callback_add(genlist, "unrealized", _unrealized_cb, NULL);

	// Title & Separator
	itc->item_style = "grouptitle";
	itc->func.text_get = _gl_title_get;
	itc->func.del = _gl_del;

	itc0->item_style = "dialogue/separator";
	itc0->func.del = _gl_del;

	// Normal editfield
	itca->item_style = "editfield/no_bg";
	itca->func.content_get = _gl_content_get;
	itca->func.del = _gl_del;

	itcb->item_style = "editfield/title/no_bg";
	itcb->func.text_get = _gl_text_get;
	itcb->func.content_get = _gl_content_get;
	itcb->func.del = _gl_del;

	itcc->item_style = "editfield/no_bg_line";
	itcc->func.content_get = _gl_content_get;
	itcc->func.del = _gl_del;

	// Normal editfield
	itc1->item_style = "editfield";
	itc1->func.content_get = _gl_content_get;
	itc1->func.del = _gl_del;

	itc2->item_style = "editfield/title";
	itc2->func.text_get = _gl_text_get;
	itc2->func.content_get = _gl_content_get;
	itc2->func.del = _gl_del;

	// Dialogue editfield
	itc3->item_style = "dialogue/editfield";
	itc3->func.content_get = _gl_content_get;
	itc3->func.del = _gl_del;

	itc4->item_style = "dialogue/editfield/title";
	itc4->func.text_get = _gl_text_get;
	itc4->func.content_get = _gl_content_get;
	itc4->func.del = _gl_del;

	// Multiline editfield
	itc5->item_style = "editfield";
	itc5->func.text_get = NULL;
	itc5->func.content_get = _gl_multiline_content_get;
	itc5->func.del = _gl_del;

	itc6->item_style = "editfield/title";
	itc6->func.text_get = _gl_text_get;
	itc6->func.content_get = _gl_multiline_content_get;
	itc6->func.del = _gl_del;

	itc7->item_style = "dialogue/editfield";
	itc7->func.content_get = _gl_multiline_content_get;
	itc7->func.del = _gl_del;

	itc8->item_style = "dialogue/editfield/title";
	itc8->func.text_get = _gl_text_get;
	itc8->func.content_get = _gl_multiline_content_get;
	itc8->func.del = _gl_del;;

	// ************************** Singline Editfield
	id = calloc(sizeof(Item_Data), 1);
	id->title = 1;
	item = elm_genlist_item_append(genlist, itc, id, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	// Editfields in Normal styles
	id = calloc(sizeof(Item_Data), 1);
	item = elm_genlist_item_append(genlist, itc1, id, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	id->item = item;

	id = calloc(sizeof(Item_Data), 1);
	item = elm_genlist_item_append(genlist, itc2, id, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	id->item = item;

	// Editfields in Dialogue styles
	id = calloc(sizeof(Item_Data), 1);
	id->dial = 1;
	item = elm_genlist_item_append(genlist, itc3, id, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	id->item = item;

	id = calloc(sizeof(Item_Data), 1);
	id->dial = 2;
	item = elm_genlist_item_append(genlist, itc4, id, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	id->item = item;

	// Editfields in Dialogue styles with editmode
	id = calloc(sizeof(Item_Data), 1);
	id->dial = 3;
	item = elm_genlist_item_append(genlist, itc3, id, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	id->item = item;

	id = calloc(sizeof(Item_Data), 1);
	id->dial = 4;
	item = elm_genlist_item_append(genlist, itc4, id, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	id->item = item;

	// Separator
	item = elm_genlist_item_append(genlist, itc0, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	//************************************** Multiline Editfield
	id = calloc(sizeof(Item_Data), 1);
	id->title = 2;
	item = elm_genlist_item_append(genlist, itc, id, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	// Editfields in Normal styles
	id = calloc(sizeof(Item_Data), 1);
	item = elm_genlist_item_append(genlist, itc5, id, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	id->item = item;

	id = calloc(sizeof(Item_Data), 1);
	item = elm_genlist_item_append(genlist, itc6, id, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	id->item = item;

	// Editfields in Dialogue styles
	id = calloc(sizeof(Item_Data), 1);
	id->dial = 1;
	item = elm_genlist_item_append(genlist, itc7, id, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	id->item = item;

	id = calloc(sizeof(Item_Data), 1);
	id->dial = 2;
	item = elm_genlist_item_append(genlist, itc8, id, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	id->item = item;

	// Editfields in Dialogue styles with editmode
	id = calloc(sizeof(Item_Data), 1);
	id->dial = 3;
	item = elm_genlist_item_append(genlist, itc7, id, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	id->item = item;

	// ************************** Without (with transparent) background
	id = calloc(sizeof(Item_Data), 1);
	id->title = 3;
	item = elm_genlist_item_append(genlist, itc, id, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	id = calloc(sizeof(Item_Data), 1);
	item = elm_genlist_item_append(genlist, itca, id, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	id->item = item;

	id = calloc(sizeof(Item_Data), 1);
	item = elm_genlist_item_append(genlist, itcb, id, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	id->item = item;

	id = calloc(sizeof(Item_Data), 1);
	item = elm_genlist_item_append(genlist, itcc, id, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	id->item = item;

	elm_genlist_item_class_free(itc0);
	elm_genlist_item_class_free(itc1);
	elm_genlist_item_class_free(itc2);
	elm_genlist_item_class_free(itc3);
	elm_genlist_item_class_free(itc4);
	elm_genlist_item_class_free(itc5);
	elm_genlist_item_class_free(itc6);
	elm_genlist_item_class_free(itc7);
	elm_genlist_item_class_free(itc8);
	elm_genlist_item_class_free(itca);
	elm_genlist_item_class_free(itcb);
	elm_genlist_item_class_free(itcc);

	elm_naviframe_item_push(ad->nf, _("Editfield"), NULL, NULL, genlist, NULL);
}
