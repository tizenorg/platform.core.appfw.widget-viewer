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
#include "naviframe.h"

/*********************************************************
  Naviframe
 ********************************************************/

static Evas_Object *layout_main = NULL;
static Evas_Object *layout_tab = NULL;
static Evas_Object *conform = NULL;

static Evas_Object *_create_nocontent(Evas_Object *parent, const char *text)
{
	Evas_Object *layout, *noc, *sc, *icon;
	char buf[128];

	//Scroller
	sc = elm_scroller_add(parent);
	if (!sc) return NULL;
	elm_scroller_bounce_set(sc, EINA_FALSE, EINA_TRUE);
	elm_scroller_policy_set(sc, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);

	//Base Layout
	layout = elm_layout_add(sc);
	if (!layout)
	{
		evas_object_del(sc);
		return NULL;
	}
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "elmdemo-test/nocontents/full");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	//NoContent Layout
	noc = elm_layout_add(layout);
	if (!noc)
	{
		evas_object_del(layout);
		evas_object_del(sc);
		return NULL;
	}
	elm_layout_theme_set(noc, "layout", "nocontents", "full");
	elm_object_part_text_set(noc, "elm.text", text);

	//Icon of NoContent Layout
	icon = elm_image_add(noc);
	if (!icon)
	{
		evas_object_del(noc);
		evas_object_del(layout);
		evas_object_del(sc);
		return NULL;
	}
	snprintf(buf, sizeof(buf), "%s/00_nocontents_text.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	elm_object_part_content_set(noc, "nocontents.image", icon);

	elm_object_part_content_set(layout, "contents", noc);
	elm_object_content_set(sc, layout);
	return sc;
}

static Evas_Object *_create_title_edit_btn(Evas_Object *parent, Evas_Smart_Cb func, void *data)
{
	Evas_Object *ic;
	char buf[PATH_MAX];
	Evas_Object *btn = elm_button_add(parent);
	if (!btn) return NULL;
	elm_object_style_set(btn, "naviframe/title_icon");
	ic = elm_image_add(parent);
	snprintf(buf, sizeof(buf), "%s/00_icon_edit.png", ICON_DIR);
	elm_image_file_set(ic, buf, NULL);
	evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_BOTH, 1, 1);
	elm_image_resizable_set(ic, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(btn, "icon", ic);
	evas_object_smart_callback_add(btn, "clicked", func, data);
	return btn;
}

static Evas_Object *_create_title_plus_btn(Evas_Object *parent, Evas_Smart_Cb func, void *data)
{
	Evas_Object *ic;
	char buf[PATH_MAX];
	Evas_Object *btn = elm_button_add(parent);
	if (!btn) return NULL;
	elm_object_style_set(btn, "naviframe/title_icon");
	ic = elm_image_add(parent);
	snprintf(buf, sizeof(buf), "%s/00_icon_plus.png", ICON_DIR);
	elm_image_file_set(ic, buf, NULL);
	evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_BOTH, 1, 1);
	elm_image_resizable_set(ic, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(btn, "icon", ic);
	evas_object_smart_callback_add(btn, "clicked", func, data);
	return btn;
}

static Evas_Object *_create_title_text_btn(Evas_Object *parent, const char *text, Evas_Smart_Cb func, void *data)
{
	Evas_Object *btn = elm_button_add(parent);
	if (!btn) return NULL;
	elm_object_style_set(btn, "naviframe/title_text");
	elm_object_text_set(btn, text);
	evas_object_smart_callback_add(btn, "clicked", func, data);
	return btn;
}

static Evas_Object *_create_drawers_btn(Evas_Object *parent, Evas_Smart_Cb func, void *data)
{
	Evas_Object *btn = elm_button_add(parent);
	if (!btn) return NULL;
	elm_object_style_set(btn, "naviframe/drawers");
	evas_object_smart_callback_add(btn, "clicked", func, data);
	return btn;
}

static Evas_Object *create_toolbar_more_btn(Evas_Object *parent, Evas_Smart_Cb func, void *data)
{
	Evas_Object *btn = elm_button_add(parent);
	if (!btn) return NULL;
	elm_object_style_set(btn, "naviframe/more/default");
	evas_object_smart_callback_add(btn, "clicked", func, data);
	return btn;
}

static void _quit_func(Evas_Object *nf)
{
	if (conform) {
		if (nf) {
			elm_object_content_unset(conform);
			evas_object_del(nf);
		}
		if (layout_main) {
			elm_object_content_set(conform, layout_main);
			evas_object_show(layout_main);
		}
		conform = NULL;
	}
}

static Eina_Bool _quit_cb(void *data, Elm_Object_Item *it)
{
	_quit_func(data);
	return EINA_FALSE;
}

static void _quit_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	_quit_func(data);
}

static void _prev_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *nf = (Evas_Object *) data;
	elm_naviframe_item_pop(nf);
	evas_object_smart_callback_del(obj, "clicked", _prev_cb);
}

static void _title_slide_stop(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *nf = (Evas_Object *) data;

	//Emit signal to start title text slide
	elm_object_signal_emit(nf, "elm,action,title,slide,stop", "");
}

static void _title_slide_start(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *nf = (Evas_Object *) data;

	//Emit signal to start title text slide
	elm_object_signal_emit(nf, "elm,action,title,slide,start", "");
}

static void _twelfth_page(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *content, *btn, *toolbar;
	Evas_Object *nf = (Evas_Object *) data;
	Elm_Object_Item *navi_it, *toolbar_it[2];

	if (nf == NULL) return;

	content = _create_nocontent(nf, "Naviframe Demo<br>Page 12");

	//Push a new item
	navi_it = elm_naviframe_item_push(nf, "This title slides to go. This title slides 3 times. Now you can read an over length text!", NULL, NULL, content, NULL);

	//Subtitle
	elm_object_item_part_text_set(navi_it, "subtitle", "This subtitle slides to go. This subtitle slides 3 times. Now you can read an over length text!");

	//Title Text Left Button
	btn = _create_title_text_btn(nf, "Prev", _prev_cb, nf);
	elm_object_item_part_content_set(navi_it, "title_left_btn", btn);

	//Title Text Right Button
	btn = _create_title_text_btn(nf, "Quit", _quit_button_cb, nf);
	elm_object_item_part_content_set(navi_it, "title_right_btn", btn);

	//2 Item Toolbar
	toolbar = elm_toolbar_add(nf);
	elm_object_style_set(toolbar, "default");
	elm_toolbar_shrink_mode_set(toolbar, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_transverse_expanded_set(toolbar, EINA_TRUE);
	elm_toolbar_select_mode_set(toolbar, ELM_OBJECT_SELECT_MODE_NONE);
	toolbar_it[0] = elm_toolbar_item_append(toolbar, NULL, "Title slide stop", _title_slide_stop, nf);
	toolbar_it[1] = elm_toolbar_item_append(toolbar, NULL, "Title slide start", _title_slide_start, nf);
	elm_object_item_part_content_set(navi_it, "toolbar", toolbar);
}

static Eina_Bool _eleventh_pop_cb(void *data, Elm_Object_Item *it)
{
	//Delete item without transition effect
	elm_object_item_del(it);

	/* Return EINA_FALSE not to pop the item in this case
		since the item is deleted explicitly. */
	return EINA_FALSE;
}

static void _eleventh_page(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *content, *btn, *toolbar;
	Evas_Object *nf = (Evas_Object *) data;
	Elm_Object_Item *top_it, *navi_it, *toolbar_it[2];

	if (nf == NULL) return;

	content = _create_nocontent(nf, "Naviframe Demo<br>Page 11");

	//Get the top item of naviframe
	top_it = elm_naviframe_top_item_get(nf);

	/* Insert a new item after the input item.
		elm_naviframe_item_insert_after does not show transition effect. */
	navi_it = elm_naviframe_item_insert_after(nf, top_it, "No Transition", NULL, NULL, content, NULL);

	//Register item pop callback function
	elm_naviframe_item_pop_cb_set(navi_it, _eleventh_pop_cb, NULL);

	//Title Icon Left Button
	btn = _create_title_edit_btn(nf, NULL, nf);
	elm_object_item_part_content_set(navi_it, "title_left_btn", btn);

	//Title Icon Right Button
	btn = _create_title_plus_btn(nf, NULL, nf);
	elm_object_item_part_content_set(navi_it, "title_right_btn", btn);

	//2 Item Toolbar
	toolbar = elm_toolbar_add(nf);
	elm_object_style_set(toolbar, "default");
	elm_toolbar_shrink_mode_set(toolbar, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_transverse_expanded_set(toolbar, EINA_TRUE);
	elm_toolbar_select_mode_set(toolbar, ELM_OBJECT_SELECT_MODE_NONE);
	toolbar_it[0] = elm_toolbar_item_append(toolbar, NULL, "Prev", _prev_cb, nf);
	toolbar_it[1] = elm_toolbar_item_append(toolbar, NULL, "Next", _twelfth_page, nf);
	elm_object_item_part_content_set(navi_it, "toolbar", toolbar);
}

static void _title_hide(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *navi_it = (Elm_Object_Item *) data;

	/* Disable naviframe title with title transition effect.
		The third argument of elm_naviframe_item_title_enabled_set
		sets the title transition effect. */
	elm_naviframe_item_title_enabled_set(navi_it, EINA_FALSE, EINA_TRUE);
}

static void _title_show(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *navi_it = (Elm_Object_Item *) data;

	/* Enable naviframe title with title transition effect
		The third argument of elm_naviframe_item_title_enabled_set
		sets the title transition effect. */
	elm_naviframe_item_title_enabled_set(navi_it, EINA_TRUE, EINA_TRUE);
}

static void _tenth_page(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *content, *btn, *toolbar;
	Evas_Object *nf = (Evas_Object *) data;
	Elm_Object_Item *navi_it, *toolbar_it[2];

	if (nf == NULL) return;

	content = _create_nocontent(nf, "Naviframe Demo<br>Page 10");

	//Push a new item
	navi_it = elm_naviframe_item_push(nf, "Title Show/Hide", NULL, NULL, content, NULL);

	//Title Text Left Button
	btn = _create_title_text_btn(nf, "Prev", _prev_cb, nf);
	elm_object_item_part_content_set(navi_it, "title_left_btn", btn);

	//Title Text Right Button
	btn = _create_title_text_btn(nf, "Next", _eleventh_page, nf);
	elm_object_item_part_content_set(navi_it, "title_right_btn", btn);

	//2 Item Toolbar
	toolbar = elm_toolbar_add(nf);
	elm_object_style_set(toolbar, "default");
	elm_toolbar_shrink_mode_set(toolbar, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_transverse_expanded_set(toolbar, EINA_TRUE);
	elm_toolbar_select_mode_set(toolbar, ELM_OBJECT_SELECT_MODE_NONE);
	toolbar_it[0] = elm_toolbar_item_append(toolbar, NULL, "Title Hide", _title_hide, navi_it);
	toolbar_it[1] = elm_toolbar_item_append(toolbar, NULL, "Title Show", _title_show, navi_it);
	elm_object_item_part_content_set(navi_it, "toolbar", toolbar);
}

static void _changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (elm_object_focus_get(data)) {
		if (elm_entry_is_empty(obj))
			elm_object_signal_emit(data, "elm,state,eraser,hide", "elm");
		else
			elm_object_signal_emit(data, "elm,state,eraser,show", "elm");
	}
	if (!elm_entry_is_empty(obj))
		elm_object_signal_emit(data, "elm,state,guidetext,hide", "elm");
}

static void _focused_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!elm_entry_is_empty(obj))
		elm_object_signal_emit(data, "elm,state,eraser,show", "elm");
	elm_object_signal_emit(data, "elm,state,guidetext,hide", "elm");
	elm_object_signal_emit(data, "cancel,in", "");
}

static void _unfocused_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (elm_entry_is_empty(obj))
		elm_object_signal_emit(data, "elm,state,guidetext,show", "elm");
	elm_object_signal_emit(data, "elm,state,eraser,hide", "elm");
}

static void _eraser_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	elm_entry_entry_set(data, "");
}

static void _bg_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	elm_object_focus_set(data, EINA_TRUE);
}

static void _searchsymbol_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	elm_object_focus_set(data, EINA_TRUE);
	printf("\n[Search Bar] SearchSymbol Callback Called\n");
}

static Evas_Object *_create_searchbar(Evas_Object *parent)
{
	Evas_Object *searchbar_layout, *entry;

	if (parent == NULL) return NULL;

	//Searchbar
	searchbar_layout = elm_layout_add(parent);
	elm_layout_theme_set(searchbar_layout, "layout", "searchbar", "default");

	//Entry
	entry = ea_editfield_add(searchbar_layout, EA_EDITFIELD_SEARCHBAR);
	elm_object_part_text_set(entry, "elm.guide", "Search");
	elm_object_part_content_set(searchbar_layout, "elm.swallow.content", entry);

	evas_object_smart_callback_add(entry, "changed", _changed_cb, searchbar_layout);
	evas_object_smart_callback_add(entry, "focused", _focused_cb, searchbar_layout);
	evas_object_smart_callback_add(entry, "unfocused", _unfocused_cb, searchbar_layout);
	elm_object_signal_callback_add(searchbar_layout, "elm,bg,clicked", "elm", _bg_clicked_cb, entry);
	elm_object_signal_callback_add(searchbar_layout, "elm,eraser,clicked", "elm", _eraser_clicked_cb, entry);

	elm_entry_input_panel_layout_set(entry, ELM_INPUT_PANEL_LAYOUT_NORMAL);
	evas_object_size_hint_weight_set(searchbar_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(searchbar_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	elm_object_signal_callback_add(searchbar_layout, "elm,action,click", "", _searchsymbol_clicked_cb, entry);

	evas_object_show(searchbar_layout);
	return searchbar_layout;
}

static void _nineth_trans_finished_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *searchbar_layout = (Evas_Object *) data;

	//Unregister smart callback function which is called after naviframe transition is finished
	evas_object_smart_callback_del(obj, "transition,finished", _nineth_trans_finished_cb);

	//Enable focus set to entry
	elm_object_tree_focus_allow_set(searchbar_layout, EINA_TRUE);

	//Set focus to entry to show keypad
	elm_object_focus_set(searchbar_layout, EINA_TRUE);
}

static void _nineth_page(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *searchbar_layout, *content, *toolbar;
	Evas_Object *nf = (Evas_Object *) data;
	Elm_Object_Item *navi_it, *toolbar_it[2];

	if (nf == NULL) return;

	content = _create_nocontent(nf, "Naviframe Demo<br>Page 9");

	//Searchbar
	searchbar_layout = _create_searchbar(nf);

	//Register smart callback function which is called after naviframe transition is finished
	evas_object_smart_callback_add(nf, "transition,finished", _nineth_trans_finished_cb, searchbar_layout);

	//Disable focus set to entry
	elm_object_tree_focus_allow_set(searchbar_layout, EINA_FALSE);

	//Push a new item
	navi_it = elm_naviframe_item_push(nf, NULL, NULL, NULL, content, "empty");

	//Set Searchbar to "title" part in "empty" item style
	elm_object_item_part_content_set(navi_it, "title", searchbar_layout);

	//2 Item Toolbar
	toolbar = elm_toolbar_add(nf);
	elm_object_style_set(toolbar, "default");
	elm_toolbar_shrink_mode_set(toolbar, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_transverse_expanded_set(toolbar, EINA_TRUE);
	elm_toolbar_select_mode_set(toolbar, ELM_OBJECT_SELECT_MODE_NONE);
	toolbar_it[0] = elm_toolbar_item_append(toolbar, NULL, "Prev", _prev_cb, nf);
	toolbar_it[1] = elm_toolbar_item_append(toolbar, NULL, "Next", _tenth_page, nf);
	elm_object_item_part_content_set(navi_it, "toolbar", toolbar);
}

static void _view_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *sub_content = NULL;

	switch ((int)data)
	{
		case 1:
			sub_content = _create_nocontent(layout_tab, "Naviframe Demo<br>Page 8-1");
			break;
		case 2:
			sub_content = _create_nocontent(layout_tab, "Naviframe Demo<br>Page 8-2");
			break;
		case 3:
			sub_content = _create_nocontent(layout_tab, "Naviframe Demo<br>Page 8-3");
			break;
		case 4:
			sub_content = _create_nocontent(layout_tab, "Naviframe Demo<br>Page 8-4");
			break;
		case 5:
			sub_content = _create_nocontent(layout_tab, "Naviframe Demo<br>Page 8-5");
			break;
	}
	//Previous content is deleted automatically
	elm_object_part_content_set(layout_tab, "elm.swallow.content", sub_content);
}

static void _eighth_page(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *sub_content, *tabbar, *toolbar;
	Evas_Object *nf = (Evas_Object *) data;
	Elm_Object_Item *navi_it, *toolbar_it[2];
	Elm_Object_Item *tabbar_it;

	if (nf == NULL) return;

	layout_tab = elm_layout_add(nf);
	elm_layout_theme_set(layout_tab, "layout", "application", "default");
	sub_content = _create_nocontent(layout_tab, "Naviframe Demo<br>Page 8-1");
	elm_object_part_content_set(layout_tab, "elm.swallow.content", sub_content);

	//Push a new item
	navi_it = elm_naviframe_item_push(nf, NULL, NULL, NULL, layout_tab, "tabbar/notitle");

	//Tabbar
	tabbar = elm_toolbar_add(nf);
	if (tabbar) {
		elm_object_style_set(tabbar, "tabbar");
		elm_toolbar_shrink_mode_set(tabbar, ELM_TOOLBAR_SHRINK_EXPAND);
		elm_toolbar_reorder_mode_set(tabbar, EINA_TRUE);
		elm_toolbar_transverse_expanded_set(tabbar, EINA_TRUE);
		elm_toolbar_select_mode_set(tabbar, ELM_OBJECT_SELECT_MODE_ALWAYS);
		tabbar_it = elm_toolbar_item_append(tabbar, ICON_DIR"/00_controlbar_icon_favorites.png", "Main", _view_changed_cb, (void *)1);
		elm_toolbar_item_append(tabbar, ICON_DIR"/00_controlbar_icon_playlist.png", "Playlist", _view_changed_cb, (void *)2);
		elm_toolbar_item_append(tabbar, ICON_DIR"/00_controlbar_icon_artists.png", "Artists list", _view_changed_cb, (void *)3);
		elm_toolbar_item_append(tabbar, ICON_DIR"/00_controlbar_icon_songs.png", "Songs", _view_changed_cb, (void *)4);
		elm_toolbar_item_append(tabbar, ICON_DIR"/00_controlbar_icon_dialer.png", "Dialer for call", _view_changed_cb, (void *)5);

		elm_toolbar_item_selected_set(tabbar_it, EINA_TRUE);
		elm_object_item_part_content_set(navi_it, "tabbar", tabbar);
	}

	//2 Item Toolbar
	toolbar = elm_toolbar_add(nf);
	elm_object_style_set(toolbar, "default");
	elm_toolbar_shrink_mode_set(toolbar, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_transverse_expanded_set(toolbar, EINA_TRUE);
	elm_toolbar_select_mode_set(toolbar, ELM_OBJECT_SELECT_MODE_NONE);
	toolbar_it[0] = elm_toolbar_item_append(toolbar, NULL, "Prev", _prev_cb, nf);
	toolbar_it[1] = elm_toolbar_item_append(toolbar, NULL, "Next", _nineth_page, nf);
	elm_object_item_part_content_set(navi_it, "toolbar", toolbar);
}

static void _seventh_view_2_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *content;
	Evas_Object *nf = (Evas_Object *) data;

	content = _create_nocontent(nf, "Naviframe Demo<br>Page 7-2");

	//Previous content is deleted automatically
	elm_object_part_content_set(nf, NULL, content);
}

static void _seventh_view_1_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *content;
	Evas_Object *nf = (Evas_Object *) data;

	content = _create_nocontent(nf, "Naviframe Demo<br>Page 7-1");

	//Previous content is deleted automatically
	elm_object_part_content_set(nf, NULL, content);
}

static void _seventh_page(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *content, *tabbar, *toolbar, *btn;
	Evas_Object *nf = (Evas_Object *) data;
	Elm_Object_Item *navi_it, *tabbar_it, *toolbar_it[3];

	if (nf == NULL) return;

	content = _create_nocontent(nf, "Naviframe Demo<br>Page 7-1");

	//Push a new item
	navi_it = elm_naviframe_item_push(nf, "Tabbar", NULL, NULL, content, "tabbar");

	//Tabbar
	tabbar = elm_toolbar_add(nf);
	elm_toolbar_shrink_mode_set(tabbar, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_transverse_expanded_set(tabbar, EINA_TRUE);
	elm_toolbar_select_mode_set(tabbar, ELM_OBJECT_SELECT_MODE_ALWAYS);
	elm_object_style_set(tabbar, "tabbar/item_with_title");
	if (tabbar) {
		tabbar_it = elm_toolbar_item_append(tabbar, NULL, "View 1", _seventh_view_1_cb, nf);
		elm_toolbar_item_append(tabbar, NULL, "View 2", _seventh_view_2_cb, nf);

		elm_toolbar_item_selected_set(tabbar_it, EINA_TRUE);
		elm_object_item_part_content_set(navi_it, "tabbar", tabbar);
	}

	//Title Icon Left Button
	btn = _create_title_edit_btn(nf, NULL, nf);
	elm_object_item_part_content_set(navi_it, "title_left_btn", btn);

	//Title Icon Right Button
	btn = _create_title_plus_btn(nf, NULL, nf);
	elm_object_item_part_content_set(navi_it, "title_right_btn", btn);

	//3 Item Toolbar
	toolbar = elm_toolbar_add(nf);
	elm_object_style_set(toolbar, "default");
	elm_toolbar_shrink_mode_set(toolbar, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_transverse_expanded_set(toolbar, EINA_TRUE);
	elm_toolbar_select_mode_set(toolbar, ELM_OBJECT_SELECT_MODE_NONE);
	toolbar_it[0] = elm_toolbar_item_append(toolbar, NULL, "Prev", _prev_cb, nf);
	toolbar_it[1] = elm_toolbar_item_append(toolbar, NULL, "None", NULL, nf);
	toolbar_it[2] = elm_toolbar_item_append(toolbar, NULL, "Next", _eighth_page, nf);
	elm_object_item_part_content_set(navi_it, "toolbar", toolbar);
}

static void _go_check_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	Eina_Bool state = elm_check_state_get(obj);

	printf("Check state : %d\n", state);
}

static void _sixth_page(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *content, *check, *toolbar;
	Evas_Object *nf = (Evas_Object *) data;
	Elm_Object_Item *navi_it, *toolbar_it[2];

	if (nf == NULL) return;

	content = _create_nocontent(nf, "Naviframe Demo<br>Page 6");

	//Push a new item
	navi_it = elm_naviframe_item_push(nf, "Title on&off Button", NULL, NULL, content, NULL);

	// Title on&off Button
	check = elm_check_add(nf);
	elm_check_state_set(check, EINA_FALSE);
	elm_object_style_set(check, "naviframe/title_on&off");
	elm_object_item_part_content_set(navi_it, "title_right_btn", check);
	evas_object_smart_callback_add(check, "changed", _go_check_clicked_cb, NULL);

	//2 Item Toolbar
	toolbar = elm_toolbar_add(nf);
	elm_object_style_set(toolbar, "default");
	elm_toolbar_shrink_mode_set(toolbar, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_transverse_expanded_set(toolbar, EINA_TRUE);
	elm_toolbar_select_mode_set(toolbar, ELM_OBJECT_SELECT_MODE_NONE);
	toolbar_it[0] = elm_toolbar_item_append(toolbar, NULL, "Prev", _prev_cb, nf);
	toolbar_it[1] = elm_toolbar_item_append(toolbar, NULL, "Next", _seventh_page, nf);
	elm_object_item_part_content_set(navi_it, "toolbar", toolbar);
}

static void _fifth_page(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *content, *btn, *toolbar;
	Evas_Object *nf = (Evas_Object *) data;
	Elm_Object_Item *navi_it, *toolbar_it[2];

	if (nf == NULL) return;

	content = _create_nocontent(nf, "Naviframe Demo<br>Page 5");

	//Push a new item
	navi_it = elm_naviframe_item_push(nf, "Drawers Button", NULL, NULL, content, "drawers");

	//Subtitle
	elm_object_item_part_text_set(navi_it, "subtitle", "Subtitle Text");

	//Drawers Button
	btn = _create_drawers_btn(nf, _sixth_page, nf);
	elm_object_item_part_content_set(navi_it, "drawers", btn);

	//Title Icon Left Button
	btn = _create_title_edit_btn(nf, NULL, nf);
	elm_object_item_part_content_set(navi_it, "title_left_btn", btn);

	//Title Icon Right Button
	btn = _create_title_plus_btn(nf, NULL, nf);
	elm_object_item_part_content_set(navi_it, "title_right_btn", btn);

	//2 Item Toolbar
	toolbar = elm_toolbar_add(nf);
	elm_object_style_set(toolbar, "default");
	elm_toolbar_shrink_mode_set(toolbar, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_transverse_expanded_set(toolbar, EINA_TRUE);
	elm_toolbar_select_mode_set(toolbar, ELM_OBJECT_SELECT_MODE_NONE);
	toolbar_it[0] = elm_toolbar_item_append(toolbar, NULL, "Prev", _prev_cb, nf);
	toolbar_it[1] = elm_toolbar_item_append(toolbar, NULL, "Next", _sixth_page, nf);
	elm_object_item_part_content_set(navi_it, "toolbar", toolbar);
}

static void _fourth_page(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *content, *btn, *toolbar;
	Evas_Object *nf = (Evas_Object *) data;
	Elm_Object_Item *navi_it, *toolbar_it[2];

	if (nf == NULL) return;

	content = _create_nocontent(nf, "Naviframe Demo<br>Page 4");

	//Push a new item
	navi_it = elm_naviframe_item_push(nf, "Title Badge", NULL, NULL, content, NULL);

	//Subtitle
	elm_object_item_part_text_set(navi_it, "subtitle", "Subtitle Text");

	//Title Badge
	elm_object_item_part_text_set(navi_it, "title_badge", "999");

	//Title Icon Left Button
	btn = _create_title_edit_btn(nf, NULL, nf);
	elm_object_item_part_content_set(navi_it, "title_left_btn", btn);

	//Title Icon Right Button
	btn = _create_title_plus_btn(nf, NULL, nf);
	elm_object_item_part_content_set(navi_it, "title_right_btn", btn);

	//2 Item Toolbar
	toolbar = elm_toolbar_add(nf);
	elm_object_style_set(toolbar, "default");
	elm_toolbar_shrink_mode_set(toolbar, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_transverse_expanded_set(toolbar, EINA_TRUE);
	elm_toolbar_select_mode_set(toolbar, ELM_OBJECT_SELECT_MODE_NONE);
	toolbar_it[0] = elm_toolbar_item_append(toolbar, NULL, "Prev", _prev_cb, nf);
	toolbar_it[1] = elm_toolbar_item_append(toolbar, NULL, "Next", _fifth_page, nf);
	elm_object_item_part_content_set(navi_it, "toolbar", toolbar);
}

static void _third_page(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *content, *btn, *toolbar;
	Evas_Object *ic;
	Evas_Object *nf = (Evas_Object *) data;
	Elm_Object_Item *navi_it, *toolbar_it[2];
	char buf[PATH_MAX] = {0,};

	if (nf == NULL) return;

	content = _create_nocontent(nf, "Naviframe Demo<br>Page 3");

	//Push a new item
	navi_it = elm_naviframe_item_push(nf, "Title Icon", NULL, NULL, content, NULL);

	//Subtitle
	elm_object_item_part_text_set(navi_it, "subtitle", "Subtitle Text");

	//Title Icon
	ic = elm_image_add(nf);
	snprintf(buf, sizeof(buf), "%s/g1.png", ICON_DIR);
	elm_image_file_set(ic, buf, NULL);
	evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(ic, EINA_TRUE, EINA_TRUE);
	elm_object_item_part_content_set(navi_it, "icon", ic);

	//Title Icon Left Button
	btn = _create_title_edit_btn(nf, NULL, nf);
	elm_object_item_part_content_set(navi_it, "title_left_btn", btn);

	//Title Icon Right Button
	btn = _create_title_plus_btn(nf, NULL, nf);
	elm_object_item_part_content_set(navi_it, "title_right_btn", btn);

	//2 Item Toolbar
	toolbar = elm_toolbar_add(nf);
	elm_object_style_set(toolbar, "default");
	elm_toolbar_shrink_mode_set(toolbar, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_transverse_expanded_set(toolbar, EINA_TRUE);
	elm_toolbar_select_mode_set(toolbar, ELM_OBJECT_SELECT_MODE_NONE);
	toolbar_it[0] = elm_toolbar_item_append(toolbar, NULL, "Prev", _prev_cb, nf);
	toolbar_it[1] = elm_toolbar_item_append(toolbar, NULL, "Next", _fourth_page, nf);
	elm_object_item_part_content_set(navi_it, "toolbar", toolbar);
}

static void _second_page(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *content, *btn, *toolbar;
	Evas_Object *nf = (Evas_Object *) data;
	Elm_Object_Item *navi_it, *toolbar_it[2];

	if (nf == NULL) return;

	content = _create_nocontent(nf, "Naviframe Demo<br>Page 2");

	//Push a new item
	navi_it = elm_naviframe_item_push(nf, "Title Text Button", NULL, NULL, content, NULL);

	//Title Text Left Button
	btn = _create_title_text_btn(nf, "Prev", _prev_cb, nf);
	elm_object_item_part_content_set(navi_it, "title_left_btn", btn);

	//Title Text Right Button
	btn = _create_title_text_btn(nf, "Next", _third_page, nf);
	elm_object_item_part_content_set(navi_it, "title_right_btn", btn);

	//2 Item Toolbar
	toolbar = elm_toolbar_add(nf);
	elm_object_style_set(toolbar, "default");
	elm_toolbar_shrink_mode_set(toolbar, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_transverse_expanded_set(toolbar, EINA_TRUE);
	elm_toolbar_select_mode_set(toolbar, ELM_OBJECT_SELECT_MODE_NONE);
	toolbar_it[0] = elm_toolbar_item_append(toolbar, NULL, "Prev", _prev_cb, nf);
	toolbar_it[1] = elm_toolbar_item_append(toolbar, NULL, "Next", _third_page, nf);
	elm_object_item_part_content_set(navi_it, "toolbar", toolbar);
}

static void _first_page(Evas_Object *nf)
{
	Evas_Object *content, *btn, *toolbar;
	Elm_Object_Item *navi_it;

	if (nf == NULL) return;

	content = _create_nocontent(nf, "Naviframe Demo<br>Page 1");

	//Push a new item
	navi_it = elm_naviframe_item_push(nf, "Title Text", NULL, NULL, content, NULL);

	//Register item pop callback function
	elm_naviframe_item_pop_cb_set(navi_it, _quit_cb, nf);

	//Title Icon Right Button
	btn = _create_title_plus_btn(nf, _second_page, nf);
	elm_object_item_part_content_set(navi_it, "title_right_btn", btn);

	//HW More Button
	btn = create_toolbar_more_btn(nf, _second_page, navi_it);
	elm_object_item_part_content_set(navi_it, "toolbar_more_btn", btn);

	//1 Item Toolbar
	toolbar = elm_toolbar_add(nf);
	elm_object_style_set(toolbar, "default");
	elm_toolbar_shrink_mode_set(toolbar, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_transverse_expanded_set(toolbar, EINA_TRUE);
	elm_toolbar_select_mode_set(toolbar, ELM_OBJECT_SELECT_MODE_NONE);
	elm_toolbar_item_append(toolbar, NULL, "Next", _second_page, nf);
	elm_object_item_part_content_set(navi_it, "toolbar", toolbar);
}

void naviframe_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad;
	Evas_Object *nf;

	ad = (struct appdata *) data;
	if (ad == NULL) return;

	layout_main = ad->layout_main;
	elm_object_content_unset(ad->conform);
	evas_object_hide(ad->layout_main);

	nf = elm_naviframe_add(ad->conform);
	elm_object_content_set(ad->conform, nf);
	evas_object_show(nf);

	//Register HW back button and HW more button for naviframe
	ea_object_event_callback_add(nf, EA_CALLBACK_BACK, ea_naviframe_back_cb, NULL);
	ea_object_event_callback_add(nf, EA_CALLBACK_MORE, ea_naviframe_more_cb, NULL);

	conform = ad->conform;
	_first_page(nf);
}
