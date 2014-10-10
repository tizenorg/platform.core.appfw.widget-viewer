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
#include "layout.h"

/*********************************************************
 SearchBar Normal
 ********************************************************/

static struct _menu_item menu_its[] = {
	{ "Nocontent Styles", layout_nocontent_styles_cb },
	{ NULL, NULL }
};

static void _list_click(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *it = (Elm_Object_Item *) elm_list_selected_item_get(obj);
	if(it == NULL) return;

	elm_list_item_selected_set(it, EINA_FALSE);
}

static Evas_Object* _create_list_winset(struct appdata* ad)
{
	Evas_Object *li;

	if(ad == NULL) return NULL;

	li = elm_list_add(ad->nf);
	elm_list_mode_set(li, ELM_LIST_COMPRESS);
	evas_object_smart_callback_add(li, "selected", _list_click, NULL);

	int idx = 0;

	while (menu_its[ idx ].name != NULL) {
		elm_list_item_append(
				li,
				menu_its[ idx ].name,
				NULL,
				NULL,
				menu_its[ idx ].func,
			 	ad);
		++idx;
	}
	elm_list_go(li);

	return li;
}


void layout_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad;
	Evas_Object *list;

	ad = (struct appdata *) data;
	if(ad == NULL) return;

	list = _create_list_winset(ad);
	elm_naviframe_item_push(ad->nf, _("Layout"), NULL, NULL, list, NULL);
}

