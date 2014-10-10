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
#include "entry.h"
#include <Ecore_X.h>


//for change text size
static Evas_Object *tsobj;
static Evas_Object *txtobj1;
static Evas_Object *txtobj2;
//end for change text size

static void
_input_panel_event_callback(void *data, Ecore_IMF_Context *imf_context,
			    int value)
{
	int x, y, w, h;
	switch (value) {
	case ECORE_IMF_INPUT_PANEL_STATE_SHOW:
		// ISE state has changed to ISE_STATE_SHOW status
		// Get ISE position of current active ISE
		ecore_imf_context_input_panel_geometry_get(imf_context, &x, &y,
							   &w, &h);
		printf("keypad is shown\n");
		printf
		    ("The coordination of input panel. x : %d, y : %d, w : %d, h : %d\n",
		     x, y, w, h);
		break;
	case ECORE_IMF_INPUT_PANEL_STATE_HIDE:
		// ISE state has changed to ISE_STATE_HIDE status
		printf("keypad is hided\n");
		break;
	}
}

static void _sample_press_cb(void *data, Evas_Object *obj, void *event_info)
{
	;
}

static void _show_title_toolbar(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *navi_item = elm_naviframe_top_item_get(data);
	elm_object_item_signal_emit(navi_item, "elm,state,sip,shown", "");
}

static void _hide_title_toolbar(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *navi_item = elm_naviframe_top_item_get(data);
	elm_object_item_signal_emit(navi_item, "elm,state,sip,hidden", "");
}

/*
static void callback(void *data, Evas_Object *obj, void *event_info)
{
	printf("hurray I am called\n");
}
*/
Evas_Object *to_del;
static void response_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (to_del) {
		evas_object_del(to_del);
		to_del = NULL;
	}
	evas_object_del(obj);
}

void maxlength_reached(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup;
	struct appdata *ad;
	if (to_del) return;

	ad = (struct appdata *)data;

	popup = elm_popup_add(ad->win_main);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	elm_object_text_set(popup, "maximum length reached");
	elm_popup_timeout_set(popup, 3.0);
	evas_object_smart_callback_add(popup, "block,clicked", response_cb,
		NULL);
	evas_object_smart_callback_add(popup, "timeout", response_cb, NULL);
	evas_object_show(popup);
	to_del = popup;
}

static void password_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	Evas_Object *en;
	Evas_Object *lay;

	if (ad == NULL) return;

	lay = elm_layout_add(ad->nf);
	evas_object_size_hint_weight_set(lay, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	elm_layout_file_set(lay, ELM_DEMO_EDJ, "entry_test");
	elm_naviframe_item_push(ad->nf, _("password"), NULL, NULL,lay, NULL);

	en = elm_entry_add(ad->nf);
	ea_entry_selection_back_event_allow_set(en, EINA_TRUE);
	evas_object_smart_callback_add(en, "changed", _sample_press_cb, NULL);
	elm_entry_single_line_set(en, EINA_TRUE);
	elm_entry_password_set(en, EINA_TRUE);
	elm_object_part_content_set(lay, "entry_part", en);
	elm_entry_entry_set(en, "pass");
	elm_entry_cursor_end_set(en);
	Ecore_IMF_Context *imf_context = elm_entry_imf_context_get(en);
	ecore_imf_context_input_panel_event_callback_add(imf_context,
							 ECORE_IMF_INPUT_PANEL_STATE_EVENT,
							 _input_panel_event_callback,
							 NULL);
}

static void entry_multiline(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	Evas_Object *en;
	Evas_Object *lay;
	static Elm_Entry_Filter_Limit_Size limit_filter_data;

	if (ad == NULL) return;

	lay = elm_layout_add(ad->nf);
	evas_object_size_hint_weight_set(lay, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	elm_layout_file_set(lay, ELM_DEMO_EDJ, "entry_test_multi");
	elm_naviframe_item_push(ad->nf, _("entry_multiline"), NULL, NULL, lay, NULL);

	en = elm_entry_add(ad->nf);
	ea_entry_selection_back_event_allow_set(en, EINA_TRUE);
	Ecore_IMF_Context *imf_context = elm_entry_imf_context_get(en);
	ecore_imf_context_input_panel_event_callback_add(imf_context,
							 ECORE_IMF_INPUT_PANEL_STATE_EVENT,
							 _input_panel_event_callback,
							 NULL);

	evas_object_smart_callback_add(en, "changed", _sample_press_cb, NULL);
	elm_entry_cursor_end_set(en);
	limit_filter_data.max_byte_count = 0;
	limit_filter_data.max_char_count = 200;
	elm_entry_markup_filter_append(en, elm_entry_filter_limit_size,
				     &limit_filter_data);
	elm_entry_entry_set(en,
			    "This is a Multiline entry, you can input many lines");
	elm_entry_cursor_end_set(en);
	evas_object_smart_callback_add(en, "maxlength,reached",
				       maxlength_reached, ad);
	elm_object_part_content_set(lay, "entry_part", en);

	/*
	 * Sample Code for custom menu using elm_entry_context_menu_item_add()
	 elm_entry_context_menu_item_add(en, "Menu",NULL, ELM_ICON_NONE, callback, NULL);
	 */
}

static Evas_Object *_create_scroller(Evas_Object *parent)
{
	Evas_Object *scroller = elm_scroller_add(parent);
	elm_scroller_bounce_set(scroller, EINA_FALSE, EINA_TRUE);
	elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF,
				ELM_SCROLLER_POLICY_AUTO);
	evas_object_show(scroller);

	return scroller;
}

static void entry_copy_paste(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	Evas_Object *en;
	Evas_Object *lay;
	Evas_Object *scroller_main;

	if (ad == NULL) return;

	scroller_main = _create_scroller(ad->nf);
	elm_naviframe_item_push(ad->nf, _("Copy & Paste"), NULL, NULL, scroller_main, NULL);

	lay = elm_layout_add(ad->nf);
	evas_object_size_hint_weight_set(lay, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	elm_layout_file_set(lay, ELM_DEMO_EDJ, "entry_test");
	elm_object_content_set(scroller_main, lay);

	en = elm_entry_add(ad->nf);
	ea_entry_selection_back_event_allow_set(en, EINA_TRUE);
	Ecore_IMF_Context *imf_context = elm_entry_imf_context_get(en);
	ecore_imf_context_input_panel_event_callback_add(imf_context,
							 ECORE_IMF_INPUT_PANEL_STATE_EVENT,
							 _input_panel_event_callback,
							 NULL);

	evas_object_smart_callback_add(en, "changed", _sample_press_cb, NULL);
	elm_entry_cursor_end_set(en);

	elm_entry_entry_set(en,
			    "<b>What is Enlightenment?</b><br><br>"
			    "Enlightenment is not just a window manager for Linux/X11 and others, but also a whole suite of libraries to help you create beautiful user interfaces with much less work than doing it the old fashioned way and fighting with traditional toolkits, not to mention a traditional window manager. It covers uses from small mobile devices like phones all the way to powerful multi-core desktops (which are the primary development environment)."
			    "<br><br>"
			    "<b>Enlightenment Foundation Libraries</b><br><br>"
			    "These provide both a semi-traditional toolkit set in Elementary as well as the object canvas (Evas) and powerful abstracted objects (Edje) that you can combine, mix and match, even layer on top of each other with alpha channels and events in-tact. It has 3D transformations for all objects and more."
			    "A simple overview of the EFL (Enlightenment Foundation Libraries) stack is here. There is more to this, but this gives a quick overview of where it fits in."
			    "<br><br>"
			    "<b>Enlightenment</b><br><br>"
			    "Enlightenment is the flagship and original name bearer for this project. Once it was just a humble window manager for X11 that wanted to do things differently. To do them better, but it has expanded. This can be confusing so when we refer to Enlightenment, we may mean the project as a whole or just the window manager proper. The libraries behind Enlightenment are referred to as EFL collectively, each with a specific name and purpose.");

	elm_entry_cursor_end_set(en);
	elm_object_part_content_set(lay, "entry_part", en);
}

static Evas_Object *_create_autocapital_entry(Evas_Object *parent, const char *label, Elm_Autocapital_Type autocap)
{
	Evas_Object *ly;
	Evas_Object *en;
	Ecore_IMF_Context *imf_context;

	ly = elm_layout_add(parent);
	elm_layout_theme_set(ly, "layout", "editfield", "title");
	evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, 0);
	evas_object_size_hint_align_set(ly, EVAS_HINT_FILL, 0);

	en = elm_entry_add(parent);
	ea_entry_selection_back_event_allow_set(en, EINA_TRUE);
	elm_object_part_content_set(ly, "elm.swallow.content", en);

	elm_object_part_text_set(ly, "elm.text", label);
	elm_entry_autocapital_type_set (en, autocap);
	evas_object_show(ly);

	imf_context = elm_entry_imf_context_get(en);
	ecore_imf_context_input_panel_event_callback_add(imf_context,
							 ECORE_IMF_INPUT_PANEL_STATE_EVENT,
							 _input_panel_event_callback,
							 NULL);

	return ly;
}

static void entry_autocapital(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	Evas_Object *en;
	Evas_Object *scroller_main;
	Evas_Object *bx;

	if (ad == NULL) return;

	scroller_main = _create_scroller(ad->nf);
	elm_naviframe_item_push(ad->nf, _("Autocapital"), NULL, NULL, scroller_main, NULL);

	bx = elm_box_add(ad->nf);
	evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, 0);
	evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, 0);

	en = _create_autocapital_entry(bx, "None", ELM_AUTOCAPITAL_TYPE_NONE);
	elm_box_pack_end(bx, en);

	en = _create_autocapital_entry(bx, "Word", ELM_AUTOCAPITAL_TYPE_WORD);
	elm_box_pack_end(bx, en);

	en = _create_autocapital_entry(bx, "Sentence", ELM_AUTOCAPITAL_TYPE_SENTENCE);
	elm_box_pack_end(bx, en);

	en = _create_autocapital_entry(bx, "Allcharacter", ELM_AUTOCAPITAL_TYPE_ALLCHARACTER);
	elm_box_pack_end(bx, en);

	elm_object_content_set(scroller_main, bx);
}

static void _entry_enter_click(void *data, Evas_Object *obj, void *event_info)
{
	printf("enter key clicked!!\n");
}

static void single_line_scrolled_entry(void *data, Evas_Object *obj,
				       void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	Evas_Object *en;
	Evas_Object *lay;
	static Elm_Entry_Filter_Limit_Size limit_filter_data;

	if (ad == NULL) return;

	lay = elm_layout_add(ad->nf);
	evas_object_size_hint_weight_set(lay, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	elm_layout_file_set(lay, ELM_DEMO_EDJ, "entry_test");

	elm_naviframe_item_push(ad->nf, _("Scrolled_entry"), NULL, NULL, lay, NULL);

	en = elm_entry_add(ad->nf);
	ea_entry_selection_back_event_allow_set(en, EINA_TRUE);
	elm_entry_scrollable_set(en, EINA_TRUE);
	elm_entry_select_all(en);
	elm_scroller_policy_set(en, ELM_SCROLLER_POLICY_OFF,
						ELM_SCROLLER_POLICY_AUTO);
	evas_object_smart_callback_add(en, "maxlength,reached",
				       maxlength_reached, ad);

	Ecore_IMF_Context *imf_context = elm_entry_imf_context_get(en);
	ecore_imf_context_input_panel_event_callback_add(imf_context,
							 ECORE_IMF_INPUT_PANEL_STATE_EVENT,
							 _input_panel_event_callback,
							 NULL);

	elm_entry_single_line_set(en, EINA_TRUE);
	limit_filter_data.max_char_count = 0;
	limit_filter_data.max_byte_count = 50;
	elm_entry_markup_filter_append(en, elm_entry_filter_limit_size,
					      &limit_filter_data);
	elm_entry_entry_set(en, "Text");
	elm_entry_cursor_end_set(en);
	evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(en, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(en);

	evas_object_smart_callback_add(en, "activated", _entry_enter_click,
				       NULL);

	elm_object_part_content_set(lay, "entry_part", en);
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


static void _btn_change_text_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	printf("%s %d: Get in\n", __func__, __LINE__);
	const char *txt;
	txt = elm_entry_entry_get(tsobj);
	char style[256];
	sprintf(style, "DEFAULT='font_size=%s'", txt);
	//text object 1
	elm_entry_text_style_user_pop(txtobj1);
	elm_entry_text_style_user_push(txtobj1, style);

	//text object 2
	elm_entry_text_style_user_pop(txtobj2);
	elm_entry_text_style_user_push(txtobj2, style);

}

static void _entry_text_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	printf("%s %d: Get in\n", __func__, __LINE__);
	const char *t1;
	t1 = elm_entry_entry_get(txtobj1);
	elm_entry_entry_set(txtobj2, t1);
}

static void entry_change_text_size(void *data, Evas_Object *obj,
				       void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	Evas_Object *layout;
	Evas_Object *lbl;
	Evas_Object *ts;
	Evas_Object *btn;
	Evas_Object *en1, *en2;

	if (ad == NULL) return;

	//layout
	layout = elm_layout_add(ad->nf);
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "entry_test_change_text_size");
	evas_object_show(layout);

	elm_naviframe_item_push(ad->nf, _("Change Text Size"), NULL, NULL, layout, NULL);

	//label
	lbl = elm_label_add(ad->nf);
	elm_object_text_set(lbl, "<font_size=40><align=left>Font size: </align></font_size>");
	elm_object_part_content_set(layout, "label", lbl);

	//text size
	ts = elm_entry_add(ad->nf);
	ea_entry_selection_back_event_allow_set(ts, EINA_TRUE);
	elm_entry_scrollable_set(ts, EINA_FALSE);
	elm_entry_single_line_set(ts, EINA_TRUE);
	elm_entry_cursor_end_set(ts);
	evas_object_size_hint_weight_set(ts, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(ts, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_entry_entry_set(ts, "40");
	evas_object_show(ts);
	elm_object_part_content_set(layout, "entry_size", ts);
	tsobj = ts;

	//button
	btn = elm_button_add(ad->nf);
	elm_object_text_set(btn, "Change");
	evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_smart_callback_add(btn, "clicked", _btn_change_text_clicked_cb, NULL);
	evas_object_show(btn);
	elm_object_part_content_set(layout, "button", btn);

	//input entry
	en1 = elm_entry_add(ad->nf);
	ea_entry_selection_back_event_allow_set(en1, EINA_TRUE);
	evas_object_size_hint_weight_set(en1, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(en1, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_entry_scrollable_set(en1, EINA_TRUE);
	elm_entry_select_all(en1);
	elm_entry_single_line_set(en1, EINA_FALSE);
	elm_scroller_policy_set(en1, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);

	Ecore_IMF_Context *imf_context = elm_entry_imf_context_get(en1);
	ecore_imf_context_input_panel_event_callback_add(imf_context,
			                 ECORE_IMF_INPUT_PANEL_STATE_EVENT,
							 _input_panel_event_callback,
							 NULL);

	elm_entry_entry_set(en1, "Text");
	elm_entry_cursor_end_set(en1);
	evas_object_show(en1);
	elm_object_part_content_set(layout, "entry_part1", en1);
	txtobj1 = en1;
	evas_object_smart_callback_add(txtobj1, "changed", _entry_text_changed_cb, NULL);
	evas_object_smart_callback_add(txtobj1, "preedit,changed", _entry_text_changed_cb, NULL); //for pre-edit
	//evas_object_smart_callback_add(txtobj1, "cursor,changed", _entry_text_changed_cb, NULL); //can be used

	//view entry
	en2 = elm_entry_add(ad->nf);
	ea_entry_selection_back_event_allow_set(en2, EINA_TRUE);
	evas_object_size_hint_weight_set(en2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(en2, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_entry_scrollable_set(en2, EINA_TRUE);
	elm_entry_select_all(en2);
	elm_entry_single_line_set(en2, EINA_FALSE);
	elm_scroller_policy_set(en2, ELM_SCROLLER_POLICY_OFF,
									ELM_SCROLLER_POLICY_AUTO);
	elm_entry_entry_set(en2, "Text");
	evas_object_show(en2);
	elm_object_part_content_set(layout, "entry_part2", en2);
	txtobj2 = en2;

	//apply text style
	elm_entry_text_style_user_push(txtobj1, "DEFAULT='font_size=40'");
	elm_entry_text_style_user_push(txtobj2, "DEFAULT='font_size=40'");
}

static Eina_Bool _pop_cb(void *data, Elm_Object_Item *it)
{
	evas_object_smart_callback_del(data, "virtualkeypad,state,on", _show_title_toolbar);
	evas_object_smart_callback_del(data, "virtualkeypad,state,off", _hide_title_toolbar);

	return EINA_TRUE;
}

static void light_theme_entry(void *data, Evas_Object *obj,
				       void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	Evas_Object *en;
	Evas_Object *lay;

	if (ad == NULL) return;

	lay = elm_layout_add(ad->nf);
	evas_object_size_hint_weight_set(lay, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	elm_layout_file_set(lay, ELM_DEMO_EDJ, "entry_test");

	elm_naviframe_item_push(ad->nf, _("Entry with light theme"), NULL, NULL, lay, NULL);

	en = elm_entry_add(ad->nf);
	ea_entry_selection_back_event_allow_set(en, EINA_TRUE);
	elm_entry_scrollable_set(en, EINA_TRUE);
	elm_entry_select_all(en);
	elm_scroller_policy_set(en, ELM_SCROLLER_POLICY_OFF,
						ELM_SCROLLER_POLICY_AUTO);

	Ecore_IMF_Context *imf_context = elm_entry_imf_context_get(en);
	ecore_imf_context_input_panel_event_callback_add(imf_context,
							 ECORE_IMF_INPUT_PANEL_STATE_EVENT,
							 _input_panel_event_callback,
							 NULL);

	elm_entry_single_line_set(en, EINA_TRUE);
	elm_entry_entry_set(en, "This is an example of entry with ligth theme");
	elm_entry_cursor_end_set(en);
	evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(en, EVAS_HINT_FILL, EVAS_HINT_FILL);

	//you can also customize entry further
	//every customized function for entry should be called after theme set function
	elm_entry_text_style_user_push(en, "DEFAULT='color=#0000FF'");

	evas_object_show(en);

	evas_object_smart_callback_add(en, "activated", _entry_enter_click,
				       NULL);

	elm_object_part_content_set(lay, "entry_part", en);
}

static void _check_translate_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *en = (Evas_Object *)data;
	Eina_Bool state = elm_check_state_get(obj);
	if (en)
	{
		if (state)
		{
			elm_entry_context_menu_item_add(en, "Translate", NULL, ELM_ICON_STANDARD, NULL, NULL);
		}
		else
		{
			elm_entry_context_menu_clear(en);
		}
	}
}

static void entry_translate_item(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	Evas_Object *en, *check;
	Evas_Object *lay;

	if (ad == NULL) return;

	lay = elm_layout_add(ad->nf);
	evas_object_size_hint_weight_set(lay, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_layout_file_set(lay, ELM_DEMO_EDJ, "entry_test_translate");

	elm_naviframe_item_push(ad->nf, _("Enable/Disable Translate item"), NULL, NULL, lay, NULL);

	en = elm_entry_add(ad->nf);
	ea_entry_selection_back_event_allow_set(en, EINA_TRUE);
	elm_entry_scrollable_set(en, EINA_TRUE);
	elm_scroller_policy_set(en, ELM_SCROLLER_POLICY_AUTO,
							ELM_SCROLLER_POLICY_AUTO);

	Ecore_IMF_Context *imf_context = elm_entry_imf_context_get(en);
	ecore_imf_context_input_panel_event_callback_add(imf_context,
							ECORE_IMF_INPUT_PANEL_STATE_EVENT,
							_input_panel_event_callback,
							NULL);

	elm_entry_single_line_set(en, EINA_FALSE);
	elm_entry_entry_set(en, "This is an example of entry. You can enable/disable translate item in copy/paste popup.");
	evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(en, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(en);
	elm_object_part_content_set(lay, "entry_part", en);

	check = elm_check_add(ad->nf);
	elm_check_state_set(check, EINA_TRUE);
	elm_object_text_set(check, "Enable Translate item");
	evas_object_smart_callback_add(check, "changed", _check_translate_changed_cb, en);
	elm_object_part_content_set(lay, "check_part", check);
}

/* UI function to create entries */
void entry_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	Evas_Object *list;
	Elm_Object_Item *navi_it;

	list = elm_list_add(ad->nf);
	elm_list_mode_set(list, ELM_LIST_COMPRESS);
	evas_object_smart_callback_add(list, "selected", _list_click, NULL);

	elm_list_item_append(list, "Password", NULL, NULL, password_cb, ad);
	elm_list_item_append(list, "Scrolled_entry", NULL, NULL,
			     single_line_scrolled_entry, ad);
	elm_list_item_append(list, "Entry_multiline", NULL, NULL,
			     entry_multiline, ad);
	elm_list_item_append(list, "Copy & Paste", NULL, NULL,
			     entry_copy_paste, ad);
	elm_list_item_append(list, "Autocapitalization", NULL, NULL,
			     entry_autocapital, ad);
	elm_list_item_append(list, "Change Text Size", NULL, NULL,
			     entry_change_text_size, ad);
	elm_list_item_append(list, "Entry with light theme", NULL, NULL,
			     light_theme_entry, ad);
	elm_list_item_append(list, "Enable/Disable Translate item", NULL, NULL,
			     entry_translate_item, ad);
	elm_list_go(list);
	navi_it = elm_naviframe_item_push(ad->nf, _("Entry"), NULL, NULL, list, NULL);
	elm_naviframe_item_pop_cb_set(navi_it, _pop_cb, ad->conform);
	evas_object_smart_callback_add(ad->conform, "virtualkeypad,state,on", _show_title_toolbar, ad->nf);
	evas_object_smart_callback_add(ad->conform, "virtualkeypad,state,off", _hide_title_toolbar, ad->nf);
}
