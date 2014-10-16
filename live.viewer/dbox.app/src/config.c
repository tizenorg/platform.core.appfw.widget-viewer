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
#include "config.h"
#include "config_scrolling.h"
#include "config_abort.h"
#include "config_mirror.h"

/*********************************************************
  config
 ********************************************************/
static struct _menu_item _menu_its[] = {
	//{ "Theme", NULL},
	//{ "Fonts", NULL},
	{ "Scrolling", config_scrolling_cb},
	{ "Abort on errors", config_abort_cb},
	{ "UI mirroring", config_mirror_cb},
	/* do not delete below */
	{ NULL, NULL }
};

static void _list_click(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *it = (Elm_Object_Item *) elm_list_selected_item_get(obj);
	if(it == NULL) return;

	elm_list_item_selected_set(it, EINA_FALSE);
}

static Evas_Object *_create_list_winset(struct appdata *ad)
{
	Evas_Object *li;
	struct _menu_item *menu_its;
	if (ad == NULL) return NULL;

	li = elm_list_add(ad->nf);
	elm_list_mode_set(li, ELM_LIST_COMPRESS);
	evas_object_smart_callback_add(li, "selected", _list_click, NULL);

	int idx = 0;
	menu_its = _menu_its;
	while (menu_its[idx].name != NULL) {
		elm_list_item_append(li, menu_its[idx].name, NULL, NULL, menu_its[idx].func, ad);
		++idx;
	}
	elm_list_go(li);

	return li;
}


void config_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *list;
	struct appdata *ad;
	ad = (struct appdata *) data;
	if(ad == NULL) return;

	list = _create_list_winset(ad);
	elm_naviframe_item_push(ad->nf, _("Config"), NULL, NULL, list, NULL);
}
