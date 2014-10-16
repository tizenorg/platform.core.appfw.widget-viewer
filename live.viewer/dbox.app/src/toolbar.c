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
#include "toolbar_tab.h"
#include "toolbar_tool.h"
#include "toolbar_mix.h"
#include <math.h>

static struct _menu_item menu_main[] = {
	{ "Tab Style", tab_cb},
	{ "Tool Style", tool_cb},
//	{ "Mix Style", mix_cb},
	/* do not delete below */
	{ NULL, NULL }
};

static void _list_click(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *it = (Elm_Object_Item *) elm_list_selected_item_get(obj);
	if(it == NULL) return;

	elm_list_item_selected_set(it, EINA_FALSE);
}

static Evas_Object* _create_list_winset(Evas_Object* parent, struct _menu_item *menu, struct appdata* ad)
{
	Evas_Object *li;

	if(parent == NULL || ad == NULL) return NULL;

	li = elm_list_add(parent);
	elm_list_mode_set(li, ELM_LIST_COMPRESS);
	evas_object_smart_callback_add(li, "selected", _list_click, NULL);

	int idx = 0;

	while(menu[ idx ].name != NULL) {

		elm_list_item_append(
				li,
				menu[ idx ].name,
				NULL,
				NULL,
				menu[ idx ].func,
				ad);
		++idx;
	}

	elm_list_go(li);

	return li;
}

void toolbar_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *list;
	struct appdata *ad;
	ad = (struct appdata *) data;
	if(ad == NULL) return;

	list = _create_list_winset(ad->layout_main, menu_main, ad);
	elm_naviframe_item_push(ad->nf, _("Toolbar"), NULL, NULL, list, NULL);
}
