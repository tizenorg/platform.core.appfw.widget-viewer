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
  Genlist Group Title
 ********************************************************/
#define NUM_OF_ITEMS 2000
static Elm_Genlist_Item_Class itc, itc2;
//static Elm_Genlist_Item_Class itc3;

#define NUM_OF_GROUPS 5
static char *group[] = {
	N_("Family"), N_("Friends"), N_("Colleagues"), N_("Relatives"), N_("Classmates")
};
#define NUM_OF_NAMES 40
static char **genlist_demo_names = NULL;

static char *_gl_text_get(void *data, Evas_Object *obj, const char *part)
{
	int index = ((int)data) % NUM_OF_NAMES;

	if (!strcmp(part, "elm.text")) {
		return strdup(genlist_demo_names[index]);
	}

	return NULL;
}

static char* _gl_text_get_title(void *data, Evas_Object *obj,
		const char *part)
{
	char buf[PATH_MAX];
	int index = ((int) data / 5) % NUM_OF_GROUPS;

	if (!strcmp(part, "elm.text")) {
		snprintf(buf, sizeof(buf), "%s", group[index]);
		return strdup(buf);
	}
	return NULL;
}

/*
static Evas_Object* _gl_content_get_title(void *data, Evas_Object *obj,
		const char *part)
{
	int index = ((int) data / 5) % NUM_OF_GROUPS;

	if (!strcmp(part, "elm.icon")) {
		Evas_Object *icon = elm_image_add(obj);
		char buf[PATH_MAX];

		snprintf(buf, sizeof(buf), "%s/g%d.png", ICON_DIR, index);

		elm_image_file_set(icon, buf, NULL);
		return icon;
	}
	return NULL;
}
*/

static Eina_Bool _gl_state_get(void *data, Evas_Object *obj, const char *part)
{
	return EINA_FALSE;
}

static void _gl_del(void *data, Evas_Object *obj)
{
	return;
}

static void _gl_sel(void *data, Evas_Object *obj, void *event_info)
{
	if (event_info)
		elm_genlist_item_selected_set(event_info, EINA_FALSE);
	return;
}

static Evas_Object *_create_genlist(struct appdata *ad)
{
	int index;
	Elm_Object_Item *item;
	Elm_Object_Item *git = NULL;
	Evas_Object *genlist;

	genlist_demo_names = genlist_get_demo_names();

	// Create genlist
	genlist = elm_genlist_add(ad->nf);

	// Set item class for normal items
	itc.item_style = "default";
	itc.func.text_get = _gl_text_get;
	itc.func.content_get = NULL;
	itc.func.state_get = _gl_state_get;
	itc.func.del = _gl_del;

	// Set item class for group title item
	itc2.item_style = "grouptitle";
	itc2.func.text_get = _gl_text_get_title;
	itc2.func.content_get = NULL;
	itc2.func.state_get = _gl_state_get;
	itc2.func.del = _gl_del;

	// Set item class for group title left item
	/*
	itc3.item_style = "grouptitle.left";
	itc3.func.content_get = _gl_content_get_title;
	*/

	for (index = 0; index < NUM_OF_ITEMS; index++) {
		if (index % 5 == 0) {
			/*
			if (index < NUM_OF_ITEMS/2)
				git = elm_genlist_item_append(genlist, &itc2, (void *) index, NULL, ELM_GENLIST_ITEM_GROUP, _gl_sel, NULL);
			else
				//git = elm_genlist_item_append(genlist, &itc3, (void *) index, NULL, ELM_GENLIST_ITEM_GROUP, _gl_sel, NULL);
				git = elm_genlist_item_append(genlist, &itc2, (void *) index, NULL, ELM_GENLIST_ITEM_GROUP, _gl_sel, NULL);
			*/
			git = elm_genlist_item_append(genlist, &itc2, (void *) index, NULL, ELM_GENLIST_ITEM_GROUP, _gl_sel, NULL);
			elm_genlist_item_select_mode_set(git, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

		} else {
			item = elm_genlist_item_append(genlist, &itc, (void *) index,
					git, ELM_GENLIST_ITEM_NONE, _gl_sel, NULL);
		}
	}

	return genlist;
}

void genlist_grouptitle_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad;
	Evas_Object *genlist;

	ad = (struct appdata *) data;
	if (ad == NULL) return;

	genlist = _create_genlist(ad);
	elm_naviframe_item_push(ad->nf, _("Group Index"), NULL, NULL, genlist, NULL);
}
