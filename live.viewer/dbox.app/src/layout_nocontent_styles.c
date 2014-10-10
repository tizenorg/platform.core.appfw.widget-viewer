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
#include "layout.h"

static Evas_Object *searchbar_layout;

static void _changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (elm_object_focus_get(data)) {
		if (elm_entry_is_empty(obj)) {
			elm_object_signal_emit(data, "elm,state,eraser,hide", "elm");
		}
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

static void _cancel_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	const char* text;
	evas_object_hide(obj);
	elm_object_signal_emit(searchbar_layout, "cancel,out", "");
	text = elm_entry_entry_get(data);
	if (text != NULL && strlen(text) > 0)
		elm_entry_entry_set(data, NULL);
	elm_object_focus_set(data, EINA_FALSE);
}

static void _searchsymbol_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	elm_object_focus_set(data, EINA_TRUE);
	printf("\n[Search Bar] SearchSymbol Callback Called\n");
}

static void _google_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	printf("google clicked\n");
}

static void _create_searchbar(void *data)
{
	Evas_Object *entry, *cancel_btn;
	Evas_Object *layout = data;

	searchbar_layout = elm_layout_add(layout);
	elm_layout_theme_set(searchbar_layout, "layout", "searchbar", "cancel_button");
	elm_object_part_content_set(layout, "searchbar", searchbar_layout);

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

	//in case cancel button is required add the following code
	cancel_btn = elm_button_add(searchbar_layout);
	elm_object_part_content_set(searchbar_layout, "button_cancel", cancel_btn);
	elm_object_style_set(cancel_btn, "searchbar/default");
	elm_object_text_set(cancel_btn, "Cancel");
	elm_object_focus_allow_set(cancel_btn, EINA_FALSE);
	elm_object_signal_emit(searchbar_layout, "cancel,show", "");
	evas_object_smart_callback_add(cancel_btn, "clicked", _cancel_clicked_cb, entry);

	elm_object_signal_callback_add(searchbar_layout, "elm,action,click", "", _searchsymbol_clicked_cb, entry);

	evas_object_show(searchbar_layout);

}

/* nocontents/search style of layout */
static void nocontents_search_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *layout, *lay;
	struct appdata *ad = (struct appdata *)data;
	if (ad == NULL) return;

	/* Create elm_layout with nocontents/search style */
	lay = elm_layout_add (ad->nf);
	elm_layout_theme_set(lay, "layout", "nocontents", "search");
	elm_object_part_text_set (lay, "elm.text", _("No results found"));

	/* Search(without google) view layout */
	layout = elm_layout_add (ad->nf);
	elm_layout_theme_set(layout, "layout", "application", "searchbar_base");

	_create_searchbar(layout);

	elm_object_signal_emit(layout, "elm,state,show,searchbar", "elm");

	elm_object_part_content_set(layout, "elm.swallow.content", lay);
	elm_naviframe_item_push(ad->nf, _("Nocontents search"), NULL, NULL,layout, NULL);
}

/* nocontents/search style of layout displaying a button*/
static void nocontents_google_search_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *layout, *lay, *custom_area, *btn, *icon;
	char buf[255] = {0,};
	struct appdata *ad = (struct appdata *)data;
	if (ad == NULL) return;

	_create_searchbar(ad);

	/* Create google button */
	custom_area = elm_layout_add (ad->nf);
	elm_layout_file_set (custom_area, ELM_DEMO_EDJ, "elmdemo-test/nocontents/search_google");
	evas_object_size_hint_weight_set (custom_area, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	btn = elm_button_add (ad->nf);
	elm_object_style_set (btn, "searchbar/default");
	icon = elm_image_add (ad->nf);
	snprintf (buf, sizeof(buf), "%s/30_SmartSearch_google_icon.png", ICON_DIR);
	elm_image_file_set (icon, buf, NULL);
	elm_image_resizable_set (icon, 1, 1);
	elm_object_content_set (btn, icon);
	elm_object_text_set (btn, _("Search by Google"));
	evas_object_smart_callback_add (btn, "clicked", _google_clicked_cb, ad);
	elm_object_part_content_set (custom_area, "buttons", btn);

	/* Create layout and set the theme as nocontents/search */
	lay = elm_layout_add (ad->nf);
	elm_layout_theme_set(lay, "layout", "nocontents", "search");
	elm_object_part_text_set (lay, "elm.text", _("No search result"));
	elm_object_part_content_set(lay, "custom", custom_area);

	/* Search(with google) view layout */
	layout = elm_layout_add(ad->nf);
	elm_layout_theme_set(layout, "layout", "application", "searchbar_base");
	elm_object_signal_emit(layout, "elm,state,show,searchbar", "elm");

	elm_object_part_content_set(layout, "searchbar", searchbar_layout);
	elm_object_part_content_set(layout, "elm.swallow.content", lay);
	elm_naviframe_item_push(ad->nf, _("Nocontents google search"), NULL, NULL, layout, NULL);
}

static Eina_Bool _access_action_cb(void *data, Evas_Object *obj,
				Elm_Access_Action_Info *action_info)
{
	if (!action_info) return EINA_FALSE;

	printf("action type: %d, action by: %d, x: %d, y: %d, mouse_type: %d\n",
			action_info->action_type, action_info->action_by,
			action_info->x, action_info->y, action_info->mouse_type);

	return EINA_TRUE;
}

static void _button_clicked_cb(void *data, Evas_Object * obj, void *event_info)
{
	printf("\n button clicked \n");
}

static void _no_content_layout_create(void* data, char *name, char *group, Eina_Bool helptext)
{
	Evas_Object *layout, *lay, *btn, *icon;
	char buf[128];
	struct appdata *ad = (struct appdata *)data;
	if (ad == NULL) return;

	/* Full view layout */
	layout = elm_layout_add (ad->nf);
	elm_layout_file_set (layout, ELM_DEMO_EDJ, group);
	evas_object_size_hint_weight_set (layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	/* Create elm_layout and set its style as nocontents/text */
	lay = elm_layout_add (layout);
	if (!lay) return;
	elm_layout_theme_set(lay, "layout", "nocontents", "default");
	evas_object_size_hint_weight_set (lay, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(lay, EVAS_HINT_FILL, EVAS_HINT_FILL);

	icon = elm_image_add(lay);
	snprintf(buf, sizeof(buf), "%s/00_nocontents_text.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	elm_object_part_content_set(lay, "nocontents.image", icon);

	snprintf(buf, sizeof(buf), "%s",_("No Items"));

	elm_object_part_text_set(lay, "elm.text", buf);
	elm_layout_signal_emit(lay, "text,disabled", "");

	if (helptext) {
		char buf1[4028];
		snprintf(buf1, sizeof(buf1), "%s",_("No Contents help text. This part is for hints and useful information for users. "
				"No Contents help text. This part is for hints and useful information for users."));
		elm_object_part_text_set(lay, "elm.help.text", buf1);
		elm_layout_signal_emit(lay, "align.center", "elm");

		btn = elm_button_add(lay);
		elm_object_style_set(btn, "style1");
		elm_object_text_set(btn, "Text button");
		evas_object_smart_callback_add(btn, "clicked", _button_clicked_cb, NULL);

		elm_object_part_content_set(lay, "swallow_area", btn);
	}

	elm_object_part_content_set (layout, "contents", lay);
	elm_naviframe_item_push(ad->nf, _(name), NULL, NULL,layout, NULL);

	Eina_List *l = NULL;
	Evas_Object *po, *ao;
	po = (Evas_Object *)edje_object_part_object_get
				(elm_layout_edje_get(layout), "access");

	/* Other toolkit (Dali, OSP, Web, etc.) would refer to the following lines to convey access action */
	ao = elm_access_object_register(po, layout);
	elm_access_info_set(ao, ELM_ACCESS_INFO, "screen reader test");
	l = eina_list_append(l, ao);
	elm_object_focus_custom_chain_set(layout, l);

	elm_access_action_cb_set(ao, ELM_ACCESS_ACTION_HIGHLIGHT,
							_access_action_cb, NULL);

	elm_access_action_cb_set(ao, ELM_ACCESS_ACTION_UNHIGHLIGHT,
							_access_action_cb, NULL);

	elm_access_action_cb_set(ao, ELM_ACCESS_ACTION_HIGHLIGHT_NEXT,
							_access_action_cb, NULL);

	elm_access_action_cb_set(ao, ELM_ACCESS_ACTION_HIGHLIGHT_PREV,
							_access_action_cb, NULL);

	elm_access_action_cb_set(ao, ELM_ACCESS_ACTION_ACTIVATE,
							_access_action_cb, NULL);

	elm_access_action_cb_set(ao, ELM_ACCESS_ACTION_UP,
							_access_action_cb, NULL);

	elm_access_action_cb_set(ao, ELM_ACCESS_ACTION_DOWN,
							_access_action_cb, NULL);

	elm_access_action_cb_set(ao, ELM_ACCESS_ACTION_READ,
							_access_action_cb, NULL);

	elm_access_action_cb_set(ao, ELM_ACCESS_ACTION_BACK,
							_access_action_cb, NULL);

	elm_access_action_cb_set(ao, ELM_ACCESS_ACTION_SCROLL,
							_access_action_cb, NULL);
}

static void nocontents_full_without_helptext_cb(void *data, Evas_Object *obj, void *event_info)
{
	_no_content_layout_create(data, "Nocontents full", "elmdemo-test/nocontents/full",
								EINA_FALSE);
}

static void nocontents_full_with_helptext_cb(void *data, Evas_Object *obj, void *event_info)
{
	_no_content_layout_create(data, "Nocontents helptext full", "elmdemo-test/nocontents_helptext/full",
								EINA_TRUE);
}

static void _list_click(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *it = (Elm_Object_Item *) elm_list_selected_item_get(obj);

	if (it == NULL)
	{
		fprintf((LOG_PRI(LOG_ERR) == LOG_ERR?stderr:stdout), "List item is NULL.\n");
		return;
	}

	elm_list_item_selected_set(it, EINA_FALSE);
}

void layout_nocontent_styles_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	if (ad == NULL) return;
	Evas_Object *list;

	list = elm_list_add(ad->nf);
	elm_list_mode_set(list, ELM_LIST_COMPRESS);
	evas_object_smart_callback_add(list, "selected", _list_click, NULL);

	elm_list_item_append(list, "Nocontents search", NULL, NULL, nocontents_search_cb, ad);
	elm_list_item_append(list, "Nocontents google search", NULL, NULL, nocontents_google_search_cb, ad);
	elm_list_item_append(list, "Nocontents full without Help Text", NULL, NULL, nocontents_full_without_helptext_cb, ad);
	elm_list_item_append(list, "Nocontents full with Help Text", NULL, NULL, nocontents_full_with_helptext_cb, ad);

	elm_list_go(list);
	elm_naviframe_item_push(ad->nf, _("Nocontent Styles"), NULL, NULL, list, NULL);
}
