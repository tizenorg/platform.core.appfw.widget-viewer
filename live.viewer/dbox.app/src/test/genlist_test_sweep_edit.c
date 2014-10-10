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
  Genlist Sweep with Edit mode
 ********************************************************/

static Evas_Object *genlist, *toolbar, *edit_btn;

#define NUM_OF_ITEMS 2000
#define NUM_OF_NAMES 4
static Elm_Genlist_Item_Class itc, itc2, itc3, itc4;
static char *names[] = {
	"One Button After Slide", "Two Buttons After Slide",
	"Three Buttons After Slide", "Four Buttons After Slide"
};

static void _button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	fprintf(stderr, "Button Clicked : %s\n", names[(int)data]);
}

/**************************** Genlist Handling Routines ******************************/
/**
 * gl_text_get is called for each text part when the item is realized.
 * This is also called when the sweep animation is started.
 *
 * @param data  :  3th parameter of the elm_genlist_item_append api.
 * @param obj   :  the genlist object.
 * @param part  :  text part name.
 *
 */
static char *_gl_text_get(void *data, Evas_Object *obj, const char *part)
{
	if (!strcmp(part, "elm.text")) {
		return strdup("slide style");
	} else if (!strcmp(part, "elm.slide.text.1")) {
		return strdup("slide");
	}
	return NULL;
}

static char *_gl_text_get2(void *data, Evas_Object *obj, const char *part)
{
	if (!strcmp(part, "elm.text.1")) {
		return strdup("slide2 style");
	} else if (!strcmp(part, "elm.text.2")) {
		return strdup("Two Buttons After Slide.");
	} else if (!strcmp(part, "elm.slide.text.1")) {
		return strdup("slide2");
	}
	return NULL;
}

static char *_gl_text_get3(void *data, Evas_Object *obj, const char *part)
{
	if (!strcmp(part, "elm.text")) {
		return strdup("slide3 style");
	} else if (!strcmp(part, "elm.slide.text.1")) {
		return strdup("slide3");
	}
	return NULL;
}

static char *_gl_text_get4(void *data, Evas_Object *obj, const char *part)
{
	if (!strcmp(part, "elm.text.1")) {
		return strdup("slide4 style");
	} else if (!strcmp(part, "elm.text.2")) {
		return strdup("Four Buttons After Slide.");
	} else if (!strcmp(part, "elm.slide.text.1")) {
		return strdup("slide4");
	}
	return NULL;
}

/**
 * gl_content_get is called for each swallow part when the item is realized.
 * This is also called when the sweep animation is started.
 *
 * @param data  :  3th parameter of the elm_genlist_item_append api.
 * @param obj   :  the genlist object.
 * @param part  :  text part name.
 *
 */
static Evas_Object *_gl_content_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *button, *icon;
	int index = (int) data % NUM_OF_NAMES;

	if (!strcmp(part, "elm.slide.swallow.1")) {
		button = elm_button_add(obj);
		elm_object_style_set(button, "sweep");
		elm_object_text_set(button, "Call");
		evas_object_smart_callback_add(button, "clicked", _button_clicked_cb, (void *)index);
		return button;
	} else if (!strcmp(part, "elm.slide.swallow.2")) {
		button = elm_button_add(obj);
		elm_object_style_set(button, "sweep");
		elm_object_text_set(button, "Delete");
		evas_object_smart_callback_add(button, "clicked", _button_clicked_cb, (void *)index);
		return button;
	} else if (!strcmp(part, "elm.slide.swallow.3")) {
		button = elm_button_add(obj);
		elm_object_style_set(button, "sweep");
		elm_object_text_set(button, "Remove");
		evas_object_smart_callback_add(button, "clicked", _button_clicked_cb, (void *)index);
		return button;
	} else if (!strcmp(part, "elm.slide.swallow.4")) {
		button = elm_button_add(obj);
		elm_object_style_set(button, "sweep");
		elm_object_text_set(button, "Answer");
		evas_object_smart_callback_add(button, "clicked", _button_clicked_cb, (void *)index);
		return button;
	} else {
		icon = elm_image_add(obj);
		elm_image_file_set(icon, ICON_DIR"/genlist/00_brightness_right.png", NULL);
		evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
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

/**************************** Genlist Smart Callbacks ******************************/
static void _my_gl_mode_right(void *data, Evas_Object *obj, void *event_info)
{
	if (!event_info) return;

	// Start genlist sweep
	elm_genlist_item_decorate_mode_set(event_info, "slide", EINA_TRUE);
	elm_genlist_item_select_mode_set(event_info, ELM_OBJECT_SELECT_MODE_NONE);
}

static void _my_gl_mode_left(void *data, Evas_Object *obj, void *event_info)
{
	if (!event_info) return;

	// Finish genlist sweep
	elm_genlist_item_decorate_mode_set(event_info, "slide", EINA_FALSE);
	elm_genlist_item_select_mode_set(event_info, ELM_OBJECT_SELECT_MODE_DEFAULT);
}

static void _my_gl_mode_cancel(void *data, Evas_Object *obj, void *event_info)
{
	if (!obj) return;

	// Get sweeped item
	Elm_Object_Item *it = (Elm_Object_Item *)elm_genlist_decorated_item_get(obj);

	// Finish genlist sweep
	if (it) {
		elm_genlist_item_decorate_mode_set(it, "slide", EINA_FALSE);
		elm_genlist_item_select_mode_set(event_info, ELM_OBJECT_SELECT_MODE_DEFAULT);
	}
}

/**************************** Genlist Settings ******************************/
// Set genlist item styles for slide animation
static void _set_genlist_item_styles(void)
{
	itc.item_style = "1text.1icon.2";
	itc.decorate_item_style = "mode/slide";
	itc.func.text_get = _gl_text_get;
	itc.func.content_get = _gl_content_get;
	itc.func.state_get = _gl_state_get;
	itc.func.del = _gl_del;
	itc.decorate_all_item_style = "edit_default";

	itc2.item_style = "2text.1icon.2";
	itc2.decorate_item_style = "mode/slide2";
	itc2.func.text_get = _gl_text_get2;
	itc2.func.content_get = _gl_content_get;
	itc2.func.state_get = _gl_state_get;
	itc2.func.del = _gl_del;
	itc2.decorate_all_item_style = "edit_default";

	itc3.item_style = "1text.1icon.2";
	itc3.decorate_item_style = "mode/slide3";
	itc3.func.text_get = _gl_text_get3;
	itc3.func.content_get = _gl_content_get;
	itc3.func.state_get = _gl_state_get;
	itc3.func.del = _gl_del;
	itc3.decorate_all_item_style = "edit_default";

	itc4.item_style = "2text.1icon.2";
	itc4.decorate_item_style = "mode/slide4";
	itc4.func.text_get = _gl_text_get4;
	itc4.func.content_get = _gl_content_get;
	itc4.func.state_get = _gl_state_get;
	itc4.func.del = _gl_del;
	itc4.decorate_all_item_style = "edit_default";
}

static Evas_Object *_create_genlist(Evas_Object *parent)
{
	int index;
	Evas_Object *genlist;

	// Create genlist
	genlist = elm_genlist_add(parent);
	evas_object_smart_callback_add(genlist, "drag,start,right", _my_gl_mode_right, NULL);
	evas_object_smart_callback_add(genlist, "drag,start,left", _my_gl_mode_left, NULL);
	evas_object_smart_callback_add(genlist, "drag,start,up", _my_gl_mode_cancel, NULL);
	evas_object_smart_callback_add(genlist, "drag,start,down", _my_gl_mode_cancel, NULL);

	// Append genlist items
	for (index = 0; index < NUM_OF_ITEMS; index++) {
		elm_genlist_item_append(genlist, &itc, (void *) index, NULL,
				ELM_GENLIST_ITEM_NONE, NULL, NULL);
		elm_genlist_item_append(genlist, &itc2, (void *) index, NULL,
				ELM_GENLIST_ITEM_NONE, NULL, NULL);
		elm_genlist_item_append(genlist, &itc3, (void *) index, NULL,
				ELM_GENLIST_ITEM_NONE, NULL, NULL);
		elm_genlist_item_append(genlist, &itc4, (void *) index, NULL,
				ELM_GENLIST_ITEM_NONE, NULL, NULL);
	}

	return genlist;
}

static void _edit_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!elm_genlist_decorate_mode_get(genlist)) {
		// Change button label
		elm_object_text_set(edit_btn, _("Done"));

		// Set reorder and edit mode
		elm_genlist_decorate_mode_set(genlist, EINA_TRUE);

		// This means even if selected, every click will make the selected callbacks be called.
		elm_genlist_select_mode_set(genlist, ELM_OBJECT_SELECT_MODE_ALWAYS);
	} else {
		// Change button label
		elm_object_text_set(edit_btn, _("Edit"));

		// Unset edit mode
		elm_genlist_decorate_mode_set(genlist, EINA_FALSE);
		elm_genlist_select_mode_set(genlist, ELM_OBJECT_SELECT_MODE_DEFAULT);
	}
}

// Create 'Edit'/'Done' button
static Evas_Object *_create_edit_button()
{
	Evas_Object *button;

	button = elm_button_add(toolbar);
	elm_object_style_set(button, "naviframe_control/default");
	evas_object_size_hint_weight_set(button, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(button, EVAS_HINT_FILL, 0.5);
	elm_object_text_set(button, _("Edit"));
	evas_object_show(button);
	evas_object_smart_callback_add(button, "clicked", _edit_button_clicked_cb, NULL);

	return button;
}
/**************************** Main Code ******************************/
void genlist_test_sweep_edit_cb(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *it;
	if (!data) return;
	struct appdata *ad = (struct appdata *) data;

	// Set Genlist Item Styles for 4 itcs
	_set_genlist_item_styles();

	// Create a Genlist and append items.
	genlist = _create_genlist(ad->nf);

	// Create Controlbar for naviframe menu
	toolbar = elm_toolbar_add(ad->nf);
	elm_toolbar_shrink_mode_set(toolbar, ELM_TOOLBAR_SHRINK_EXPAND);
	if (toolbar == NULL) return;
	elm_object_style_set(toolbar, "naviframe");

	edit_btn = _create_edit_button();
	it = elm_toolbar_item_append(toolbar, NULL, NULL, NULL, NULL);
	elm_object_item_part_content_set(it, "object", edit_btn);

	// Push genlist to naviframe
	it = elm_naviframe_item_push(ad->nf,  _("Sweep"), NULL, NULL, genlist, NULL);
	elm_object_item_part_content_set(it, "optionheader", toolbar);
}
