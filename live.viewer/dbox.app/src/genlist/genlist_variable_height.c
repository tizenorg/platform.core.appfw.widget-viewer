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
 Customize Genlist Styles (variable height)
 ********************************************************/

#define NUM_OF_ITEMS 2000

static char *label[] = {
	"Lover Tonight", "Lucid Dream", "Listen To The Music",
	"Sky In My Heart", "French Love",
	NULL
};

static char *_gl_text_get(void *data, Evas_Object *obj, const char *part);
static Evas_Object *_gl_content_get(void *data, Evas_Object *obj, const char *part);
static Eina_Bool _gl_state_get(void *data, Evas_Object *obj, const char *part);
static void _gl_del(void *data, Evas_Object *obj);
static void _gl_sel(void *data, Evas_Object *obj, void *event_info);
static Evas_Object * _create_genlist(Evas_Object *parent, Elm_Theme *th);

static char *_gl_text_get(void *data, Evas_Object *obj, const char *part)
{
	if (strcmp(part, "elm.text")) return NULL;

	char buf[PATH_MAX];
	int index = ((int)data) % 10;
	int size = (index * 25) * elm_config_scale_get() + 10;

	if ((index == 0) || (index == 1) || (index == 2) || (index == 3) ||
		(index == 4)) {
		sprintf(buf, "<font_size=%d>%s</font_size>", size,
				label[index % (sizeof(label)/sizeof(char *))]);
		return strdup(buf);
	}
	return NULL;
}

static Evas_Object *_gl_content_get(void *data, Evas_Object *obj, const char *part)
{
	if (strcmp(part, "elm.icon")) return NULL;

	Evas_Object *icon;
	int index = ((int)data) % 10;
	int size = (index * 25) * elm_config_scale_get();

	if ((index == 5) || (index == 6) || (index == 7)) {
		icon = elm_image_add(obj);
		elm_image_file_set(icon, ICON_DIR"/genlist/00_brightness_right.png", NULL);
	} else  if ((index  == 8) || (index == 9) || (index == 10)) {
		icon = elm_button_add(obj);
		elm_object_style_set(icon, "style1");
		elm_object_text_set(icon, "Button");
		evas_object_propagate_events_set(icon, EINA_FALSE);
	} else return NULL;

	evas_object_size_hint_min_set(icon, 0, size);
	return icon;
}

static Eina_Bool _gl_state_get(void *data, Evas_Object *obj, const char *part)
{
	return EINA_FALSE;
}

static void _gl_del(void *data, Evas_Object *obj)
{
	return;
}

static void _gl_sel(void *data, Evas_Object *obj, void *ei)
{
	if (!ei) return;
	Elm_Object_Item *item = ei;
	int index = (int)data;
	index = index % (sizeof(label)/sizeof(char *));

	elm_genlist_item_selected_set(item, EINA_FALSE);
}

static void
_view_free_cb(void *data, Evas *e, Evas_Object *obj, void *ei)
{
	Elm_Theme *th = data;
	// Free customized theme
	elm_theme_free(th);
}

static Evas_Object * _create_genlist(Evas_Object *parent, Elm_Theme *th)
{
	int index = 0;
	Evas_Object *genlist;

	// Create a genlist
	genlist = elm_genlist_add(parent);
	// Apply created theme to genlist
	elm_object_theme_set(genlist, th);

	// Set genlist item class
	Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
	itc->item_style = "variable_height/default";
	itc->func.text_get = _gl_text_get;
	itc->func.content_get = _gl_content_get;
	itc->func.state_get = _gl_state_get;
	itc->func.del = _gl_del;

	// Append item to the end of genlist
	for (index = 0; index < NUM_OF_ITEMS; index++) {
		Elm_Object_Item *it = elm_genlist_item_append(genlist,
				itc,
				(void *)index,
				NULL,
				ELM_GENLIST_ITEM_NONE,
				_gl_sel,
				(void *)index
			);
		elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
	}
	elm_genlist_item_class_free(itc);

	return genlist;
}

void genlist_variable_height_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *genlist;
	struct appdata *ad = (struct appdata *)data;

	if (data == NULL) return;

	// Create new theme
	Elm_Theme *th = elm_theme_new();
	// Add default (Tizen) theme into new theme user created.
	elm_theme_ref_set(th, NULL);
	// Add customized theme (user created) into created new theme.
	elm_theme_extension_add(th, ELM_DEMO_EDJ);

	// Create a genlist
	genlist = _create_genlist(ad->nf, th);

	// Push genlist to the top of naviframe stack
	elm_naviframe_item_push(ad->nf, _("Variable Height") , NULL, NULL, genlist, NULL);
	evas_object_event_callback_add(genlist, EVAS_CALLBACK_FREE, _view_free_cb, th);
}
