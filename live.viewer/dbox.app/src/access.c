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
#include "access.h"
#include "genlist.h"

#define AFTER_MIGRATION 1
#include <Ecore_X.h>
/*********************************************************
 access
 ********************************************************/
#define NUM_OF_ITEMS 100

extern char *genlist_demo_names[];

typedef struct _Item_Data
{
	int index;
	Elm_Object_Item *item;
} Item_Data;

static char *_access_info_cb(void *data, Evas_Object *obj)
{
	return strdup(_("edje button"));
}

static void _highlighted_cb(void *data, Evas_Object * obj, void *event_info)
{
	printf("Highlight\n");
}

static void _unhighlighted_cb(void *data, Evas_Object * obj, void *event_info)
{
	printf("Unhighlight\n");
}

static void _read_start_cb(void *data, Evas_Object * obj, void *event_info)
{
	printf("Read start\n");
}

static void _read_stop_cb(void *data, Evas_Object * obj, void *event_info)
{
	printf("Read stop\n");
}

static void _activated_cb(void *data, Evas_Object * obj, void *event_info)
{
	printf("Activated\n");
}

static void _clicked_cb(void *data, Evas_Object * obj, void *event_info)
{
	printf("Button Clicked\n");
}

static Evas_Object *_create_toolbar(Evas_Object *parent)
{
	Evas_Object *obj;
	Elm_Object_Item *item[10];

	/* create toolbar */
	obj = elm_toolbar_add(parent);
	if(obj == NULL) return NULL;
	elm_object_style_set(obj, "default");
	elm_toolbar_shrink_mode_set(obj, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_transverse_expanded_set(obj, EINA_TRUE);
	elm_toolbar_select_mode_set(obj, ELM_OBJECT_SELECT_MODE_NONE);

	item[0] = elm_toolbar_item_append(obj, NULL, "Main", NULL, NULL);
	item[1] = elm_toolbar_item_append(obj, NULL, "Playlist", NULL, NULL);
	item[2] = elm_toolbar_item_append(obj, NULL, "Artists list", NULL, NULL);
	return obj;
}

static char *
_gli_text_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024];
	Item_Data *id = data;
	int index = id->index;

	if (!strcmp(part, "elm.text.2")) index++;

	snprintf(buf, 1023, "%s:%s", part, genlist_demo_names[index%NUM_OF_GENLIST_DEMO_NAMES]);
	return strdup(buf);
}

static Evas_Object *
_gli_content_get(void *data, Evas_Object *obj, const char *part)
{
	return NULL;
}

static void
_gli_sel(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *item;

	item = (Elm_Object_Item *) event_info;
	if (!item) return;

	elm_genlist_item_selected_set(item, EINA_FALSE);
}

static Evas_Object *
_create_inner_genlist(Evas_Object *parent)
{
	Evas_Object * genlist;
	int index;
	Elm_Object_Item *item;

	// Create genlist
	genlist = elm_genlist_add(parent);

	// Set genlist item class
	Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
	itc->item_style = "1text.1icon";
	itc->func.text_get = _gli_text_get;
	itc->func.content_get = _gli_content_get;
	itc->func.state_get = NULL;
	itc->func.del = NULL;

	// Append items
	for (index = 0; index < NUM_OF_ITEMS; index++) {
		Item_Data *id = calloc(sizeof(Item_Data), 1);
		id->index = index;
		item = elm_genlist_item_append(
				genlist,			// genlist object
				itc,				// item class
				id,		            // data
				NULL,
				ELM_GENLIST_ITEM_NONE,
				_gli_sel,
				NULL);
		id->item = item;
		elm_object_item_data_set(item,id);
	}
	elm_genlist_item_class_free(itc);

	return genlist;
}

static char *
_gl_text_get(void *data, Evas_Object *obj, const char *part)
{
	int index = (int)data;

	if (!part) return NULL;
	if (index == 1 && !strcmp(part, "elm.text")) return strdup("Screen reader (TTS)");
	if (index == 2 && !strcmp(part, "elm.text")) return strdup("Scroll Test");
	if (index == 3 && !strcmp(part, "elm.text")) return strdup("Overlap Test");
	if (index == 4 && !strcmp(part, "elm.text")) return strdup("Access Changed Test");
	if (index == 5 && !strcmp(part, "elm.text")) return strdup("Access Activate Test");
	return NULL;
}

static Evas_Object *
_gl_content_get(void *data, Evas_Object *obj, const char *part)
{
	int enable = 0;
	Evas_Object *icon;
	int index = (int)data;

	if (!part) return NULL;
	if (index != 1) return NULL;
	if (!strcmp(part, "elm.icon") || !strcmp(part, "elm.swallow.icon")) {
		icon = elm_check_add(obj);

#ifdef DESKTOP
		enable = elm_config_access_get();
#else
		if (0 != vconf_get_bool(VCONFKEY_SETAPPL_ACCESSIBILITY_TTS, &enable))
			printf("Failed to get vconf value\n");
#endif
		elm_check_state_set(icon, enable);

		elm_object_style_set(icon, "on&off");
		evas_object_pass_events_set(icon, 1);
		evas_object_propagate_events_set(icon, 0);
		return icon;
	}

	return NULL;
}

static void
_gl_sel(void *data, Evas_Object *obj, void *event_info)
{
	int enable;
	Elm_Object_Item *item;
	Evas_Object *icon;

	item = (Elm_Object_Item *) event_info;
	if (!item) return;

	elm_genlist_item_selected_set(item, EINA_FALSE);

	icon = elm_object_item_part_content_get(item, "elm.icon");
	if (!icon) return;

	enable = (int) !elm_check_state_get(icon);
#ifdef DESKTOP
	elm_config_access_set(enable);
	_elm_access_mouse_event_enabled_set(EINA_TRUE);
#else
	if (0 != vconf_set_bool(VCONFKEY_SETAPPL_ACCESSIBILITY_TTS, enable)) {
		printf("Faile to set vconf value\n");
		return;
	}
#endif
	elm_check_state_set(icon, enable);
}

static void
_gl_second_sel(void *data, Evas_Object *obj, void * event_info)
{
	Evas_Object *navi, *layout, *scroller, *toolbar, *genlist, *btn, *ao;

	Elm_Object_Item *item;

	item = (Elm_Object_Item *) event_info;
	if (!item) return;

	elm_genlist_item_selected_set(item, EINA_FALSE);

	navi = (Evas_Object *)data;

	scroller = elm_scroller_add(navi);

	layout = elm_layout_add(scroller);
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "scroll_test");

	ao = elm_object_part_access_register(layout, "edje_button");
	elm_access_info_cb_set(ao, ELM_ACCESS_INFO, _access_info_cb, layout);
	elm_object_focus_custom_chain_append(layout, ao, NULL);

	ao = elm_object_part_access_register(layout, "edje_button2");
	elm_access_info_cb_set(ao, ELM_ACCESS_INFO, _access_info_cb, layout);
	elm_object_focus_custom_chain_append(layout, ao, NULL);

	ao = elm_object_part_access_register(layout, "edje_button3");
	elm_access_info_cb_set(ao, ELM_ACCESS_INFO, _access_info_cb, layout);
	elm_object_focus_custom_chain_append(layout, ao, NULL);

	ao = elm_object_part_access_register(layout, "edje_button4");
	elm_access_info_cb_set(ao, ELM_ACCESS_INFO, _access_info_cb, layout);
	elm_object_focus_custom_chain_append(layout, ao, NULL);

	ao = elm_object_part_access_register(layout, "edje_button5");
	elm_access_info_cb_set(ao, ELM_ACCESS_INFO, _access_info_cb, layout);
	elm_object_focus_custom_chain_append(layout, ao, NULL);

	ao = elm_object_part_access_register(layout, "edje_button6");
	elm_access_info_cb_set(ao, ELM_ACCESS_INFO, _access_info_cb, layout);
	elm_object_focus_custom_chain_append(layout, ao, NULL);

	ao = elm_object_part_access_register(layout, "edje_button7");
	elm_access_info_cb_set(ao, ELM_ACCESS_INFO, _access_info_cb, layout);
	elm_object_focus_custom_chain_append(layout, ao, NULL);

	btn = elm_button_add(layout);
	elm_object_text_set(btn, "Play");
	elm_object_part_content_set(layout, "elm.swallow.content", btn);
	elm_object_focus_custom_chain_append(layout, btn, NULL);
	evas_object_smart_callback_add(btn, "access,highlighted", _highlighted_cb, btn);
	evas_object_smart_callback_add(btn, "access,unhighlighted", _unhighlighted_cb, btn);
	evas_object_smart_callback_add(btn, "access,read,start", _read_start_cb, btn);
	evas_object_smart_callback_add(btn, "access,read,stop", _read_stop_cb, btn);
	evas_object_smart_callback_add(btn, "access,activated", _activated_cb, btn);
	evas_object_smart_callback_add(btn, "clicked", _clicked_cb, btn);

	btn = elm_button_add(layout);
	elm_object_text_set(btn, "button");
	elm_object_part_content_set(layout, "elm.swallow.content2", btn);
	elm_object_focus_custom_chain_append(layout, btn, NULL);

	btn = elm_button_add(layout);
	elm_object_text_set(btn, "button");
	elm_object_part_content_set(layout, "elm.swallow.content3", btn);
	elm_object_focus_custom_chain_append(layout, btn, NULL);

	btn = elm_button_add(layout);
	elm_object_text_set(btn, "button");
	elm_object_part_content_set(layout, "elm.swallow.content4", btn);
	elm_object_focus_custom_chain_append(layout, btn, NULL);

	genlist = _create_inner_genlist(layout);
	elm_object_part_content_set(layout, "elm.swallow.genlist", genlist);
	elm_object_focus_custom_chain_append(layout, genlist, NULL);

	toolbar = _create_toolbar(layout);
	elm_object_part_content_set(layout, "elm.swallow.toolbar", toolbar);
	elm_object_focus_custom_chain_append(layout, toolbar, NULL);

	elm_object_content_set(scroller, layout);

	elm_naviframe_item_push(navi, _("Scroll Test"), NULL, NULL, scroller, NULL);
}

static char *
_overlap_test_access_info_cb(void *data, Evas_Object *obj)
{
   if (data) return strdup(data);
   return NULL;
}

static void _access_activate_cb(void *data, Evas_Object *part_obj, Elm_Object_Item *item)
{
	Eina_Strbuf *strbuf;
	char *txt = (char *)data;

	strbuf = eina_strbuf_new();
	eina_strbuf_append_printf(strbuf, "%s rectangle is activated", txt);

	elm_access_say(eina_strbuf_string_get(strbuf));
	eina_strbuf_free(strbuf);
}

static Evas_Object *
_create_layout(Evas_Object *parent)
{
	Evas_Object *layout;
	Evas_Object *to;
	Evas_Object *red_ao, *green_ao, *blue_ao;
	Eina_List *custom_chain = NULL;

	// Create layout
	layout = elm_layout_add(parent);

	elm_layout_file_set(layout, ELM_DEMO_EDJ, "overlap_test");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	to = (Evas_Object *)edje_object_part_object_get(elm_layout_edje_get(layout), "red");
	red_ao = elm_access_object_register(to, layout);
	elm_access_info_cb_set(red_ao, ELM_ACCESS_INFO, _overlap_test_access_info_cb, "red");

	to = (Evas_Object *)edje_object_part_object_get(elm_layout_edje_get(layout), "green");
	green_ao = elm_access_object_register(to, layout);
	elm_access_info_cb_set(green_ao, ELM_ACCESS_INFO, _overlap_test_access_info_cb, "green");

	to = (Evas_Object *)edje_object_part_object_get(elm_layout_edje_get(layout), "blue");
	blue_ao = elm_access_object_register(to, layout);
	elm_access_info_cb_set(blue_ao, ELM_ACCESS_INFO, _overlap_test_access_info_cb, "blue");

	custom_chain = eina_list_append(custom_chain, red_ao);
	custom_chain = eina_list_append(custom_chain, green_ao);
	custom_chain = eina_list_append(custom_chain, blue_ao);

	elm_access_activate_cb_set(red_ao, _access_activate_cb, "red");
	elm_access_activate_cb_set(green_ao, _access_activate_cb, "green");
	elm_access_activate_cb_set(blue_ao, _access_activate_cb, "blue");

	elm_object_focus_custom_chain_set(layout, custom_chain);

	return layout;
}

static Evas_Object *
_create_fourth_layout(Evas_Object *parent)
{
	Evas_Object *layout;
	Evas_Object *to;
	Evas_Object *red_ao, *green_ao, *blue_ao;

	// Create layout
	layout = elm_layout_add(parent);

	elm_layout_file_set(layout, ELM_DEMO_EDJ, "overlap_test");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	to = (Evas_Object *)edje_object_part_object_get(elm_layout_edje_get(layout), "red");
	red_ao = elm_access_object_register(to, layout);
	elm_access_info_cb_set(red_ao, ELM_ACCESS_INFO, _overlap_test_access_info_cb, "red");

	to = (Evas_Object *)edje_object_part_object_get(elm_layout_edje_get(layout), "green");
	green_ao = elm_access_object_register(to, layout);
	elm_access_info_cb_set(green_ao, ELM_ACCESS_INFO, _overlap_test_access_info_cb, "green");

	to = (Evas_Object *)edje_object_part_object_get(elm_layout_edje_get(layout), "blue");
	blue_ao = elm_access_object_register(to, layout);
	elm_access_info_cb_set(blue_ao, ELM_ACCESS_INFO, _overlap_test_access_info_cb, "blue");

	elm_access_activate_cb_set(red_ao, _access_activate_cb, "red");
	elm_access_activate_cb_set(green_ao, _access_activate_cb, "green");
	elm_access_activate_cb_set(blue_ao, _access_activate_cb, "blue");

	return layout;
}

static void
_gl_third_sel(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *item;

	item = (Elm_Object_Item *) event_info;
	if (!item) return;

	elm_genlist_item_selected_set(item, EINA_FALSE);

	Evas_Object *layout;

	layout = _create_layout(data);

	elm_naviframe_item_push(data, _("Overlap Test"), NULL, NULL, layout, NULL);
}

static void _activate_cb(void *data, Evas_Object * obj, void *event_info)
{
	Evas_Object *entry;

	if (data) entry = data;
	else entry = obj;

	elm_entry_input_panel_show(entry);
}

static Evas_Object *
_create_fifth_layout(Evas_Object *parent)
{
	Evas_Object *layout, *en, *label;

	// Create layout
	layout = elm_layout_add(parent);

	elm_layout_file_set(layout, ELM_DEMO_EDJ, "activate_test");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	label = elm_label_add(parent);
	elm_object_text_set(label, "Open input panel by 1 finger double tap.");
	elm_object_part_content_set(layout, "elm.swallow.label", label);

	en = elm_entry_add(parent);
	ea_entry_selection_back_event_allow_set(en, EINA_TRUE);
	elm_entry_input_panel_enabled_set(en, EINA_FALSE);
	elm_object_part_content_set(layout, "elm.swallow.entry", en);

	evas_object_smart_callback_add(label, "access,activate", _activate_cb, en);
	evas_object_smart_callback_add(en, "access,activate", _activate_cb, NULL);

	return layout;
}

static void _access_highlight_chain_set(Elm_Object_Item *it)
{
	Evas_Object *content;
	Evas_Object *it_ao;
	Evas_Object *to, *red_ao, *green_ao, *blue_ao;

	content = elm_object_item_content_get(it);

	it_ao = elm_object_item_access_object_get(it);

	to = (Evas_Object *)edje_object_part_object_get(elm_layout_edje_get(content), "blue");
	blue_ao = elm_access_object_get(to);

	to = (Evas_Object *)edje_object_part_object_get(elm_layout_edje_get(content), "green");
	green_ao = elm_access_object_get(to);

	to = (Evas_Object *)edje_object_part_object_get(elm_layout_edje_get(content), "red");
	red_ao = elm_access_object_get(to);

	elm_access_highlight_next_set(it_ao, ELM_HIGHLIGHT_DIR_NEXT, blue_ao);
	elm_access_highlight_next_set(blue_ao, ELM_HIGHLIGHT_DIR_NEXT, green_ao);
	elm_access_highlight_next_set(green_ao, ELM_HIGHLIGHT_DIR_NEXT, red_ao);
	elm_access_highlight_next_set(red_ao, ELM_HIGHLIGHT_DIR_NEXT, it_ao);
}

static void _access_changed_cb(void *data, Evas_Object * obj, void *event_info)
{
	Eina_List *items, *l = NULL;
	Elm_Object_Item *it;

	if (!elm_config_access_get()) return;

	items = elm_naviframe_items_get(obj);
	EINA_LIST_FOREACH(items, l, it) {
		if (it == data) _access_highlight_chain_set(it);
	}
}

static void
_gl_fourth_sel(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *item;

	item = (Elm_Object_Item *) event_info;
	if (!item) return;

	elm_genlist_item_selected_set(item, EINA_FALSE);

	Evas_Object *layout;

	layout = _create_fourth_layout(data);

	item = elm_naviframe_item_push(data, _("Access Changed"), NULL, NULL, layout, NULL);
	_access_highlight_chain_set(item);

	evas_object_smart_callback_add(data, "access,changed", _access_changed_cb, item);
}

static void
_gl_fifth_sel(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *item;

	item = (Elm_Object_Item *) event_info;
	if (!item) return;

	elm_genlist_item_selected_set(item, EINA_FALSE);

	Evas_Object *layout;

	layout = _create_fifth_layout(data);

	elm_naviframe_item_push(data, _("Access Activate"), NULL, NULL, layout, NULL);
}

static Evas_Object *
_create_genlist(Evas_Object *parent)
{
	Evas_Object * genlist;

	// Create genlist
	genlist = elm_genlist_add(parent);

	// Set genlist item class
	Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
	itc->item_style = "1text.1icon";
	itc->func.text_get = _gl_text_get;
	itc->func.content_get = _gl_content_get;
	itc->func.state_get = NULL;
	itc->func.del = NULL;

	// Append items
	elm_genlist_item_append(genlist, itc, (void *)1, NULL, ELM_GENLIST_ITEM_NONE, _gl_sel, NULL);
	elm_genlist_item_append(genlist, itc, (void *)2, NULL, ELM_GENLIST_ITEM_NONE, _gl_second_sel, parent);
	elm_genlist_item_append(genlist, itc, (void *)3, NULL, ELM_GENLIST_ITEM_NONE, _gl_third_sel, parent);
	elm_genlist_item_append(genlist, itc, (void *)4, NULL, ELM_GENLIST_ITEM_NONE, _gl_fourth_sel, parent);
	elm_genlist_item_append(genlist, itc, (void *)5, NULL, ELM_GENLIST_ITEM_NONE, _gl_fifth_sel, parent);

	elm_genlist_item_class_free(itc);

	return genlist;
}

void access_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *) data;
	Evas_Object *genlist;

	genlist = _create_genlist(ad->nf);

	elm_naviframe_item_push(ad->nf, _("Accessibility"), NULL, NULL, genlist, NULL);
}
