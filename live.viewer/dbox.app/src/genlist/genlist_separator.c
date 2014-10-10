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

char *groups[] = {
	"In Dialogue Group",
	"Not in Dialogue Group",
	"End"
};

#define MAX_SEPARATORS 10
#define MAX_GROUPS 2

static char *help_dialogue_message[] = {
	"Help message in dialogue group<br>1. Activate Wi-Fi on your device<br>"
	"2. Find name in your network list.<br>"
	"3. Connect name by entering password in WPA field.",
	"Help message not in dialogue group<br>1. Activate Wi-Fi on your device<br>"
	"2. Find name in your network list.<br>"
	"3. Connect name by entering password in WPA field."
};

static char *_gl_text_get(void *data, Evas_Object *obj, const char *part)
{
	if (!strcmp(part, "elm.text")) {
		return strdup(help_dialogue_message[(int)data]);
	}
	return NULL;
}

static char *_gl_group_text_get(void *data, Evas_Object *obj, const char *part)
{
	if (!strcmp(part, "elm.text")) {
		return strdup(groups[(int)data]);
	}
	return NULL;
}

static Evas_Object *_create_genlist(struct appdata *ad)
{
	Evas_Object *genlist;
	Elm_Object_Item *git;

	if (ad == NULL) return NULL;

	Elm_Genlist_Item_Class *itc, *itc_group, *itc_sep, *itc_sep2;
	itc = elm_genlist_item_class_new();
	itc_group = elm_genlist_item_class_new();
	itc_sep = elm_genlist_item_class_new();
	itc_sep2 = elm_genlist_item_class_new();

	itc_group->item_style = "dialogue/grouptitle";
	itc_group->func.text_get = _gl_group_text_get;

	itc->item_style = "multiline/1text";
	itc->func.text_get = _gl_text_get;

	itc_sep->item_style = "dialogue/separator";

	itc_sep2->item_style = "dialogue/separator.2";

	genlist = elm_genlist_add(ad->nf);
	elm_genlist_realization_mode_set(genlist, EINA_TRUE);
	// COMPRESS MODE
	// If multiline text (multiline entry or multiline textblock or sliding mode)
	// is used, use compress mode for compressing width to fit the viewport width.
	// So genlist can calculate item's height correctly.
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	printf("Compress mode enabled\n");

	// In dialogue group ***********************************
	git = elm_genlist_item_append(genlist, itc_group, (void *)0, NULL,
			ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(git, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	// Top separator in dialogue group
	git = elm_genlist_item_append(genlist, itc_sep2, (void *)0, NULL,
			ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(git, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	git = elm_genlist_item_append(genlist, itc, (void *)0, NULL,
			ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(git, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	// Bottom separator in dialogue group
	git = elm_genlist_item_append(genlist, itc_sep, (void *)0, NULL,
			ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(git, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	// Not in dialogue group *************************
	git = elm_genlist_item_append(genlist, itc_group, (void *)1, NULL,
			ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(git, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	// Top separator in dialogue group
	git = elm_genlist_item_append(genlist, itc_sep, (void *)1, NULL,
			ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(git, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	git = elm_genlist_item_append(genlist, itc, (void *)1, NULL,
			ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(git, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	// Bottom separator not in dialogue group
	git = elm_genlist_item_append(genlist, itc_sep, (void *)1, NULL,
			ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(git, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	git = elm_genlist_item_append(genlist, itc_group, (void *)2, NULL,
			ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(git, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	elm_genlist_item_class_free(itc);
	elm_genlist_item_class_free(itc_group);
	elm_genlist_item_class_free(itc_sep);
	elm_genlist_item_class_free(itc_sep2);

	return genlist;
}

void genlist_separator_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad;
	Evas_Object *genlist;

	ad = (struct appdata *) data;
	if (ad == NULL) return;

	genlist = _create_genlist(ad);
	elm_naviframe_item_push(ad->nf, _("Genlist Separator"), NULL, NULL, genlist, NULL);
}
