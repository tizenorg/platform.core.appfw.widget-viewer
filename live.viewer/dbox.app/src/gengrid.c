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

#include "gengrid.h"

static Elm_Genlist_Item_Class itc={0,};
static struct appdata *menu_ad;

static struct _menu_item menu_its[] = {
	{ "Gengrid GalleryGrid Style", gengrid_gallerygrid_cb },
	{ "Gengrid MyfileGrid Style", gengrid_myfilegrid_cb },
	{ "Gengrid MyfileGridtext Style", gengrid_myfilegridtext_cb },
	{ "Group Style Sample", gengrid_groupindex_cb },
	{ "Customized Style Sample", gengrid_theme_cb },
	{ "Customized InCheck Sample", gengrid_incheck_cb },
	/* do not delete below */
	{ NULL, NULL }
};

static char *_gl_text_get(void *data, Evas_Object *obj, const char *part)
{
	int index = (int)data;

	if (!strcmp(part, "elm.text")) {
		return strdup(menu_its[index].name);
	}

	return NULL;
}

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
	int index = (int)data;

	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);
	menu_its[index].func(menu_ad, NULL, NULL);
	return;
}

static Evas_Object* _create_genlist(struct appdata* ad)
{
	Evas_Object *genlist;
	int index;

	if (ad == NULL) return NULL;

	itc.item_style = "default";
	itc.func.text_get = _gl_text_get;
	itc.func.content_get = NULL;
	itc.func.state_get = _gl_state_get;
	itc.func.del = _gl_del;

	genlist = elm_genlist_add(ad->nf);

	for (index = 0; menu_its[index].name; index++) {
		elm_genlist_item_append(genlist, &itc, (void *)index, NULL,
				ELM_GENLIST_ITEM_NONE, _gl_sel, (void *)index);
	}

	return genlist;
}

void gengrid_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad;
	Evas_Object *genlist;

	ad = (struct appdata *) data;
	if (ad == NULL) return;

	menu_ad = ad;
	genlist = _create_genlist(ad);
	elm_naviframe_item_push(ad->nf, _("Gengrid"), NULL, NULL, genlist, NULL);
}
