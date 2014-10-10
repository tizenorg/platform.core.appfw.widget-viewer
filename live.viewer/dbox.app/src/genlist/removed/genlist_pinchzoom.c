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
 * Genlist Pinch Zoom
 ********************************************************/

static Elm_Genlist_Item_Class itci, itch;

static char *_gli_text_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024];
	int index = (int) data;

	if ((index>>4) <= 15) {
		snprintf(buf, sizeof(buf), "List item %d", index+1);
	} else {
		snprintf(buf, sizeof(buf), "List item %d", index+1);
	}

	return strdup(buf);
}

static char *_gli_title_text_get(void *data, Evas_Object *obj, const char *part)
{
	return strdup((char *)data);
}

static Evas_Object* _create_layout_inner(Evas_Object* parent)
{
	Evas_Object* layout_inner;

	layout_inner = elm_layout_add(parent);
	elm_layout_file_set(layout_inner, ELM_DEMO_EDJ, "fastscroll");
	evas_object_size_hint_weight_set(layout_inner, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	return layout_inner;
}

static void _gl_sel(void *data, Evas_Object *obj, void *event_info)
{
	if (event_info)
		elm_genlist_item_selected_set(event_info, EINA_FALSE);
}

static Evas_Object* _create_fastscroll_genlist(Evas_Object* parent)
{
	Elm_Object_Item *item;
	Elm_Object_Item *git=NULL;
	Evas_Object *genlist;
	int index1, index2;
	char buf[32];

	// Create genlist
	genlist = elm_genlist_add(parent);
	elm_genlist_scroller_policy_set(genlist, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);

   elm_genlist_effect_set(genlist, 1);
   elm_genlist_pinch_zoom_set(genlist, 1);

	// Set item class
	itci.item_style = "default";
	itci.func.text_get = _gli_text_get;
	itci.func.content_get = NULL;
	itci.func.state_get = NULL;
	itci.func.del = NULL;

	itch.item_style = "grouptitle";
	itch.func.text_get = _gli_title_text_get;

	index2 = 0;
	for (index1 = 0; index1 < 78; index1++) {

		if ((index1 % 3) == 0) {
				snprintf(buf, sizeof(buf), "%c", 'A' + index1 / 3);

			git = elm_genlist_item_append(genlist, &itch, strdup(buf), NULL, ELM_GENLIST_ITEM_GROUP, _gl_sel, NULL);
			elm_genlist_item_select_mode_set(git, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		}

		item = elm_genlist_item_append(genlist, &itci, (void *) index1,
				git, ELM_GENLIST_ITEM_NONE,
				_gl_sel, NULL);
	}

	return genlist;
}

void genlist_pinchzoom_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad;
	Evas_Object *genlist;
	Evas_Object *layout_inner;

	ad = (struct appdata *) data;
	if (ad == NULL) return;

	layout_inner = _create_layout_inner(ad->nf);
	genlist = _create_fastscroll_genlist(layout_inner);
	if (genlist == NULL) return;

	elm_layout_content_set(layout_inner, "elm.swallow.content.genlist", genlist);

	elm_naviframe_item_push(ad->nf, _("Pinch Zoom"), NULL, NULL, layout_inner, NULL);
}
