/*
 * Copyright (c) 2013 Samsung Electronics Co., Ltd All Rights Reserved
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
#include "slider_custom.h"

/*********************************************************
   Custom Slider
 ********************************************************/

#define SLIDER_POPUP_X 438
#define SLIDER_POPUP_Y 96
#define NUM_OF_ITEMS 6

static char *slider_itemlist[] = {
	"Bubble Slider",
	"Bubble Slider with Indicator",
	"Wave Slider",
	NULL
};

static Eina_Bool _pop_cb(void *data, Elm_Object_Item *it)
{
	elm_theme_extension_del(NULL, ELM_DEMO_EDJ);
	return EINA_TRUE;
}

static char *_gl_text_get(void *data, Evas_Object *obj, const char *part)
{
	int index = ((int)(long)data) / 2;
	return strdup(slider_itemlist[index]);
}

static Evas_Object *_gl_content_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *slider;
	int index = ((int)(long)data);

	slider = elm_slider_add(obj);

	if (index == 1) {
		elm_slider_indicator_show_set(slider, EINA_FALSE);
		elm_object_style_set(slider, "bubble");
	} else if (index == 3) {
		elm_object_style_set(slider, "bubble_sent");
	} else if (index == 5) {
		elm_slider_indicator_show_set(slider, EINA_FALSE);
		elm_object_style_set(slider, "wave");
	}
	return slider;
}

static Evas_Object* _create_genlist(Evas_Object* parent)
{;
	Evas_Object *genlist;
	Elm_Object_Item *gl_item = NULL;
	int index;

	genlist = elm_genlist_add(parent);
	Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
	Elm_Genlist_Item_Class *itc2 = elm_genlist_item_class_new();

	itc->item_style = "dialogue/1text";
	itc->func.text_get = _gl_text_get;

	itc2->item_style = "dialogue/1icon";
	itc2->func.content_get = _gl_content_get;

	evas_object_show(genlist);

	for (index = 0; index < NUM_OF_ITEMS; index++) {
		if (index % 2 == 0) {
			gl_item = elm_genlist_item_append(genlist, itc, (void *) index, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
			elm_genlist_item_select_mode_set(gl_item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		} else {
			gl_item = elm_genlist_item_append(genlist, itc2, (void *) index, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
			elm_genlist_item_select_mode_set(gl_item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		}
	}
	elm_genlist_item_class_free(itc);
	elm_genlist_item_class_free(itc2);

	return genlist;
}

void slider_custom_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *genlist;
	struct appdata *ad = (struct appdata *) data;
	Elm_Object_Item *it;
	if (ad == NULL) return;

	elm_theme_extension_add(NULL, ELM_DEMO_EDJ);

	genlist = _create_genlist(ad->nf);
	it = elm_naviframe_item_push(ad->nf, _("Custom Style"), NULL, NULL, genlist, NULL);
	elm_naviframe_item_pop_cb_set(it, _pop_cb, NULL);
}
