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

#define _GNU_SOURCE

#include "elmdemo_test.h"
#include "elmdemo_util.h"
#include "searchbar.h"

/*********************************************************
 SearchBar
 ********************************************************/

#define NUM_OF_ITEMS 50

static Evas_Object *search_gl = NULL;
static Elm_Object_Item *git = NULL;
static Elm_Genlist_Item_Class itc, itc2;
static Evas_Object *searchbar_layout, *layout;

static char *names[] = { "Aaliyah", "Aamir", "Aaralyn", "Aaron", "Abagail",
	"Babitha", "Bahuratna", "Bandana", "Bulbul", "Cade", "Caldwell",
	"Chandan", "Caster", "Dagan ", "Daulat", "Dag", "Earl", "Ebenzer",
	"Ellison", "Elizabeth", "Filbert", "Fitzpatrick", "Florian", "Fulton",
	"Frazer", "Gabriel", "Gage", "Galen", "Garland", "Gauhar", "Hadden",
	"Hafiz", "Hakon", "Haleem", "Hank", "Hanuman", "Jabali ", "Jaimini",
	"Jayadev", "Jake", "Jayatsena", "Jonathan", "Kamaal", "Jeirk",
	"Jasper", "Jack", "Mac", "Macy", "Marlon", "Milson" };

static char *group[5] = { N_("Family"), N_("Friends"), N_("Colleagues"), N_("Relatives"),
	N_("Classmates") };

static void genlist_item_search(const char *search_word);
static void genlist_clear(void);
static void genlist_item_add();
static char *_gl_text_get(void *data, Evas_Object *obj, const char *part);
static Eina_Bool _gl_state_get(void *data, Evas_Object *obj, const char *part);
static void _gl_del(void *data, Evas_Object *obj);
static void _gl_sel(void *data, Evas_Object *obj, void *event_info);
static char* _gl_text_get_title(void *data, Evas_Object *obj, const char *part);
static void _create_genlist_group_title_style(void *data);

static int _search(void *data);
static void _changed_cb(void *data, Evas_Object *obj, void *event_info);
static void _cancel_clicked_cb(void *data, Evas_Object *obj, void *event_info);
static void _create_searchbar(void *data);

static void genlist_item_search(const char *search_word)
{
   int index;
   Elm_Object_Item *item_del = NULL;

	int flag = 0;

	for (index=0; index < NUM_OF_ITEMS; index++) {
		if (index % 5 == 0) {
			if (item_del && flag == 0) {
				elm_object_item_del(item_del);
				item_del = NULL;
			}
			git = elm_genlist_item_append(search_gl, &itc2, (void *) index, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
			flag = 0;
			item_del = git;
		}
		else {
			if (strcasestr(names[index], search_word)) {
			elm_genlist_item_append(search_gl, &itc, (void *) index, NULL, ELM_GENLIST_ITEM_NONE, _gl_sel, NULL);
			flag = 1;
			}
		}
		if (index == (NUM_OF_ITEMS-1)) {
			if (item_del && flag == 0) {
				elm_object_item_del(item_del);
				item_del = NULL;
			}
		}
	}
}

static void genlist_clear(void)
{
	if(search_gl == NULL) return;
	elm_genlist_clear(search_gl);
}

static void genlist_item_add()
{
	int index;

	for (index=0; index < NUM_OF_ITEMS; index++) {
		if (index % 5 == 0) {
			git = elm_genlist_item_append(search_gl, &itc2, (void *) index, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
		}
		else {
			elm_genlist_item_append(search_gl, &itc, (void *) index, NULL, ELM_GENLIST_ITEM_NONE, _gl_sel, NULL);
		}
	}
}

static char *_gl_text_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[256];
	int index = (int) data % NUM_OF_ITEMS;

	if (strcmp(part, "elm.text") == 0) {
		snprintf(buf, sizeof(buf), "%s", names[index]);
		return strdup(buf);
	}
	else {
		snprintf(buf, sizeof(buf), "%s", names[index]);
		return strdup(buf);
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
	static Eina_Bool flag = EINA_FALSE;
	if (!flag) {
		elm_object_signal_emit(layout, "elm,state,hide,searchbar,animation", "elm");
		flag = EINA_TRUE;
	}
	else {
		elm_object_signal_emit(layout, "elm,state,show,searchbar,animation", "elm");
		flag = EINA_FALSE;
	}
	if (item!=NULL) {
		int index= (int)elm_object_item_data_get(item);
		fprintf(stdout, "selected text %s\n", names[index]);
	}
}

static char* _gl_text_get_title(void *data, Evas_Object *obj, const char *part)
{
	char buf[PATH_MAX];
	int index = ((int) data / 5) % 5;

	if (strcmp(part, "elm.text") == 0) {
		snprintf(buf, sizeof(buf), "%s", group[index]);
		return strdup(buf);
	}
	return NULL;
}

static void _genlist_scroll_edge_top_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_object_signal_emit(layout, "elm,state,show,searchbar,animation", "elm");
}

static void _create_genlist_group_title_style(void *data)
{
	Evas_Object *genlist;
	struct appdata *ad = (struct appdata *) data;

	itc.item_style = "default";
	itc.func.text_get = _gl_text_get;
	itc.func.content_get = NULL;
	itc.func.state_get = _gl_state_get;
	itc.func.del = _gl_del;

	itc2.item_style = "groupindex";
	itc2.func.text_get = _gl_text_get_title;

	genlist = elm_genlist_add(ad->nf);
	elm_genlist_select_mode_set(genlist, ELM_OBJECT_SELECT_MODE_ALWAYS);
	evas_object_smart_callback_add(genlist, "scroll,edge,top", _genlist_scroll_edge_top_cb, NULL);

	search_gl = genlist;

	genlist_item_add();
}

static int _search(void *data)
{
	Evas_Object *entry = data;
	const char* text;

	text = elm_object_text_get(entry);

	printf("\n[Search Bar] Changed Callback Called, Search Word : %s\n", text);
	genlist_clear();

	if (text != NULL && strlen(text) > 0)
		genlist_item_search(text);
	else
		genlist_item_add();

	return 0;
}

static void _changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	_search(obj);
}

static void _bg_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	elm_object_focus_set(data, EINA_TRUE);
}

static void _cancel_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *searchbar_layout = (Evas_Object *)data;
	Evas_Object *entry = elm_object_part_content_get(searchbar_layout, "elm.swallow.content");
	const char* text;
	text = elm_entry_entry_get(entry);
	if (text != NULL && strlen(text) > 0)
		elm_entry_entry_set(entry, NULL);
	elm_object_focus_set(search_gl, EINA_TRUE); /*Transfer the focus to some other object*/
}

static void _create_searchbar(void *data)
{
	Evas_Object *entry, *cancel_btn;
	Evas_Object *layout = data;

	searchbar_layout = elm_layout_add(layout);
	elm_layout_theme_set(searchbar_layout, "layout", "searchbar", "cancel_button");
	elm_object_part_content_set(layout, "searchbar", searchbar_layout);

	entry = ea_editfield_add(searchbar_layout, EA_EDITFIELD_SEARCHBAR);
	elm_object_signal_emit(entry, "elm,action,hide,search_icon", "");
	elm_object_domain_translatable_part_text_set(entry, "elm.guide", "sys_string", "IDS_COM_BODY_SEARCH");
	elm_object_part_content_set(searchbar_layout, "elm.swallow.content", entry);

	evas_object_smart_callback_add(entry, "changed", _changed_cb, NULL);
	elm_object_signal_callback_add(searchbar_layout, "elm,bg,clicked", "elm", _bg_clicked_cb, entry);

	elm_entry_input_panel_layout_set(entry, ELM_INPUT_PANEL_LAYOUT_NORMAL);
	evas_object_size_hint_weight_set(searchbar_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(searchbar_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	//in case cancel button is required add the following code
	cancel_btn = elm_button_add(searchbar_layout);
	elm_object_part_content_set(searchbar_layout, "button_cancel", cancel_btn);
	elm_object_style_set(cancel_btn, "searchbar/default");
	elm_object_domain_translatable_text_set(cancel_btn, "sys_string", "IDS_IDLE_BUTTON_CANCEL");
	evas_object_smart_callback_add(cancel_btn, "clicked", _cancel_clicked_cb, searchbar_layout);
	elm_object_signal_emit(searchbar_layout, "cancel,show", "");
}

void searchbar_no_search_icon_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	if(ad == NULL) return;

	_create_genlist_group_title_style(ad);

	layout = elm_layout_add(ad->nf);
	elm_layout_theme_set(layout, "layout", "application", "searchbar_base");
	elm_object_signal_emit(layout, "elm,state,show,searchbar", "elm");

	_create_searchbar(layout);

	elm_object_part_content_set(layout, "elm.swallow.content", search_gl);
	elm_naviframe_item_push(ad->nf, _("SearchBar"), NULL, NULL, layout, NULL);
}
