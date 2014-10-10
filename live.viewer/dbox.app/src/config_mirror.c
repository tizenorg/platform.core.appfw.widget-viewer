/*
 * Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *		http://www.apache.org/licenses/LICENSE-2.0
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
#include "config_mirror.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/reboot.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <errno.h>


static void
_check_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	Eina_Bool enabled = elm_check_state_get(obj);

	elm_config_mirrored_set(enabled);
}

static char *
_gl_text_get(void *data, Evas_Object *obj, const char *part)
{
	if (!part) return NULL;
	if (!strcmp(part, "elm.text")) return strdup("UI Mirroring");

	return NULL;
}

static Evas_Object *
_gl_content_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *icon;
	Eina_Bool enabled = EINA_FALSE;

	if (!part) return NULL;
	if (!strcmp(part, "elm.icon") || !strcmp(part, "elm.swallow.icon")) {
		icon = elm_check_add(obj);

		enabled = elm_config_mirrored_get();

		elm_check_state_set(icon, enabled);
		elm_object_style_set(icon, "on&off");

		evas_object_pass_events_set(icon, 1);
		evas_object_propagate_events_set(icon, 0);
		evas_object_smart_callback_add(icon, "changed", _check_clicked_cb, data);

		return icon;
	}

	return NULL;
}

static void
_gl_sel(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *item;
	Evas_Object *icon;
	Eina_Bool enabled;

	item = (Elm_Object_Item *)event_info;
	if (!item) return;

	elm_genlist_item_selected_set(item, EINA_FALSE);

	icon = elm_object_item_part_content_get(item, "elm.icon");
	if (!icon) return;

	enabled = !elm_check_state_get(icon);

	elm_check_state_set(icon, enabled);
}

static Evas_Object *
_create_genlist(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	Evas_Object *genlist;
	Elm_Genlist_Item_Class *itc;

	// Create genlist
	genlist = elm_genlist_add(ad->nf);

	// Set genlist item class
	itc = elm_genlist_item_class_new();
	itc->item_style = "1text.1icon";
	itc->func.text_get = _gl_text_get;
	itc->func.content_get = _gl_content_get;
	itc->func.state_get = NULL;
	itc->func.del = NULL;

	// Append items
	elm_genlist_item_append(genlist, itc, ad, NULL, ELM_GENLIST_ITEM_NONE, _gl_sel, ad);

	elm_genlist_item_class_free(itc);

	return genlist;
}

void config_mirror_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *) data;
	Evas_Object *genlist;

	genlist = _create_genlist(ad);

	elm_naviframe_item_push(ad->nf, _("UI Mirroring"), NULL, NULL, genlist, NULL);
}
