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

#include "elmdemo_util.h"
#include "elmdemo_test.h"
#include "genlist.h"

/*********************************************************
  Genlist Dialogue Group
 ********************************************************/
#define NUM_OF_ITEMS 2000
#define NUM_OF_NAMES 50
#define NUM_OF_GROUPS 5

typedef struct _Item_Data
{
	int index;
	int highlighted;
	Elm_Object_Item *item;
} Item_Data;

static char *menu_its[] = {
	"1icon",
	"1icon/no_padding",
	"1text",
	"1text.1icon",
	"1text.1icon.2",
	"1text.1icon.2.tb",
	"1text.2icon",
	"2text.1icon.6",
	"2text.1icon.6.tb",
	"1text.1icon.3",
	"1text.1icon.3.tb",
	"1text.2icon.2",
	"1text.3icon",
	"1text.1icon.5",
	"2text.1icon.4",
	"2text.1icon.4.tb",
	"2text.6",
	"2text.9",
	"2text.4",
	"2text.4.tb",
	"1text.3icon.2",
	"1text.1icon.4",
	"2text.5",
	"multiline/1text.1icon",
	"1text.1icon.divider",
	"1text.2icon.divider",
	"1text.2icon.10",
	"2text",
	"2text.2",
	"2text.3",
	"2text.1icon.2",
	"2text.1icon.3",
	"2text.3icon",
	"2text.2icon",
	"2text.2icon.2",
	"2text.2icon.3",
	"2text.2icon.3.tb",
	"2text.1icon.5",
	"multiline/1title.1text",
	"multiline/1title.2text",
	"1text.4icon",
	"2text.2icon.4",
	"2text.2icon.4.tb",
	"2text.1icon.10",
	"2text.1icon.10.tb",
	"2text.1icon.7",
	"2text.1icon.8",
	"multiline/2text",
	"2text.1icon.15",
	"2text.1icon.divider",
	"3text.2icon",
	"grouptitle",
	"multiline/1text",
	"2text/expandable",
	"2text.2/expandable",
	"2text.3/expandable",
	"2text.1icon/expandable",
	"3text.1icon/expandable",
	"1text/expandable",
	NULL
};

static char *title[] =
{ "Family", "Friends", "Colleagues", "Club", "Classmates" };

static char *help_dialogue_message =
	"Help message<br>1. Activate Wi-Fi on your device<br>"
	"2. Find name in your network list.<br>"
	"3. Connect name by entering password in WPA field.";

static char **genlist_demo_names;

static const char *color_set[5] =
{
	"#ff0000ff",
	"#00ff00ff",
	"#0000ffff",
	"#ff00ffff",
	"#00ffffff"
};

static char *photo_path[] = {
	"00_list_photo_default.png", "iu.jpg", "iu2.jpg", "koo.jpg", "top.jpg", "boa.jpg",
	"kimtaehee.jpg", "moon.jpg", "taeyon.jpg"
};

static void _gl_del(void *data, Evas_Object *obj)
{
	// FIXME: unrealized callback will be called after this.
	// accessing Item_Data can be dangerous on unrealized callback.
	Item_Data *id = data;
	if (id) free(id);
}

static void _list_selected(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *it = (Elm_Object_Item *) elm_list_selected_item_get(obj);

	if (it == NULL)
	{
		fprintf((LOG_PRI(LOG_ERR) == LOG_ERR?stderr:stdout), "List item is NULL.\n");
		return;
	}

	elm_list_item_selected_set(it, EINA_FALSE);
}

static char *
_gl_text_get_title(void *data, Evas_Object *obj, const char *part)
{
	Item_Data *id = data;
	int index = id->index;
	index = (index / 5) % NUM_OF_GROUPS;
	return strdup(title[index]);
}

static char *
_gl_text_get_password(void *data, Evas_Object *obj, const char *part)
{
	return strdup("password");
}

static char *
_gl_text_get(void *data, Evas_Object *obj, const char *part)
{
	Item_Data *id = data;
	int index = id->index;
	char buf[1024];

	if (!strcmp(part, "elm.text.2")) index+=2;

	snprintf(buf, 1023, "%s:%s", part, genlist_demo_names[index%NUM_OF_NAMES]);
	return strdup(buf);
}

static char *
_gl_text_multline_2text_get(void *data, Evas_Object *obj, const char *part)
{
	Item_Data *id = data;
	int index = id->index;
	char buf[2048];

	if (!strcmp(part, "elm.text.1")) {
		snprintf(buf, 1023, "%s:%s", part, genlist_demo_names[index%NUM_OF_NAMES]);
		return strdup(buf);
	} else {
		int size = (index % 30)*4 + 32;
		const char *color = color_set[index % 5];
		sprintf(buf, "<font_size=%d><color=%s>%s</color></font_size>",
				size, color, genlist_demo_names[index%NUM_OF_NAMES]);
		return strdup(buf);
	}
}

static char *_gl_text_get_textblock(void *data, Evas_Object *obj, const char *part)
{
	Item_Data *id = data;
	int index = id->index % NUM_OF_NAMES;
	char buf[PATH_MAX] = {0, };
	int size;
	const char *color;

	size = (index % 30)*4 + 32;
	color = color_set[index % 5];
	// FIXME: textblock is too slow....
	if (!strcmp(part, "elm.text") || !strcmp(part, "elm.text.2")) {
		sprintf(buf, "<font_size=%d><color=%s>%s</color></font_size>",
				size, color, genlist_demo_names[index]);
	} else
		sprintf(buf, "<color=%s>%s</color>",
				color, genlist_demo_names[index]);

	return strdup(buf);
}

static char *_gl_text_help_dialogue_get(void *data, Evas_Object *obj, const char *part)
{
	return strdup(help_dialogue_message);
}

static Evas_Object *_gl_minus_btn_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object * icon = elm_button_add(obj);
	elm_object_style_set(icon, "minus");
	evas_object_propagate_events_set(icon, EINA_FALSE);
	return icon;
}

static Evas_Object *
_gl_icon_colorbar_get(void *data, Evas_Object *obj, const char *part)
{
	Item_Data *id = data;
	int index = id->index;
	Evas_Object *icon = evas_object_rectangle_add(evas_object_evas_get(obj));
	evas_object_size_hint_align_set(icon, EVAS_HINT_FILL, EVAS_HINT_FILL);
	if (index%3 == 0) evas_object_color_set(icon, 80, 107, 207, 255);
	else if (index%3 == 1) evas_object_color_set(icon, 72, 136, 42, 255);
	else if (index%3 == 2) evas_object_color_set(icon, 204, 52, 52, 255);
	return icon;
}

static Evas_Object *
_gl_icon_progressbar_get(void *data, Evas_Object *obj, const char *part)
{
	if (!strcmp(part, "elm.icon.edit"))
		return _gl_minus_btn_get(data, obj, part);

	Evas_Object *icon = elm_progressbar_add(obj);
	elm_object_style_set(icon, "pending_list");
	elm_progressbar_horizontal_set(icon, EINA_TRUE);
	elm_progressbar_pulse(icon, EINA_TRUE);
	return icon;
}

static Evas_Object *
_gl_icon_reveal_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *icon = elm_button_add(obj);
	elm_object_style_set(icon, "reveal");
	evas_object_propagate_events_set(icon, EINA_FALSE);
	return icon;
}

static Evas_Object *
_gl_icon_onoff_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *icon = elm_check_add(obj);
	elm_check_state_set(icon, EINA_TRUE);
	elm_object_style_set(icon, "on&off");
	evas_object_propagate_events_set(icon, EINA_FALSE);

	Item_Data *id = data;
	const Elm_Genlist_Item_Class *itc;
	itc = elm_genlist_item_item_class_get(id->item);
	// If no divider, unregister access object
	if (itc && !strstr(itc->item_style, ".divider")) {
		elm_access_object_unregister(icon);
	}
	return icon;
}

static Evas_Object *
_gl_icon_chk_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *icon = elm_check_add(obj);
	elm_object_style_set(icon, "default/genlist");
	elm_check_state_set(icon, EINA_TRUE);
	evas_object_repeat_events_set(icon, EINA_TRUE);
	evas_object_propagate_events_set(icon, EINA_FALSE);

	Item_Data *id = data;
	const Elm_Genlist_Item_Class *itc;
	itc = elm_genlist_item_item_class_get(id->item);
	// If no divider, unregister access object
	if (itc && !strstr(itc->item_style, ".divider")) {
		elm_access_object_unregister(icon);
	}
	return icon;
}

static Evas_Object *
_gl_icon_btn_text_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *icon = elm_button_add(obj);
	elm_object_text_set(icon, "Text Button");
	evas_object_propagate_events_set(icon, EINA_FALSE);
	return icon;
}

static Evas_Object *
_gl_icon_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *icon = elm_image_add(obj);
	elm_image_file_set(icon, ICON_DIR"/genlist/iu.jpg", NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	return icon;
}

static Evas_Object *
_gl_icon_photo_get(void *data, Evas_Object *obj, const char *part)
{

	if (!strcmp(part, "elm.icon.edit"))
		return _gl_minus_btn_get(data, obj, part);

	Item_Data *id = data;
	int index = id->index % (sizeof(photo_path)/sizeof(*photo_path));
	char buf[PATH_MAX];
	Evas_Object *icon = elm_image_add(obj);
	sprintf(buf, ICON_DIR"/genlist/%s", photo_path[index]);
	elm_image_file_set(icon, buf, NULL);
	evas_object_size_hint_align_set(icon, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	return icon;
}

static Evas_Object *_gl_content_get(void *data, Evas_Object *obj, const char *part)
{
	if (!strcmp(part, "elm.icon.edit"))
		return _gl_minus_btn_get(data, obj, part);
	else if (!strcmp(part, "elm.swallow.end")) {
		return _gl_icon_reveal_get(data, obj, part);
	} else if (!strcmp(part, "elm.swallow.colorbar")) {
		return _gl_icon_colorbar_get(data, obj, part);
	} else if (!strcmp(part, "elm.swallow.progress")) {
		return _gl_icon_progressbar_get(data, obj, part);
	}
	return _gl_icon_get(data, obj, part);
}

static Evas_Object *_gl_content_get_title(void *data, Evas_Object *obj, const char *part)
{
	int index = (int) data;
	if (!strcmp(part, "elm.icon") && index == 0) {
		Evas_Object *progressbar  = elm_progressbar_add(obj);
		elm_object_style_set(progressbar, "list_process_small");
		elm_progressbar_horizontal_set(progressbar, EINA_TRUE);
		elm_progressbar_pulse(progressbar, EINA_TRUE);
		return progressbar;
	}
	return NULL;
}

static Evas_Object *_gl_icon_grouptitle_get(void *data, Evas_Object *obj, const char *part)
{
	Item_Data *id = data;
	int index = id->index;
	if (!strcmp(part, "elm.icon") && ((index % 4) == 0)) {
		Evas_Object *progressbar  = elm_progressbar_add(obj);
		elm_object_style_set(progressbar, "list_process_small");
		elm_progressbar_horizontal_set(progressbar, EINA_TRUE);
		elm_progressbar_pulse(progressbar, EINA_TRUE);
		return progressbar;
	}
	return NULL;
}

static Evas_Object *
_gl_2icon_chk_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *icon;
	if (!strcmp(part, "elm.icon") || !strcmp(part, "elm.icon.1"))
		icon = _gl_icon_chk_get(data, obj, part);
	else icon = _gl_content_get(data, obj, part);
	return icon;
}

static Evas_Object *
_gl_2icon_onoff_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *icon;
	if (!strcmp(part, "elm.icon") || !strcmp(part, "elm.icon.2"))
		icon = _gl_icon_onoff_get(data, obj, part);
	else icon = _gl_content_get(data, obj, part);
	return icon;
}

static Evas_Object *
_gl_2icon_reveal_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *icon;
	if (!strcmp(part, "elm.icon") || !strcmp(part, "elm.icon.2"))
		icon = _gl_icon_reveal_get(data, obj, part);
	else icon = _gl_content_get(data, obj, part);
	return icon;
}

static Evas_Object *
_gl_2icon_chk_reveal_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *icon;
	if (!strcmp(part, "elm.icon") || !strcmp(part, "elm.icon.2"))
		icon = _gl_icon_reveal_get(data, obj, part);
	else if (!strcmp(part, "elm.icon.1"))
		icon = _gl_icon_chk_get(data, obj, part);
	else
		icon = _gl_content_get(data, obj, part);
	return icon;
}

static Evas_Object *
_gl_3icon_chk_reveal_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *icon;
	if (!strcmp(part, "elm.icon.3"))
		icon = _gl_icon_reveal_get(data, obj, part);
	else if (!strcmp(part, "elm.icon.1"))
		icon = _gl_icon_chk_get(data, obj, part);
	else
		icon = _gl_content_get(data, obj, part);
	return icon;
}

static Evas_Object *
_gl_icons_get(void *data, Evas_Object *obj, const char *part)
{
	Item_Data *id = data;
	int index = id->index % 7;
	Evas_Object *icon;

	if (!strcmp(part, "elm.icon.edit"))
		return _gl_minus_btn_get(data, obj, part);
	else if (!strcmp(part, "elm.swallow.end")) {
		return _gl_icon_reveal_get(data, obj, part);
	} else if (!strcmp(part, "elm.swallow.colorbar")) {
		return _gl_icon_colorbar_get(data, obj, part);
	} else if (!strcmp(part, "elm.swallow.progress")) {
		return _gl_icon_progressbar_get(data, obj, part);
	}

	if (0 == index) {
		icon = _gl_icon_progressbar_get(data, obj, part);
	} else if (1 == index) {
		icon = _gl_icon_reveal_get(data, obj, part);
	} else if (2 == index) {
		icon = _gl_icon_onoff_get(data, obj, part);
	} else if (3 == index) {
		icon = _gl_icon_btn_text_get(data, obj, part);
	} else if (4 == index) {
		icon = _gl_icon_chk_get(data, obj, part);
	} else {
		icon = _gl_icon_photo_get(data, obj, part);
	}
	return icon;
}

static Evas_Object *
_gl_icon_slider_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *slider, *icon, *icon2;

	if (!strcmp(part, "elm.icon.edit"))
		return _gl_minus_btn_get(data, obj, part);

	slider = elm_slider_add(obj);
	elm_slider_indicator_show_set(slider, EINA_TRUE);
	evas_object_size_hint_weight_set(slider, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(slider, EVAS_HINT_FILL, 0.5);
	elm_slider_indicator_format_set(slider, "%1.0f");

	icon = elm_image_add(obj);
	elm_image_file_set(icon, ICON_DIR"/00_slider_button_volume_01.png", NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_object_content_set(slider, icon);
	icon2 = elm_image_add(obj);
	elm_image_file_set(icon2, ICON_DIR"/00_slider_button_volume_02.png", NULL);
	evas_object_size_hint_aspect_set(icon2, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_object_part_content_set(slider, "end", icon2);
	evas_object_propagate_events_set(slider, EINA_FALSE);

	return slider;
}

static Evas_Object *
_gl_icon_entry_get(void *data, Evas_Object *obj, const char *part)
{
	static Elm_Entry_Filter_Limit_Size limit_filter_data;
	Evas_Object *en = elm_entry_add(obj);
	ea_entry_selection_back_event_allow_set(en, EINA_TRUE);
	evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(en, 1, EVAS_HINT_FILL);

	limit_filter_data.max_byte_count = 0;
	limit_filter_data.max_char_count = 1;
	elm_entry_markup_filter_append(en, elm_entry_filter_limit_size,
		&limit_filter_data);

	elm_entry_single_line_set(en, EINA_TRUE);
	elm_entry_password_set(en, EINA_TRUE);
	elm_object_style_set(en, "list");
	//elm_entry_entry_set(en, "pass");
	elm_entry_cursor_end_set(en);
	return en;
}

static char *_accinfo_cb(void *data, Evas_Object *acc)
{
	char *dup = NULL;
	Elm_Object_Item *it = data;
	const char *txt = elm_object_item_part_text_get(it, "elm.text");
	if (!txt) {
		txt = elm_object_item_part_text_get(it, "elm.text.1");
	}
	if (txt) dup = strdup(txt);
	return dup;
}

static void _gl_sel(void *data, Evas_Object *obj, void *ei)
{
	if (!ei) return;
	Elm_Object_Item *item = ei;
	Item_Data *id = elm_object_item_data_get(item);
	int index = id->index;
	printf("Select callback: %s\n", genlist_demo_names[index%NUM_OF_GENLIST_DEMO_NAMES]);

	// Make item as unselected
	elm_genlist_item_selected_set(ei, EINA_FALSE);

	const Elm_Genlist_Item_Class *itc = elm_genlist_item_item_class_get(item);
	if (!strcmp(itc->item_style, "dialogue/1text.1icon.3") ||
		!strcmp(itc->item_style, "dialogue/1text.1icon") ||
		!strcmp(itc->item_style, "dialogue/1text.2icon.2") ||
		!strcmp(itc->item_style, "dialogue/1text.3icon") ||
		!strcmp(itc->item_style, "dialogue/1text.3icon.2") ||
		!strcmp(itc->item_style, "dialogue/multiline/1text.1icon") ||
		!strcmp(itc->item_style, "dialogue/1text.2icon.10") ||
		!strcmp(itc->item_style, "dialogue/2text.1icon.10") ||
		!strcmp(itc->item_style, "dialogue/2text.1icon.10.tb") ||
		!strcmp(itc->item_style, "dialogue/2text.2icon")) {
		printf("update check\n");
		// Update check button
		Evas_Object *ck = elm_object_item_part_content_get(ei, "elm.icon");
		if (!ck) ck = elm_object_item_part_content_get(ei, "elm.icon.1");
		if (ck) {
			Eina_Bool state = elm_check_state_get(ck);
			elm_check_state_set(ck, !state);
		}
	}

}

static void _access_activate_cb(void *data, Evas_Object *part_obj, Elm_Object_Item *item)
{
	//Eina_Bool state;

	elm_genlist_item_selected_set(data, EINA_TRUE);
	//state = elm_check_state_get(data);
	//elm_check_state_set(data, !state);
}

static void _gl_realized(void *data, Evas_Object *obj, void *ei)
{
	if (!ei) return;
	Item_Data *id = elm_object_item_data_get(ei);
	int index = id->index;

	// ===== Accessibility ====
	if (elm_config_access_get()) {
		const Elm_Genlist_Item_Class *itc = elm_genlist_item_item_class_get(ei);

		// Register bg_dialogue image to be highlighted instead of whole item
		if (strcmp(itc->item_style, "dialogue/grouptitle")) {
			Evas_Object *acc = elm_object_item_part_access_register(ei, "bg_dialogue");
			elm_access_info_cb_set(acc, ELM_ACCESS_INFO, _accinfo_cb, ei);
			elm_access_activate_cb_set(acc, _access_activate_cb, ei);
		}

		// Unregister item not to be highglihted
		if (!strcmp(itc->item_style, "dialogue/1icon/no_padding")) {
			elm_object_item_access_unregister(ei);
		}

		// if icon is not appended yet, register icon to be highglihted
		if (!strcmp(itc->item_style, "dialogue/1icon") ||
				!strcmp(itc->item_style, "dialogue/1icon/no_padding")) {
			const Eina_List *org = elm_object_item_access_order_get(ei);
			Evas_Object *content =
				elm_object_item_part_content_get(ei, "elm.icon");
			if (content &&  !eina_list_data_find(org, content)) {
				Evas_Object *tmp;
				Eina_List *l;
				Eina_List *items = NULL;

				// Duplicate original access order
				EINA_LIST_FOREACH((Eina_List *)org, l, tmp)
					items = eina_list_append(items, tmp);

				Evas_Object *ao = elm_access_object_register(content, obj);
				items = eina_list_append(items, ao);
				elm_object_item_access_order_set(ei, items);
			}
		}

		// Remove progress from access order not to be highlighted
		if (!strcmp(itc->item_style, "dialogue/grouptitle")) {
			printf("laskjdasf\n");
			const Eina_List *org = elm_object_item_access_order_get(ei);
			Evas_Object *content =
				elm_object_item_part_content_get(ei, "elm.icon");
			printf("content: %p\n", content);

			if (content && eina_list_data_find(org, content)) {
				Evas_Object *tmp;
				Eina_List *l;
				Eina_List *items = NULL;

				// Duplicate original access order
				EINA_LIST_FOREACH((Eina_List *)org, l, tmp)
					items = eina_list_append(items, tmp);

				items = eina_list_remove(items, content);
				elm_object_item_access_order_set(ei, items);
			}
		}
	}

	if (index == 3) {
		elm_object_item_signal_emit(ei, "elm,state,edit,enabled", "");
	}

	if (index == 0) {
		elm_object_item_signal_emit(ei, "elm,state,normal", "");
	} else if (index % 6 == 3) {// right after title
		elm_object_item_signal_emit(ei, "elm,state,top", "");
	} else if (index % 6 == 0) {
		elm_object_item_signal_emit(ei, "elm,state,bottom", "");
	} else {
		elm_object_item_signal_emit(ei, "elm,state,center", "");
	}


}

static void _create_genlist(void *data, Evas_Object *obj, void *event_info)
{
	if (!data) return;
	struct appdata *ad = data;
	int index;
	Elm_Object_Item *item;
	Evas_Object *genlist;
	const char *style = elm_object_item_text_get(event_info);

	char str[256];
	sprintf(str, "dialogue/%s", style);

	// Create item class
	Elm_Genlist_Item_Class *itc, *itc2, *itc3;
	itc = elm_genlist_item_class_new();
	itc2 = elm_genlist_item_class_new();
	itc3 = elm_genlist_item_class_new();

	// Set item class for dialogue normal items
	// If str is heap memory, not static, use stringshare add.
	itc->item_style = eina_stringshare_add(str);
	itc->func.text_get = _gl_text_get;
	itc->func.content_get = _gl_content_get;
	itc->func.del = _gl_del;

	// Set item class for dialogue separator
	itc2->item_style = "dialogue/separator";
	itc2->func.del = _gl_del;

	// Set item class for dialogue title
	itc3->item_style = "dialogue/grouptitle";
	itc3->func.text_get = _gl_text_get_title;
	itc3->func.content_get = _gl_content_get_title;
	itc3->func.del = _gl_del;

	// Create genlist
	genlist = elm_genlist_add(ad->nf);
	// Optimize your application with appropriate genlist block size.
	elm_genlist_block_count_set(genlist, 14);

	// COMPRESS MODE
	// If multiline text (entry, textblock, sliding mode) is used, use compress mode for
	// compressing width.
	if (strstr(style, ".tb") || strstr(style, "multiline")) {
		printf("Compress mode enabled\n");
		elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	}
	// HOMOGENEOUS MODE
	// If item height is same when each style name is same,
	// Use homogeneous mode.
	if (!strstr(style, ".tb") && !strstr(style, "multiline")) {
		printf("Homogeneous mode enabled\n");
		elm_genlist_homogeneous_set(genlist, EINA_TRUE);
	}
	// REALIZATION MODE
	// password part is entry, for resize entry
	// Use realization mode
	if (!strcmp(style, "1text.4icon")) {
		elm_genlist_realization_mode_set(genlist, EINA_TRUE);
	}

	evas_object_smart_callback_add(genlist, "realized", _gl_realized, NULL);

	if (strstr(style, ".tb"))
		itc->func.text_get = _gl_text_get_textblock;
	else if (!strcmp(style, "multiline/1text")) {
		eina_stringshare_del(itc->item_style);
		itc->item_style = eina_stringshare_add(style);
		itc->func.text_get = _gl_text_help_dialogue_get;
	}

	if (strstr(style, ".thumb."))
		itc->func.content_get = _gl_icon_photo_get;

	if (!strcmp(style, "1icon") || !strcmp(style, "1icon/no_padding"))
		itc->func.content_get = _gl_icons_get;
	else if (!strcmp(style, "multiline/1text.1icon"))
		itc->func.content_get = _gl_icon_chk_get;
	else if (!strcmp(style, "1text.1icon") || !strcmp(style, "1text.2icon") ||
			 !strcmp(style, "1text.1icon.divider") ||
			 !strcmp(style, "1text.2icon.divider") ||
			 !strcmp(style, "2text.1icon.divider") ||
			 !strcmp(style, "2text.1icon.10") ||
			 !strcmp(style, "2text.1icon.10.tb"))
		itc->func.content_get = _gl_2icon_onoff_get;
	else if (!strcmp(style, "1text.1icon.5"))
		itc->func.content_get = _gl_icon_slider_get;
	else if (!strcmp(style, "2text.1icon.4") ||
			!strcmp(style, "2text.1icon.4.tb"))
		itc->func.content_get = _gl_icon_progressbar_get;
	else if (!strcmp(style, "1text.1icon.3") ||
			 !strcmp(style, "1text.2icon.10") ||
			 !strcmp(style, "2text.2icon"))
		itc->func.content_get = _gl_2icon_chk_get;
	else if (!strcmp(style, "2text.2icon.3") ||
			 !strcmp(style, "2text.2icon.3.tb"))
		itc->func.content_get = _gl_2icon_reveal_get;
	else if (!strcmp(style, "grouptitle"))
		itc->func.content_get = _gl_icon_grouptitle_get;
	else if (!strcmp(style, "1text.2icon.2"))
		itc->func.content_get = _gl_2icon_chk_reveal_get;
	else if (!strcmp(style, "1text.3icon") ||
			 !strcmp(style, "1text.3icon.2"))
		itc->func.content_get = _gl_3icon_chk_reveal_get;

	int num = NUM_OF_ITEMS;
	if (!strcmp(style, "1text.4icon")) {
		num = 20;
		itc->func.text_get = _gl_text_get_password;
		itc->func.content_get = _gl_icon_entry_get;
	} else if (!strcmp(style, "multiline/2text"))
		itc->func.text_get = _gl_text_multline_2text_get;

	for (index = 0; index < num; index++) {
		Item_Data *id = calloc(sizeof(Item_Data), 1);
		id->index = index;
		if (index == 0) {
			item = elm_genlist_item_append(genlist,
					itc,
					id,
					NULL,
					ELM_GENLIST_ITEM_NONE,
					_gl_sel,
					NULL);
		} else if (index % 6 == 1) { // Separator
			item = elm_genlist_item_append(genlist,
					itc2,
					id,
					NULL,
					ELM_GENLIST_ITEM_NONE,
					_gl_sel,
					NULL);
			elm_genlist_item_select_mode_set(item,
					ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		} else if (index % 6 == 2) { // Title
			item = elm_genlist_item_append(
					genlist,
					itc3,
					id,
					NULL,
					ELM_GENLIST_ITEM_NONE,
					_gl_sel,
					NULL);
			elm_genlist_item_select_mode_set(item,
					ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		} else {
			// Normal item
			item = elm_genlist_item_append(
					genlist,
					itc,
					id, NULL,
					ELM_GENLIST_ITEM_NONE,
					_gl_sel,
					NULL);
		}
		if (index == 0) elm_object_item_disabled_set(item, EINA_TRUE);
		id->item = item;
	}
	// Unref item class
	elm_genlist_item_class_free(itc);
	elm_genlist_item_class_free(itc2);
	elm_genlist_item_class_free(itc3);

	elm_naviframe_item_push(ad->nf, _(style), NULL, NULL, genlist, NULL);
}

void genlist_dialogue_item_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad;
	Evas_Object *list;
	int i;

	ad = (struct appdata *) data;
	if (ad == NULL) return;

	list = elm_list_add(ad->nf);
	elm_list_mode_set(list, ELM_LIST_COMPRESS);
	evas_object_smart_callback_add(list, "selected", _list_selected, NULL);

	for (i = 0; menu_its[i]; i++) {
		elm_list_item_append(list, menu_its[i], NULL, NULL, _create_genlist, ad);
	}
	elm_list_go(list);

	elm_naviframe_item_push(ad->nf, _("Items"), NULL, NULL, list, NULL);

	genlist_demo_names = genlist_get_demo_names();
}
