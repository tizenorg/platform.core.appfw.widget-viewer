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
 Customize Genlist Styles (Variable Width)
 ********************************************************/

static struct appdata *_ad;
static int label_max_item_num;
static Elm_Genlist_Item_Class itc;

static int _get_label_max_item_num(void);
static void _set_genlist_item_class(void);
static Evas_Object *_gl_content_get(void *data, Evas_Object *obj, const char *part);
static Eina_Bool _gl_state_get(void *data, Evas_Object *obj, const char *part);
static void _gl_del(void *data, Evas_Object *obj);
static void _gl_sel(void *data, Evas_Object *obj, void *event_info);
static Evas_Object * _create_genlist(Evas_Object *parent);

static int _get_label_max_item_num(void)
{
	return 10;
}

static void _set_genlist_item_class(void)
{
	itc.item_style = "variable_width/default";
	itc.func.text_get = NULL;
	itc.func.content_get = _gl_content_get;
	itc.func.state_get = _gl_state_get;
	itc.func.del = _gl_del;
}

static Evas_Object *_gl_content_get(void *data, Evas_Object *obj, const char *part)
{
	int index = (int)data;
	int size = (int) (((index % label_max_item_num) + 1) * 30 * elm_config_scale_get());
	int type = (int) (index / label_max_item_num);
	int width = 0;

	if (!strcmp(part, "elm.swallow.icon")) {
		width = size + 100 * elm_config_scale_get();
		if (width > (_ad->root_w - 100 - 10)) {
			width = _ad->root_w - 100 - 10;
		}
		if (type == 0) {
			Evas_Object *label;
			label = elm_label_add(obj);
			elm_object_style_set(label, "genlist_variable_width/label_ellipsis");
			elm_label_wrap_width_set(label, width);

			/* Ellipsis downgrades performance. But it is needed for variable width feature */
			elm_label_ellipsis_set(label, EINA_TRUE);

			elm_label_label_set(label, "Label changes line automatically if label wrap with is serj");
			return label;
		} else if (type == 1) {
			Evas_Object *button;
			button = elm_button_add(obj);
			elm_object_style_set(button, "style1");
			elm_object_text_set(button, "Button");
			evas_object_size_hint_min_set(button, width, 50 * elm_config_scale_get());
			evas_object_size_hint_max_set(button, width, 50 * elm_config_scale_get());
			evas_object_propagate_events_set(button, EINA_FALSE);
			return button;
		}
	} else if (!strcmp(part, "elm.swallow.icon.small")) {
		Evas_Object *icon;
		icon = elm_image_add(obj);
		elm_image_file_set(icon, ICON_DIR"/genlist/00_brightness_right.png", NULL);
		return icon;
	} else if (!strcmp(part, "elm.swallow.icon.right")) {
		Evas_Object *icon;
		icon = elm_image_add(obj);
		elm_image_file_set(icon, ICON_DIR"/genlist/00_list_photo_default.png", NULL);
		return icon;
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
	Elm_Object_Item *item = (Elm_Object_Item *)event_info;
	int index = (int) data;

	if (item != NULL) {
		elm_genlist_item_selected_set(item, EINA_FALSE);
		if (index < label_max_item_num) {
			fprintf(stderr, "Text Selected\n");
		} else {
			fprintf(stderr, "Icon Selected\n");
		}
	}
}

static Eina_Bool _pop_cb(void *data, Elm_Object_Item *it)
{
	// Deletes a theme extension from list of extensions.
	elm_theme_extension_del(NULL, ELM_DEMO_EDJ);
	set_rotate_cb_for_winset(NULL, NULL);

	return EINA_TRUE;
}

static Evas_Object * _create_genlist(Evas_Object *parent)
{
	int index = 0;
	Evas_Object *genlist;
	Elm_Object_Item *item = NULL;

	// Create a genlist
	genlist = elm_genlist_add(parent);

	// Set genlist item class
	_set_genlist_item_class();

	// Append item to the end of genlist
	for (index = 0; index < label_max_item_num * 2; index++) {
		item = elm_genlist_item_append(
				genlist,
				&itc,
				(void *)index,
				NULL,
				ELM_GENLIST_ITEM_NONE,
				_gl_sel,
				(void *)index
			);
	}

	return genlist;
}

static int _rotate_cb(enum appcore_rm rotmode, void *data)
{
	Evas_Object *genlist = NULL;
	Eina_List *realized_list, *l;
	Elm_Object_Item *it;
	if (!data) return -1;

	genlist = (Evas_Object *)data;
	realized_list = elm_genlist_realized_items_get(genlist);
	EINA_LIST_FOREACH(realized_list, l, it)
	{
		elm_genlist_item_update(it);
	}

	return 0;
}

void genlist_variable_width_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *genlist;
	struct appdata *ad = (struct appdata *)data;
	Elm_Object_Item *navi_it;

	if (data == NULL) return;

	_ad = ad;

	label_max_item_num = _get_label_max_item_num();

	// Appends a theme extension to list of extensions.
	elm_theme_extension_add(NULL, ELM_DEMO_EDJ);

	// Create a genlist
	genlist = _create_genlist(ad->nf);

	// Push genlist to the top of naviframe stack
	navi_it = elm_naviframe_item_push(ad->nf, _("Variable Width") , NULL, NULL, genlist, NULL);
	elm_naviframe_item_pop_cb_set(navi_it, _pop_cb, NULL);

	// Redraw genlist when rotated
	set_rotate_cb_for_winset(_rotate_cb, genlist);
}
