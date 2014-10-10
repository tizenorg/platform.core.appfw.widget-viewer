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
#include "dialoguegroup.h"
#include <Ecore_X.h>

/*********************************************************
   AutoScroll
 ********************************************************/

typedef struct _Autoscroll_Data Autoscroll_Data;
struct _Autoscroll_Data
{
	Evas_Object *outer_box;
};

/* a sample text string */
static const char entry_input_text[] = {
	"Enlightenment is the flagship and original name bearer "
	"for this project. Once it was just a humble window manager "
	"for X11 that wanted to do things differently. To do them "
	"better, but it has expanded. This can be confusing so when"
	" we refer to Enlightenment, we may mean the project as a "
	"whole or just the window manager proper. The libraries "
	"behind Enlightenment are referred to as EFL collectively,"
	" each with a specific name and purpose. The window manager "
	"is a lean, fast, modular and very extensible window manager "
	"for X11 and Linux. It is classed as a desktop shell providing"
	" the things you need to operate your desktop (or laptop), but"
	"is not a whole application suite. This covered launching "
	"applications, managing their windows and doing other system "
	"tasks like suspending, reboots, managing files etc. "
};

/* function to create a scroller object */
static Evas_Object* _create_scroller(Evas_Object* parent)
{
	Evas_Object* scroller = elm_scroller_add(parent);
	elm_scroller_bounce_set(scroller, EINA_FALSE, EINA_TRUE);
	elm_scroller_policy_set(scroller,ELM_SCROLLER_POLICY_OFF,ELM_SCROLLER_POLICY_AUTO);
	evas_object_show(scroller);

	return scroller;
}

static Evas_Object* _create_mutli_scroll_layout(Evas_Object* parent)
{
	Evas_Object *en;

	//Layout for Entries.
	Evas_Object* layout = elm_layout_add(parent);
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "ui_entryfields");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);


	en = elm_entry_add(layout);
	ea_entry_selection_back_event_allow_set(en, EINA_TRUE);
	elm_object_part_content_set(layout, "en_part1", en);
	elm_entry_entry_set(en, "Enlightenment Foundation Lib (EFL).");
	elm_entry_scrollable_set(en, EINA_TRUE);
	elm_scroller_policy_set(en, ELM_SCROLLER_POLICY_OFF,ELM_SCROLLER_POLICY_AUTO);

	en = elm_entry_add(layout);
	ea_entry_selection_back_event_allow_set(en, EINA_TRUE);
	elm_object_part_content_set(layout, "en_part2", en);
	elm_entry_entry_set(en, "Elementary is a VERY SIMPLE toolkit. It is not meant for "
				"writing extensive desktop applications");
	elm_entry_scrollable_set(en, EINA_TRUE);
	elm_scroller_policy_set(en, ELM_SCROLLER_POLICY_OFF,ELM_SCROLLER_POLICY_AUTO);

	en = elm_entry_add(layout);
	ea_entry_selection_back_event_allow_set(en, EINA_TRUE);
	elm_object_part_content_set(layout, "en_part3", en);
	elm_entry_entry_set(en, "Elementary is meant to make the programmers work almost "
				"brainless but give them lots of flexibility.");
	elm_entry_scrollable_set(en, EINA_TRUE);
	elm_scroller_policy_set(en, ELM_SCROLLER_POLICY_OFF,ELM_SCROLLER_POLICY_AUTO);

	en = elm_entry_add(layout);
	ea_entry_selection_back_event_allow_set(en, EINA_TRUE);
	elm_object_part_content_set(layout, "en_part4", en);
	elm_entry_entry_set(en, "Enlightenment, the window manager is built on top of "
				"building blocks known as EFL (the Enlightenment Foundation Libraries).");
	elm_entry_scrollable_set(en, EINA_TRUE);
	elm_scroller_policy_set(en, ELM_SCROLLER_POLICY_OFF,ELM_SCROLLER_POLICY_AUTO);

	en = elm_entry_add(layout);
	ea_entry_selection_back_event_allow_set(en, EINA_TRUE);
	elm_object_part_content_set(layout, "en_part5", en);
	elm_entry_entry_set(en, "Enlightenment covers uses from small mobile devices like phones all the "
				"way to powerful multi-core desktops (which are the primary "
				"development environment).");
	elm_entry_scrollable_set(en, EINA_TRUE);
	elm_scroller_policy_set(en, ELM_SCROLLER_POLICY_OFF,ELM_SCROLLER_POLICY_AUTO);

	en = elm_entry_add(layout);
	ea_entry_selection_back_event_allow_set(en, EINA_TRUE);
	elm_object_part_content_set(layout, "en_part6", en);
	elm_entry_entry_set(en, "Enlightenment is not just a window manager for Linux/X11 and "
				"others, but also a whole suite of libraries to help you create"
				" beautiful user interfaces with much less work than doing it "
				"the old fashioned way and fighting with traditional toolkits, "
				"not to mention a traditional window manager.");
	elm_entry_scrollable_set(en, EINA_TRUE);
	elm_scroller_policy_set(en, ELM_SCROLLER_POLICY_OFF,ELM_SCROLLER_POLICY_AUTO);

	return layout;
}

static Evas_Object* _create_mutli_scroll2_layout(Evas_Object* parent)
{
	Evas_Object *en;

	//Layout for Entries.
	Evas_Object* layout = elm_layout_add(parent);
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "ui_entryfields1");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	en = elm_entry_add(layout);
	ea_entry_selection_back_event_allow_set(en, EINA_TRUE);
	elm_object_part_content_set(layout, "en_part1", en);
	elm_entry_scrollable_set(en, EINA_TRUE);
	elm_entry_entry_set(en,
			"Enlightenment is the flagship and original name bearer "
			"for this project. Once it was just a humble window manager "
			"for X11 that wanted to do things differently. To do them "
			"better, but it has expanded. This can be confusing so when"
			" we refer to Enlightenment, we may mean the project as a"
			" whole or just the window manager proper. The libraries "
			"behind Enlightenment are referred to as EFL collectively,"
			" each with a specific name and purpose. "
			);

	en = elm_entry_add(layout);
	ea_entry_selection_back_event_allow_set(en, EINA_TRUE);
	elm_object_part_content_set(layout, "en_part2", en);
	elm_entry_entry_set(en, "Enlightenment covers uses from small mobile devices like phones all the "
				"way to powerful multi-core desktops (which are the primary "
				"development environment).");
	elm_entry_scrollable_set(en, EINA_TRUE);
	elm_scroller_policy_set(en, ELM_SCROLLER_POLICY_OFF,ELM_SCROLLER_POLICY_AUTO);

	en = elm_entry_add(layout);
	ea_entry_selection_back_event_allow_set(en, EINA_TRUE);
	elm_object_part_content_set(layout, "en_part3", en);
	elm_entry_entry_set(en, "Enlightenment Foundation Libraries (EFL).");
	elm_entry_scrollable_set(en, EINA_TRUE);
	elm_scroller_policy_set(en, ELM_SCROLLER_POLICY_OFF,ELM_SCROLLER_POLICY_AUTO);

	return layout;

}

static Evas_Object* _create_scroll_entry_layout(Evas_Object* parent)
{
	Evas_Object  *en;
	Evas_Object *bx;

	bx = elm_box_add(parent);
	evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);

	en = elm_entry_add(parent);
	ea_entry_selection_back_event_allow_set(en, EINA_TRUE);
	elm_entry_entry_set(en,entry_input_text);

	evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(en, EVAS_HINT_FILL, EVAS_HINT_FILL);

	evas_object_show(en);
	elm_box_pack_end(bx, en);

	evas_object_show(bx);

	return bx;
}

/*
Function :
Create an outer layout
Create a conformant widget
Create a scroller widget
Add a inner layout to scroller
Add scroller to conformant widget
Add conformant widget to outer layout
*/
static void add_layout_to_conformant(void *data, Evas_Object *lay_in, const char *title)
{
	Evas_Object *scroller;
	Evas_Object *outer_layout;
	struct appdata *ad;

	ad = (struct appdata *) data;
	if(ad == NULL) return;

	/* creating a sample outer layout. Applicaitons have to use their own
	 way of creating outer layout */
	outer_layout = elm_layout_add(ad->nf);
	evas_object_size_hint_weight_set(outer_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_layout_file_set(outer_layout, ELM_DEMO_EDJ, "autoscroll_conform");

	scroller = _create_scroller(ad->nf);
	elm_object_content_set(scroller, lay_in);
	elm_object_part_content_set(outer_layout, "conform_part", scroller);
	elm_naviframe_item_push(ad->nf, title, NULL, NULL, outer_layout, NULL);
}

/*
   Test use case : Adding a scroller with editifields to another scroller
   and finally added to elm conformant widget for autoscrolling
*/
static void _use_case_multi_scroller_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad;
	Evas_Object *lay_in;

	ad = (struct appdata *) data;
	if(ad == NULL) return;

	lay_in = _create_mutli_scroll_layout(ad->nf);
	add_layout_to_conformant(data, lay_in, _("Scroller"));
}

/*
   Test use case : Adding a scroller with editifields (with a different size to
   view inner scroller) to another outer scroller
   and finally added to elm conformant widget for autoscrolling
*/
static void _use_case_multi_scroller2_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad;
	Evas_Object *lay_in;

	ad = (struct appdata *) data;
	if(ad == NULL) return;

	lay_in = _create_mutli_scroll2_layout(ad->nf);
	add_layout_to_conformant(data, lay_in, _("Scroller in Scroller"));
}

/*
   Test use case : Adding scrolled entry to elm conformant widget for auto-scrolling
*/
static void _use_case_scroll_entry_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *lay_in;
	struct appdata *ad;

	ad = (struct appdata *) data;
	if(ad == NULL) return;

	lay_in = _create_scroll_entry_layout(ad->nf);
	add_layout_to_conformant(data, lay_in, _("ScrolledEntry"));
}

static void _list_click(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *it = (Elm_Object_Item *) elm_list_selected_item_get(obj);
	if(it == NULL)
	  {
	     printf("list item is NULL\n");
	     return;
	  }

	elm_list_item_selected_set(it, EINA_FALSE);
}

void autoscroll_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *) data;
	if(ad == NULL) return;

	Evas_Object  *list;

	ecore_imf_init();

	list = elm_list_add(ad->nf);
	elm_list_mode_set(list, ELM_LIST_COMPRESS);
	evas_object_smart_callback_add(list, "selected", _list_click, NULL);

	elm_list_item_append(list, "ScrolledEntry", NULL, NULL,
			_use_case_scroll_entry_cb, ad);
	elm_list_item_append(list, "Scroller", NULL, NULL,
			_use_case_multi_scroller_cb, ad);
	elm_list_item_append(list, "Scroller in Scroller", NULL, NULL,
			_use_case_multi_scroller2_cb, ad);
	elm_list_go(list);

	elm_naviframe_item_push(ad->nf, _("Autoscroll"), NULL, NULL, list, NULL);
}
