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

#define HIGHLIGHT_TIMER 0.1
#define UNHIGHLIGHT_TIMER 0.01
static Ecore_Timer *highlight_timer;
static Eina_List *unhighlight_lists;
static Eina_Bool key_down = EINA_FALSE;

/*********************************************************
  DialogueGroup
 ********************************************************/
static void dialogue_group_1(void *data, Evas_Object * obj, void *event_info);
static void _clicked(void *data);

static void _list_click(void *data, Evas_Object * obj, void *event_info)
{
	Elm_Object_Item *it = (Elm_Object_Item *) elm_list_selected_item_get(obj);
	if (it == NULL) return;
	elm_list_item_selected_set(it, EINA_FALSE);
}

static struct _menu_item _menu_its[] = {
	{"Dialogue Group - 1", dialogue_group_1},
	//{"Dialogue Group - 2", dialogue_group_2},
	/* do not delete below */
	{NULL, NULL}
};

static Eina_Bool _unhighlight_timer(void *data)
{
	Evas_Object *layout;
	highlight_timer = NULL;
	EINA_LIST_FREE(unhighlight_lists, layout) {
		elm_object_signal_emit(layout, "elm,state,unselected", "elm");
	}
	if (data) _clicked(data);
	return EINA_FALSE;
}

static Eina_Bool _highlight_timer(void *data)
{
	highlight_timer = NULL;
	Evas_Object *layout = (Evas_Object *)data;
	elm_object_signal_emit(layout, "elm,state,selected", "elm");
	return EINA_FALSE;
}
static Eina_Bool _pop_cb(void *data, Elm_Object_Item *it)
{
	/* change the background to default and free expand data*/
	if (highlight_timer) ecore_timer_del(highlight_timer);
	highlight_timer = NULL;
	unhighlight_lists = eina_list_free(unhighlight_lists);
	return EINA_TRUE;
}

static Evas_Object* _create_scroller(Evas_Object* parent)
{
	Evas_Object* scroller = elm_scroller_add(parent);
	elm_scroller_bounce_set(scroller, EINA_FALSE, EINA_TRUE);
	elm_scroller_policy_set(scroller,ELM_SCROLLER_POLICY_OFF,ELM_SCROLLER_POLICY_AUTO);
	evas_object_show(scroller);

	return scroller;
}

///////////////////////// Codes for making Editfield : START
static char *_access_info_cb(void *data, Evas_Object *obj)
{
    const char *txt = NULL;
	txt = elm_object_part_text_get(data, "elm.text");
	if (!txt) return NULL;

	return strdup(_(txt));
}

static char *_access_main_sub_info_cb(void *data, Evas_Object *obj)
{
	Eina_Strbuf *buf;
    const char *txt = NULL;
    char *ret = NULL;

	buf = eina_strbuf_new();

	txt = elm_object_part_text_get(data, "elm.text.1");
	eina_strbuf_append(buf, _(txt));

	txt = elm_object_part_text_get(data, "elm.text.2");
	eina_strbuf_append_printf(buf, ", %s", _(txt));

	ret = eina_strbuf_string_steal(buf);
	eina_strbuf_free(buf);
	return ret;
}

static void _access_activate_cb(void *data, Evas_Object *part_obj, Elm_Object_Item *item)
{
	Eina_Bool state;

	state = elm_check_state_get(data);
	elm_check_state_set(data, !state);
}

static Evas_Object *_singleline_editfield_add(Evas_Object *parent) // For single lined editfield without top title.
{
	Evas_Object *layout, *entry;

	layout = elm_layout_add(parent);
	elm_layout_theme_set(layout, "layout", "dialogue/editfield", "default"); // Default editfield layout style without top title.

	entry = ea_editfield_add(parent, EA_EDITFIELD_SCROLL_SINGLELINE);
	elm_object_domain_translatable_part_text_set(entry, "elm.guide", "sys_string", "IDS_COM_BODY_EDIT");
	elm_object_part_content_set(layout, "elm.icon.entry", entry);

	return layout;
}

static Evas_Object *_singleline_editfield_with_title_add(Evas_Object *parent) // For single lined editfield with top title.
{
	Evas_Object *layout, *entry, *ao;

	layout = elm_layout_add(parent);
	elm_layout_theme_set(layout, "layout", "dialogue/editfield/title", "default"); // Editfield Style with top title.

	entry = ea_editfield_add(parent, EA_EDITFIELD_SCROLL_SINGLELINE);
	elm_object_domain_translatable_part_text_set(entry, "elm.guide", "sys_string", "IDS_COM_BODY_EDIT");
	elm_object_part_content_set(layout, "elm.icon.entry", entry);

	// Access for layout
	ao = elm_object_part_access_register(layout, "bg_dialogue");
	elm_access_info_cb_set(ao, ELM_ACCESS_INFO, _access_info_cb, layout);

	elm_object_focus_custom_chain_append(layout, ao, NULL);
	elm_object_focus_custom_chain_append(layout, entry, NULL);

	return layout;
}

static Evas_Object *_multiline_editfield_with_title_add(Evas_Object *parent) // For multi lined editfield with top title.
{
	Evas_Object *layout, *entry, *ao;

	layout = elm_layout_add(parent);
	elm_layout_theme_set(layout, "layout", "dialogue/editfield/title", "default"); // Default editfield layout style without top title.
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	entry = ea_editfield_add(parent, EA_EDITFIELD_MULTILINE);
	ea_editfield_clear_button_disabled_set(entry, EINA_TRUE);
	elm_object_domain_translatable_part_text_set(entry, "elm.guide", "sys_string", "IDS_COM_BODY_EDIT");
	elm_object_part_content_set(layout, "elm.icon.entry", entry);

	// Access for layout
	ao = elm_object_part_access_register(layout, "bg_dialogue");
	elm_access_info_cb_set(ao, ELM_ACCESS_INFO, _access_info_cb, layout);

	elm_object_focus_custom_chain_append(layout, ao, NULL);
	elm_object_focus_custom_chain_append(layout, entry, NULL);

	return layout;
}
///////////////////////// Codes for making Editfield : END
static Evas_Object *_gl_minus_btn_get(Evas_Object *obj)
{
	Evas_Object * icon = elm_button_add(obj);
	elm_object_style_set(icon, "icon_minus");
	evas_object_propagate_events_set(icon, EINA_FALSE);
	return icon;
}

static void _clicked(void *data)
{
	printf("\n clicked event \n");
}

static void _up(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	Evas_Object *layout = (Evas_Object *) data;
	Evas_Event_Mouse_Up *ev = (Evas_Event_Mouse_Up *)event_info;
	if (highlight_timer) {
		ecore_timer_del(highlight_timer);
		elm_object_signal_emit(layout, "elm,state,selected", "elm");
	}
	if (ev && !(ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD)) {
		highlight_timer = ecore_timer_add(UNHIGHLIGHT_TIMER, _unhighlight_timer, data);
		unhighlight_lists = eina_list_append(unhighlight_lists, layout);
	}
}

static void _move(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	/*when move event is called user intends to scroll hence unhighlight*/
	Evas_Event_Mouse_Move *ev = (Evas_Event_Mouse_Move *)event_info;
	Evas_Object *layout = (Evas_Object *) data;
	if (ev && ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) {
		if (highlight_timer) {
			ecore_timer_del(highlight_timer);
			highlight_timer = NULL;
		}
		elm_object_signal_emit(layout, "elm,state,unselected", "elm");
		return;
	}
}

static void _down(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	/*send select signal on mouse down to highlight layout*/
	Evas_Object *layout = (Evas_Object *) data;
	Evas_Event_Mouse_Down *ev = (Evas_Event_Mouse_Down *)event_info;
	if (ev && ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
		return;
	if (highlight_timer) ecore_timer_del(highlight_timer);
	highlight_timer = ecore_timer_add(HIGHLIGHT_TIMER, _highlight_timer, layout);
}

static void _key_down(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	Evas_Event_Key_Down *ev = event_info;
	if (!ev) return;
	if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;

	if ((!strcmp(ev->keyname, "Return")) ||
			(!strcmp(ev->keyname, "KP_Enter")) ||
			(!strcmp(ev->keyname, "space")))
	{
		if (!key_down && obj)
		{
			printf("item_highlighted\n");
			key_down = EINA_TRUE;
			Evas_Object *layout = (Evas_Object *)obj;
			elm_object_signal_emit(layout, "elm,state,selected", "elm");
		}
		printf("Layout(%p) is downed\n", obj);
	}
}

static void _key_up(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	Evas_Event_Key_Up *ev = event_info;
	if (!ev) return;
	if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;

	printf("Layout(%p) is uped \n", data);
	if ((!strcmp(ev->keyname, "Return")) ||
			(!strcmp(ev->keyname, "KP_Enter")) ||
			(!strcmp(ev->keyname, "space")))
	{
		if (key_down && data)
		{
			printf("item_highlighted\n");
			key_down = EINA_FALSE;
			Evas_Object *layout = (Evas_Object *)data;
			highlight_timer = ecore_timer_add(UNHIGHLIGHT_TIMER, _unhighlight_timer, layout);
			unhighlight_lists = eina_list_append(unhighlight_lists, layout);
		}
	}
}

static void dialogue_group_1(void *data, Evas_Object * obj, void *event_info)
{
	Evas_Object *ao;
	Evas_Object *box;
	Evas_Object *scroller;
	Evas_Object *layout[12];
	Evas_Object *check[3];
	Elm_Object_Item *navi_it;
	struct appdata *ad = (struct appdata *)data;
	if (ad == NULL) return;

	scroller = _create_scroller(ad->nf);
	elm_object_style_set(scroller, "dialogue");

	box = elm_box_add(scroller);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(box, EVAS_HINT_FILL, 0.0);
	elm_object_content_set(scroller, box);
	evas_object_show(box);

	layout[0] = elm_layout_add(box);
	elm_layout_theme_set(layout[0], "layout", "dialogue", "separator");
	evas_object_size_hint_weight_set(layout[0], EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(layout[0], EVAS_HINT_FILL, 0.0);
	elm_box_pack_end(box, layout[0]);
	evas_object_show(layout[0]);

	layout[1] = elm_layout_add(box);
	elm_layout_theme_set(layout[1], "layout", "dialogue", "1text");
	evas_object_size_hint_weight_set(layout[1], EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(layout[1], EVAS_HINT_FILL, 0.0);
	elm_object_part_text_set(layout[1], "elm.text", "List");
	elm_box_pack_end(box, layout[1]);
	evas_object_show(layout[1]);
	evas_object_event_callback_add(layout[1], EVAS_CALLBACK_MOUSE_DOWN, _down, layout[1]);
	evas_object_event_callback_add(layout[1], EVAS_CALLBACK_MOUSE_MOVE, _move, layout[1]);
	evas_object_event_callback_add(layout[1], EVAS_CALLBACK_MOUSE_UP, _up, layout[1]);
	elm_object_signal_emit(layout[1], "elm,state,top", "");

	evas_object_event_callback_add(layout[1], EVAS_CALLBACK_KEY_DOWN, _key_down, NULL);
	evas_object_event_callback_add(layout[1], EVAS_CALLBACK_KEY_UP, _key_up, layout[1]);

	ao = elm_object_part_access_register(layout[1], "bg_dialogue");
	elm_access_info_cb_set(ao, ELM_ACCESS_INFO, _access_info_cb, layout[1]);

	layout[2] = elm_layout_add(box);
	elm_layout_theme_set(layout[2], "layout", "dialogue", "1text.1icon");
	evas_object_size_hint_weight_set(layout[2], EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(layout[2], EVAS_HINT_FILL, 0.0);
	elm_object_part_text_set(layout[2], "elm.text", "List");
	elm_box_pack_end(box, layout[2]);
	evas_object_show(layout[2]);
	evas_object_event_callback_add(layout[2], EVAS_CALLBACK_MOUSE_DOWN, _down, layout[2]);
	evas_object_event_callback_add(layout[2], EVAS_CALLBACK_MOUSE_MOVE, _move, layout[2]);
	evas_object_event_callback_add(layout[2], EVAS_CALLBACK_MOUSE_UP, _up, layout[2]);
	check[0] = elm_check_add(layout[2]);
	elm_object_style_set(check[0], "on&off");
	elm_object_part_content_set(layout[2], "elm.icon", check[0]);
	evas_object_propagate_events_set(check[0], EINA_FALSE);
	elm_object_signal_emit(layout[2], "elm,state,bottom", "");

	evas_object_event_callback_add(layout[2], EVAS_CALLBACK_KEY_DOWN, _key_down, NULL);
	evas_object_event_callback_add(layout[2], EVAS_CALLBACK_KEY_UP, _key_up, layout[2]);

	ao = elm_object_part_access_register(layout[2], "bg_dialogue");
	elm_access_info_cb_set(ao, ELM_ACCESS_INFO, _access_info_cb, layout[2]);
	elm_access_activate_cb_set(ao, _access_activate_cb, check[0]);

	layout[3] = elm_layout_add(box);
	elm_layout_theme_set(layout[3], "layout", "dialogue", "title");
	evas_object_size_hint_weight_set(layout[3], EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(layout[3], EVAS_HINT_FILL, 0.0);
	elm_object_part_text_set(layout[3], "elm.text", "Title");
	elm_box_pack_end(box, layout[3]);
	evas_object_show(layout[3]);

	ao = elm_access_object_register(layout[3], layout[3]);
	elm_access_info_cb_set(ao, ELM_ACCESS_INFO, _access_info_cb, layout[3]);
	elm_object_focus_custom_chain_append(layout[3], ao, NULL);

	layout[4] = elm_layout_add(box);
	elm_layout_theme_set(layout[4], "layout", "dialogue", "1text.1icon.2");
	evas_object_size_hint_weight_set(layout[4], EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(layout[4], EVAS_HINT_FILL, 0.0);
	elm_object_part_text_set(layout[4], "elm.text", "List");
	elm_box_pack_end(box, layout[4]);
	evas_object_show(layout[4]);
	evas_object_event_callback_add(layout[4], EVAS_CALLBACK_MOUSE_DOWN, _down, layout[4]);
	evas_object_event_callback_add(layout[4], EVAS_CALLBACK_MOUSE_MOVE, _move, layout[4]);
	evas_object_event_callback_add(layout[4], EVAS_CALLBACK_MOUSE_UP, _up, layout[4]);
	check[1] = elm_check_add(layout[4]);
	elm_object_part_content_set(layout[4], "elm.icon", check[1]);
	elm_object_signal_emit(layout[4], "elm,state,top", "");

	evas_object_event_callback_add(layout[4], EVAS_CALLBACK_KEY_DOWN, _key_down, NULL);
	evas_object_event_callback_add(layout[4], EVAS_CALLBACK_KEY_UP, _key_up, layout[4]);

	ao = elm_object_part_access_register(layout[4], "bg_dialogue");
	elm_access_info_cb_set(ao, ELM_ACCESS_INFO, _access_info_cb, layout[4]);
	elm_access_activate_cb_set(ao, _access_activate_cb, check[1]);

	layout[5] = elm_layout_add(box);
	elm_layout_theme_set(layout[5], "layout", "dialogue", "1text.1icon.2");
	evas_object_size_hint_weight_set(layout[5], EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(layout[5], EVAS_HINT_FILL, 0.0);
	elm_object_part_text_set(layout[5], "elm.text", "List-Disabled");
	elm_box_pack_end(box, layout[5]);
	evas_object_show(layout[5]);
	check[2] = elm_check_add(layout[5]);
	elm_object_part_content_set(layout[5], "elm.icon", check[2]);
	/*Disable Item as shown below and do not connect any Mouse events, when
	 * enabling is required send "elm,state,enabled" signal and connect to mouse
	 * events
	 */
	elm_object_disabled_set(check[2], EINA_TRUE);
	elm_object_signal_emit(layout[5], "elm,state,disabled", "elm");
	elm_object_signal_emit(layout[5], "elm,state,center", "");

	ao = elm_object_part_access_register(layout[5], "bg_dialogue");
	elm_access_info_cb_set(ao, ELM_ACCESS_INFO, _access_info_cb, layout[5]);
	elm_object_focus_custom_chain_append(layout[5], ao, NULL);

	layout[6] = elm_layout_add(box);
	elm_layout_theme_set(layout[6], "layout", "dialogue", "1icon");
	evas_object_size_hint_weight_set(layout[6], EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(layout[6], EVAS_HINT_FILL, 0.0);
	elm_box_pack_end(box, layout[6]);
	evas_object_show(layout[6]);
	Evas_Object *slider = elm_slider_add(layout[6]);
	elm_slider_indicator_show_set(slider, EINA_TRUE);
	elm_slider_min_max_set(slider, 0, 9);
	elm_slider_indicator_format_set(slider, "%1.0f");
	elm_slider_value_set(slider, 7);
	elm_object_part_content_set(layout[6], "elm.icon", slider);
	elm_object_signal_emit(layout[6], "elm,state,center", "");

	layout[7] = elm_layout_add(box);
	elm_layout_theme_set(layout[7], "layout", "dialogue", "2text.3");
	evas_object_size_hint_weight_set(layout[7], EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(layout[7], EVAS_HINT_FILL, 0.0);
	elm_object_part_text_set(layout[7], "elm.text.1", "Maintext");
	elm_object_part_text_set(layout[7], "elm.text.2", "Subtext");
	evas_object_event_callback_add(layout[7], EVAS_CALLBACK_MOUSE_DOWN, _down, layout[7]);
	evas_object_event_callback_add(layout[7], EVAS_CALLBACK_MOUSE_MOVE, _move, layout[7]);
	evas_object_event_callback_add(layout[7], EVAS_CALLBACK_MOUSE_UP, _up, layout[7]);
	elm_object_part_content_set(layout[7], "elm.icon.edit", _gl_minus_btn_get(layout[7]));
	elm_box_pack_end(box, layout[7]);
	evas_object_show(layout[7]);
	elm_object_signal_emit(layout[7], "elm,state,bottom", "");
	elm_object_signal_emit(layout[7], "elm,state,edit,enabled", "");

	evas_object_event_callback_add(layout[7], EVAS_CALLBACK_KEY_DOWN, _key_down, NULL);
	evas_object_event_callback_add(layout[7], EVAS_CALLBACK_KEY_UP, _key_up, layout[7]);

	ao = elm_object_part_access_register(layout[7], "bg_dialogue");
	elm_access_info_cb_set(ao, ELM_ACCESS_INFO, _access_main_sub_info_cb, layout[7]);

	layout[8] = elm_layout_add(box);
	elm_layout_theme_set(layout[8], "layout", "dialogue", "title");
	evas_object_size_hint_weight_set(layout[8], EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(layout[8], EVAS_HINT_FILL, 0.0);
	elm_object_part_text_set(layout[8], "elm.text", "Title");
	elm_box_pack_end(box, layout[8]);
	evas_object_show(layout[8]);

	ao = elm_access_object_register(layout[8], layout[8]);
	elm_access_info_cb_set(ao, ELM_ACCESS_INFO, _access_info_cb, layout[8]);
	elm_object_focus_custom_chain_append(layout[8], ao, NULL);

	layout[9] = _singleline_editfield_with_title_add(box);
	evas_object_size_hint_weight_set(layout[9], EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(layout[9], EVAS_HINT_FILL, 0.0);
	elm_object_part_text_set(layout[9], "elm.text", _("Title Area"));
	elm_box_pack_end(box, layout[9]);
	evas_object_show(layout[9]);
	elm_object_signal_emit(layout[9], "elm,state,top", "");

	layout[10] = _multiline_editfield_with_title_add(box);
	evas_object_size_hint_weight_set(layout[10], EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(layout[10], EVAS_HINT_FILL, 0.0);
	elm_object_part_text_set(layout[10], "elm.text", _("Title Area"));
	elm_box_pack_end(box, layout[10]);
	evas_object_show(layout[10]);
	elm_object_signal_emit(layout[10], "elm,state,center", "");

	layout[11] = _singleline_editfield_add(box);
	evas_object_size_hint_weight_set(layout[11], EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(layout[11], EVAS_HINT_FILL, 0.0);
	elm_object_part_text_set(layout[11], "elm.text", _("Title Area"));
	elm_box_pack_end(box, layout[11]);
	evas_object_show(layout[11]);
	elm_object_signal_emit(layout[11], "elm,state,bottom", "");

	elm_object_focus_allow_set(layout[1], EINA_TRUE);
	elm_object_focus_allow_set(layout[2], EINA_TRUE);
	elm_object_focus_allow_set(layout[4], EINA_TRUE);
	elm_object_focus_allow_set(layout[6], EINA_TRUE);
	elm_object_focus_allow_set(layout[7], EINA_TRUE);
	elm_object_focus_allow_set(layout[9], EINA_TRUE);
	elm_object_focus_allow_set(layout[10], EINA_TRUE);
	elm_object_focus_allow_set(layout[11], EINA_TRUE);

	navi_it = elm_naviframe_item_push(ad->nf, _("Dialoguegroup-1"), NULL, NULL, scroller, NULL);
	elm_naviframe_item_pop_cb_set(navi_it, _pop_cb, NULL);
}

static Evas_Object *_create_list_winset(struct appdata *ad)
{
	Evas_Object *li;
	struct _menu_item *menu_its;

	if (ad == NULL) return NULL;

	li = elm_list_add(ad->nf);
	elm_list_mode_set(li, ELM_LIST_COMPRESS);
	evas_object_smart_callback_add(li, "selected", _list_click, NULL);

	int idx = 0;
	menu_its = _menu_its;
	while (menu_its[idx].name != NULL) {
		elm_list_item_append(li, menu_its[idx].name, NULL, NULL, menu_its[idx].func, ad);
		++idx;
	}
	elm_list_go(li);

	return li;
}

void dialoguegroup_cb(void *data, Evas_Object * obj, void *event_info)
{
	struct appdata *ad;

	Evas_Object *list;

	ad = (struct appdata *)data;
	if (ad == NULL) return;

	list = _create_list_winset(ad);
	elm_naviframe_item_push(ad->nf, _("DialogueGroup"), NULL, NULL, list, NULL);
}

