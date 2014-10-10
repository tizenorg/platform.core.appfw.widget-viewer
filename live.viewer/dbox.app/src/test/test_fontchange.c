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
#include "test_fontchange.h"

/*********************************************************
  Font Change Test
 ********************************************************/

void _list_item_select_cb(void *data, Evas_Object *obj, void *event_info)
{
	char font[256];
	Elm_Object_Item *it = elm_list_selected_item_get(obj);
	const char *fontname = elm_object_item_text_get(it);

	Eina_List *text_classes = elm_config_text_classes_list_get();
	Eina_List *l;
	Elm_Text_Class *tc;

	EINA_LIST_FOREACH(text_classes, l, tc)
		elm_config_font_overlay_set(tc->name, fontname, -100);

	sprintf(font, "%s", elm_object_item_text_get(it));
	elm_config_font_overlay_set("tizen", (const char*) font, -100);

	sprintf(font, "%s:style=Medium", elm_object_item_text_get(it));
	elm_config_font_overlay_set("tizen_medium", (const char*) font, -100);

	sprintf(font, "%s:style=Roman", elm_object_item_text_get(it));
	elm_config_font_overlay_set("tizen_roman", (const char*) font, -100);

	sprintf(font, "%s:style=Bold", elm_object_item_text_get(it));
	elm_config_font_overlay_set("tizen_bold", (const char*) font, -100);

	sprintf(font, "%s:style=Regular", elm_object_item_text_get(it));
	elm_config_font_overlay_set("tizen_regular", (const char*) font, -100);

	elm_config_font_overlay_apply();
	elm_config_all_flush();
	elm_config_save();
	change_config_owner();

	elm_config_text_classes_list_free(text_classes);
}

static Eina_Bool _font_item_add(const Eina_Hash *hash, const void *key, void *data, void *fdata)
{
	Elm_Font_Properties *efp = data;
	Evas_Object *list = fdata;

	elm_list_item_append(list, efp->name, NULL, NULL, _list_item_select_cb, NULL);

	return EINA_TRUE;
}

static Evas_Object * _create_list(Evas_Object *parent)
{
	Evas_Object *list = elm_list_add(parent);
	evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(list, EVAS_HINT_FILL, EVAS_HINT_FILL);

	//Construct available font list
	Evas *e = evas_object_evas_get(parent);
	Eina_List *evas_font = evas_font_available_list(e);
	if (!evas_font) return NULL;

	Eina_Hash *font_hash = elm_font_available_hash_add(evas_font);
	if (!font_hash) return NULL;

	evas_font_available_list_free(e, evas_font);

	eina_hash_foreach(font_hash, _font_item_add, list);

	elm_font_available_hash_del(font_hash);

	elm_list_go(list);

	evas_object_show(list);

	return list;
}

void test_fontchange_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *) data;

	Evas_Object *list = _create_list(ad->nf);
	if (!list) return;

	elm_naviframe_item_push(ad->nf, _("Font List"), NULL, NULL, list, NULL);
}

