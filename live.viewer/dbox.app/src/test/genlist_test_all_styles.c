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
#include "Ecore_X.h"

/*********************************************************
  Genlist Styles
 ********************************************************/
#define NUM_OF_GENLIST_STYLES 4
#define NUM_OF_TIMES 50
static Elm_Genlist_Item_Class itc, itc2, itc3;
static char **genlist_demo_names = NULL;
static char **genlist_demo_country_names = NULL;

static void _create_genlist(void *data, Evas_Object *obj, void *event_info);
static char *menu_its[] = {
	"1text",
	"1text.1icon",
	"1text.1icon.2",
	"1text.1icon.3",
	"1text.1icon.4",
	"1text.1icon.5",
	"1text.2icon",
	"1text.2icon.2",
	"1text.2icon.3",
	"1text.2icon.4",
	"1text.2icon.5",
	"1text.2icon.6",
	"1text.3icon",
	"1text.3icon.2",
	"1text.3icon.3",
	"2text",
	"2text.2",
	"2text.3",
	"2text.4",
	"2text.5",
	"2text.6",
	"2text.7",
	"2text.1icon",
	"2text.1icon.2",
	"2text.1icon.3",
	"2text.1icon.4",
	"2text.1icon.5",
	"2text.1icon.6",
	"2text.1icon.7",
	"2text.1icon.8",
	"2text.1icon.9",
	"2text.1icon.10",
	"2text.1icon.11",
	"2text.1icon.12",
	"2text.2icon",
	"2text.2icon.2",
	"2text.2icon.3",
	"2text.2icon.4",
	"2text.2icon.5",
	"2text.2icon.6",
	"2text.2icon.7",
	"2text.2icon.8",
	"2text.3icon",
	"2text.3icon.2",
	"2text.3icon.3",
	"3text",
	"3text.1icon",
	"3text.1icon.2",
	"3text.5icon",

	"dialogue/1text",
	"dialogue/1text.1icon",
	"dialogue/1text.1icon.2",
	"dialogue/1text.1icon.3",
	"dialogue/1text.2icon",
	"dialogue/1text.2icon.2",
	"dialogue/1text.3icon",
	"dialogue/1icon",
	"dialogue/2text",
	"dialogue/2text.2",
	"dialogue/2text.3",
	"dialogue/2text.1icon",
	"dialogue/2text.1icon.2",
	"dialogue/2text.1icon.3",
	"dialogue/2text.1icon.5",
	"dialogue/2text.2icon",
	"dialogue/2text.2icon.2",
	"dialogue/2text.2icon.3",
	"dialogue/2text.2icon.4",

	"contact/dialogue/bg/2text.2icon",
	"contact/dialogue/bg/3text.2icon",
	"contact/dialogue/1title.1text",
	"contact/dialogue/1title.1text.2",
	"contact/dialogue/1title.1text.1icon",
	"convertor/dialogue/2text.4",
	"email/dialogue/2text.3icon",
	"setting/dialogue/2text.1icon.4",
	"samsungapps/dialogue/bg/5text.2icon",

	"multiline/1title.1text",
	"multiline/2text",
	"multiline/2text.1icon",
	"multiline/2text.1icon.2",
	"multiline/3text.1icon",
	"multiline/3text.1icon.2",
	"multiline/3text.2icon",
	"multiline/3text.2icon.2",

	/* do not delete below */
	NULL
};

#define NUM_OF_PHOTO	9
static char *photo_path[] = {
	"00_list_photo_default.png", "iu.jpg", "iu2.jpg", "koo.jpg", "top.jpg", "boa.jpg",
	"kimtaehee.jpg", "moon.jpg", "taeyon.jpg",
	NULL
};
static char *photo_path50[] = {
	"00_list_photo_default.png", "iu50.jpg", "iu2-50.jpg", "koo50.jpg", "top50.jpg", "boa50.jpg",
	"kimtaehee50.jpg", "moon50.jpg", "taeyon50.jpg",
	NULL
};
static char *photo_path75[] = {
	"00_list_photo_default.png", "iu75.jpg", "iu2-75.jpg", "koo75.jpg", "top75.jpg", "boa75.jpg",
	"kimtaehee75.jpg", "moon75.jpg", "taeyon75.jpg",
	NULL
};
static char *photo_path100[] = {
	"00_list_photo_default.png", "iu100.jpg", "iu2-100.jpg", "koo100.jpg", "top100.jpg", "boa100.jpg",
	"kimtaehee100.jpg", "moon100.jpg", "taeyon100.jpg",
	NULL
};
static char *photo_path133[] = {
	"00_list_photo_default.png", "iu133.jpg", "iu2-133.jpg", "koo133.jpg", "top133.jpg", "boa133.jpg",
	"kimtaehee133.jpg", "moon133.jpg", "taeyon133.jpg",
	NULL
};

#define NUM_OF_GROUPS 5
static char *title[] = {
	N_("Family"), NULL, N_("Colleagues"), NULL, N_("Classmates")
};

static char *times[] = {"10:00 AM", "10:01 AM", "10:02 AM", "10:03 AM", "10:04 AM", "10:05 AM",
	"3:50 PM", "3:51 PM", "3:52 PM", "3:53 PM"};

static char *messages[] = {"Good morning!",
	"It's you again. How do you do this?",
	"This is the princess who fell from the sky into my arms.",
	"Yes. We keep meeting like this. You just suddenly show up.",
	"We could make plans to meet.",
	"No, it's nicer this way. I hope we meet again suddenly.",
	"Don't you know that it takes so little to make me happy?",
	"Good morning, Princess!",
	"This is incredible. You owe me an explanation.",
	"No, you're the one who owes me an explanation. You've really got a crush on me. Where shall we go, Princess?"
};

static Evas_Object *progressbar[2];
static Ecore_Timer *progressbar_timer[2];
static double progressbar_period[2];

static Eina_Bool _progressbar_timer_cb(void *data)
{
	double value = 0.0;
	int prog_index = (int)data;

	value = elm_progressbar_value_get(progressbar[prog_index]);
	if (value == 1.0)
		value = 0.0;
	value = value + 0.015;
	elm_progressbar_value_set(progressbar[prog_index], value);

	return ECORE_CALLBACK_RENEW;
}

static void _unrealized_cb(void *data, Evas_Object *obj, void *event_info)
{
	int index = (int)elm_object_item_data_get(event_info);
	if ((index == 1) || (index == 2)) {
		if (progressbar_timer[index - 1]) {
			ecore_timer_del(progressbar_timer[index - 1]);
			progressbar_timer[index - 1] = NULL;
		}
	}
}

static void _gl_selected(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *it = (Elm_Object_Item *) elm_list_selected_item_get(obj);

	if (it == NULL)
	{
		fprintf((LOG_PRI(LOG_ERR) == LOG_ERR?stderr:stdout), "List item is NULL.\n");
		return;
	}

	elm_list_item_selected_set(it, EINA_FALSE);
}

static char *_gl_text_get(void *data, Evas_Object *obj, const char *part)
{
	int index = (int) data % NUM_OF_TIMES;

	if (!strcmp(part, "elm.text")) {
		return strdup(genlist_demo_names[index]);
	} else if (!strcmp(part, "elm.text.sub")) {
		return strdup(genlist_demo_country_names[index]);
	} else {
		return strdup(genlist_demo_names[index]);
	}
	return NULL;
}

static void _button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	printf("Button Clicked : %d\n", (int)data);
}

// Codes for removing elm_editfield : START
static void _changed_cb(void *data, Evas_Object *obj, void *event_info) // This callback is for showing(hiding) X marked button.
{
	if (elm_object_focus_get(data)) {
		if (elm_entry_is_empty(obj))
			elm_object_signal_emit(data, "elm,state,eraser,hide", "elm");
		else
			elm_object_signal_emit(data, "elm,state,eraser,show", "elm");
	}
}

static void _focused_cb(void *data, Evas_Object *obj, void *event_info) // Focused callback will show X marked button and hide guidetext.
{
	if (!elm_entry_is_empty(obj))
		elm_object_signal_emit(data, "elm,state,eraser,show", "elm");
	elm_object_signal_emit(data, "elm,state,guidetext,hide", "elm");
}

static void _unfocused_cb(void *data, Evas_Object *obj, void *event_info) // Unfocused callback will show guidetext and hide X marked button.
{
	if (elm_entry_is_empty(obj))
		elm_object_signal_emit(data, "elm,state,guidetext,show", "elm");
	elm_object_signal_emit(data, "elm,state,eraser,hide", "elm");
}

static void _eraser_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source) // When X marked button clicked, make string as empty.
{
	elm_entry_entry_set(data, "");
}

static Evas_Object *_singleline_editfield_add(Evas_Object *parent) // For single lined editfield without top title.
{
	Evas_Object *layout, *entry;

	layout = elm_layout_add(parent);
	elm_layout_theme_set(layout, "layout", "editfield", "default"); // Default editfield layout style without top title.

	entry = elm_entry_add(parent);
	ea_entry_selection_back_event_allow_set(entry, EINA_TRUE);
	elm_entry_scrollable_set(entry, EINA_TRUE); // Make entry as scrollable single line.
	elm_entry_single_line_set(entry, EINA_TRUE);
	evas_object_smart_callback_add(entry, "changed", _changed_cb, layout);
	evas_object_smart_callback_add(entry, "focused", _focused_cb, layout);
	evas_object_smart_callback_add(entry, "unfocused", _unfocused_cb, layout);

	elm_object_part_content_set(layout, "elm.swallow.content", entry);
	elm_object_part_text_set(layout, "elm.guidetext", _("Text Input")); // Set guidetext.
	elm_object_signal_callback_add(layout, "elm,eraser,clicked", "elm", _eraser_clicked_cb, entry);

	return layout;
}
// Codes for removing elm_editfield : END

static Evas_Object *_gl_content_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *icon;
	Evas_Object *edit_field = NULL;
	Evas_Object *btn = NULL;
	int index = (int)data;
	char buf[PATH_MAX];

	if (!strcmp(part, "elm.swallow.end")) {
		btn = elm_button_add(obj);
		elm_object_style_set(btn, "reveal");
		evas_object_smart_callback_add(btn, "clicked", _button_clicked_cb, (void *)index);
		return btn;
	} else if (!strcmp(part, "elm.photo") || !strcmp(part, "elm.thumbnail")) {
		sprintf(buf, ICON_DIR"/genlist/%s", photo_path[index % NUM_OF_PHOTO]);
		icon = elm_image_add(obj);
		elm_image_file_set(icon, buf, NULL);
		evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
		return icon;
	} else if (!strcmp(part, "elm.swallow.icon") || !strncmp(part, "elm.icon", 8)) {
		icon = elm_image_add(obj);
		elm_image_file_set(icon, ICON_DIR"/genlist/00_brightness_right.png", NULL);
		evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
		return icon;
	} else if (!strcmp(part, "elm.swallow.edit_field")) {
		edit_field = _singleline_editfield_add(obj);
		return edit_field;
	} else if (!strcmp(part, "elm.swallow.colorbar")) {
		icon = evas_object_rectangle_add(evas_object_evas_get(obj));
		evas_object_size_hint_align_set(icon, EVAS_HINT_FILL, EVAS_HINT_FILL);
		if (index%3 == 0) evas_object_color_set(icon, 80, 107, 207, 255);
		else if (index%3 == 1) evas_object_color_set(icon, 72, 136, 42, 255);
		else if (index%3 == 2) evas_object_color_set(icon, 204, 52, 52, 255);
		return icon;
	} else if (!strcmp(part, "elm.swallow.progress")) {
		int prog_index = index - 1;
		if (prog_index == 0 || prog_index == 1) {
			if (progressbar_timer[prog_index]) {
				ecore_timer_del(progressbar_timer[prog_index]);
				progressbar_timer[prog_index] = NULL;
				progressbar[prog_index] = NULL;
			}
			progressbar[prog_index] = elm_progressbar_add(obj);
			elm_object_style_set(progressbar[prog_index], "list_progress");
			elm_progressbar_horizontal_set(progressbar[prog_index], EINA_TRUE);
			elm_progressbar_value_set(progressbar[prog_index], 0.0);
			elm_progressbar_unit_format_set(progressbar[prog_index], NULL);
			progressbar_timer[prog_index] = ecore_timer_add(progressbar_period[prog_index], _progressbar_timer_cb, (void *)prog_index);
			return progressbar[prog_index];
		}
	}

	return NULL;
}

static Evas_Object *_gl_icon2_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *icon;

	if (!strcmp(part, "elm.icon") || !strcmp(part, "elm.icon.1")) {
		icon = elm_check_add(obj);
		elm_check_state_set(icon, EINA_TRUE);
		elm_object_style_set(icon, "on&off");
	} else {
		icon = elm_image_add(obj);
		elm_image_file_set(icon, ICON_DIR"/genlist/00_brightness_right.png", NULL);
		evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	}

	return icon;
}

static Evas_Object *_gl_icon3_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *icon;

	if (!strcmp(part, "elm.icon.2")) {
		icon = elm_check_add(obj);
		elm_check_state_set(icon, EINA_TRUE);
		elm_object_style_set(icon, "on&off");
	} else {
		icon = elm_image_add(obj);
		elm_image_file_set(icon, ICON_DIR"/genlist/00_brightness_right.png", NULL);
		evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	}

	return icon;
}

static Evas_Object *_gl_icon_50x50_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *image;
	int index = (int)data;
	char buf[PATH_MAX];

	if (!strcmp(part, "elm.thumbnail")) {
		sprintf(buf, ICON_DIR"/genlist/%s", photo_path50[index % NUM_OF_PHOTO]);
		image = evas_object_image_add(evas_object_evas_get(obj));
		evas_object_image_load_size_set(image, 50, 50);
		evas_object_image_fill_set(image, 0, 0, 50, 50);
		evas_object_image_file_set(image, buf, NULL);
		return image;
	}

	return NULL;
}

static Evas_Object *_gl_icon_75x100_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *image;
	int index = (int)data;
	char buf[PATH_MAX];

	if (!strcmp(part, "elm.thumbnail")) {
		sprintf(buf, ICON_DIR"/genlist/%s", photo_path75[index % NUM_OF_PHOTO]);
		image = evas_object_image_add(evas_object_evas_get(obj));
		evas_object_image_load_size_set(image, 75, 100);
		evas_object_image_fill_set(image, 0, 0, 75, 100);
		evas_object_image_file_set(image, buf, NULL);
		return image;
	}

	return NULL;
}

static Evas_Object *_gl_icon_100x100_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *image;
	int index = (int)data;
	char buf[PATH_MAX];

	if (!strcmp(part, "elm.thumbnail")) {
		sprintf(buf, ICON_DIR"/genlist/%s", photo_path100[index % NUM_OF_PHOTO]);
		image = evas_object_image_add(evas_object_evas_get(obj));
		evas_object_image_load_size_set(image, 100, 100);
		evas_object_image_fill_set(image, 0, 0, 100, 100);
		evas_object_image_file_set(image, buf, NULL);
		return image;
	}

	return NULL;
}

static Evas_Object *_gl_icon_133x100_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *image;
	int index = (int)data;
	char buf[PATH_MAX];

	if (!strcmp(part, "elm.thumbnail")) {
		sprintf(buf, ICON_DIR"/genlist/%s", photo_path133[index % NUM_OF_PHOTO]);
		image = evas_object_image_add(evas_object_evas_get(obj));
		evas_object_image_load_size_set(image, 133, 100);
		evas_object_image_fill_set(image, 0, 0, 133, 100);
		evas_object_image_file_set(image, buf, NULL);
		return image;
	}
	return NULL;
}

static char* _gl_dlg_text_get_title(void *data, Evas_Object *obj, const char *part)
{
	int index = ((int) data / 5) % NUM_OF_GROUPS;
	return strdup(title[index]);
}

static Evas_Object *_gl_dlg_content_get_title(void *data, Evas_Object *obj, const char *part)
{
	int index = (int) data;
	if (!strcmp(part, "elm.icon") && index == 0) {
		progressbar[0] = elm_progressbar_add(obj);
		elm_object_style_set(progressbar[0], "list_process_small");
		elm_progressbar_horizontal_set(progressbar[0], EINA_TRUE);
		elm_progressbar_pulse(progressbar[0], EINA_TRUE);
		return progressbar[0];
	}
	return NULL;
}

static Evas_Object *_gl_dlg_content_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *icon;

	if (!strcmp(part, "elm.icon") || !strcmp(part, "elm.icon.1") || !strcmp(part, "elm.icon.2") || !strcmp(part, "elm.icon.3")) {
		icon = elm_image_add(obj);
		elm_image_file_set(icon, ICON_DIR"/genlist/00_brightness_right.png", NULL);
		evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
		return icon;
	}
	return NULL;
}

static Evas_Object *_gl_dlg_icon2_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *icon;
	int index = ((int) data / 5) % NUM_OF_GROUPS;

	if (!strcmp(part, "elm.icon") || !strcmp(part, "elm.icon.2")) {
		if ((index % 2) == 1) {
			icon = elm_check_add(obj);
			elm_check_state_set(icon, EINA_TRUE);
			elm_object_style_set(icon, "on&off");
		} else {
			icon = elm_image_add(obj);
			elm_image_file_set(icon, ICON_DIR"/genlist/00_brightness_right.png", NULL);
			evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
		}
		return icon;
	} else if (!strcmp(part, "elm.icon.1")) {
		icon = elm_image_add(obj);
		elm_image_file_set(icon, ICON_DIR"/genlist/00_brightness_right.png", NULL);
		evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
		return icon;
	}
	return NULL;
}

static Evas_Object *_gl_dlg_icon3_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *icon;
	int index = ((int) data / 5) % NUM_OF_GROUPS;

	if (!strcmp(part, "elm.icon.2")) {
		if ((index % 2) == 1) {
			icon = elm_button_add(obj);
			elm_object_style_set(icon, "reveal");
		} else {
			icon = elm_image_add(obj);
			elm_image_file_set(icon, ICON_DIR"/genlist/00_brightness_right.png", NULL);
			evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
		}
		return icon;
	}
	else if (!strcmp(part, "elm.icon.3")) {
		if ((index % 2) == 1) {
			icon = elm_image_add(obj);
			elm_image_file_set(icon, ICON_DIR"/genlist/00_brightness_right.png", NULL);
			evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
		} else {
			icon = elm_button_add(obj);
			elm_object_style_set(icon, "reveal");
		}
		return icon;
	} else if (!strcmp(part, "elm.icon.1")) {
		icon = elm_image_add(obj);
		elm_image_file_set(icon, ICON_DIR"/genlist/00_brightness_right.png", NULL);
		evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
		return icon;
	}
	return NULL;
}

static Evas_Object *_gl_dlg_icon4_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *icon, *icon2;
	int index = (int)data % 5;

	if (!strcmp(part, "elm.icon")) {
		if (index == 1) {
			icon = elm_slider_add(obj);
			elm_slider_indicator_show_set(icon, EINA_TRUE);
			elm_slider_min_max_set(icon, 0, 9);
			elm_slider_indicator_format_set(icon, "%1.0f");
			elm_slider_value_set(icon, 7);

			icon2 = elm_image_add(obj);
			elm_image_file_set(icon2, ICON_DIR"/00_slider_btn_volume01.png", NULL);
			evas_object_size_hint_aspect_set(icon2, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
			elm_object_content_set(icon, icon2);

			icon2 = elm_image_add(obj);
			elm_image_file_set(icon2, ICON_DIR"/00_slider_btn_volume02.png", NULL);
			evas_object_size_hint_aspect_set(icon2, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
			elm_object_part_content_set(icon, "end", icon2);

			return icon;
		} else if (index == 2) {
			icon = elm_image_add(obj);
			elm_image_file_set(icon, ICON_DIR"/genlist/00_brightness_right.png", NULL);
			evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
			return icon;
		} else if (index == 3) {
			icon = elm_image_add(obj);
			elm_image_file_set(icon, ICON_DIR"/genlist/00_list_photo_default.png", NULL);
			evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
			return icon;
		} else if (index == 4) {
			icon = elm_image_add(obj);
			elm_image_file_set(icon, ICON_DIR"/genlist/main.png", NULL);
			evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
			return icon;
		}
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
	int index;
	Elm_Object_Item *item = (Elm_Object_Item *)event_info;

	if (item != NULL) {
		elm_genlist_item_selected_set(item, EINA_FALSE);
		index = (int)elm_object_item_data_get(item);
		printf("[Genlist] Selected Text : %s\n", genlist_demo_names[index]);
	}
}

static char *_gl_mtl_label2_get(void *data, Evas_Object *obj, const char *part)
{
	int index = ((int)data) % NUM_OF_GENLIST_DEMO_NAMES;

	return strdup(genlist_demo_names[index]);
}

static char* _gl_mtl_text_get(void *data, Evas_Object *obj, const char *part)
{
	int index = (int)data;

	if (!strcmp(part, "elm.text.1") || !strcmp(part, "elm.title") ) {
		return strdup(times[index % EINA_C_ARRAY_LENGTH(times)]);
	} else {
		return strdup(messages[index % EINA_C_ARRAY_LENGTH(messages)]);
	}
	return NULL;
}

static Evas_Object *_gl_mtl_content_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *icon;

	icon = elm_image_add(obj);
	elm_image_file_set(icon, ICON_DIR"/genlist/00_brightness_right.png", NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	return icon;
}

static char *_gl_app_text_get(void *data, Evas_Object *obj, const char *part)
{
	int index = ((int)data);
	return strdup(genlist_demo_names[index]);
}

static char *_gl_app_label2_get(void *data, Evas_Object *obj, const char *part)
{
	int index = (int)data;
	if (!strcmp(part, "elm.text.1") || !strcmp(part, "elm.title") ) {
		return strdup(times[index]);
	} else {
		return strdup(messages[index]);
	}
	return NULL;
}

static char* _gl_app_text_get_title(void *data, Evas_Object *obj,
		const char *part)
{
	int index = ((int)data / 5);
	return strdup(title[index]);
}

static Evas_Object *_gl_app_content_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *icon;

	if (!strcmp(part, "elm.icon.1")) {
		icon = elm_image_add(obj);
		elm_image_file_set(icon, ICON_DIR"/genlist/00_list_photo_default.png", NULL);
		evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
		return icon;
	} else {
		icon = elm_image_add(obj);
		elm_image_file_set(icon, ICON_DIR"/genlist/00_brightness_right.png", NULL);
		evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
		return icon;
	}
	return NULL;
}

static Evas_Object *_gl_app_icon2_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *box, *rect;

	if (!strcmp(part, "elm.icon")) {
		box = elm_box_add(obj);
		elm_box_horizontal_set(box, EINA_TRUE);

		rect = evas_object_rectangle_add(evas_object_evas_get(obj));
		evas_object_color_set(rect, 204, 52, 52, 255);
		evas_object_size_hint_min_set(rect, 200, 10);
		elm_box_pack_end(box, rect);
		evas_object_show(rect);

		rect = evas_object_rectangle_add(evas_object_evas_get(obj));
		evas_object_color_set(rect, 72, 136, 42, 255);
		evas_object_size_hint_min_set(rect, 150, 10);
		elm_box_pack_end(box, rect);
		evas_object_show(rect);

		rect = evas_object_rectangle_add(evas_object_evas_get(obj));
		evas_object_color_set(rect, 80, 107, 207, 255);
		evas_object_size_hint_min_set(rect, 100, 10);
		elm_box_pack_end(box, rect);
		evas_object_show(rect);

		return box;
	}
	return NULL;
}

static Evas_Object *_gl_app_icon4_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *icon;
	int index = ((int) data / 5) % NUM_OF_GROUPS;

	if (!strcmp(part, "elm.icon.1") || !strcmp(part, "elm.icon.2")) {
		icon = elm_image_add(obj);
		elm_image_file_set(icon, ICON_DIR"/genlist/00_brightness_right.png", NULL);
		evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
		return icon;
	} else if (!strcmp(part, "elm.swallow.colorbar")) {
		icon = evas_object_rectangle_add(evas_object_evas_get(obj));
		evas_object_size_hint_align_set(icon, EVAS_HINT_FILL, EVAS_HINT_FILL);
		if (index%3 == 0) evas_object_color_set(icon, 80, 107, 207, 255);
		else if (index%3 == 1) evas_object_color_set(icon, 72, 136, 42, 255);
		else if (index%3 == 2) evas_object_color_set(icon, 204, 52, 52, 255);
		return icon;
	}
	return NULL;
}

static Evas_Object* _create_app_genlist(const char *style, void *data)
{
	int index = 0;
	struct appdata *ad = data;
	Elm_Object_Item *item = NULL;
	Evas_Object *genlist;
	Eina_Bool dlg_bg = EINA_FALSE;

	if (!strstr(style, "/bg/"))
		dlg_bg = EINA_TRUE;

	// Create genlist
	genlist = elm_genlist_add(ad->nf);

	if (!strcmp(style, "dialogue/1title.1text.2")) {
		elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);

		itc.func.text_get = _gl_app_label2_get;
		itc.func.content_get = NULL;
		itc.func.state_get = NULL;
		itc.func.del = NULL;
	}

	// Set item class for dialogue separator
	itc2.item_style = "dialogue/seperator";
	itc2.func.text_get = NULL;
	itc2.func.content_get = NULL;
	itc2.func.state_get = NULL;
	itc2.func.del = NULL;

	// Set item class for dialogue title
	itc3.item_style = "dialogue/grouptitle";
	itc3.func.text_get = _gl_app_text_get_title;
	itc3.func.content_get = NULL;
	itc3.func.state_get = NULL;
	itc3.func.del = NULL;

	if (!strcmp(style, "dialogue/1title.1text.2")) {
		for (index = 0; index < 2; index++)	{
			if (index % 5 == 0) {
				if (title[index/5%NUM_OF_GROUPS])
					item = elm_genlist_item_append(genlist, &itc3, (void *) index, NULL, ELM_GENLIST_ITEM_NONE, _gl_sel, NULL);
				else
					item = elm_genlist_item_append(genlist, &itc2, NULL, NULL, ELM_GENLIST_ITEM_NONE, _gl_sel, NULL);
				elm_genlist_item_select_mode_set(item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
			} else {
				elm_genlist_item_append(genlist, &itc, (void *) index, NULL, ELM_GENLIST_ITEM_NONE, _gl_sel, NULL);
				break;
			}
		}
	} else {
		for (index = 0; index < 2; index++) {
			if (index % 5 == 0 && !dlg_bg) {
				if (title[index/5%NUM_OF_GROUPS])
					item = elm_genlist_item_append(genlist, &itc3, (void *) index, NULL, ELM_GENLIST_ITEM_NONE, _gl_sel, NULL);
				else
					item = elm_genlist_item_append(genlist, &itc2, NULL, NULL, ELM_GENLIST_ITEM_NONE, _gl_sel, NULL);
				elm_genlist_item_select_mode_set(item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
			} else {
				elm_genlist_item_append(genlist, &itc, (void *) index, NULL, ELM_GENLIST_ITEM_NONE, _gl_sel, NULL);
				break;
			}
		}
	}

	//elm_object_style_set(ad->bg, "group_list");
	return genlist;
}

static void _listing_style(void *data, void *stylename)
{
	int index = 0;
	Elm_Object_Item *item = NULL;
	Evas_Object *genlist = NULL;
	const char *style = (const char*)stylename;
	struct appdata *ad = (struct appdata *)data;

	if (style[0] >= '0' && style[0] <= '9') {
		// Create genlist
		genlist = elm_genlist_add(ad->nf);
		evas_object_smart_callback_add(genlist, "unrealized", _unrealized_cb, NULL);

		// Set genlist block size.
		// Optimize your application with appropriate genlist block size.
		elm_genlist_block_count_set(genlist, 14);

		// Set genlist item class
		itc.item_style = style;
		itc.func.text_get = _gl_text_get;
		itc.func.content_get = _gl_content_get;
		itc.func.state_get = _gl_state_get;
		itc.func.del = _gl_del;

		// Init Private Data
		progressbar[0] = NULL;
		progressbar_timer[0] = NULL;
		progressbar_period[0] = 0.1;
		progressbar[1] = NULL;
		progressbar_timer[1] = NULL;
		progressbar_period[1] = 0.05;

		if (!strcmp(style, "1text.1icon")) {
			itc.func.content_get = _gl_icon2_get;
		} else if (!strcmp(style, "1text.2icon")) {
			itc.func.content_get = _gl_icon3_get;
		}

		// Set content_get callback for some styles
		if (!strcmp(style, "2line_thumbnail_50x50")) {
			itc.func.content_get = _gl_icon_50x50_get;
		} else if (!strcmp(style, "2line_thumbnail_75x100")) {
			itc.func.content_get = _gl_icon_75x100_get;
		} else if (!strcmp(style, "2line_thumbnail_100x100")) {
			itc.func.content_get = _gl_icon_100x100_get;
		} else if (!strcmp(style, "2line_thumbnail_133x100")) {
			itc.func.content_get = _gl_icon_133x100_get;
		}

		// Append items
		for (index = 0; index < 1; index++) {
			item = elm_genlist_item_append(
				genlist,			// genlist object
				&itc,				// item class
				(void *) index,		// data
				NULL,
				ELM_GENLIST_ITEM_NONE,
				_gl_sel,
				NULL
				);
		}
	} else if (!strncmp(style, "dialogue/", 9)) {
		// Create genlist
		genlist = elm_genlist_add(ad->nf);
		evas_object_smart_callback_add(genlist, "unrealized", _unrealized_cb, NULL);

		// Set genlist block size.
		// Optimize your application with appropriate genlist block size.
		elm_genlist_block_count_set(genlist, 14);

		// Set item class for dialogue normal items
		itc.item_style = style;
		itc.func.text_get = _gl_text_get;
		itc.func.content_get = _gl_dlg_content_get;
		itc.func.state_get = NULL;
		itc.func.del = NULL;

		if (!strcmp(style, "dialogue/1text.1icon") || !strcmp(style, "dialogue/1text.2icon"))
			itc.func.content_get = _gl_dlg_icon2_get;
		else if (!strcmp(style, "dialogue/1text.2icon.2") || !strcmp(style, "dialogue/1text.3icon"))
			itc.func.content_get = _gl_dlg_icon3_get;
		else if (!strcmp(style, "dialogue/1icon"))
			itc.func.content_get = _gl_dlg_icon4_get;

		// Set item class for dialogue seperator
		itc2.item_style = "dialogue/seperator";
		itc2.func.text_get = NULL;
		itc2.func.content_get = NULL;
		itc2.func.state_get = NULL;
		itc2.func.del = NULL;

		// Set item class for dialogue title
		itc3.item_style = "dialogue/grouptitle";
		itc3.func.text_get = _gl_dlg_text_get_title;
		itc3.func.content_get = _gl_dlg_content_get_title;
		itc3.func.state_get = NULL;
		itc3.func.del = NULL;

		for (index = 0; index < 2; index++) {
			if (index % 5 == 0) { //title
				if (title[index/5%NUM_OF_GROUPS])
					elm_genlist_item_append(genlist, &itc3, (void *) index, NULL, ELM_GENLIST_ITEM_NONE, _gl_sel, NULL);
				else
					elm_genlist_item_append(genlist, &itc2, NULL, NULL, ELM_GENLIST_ITEM_NONE, _gl_sel, NULL);
				elm_genlist_item_select_mode_set(item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

			} else { //item
				elm_genlist_item_append(genlist, &itc, (void *) index, NULL, ELM_GENLIST_ITEM_NONE, _gl_sel, NULL);
			}
		}
	} else if (!strncmp(style, "multiline/1", 11) || !strncmp(style, "multiline/2", 11)) {
		// Create genlist
		genlist = elm_genlist_add(ad->nf);
		evas_object_smart_callback_add(genlist, "unrealized", _unrealized_cb, NULL);

		// Set genlist block size.
		// Optimize your application with appropriate genlist block size.
		elm_genlist_block_count_set(genlist, 14);

		// To use multiline textblock/entry/editfield in genlist, set height_for_width mode
		// then the item's height is calculated while the item's width fits to genlist width.
		elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);

		// Set multiline item class
		itc.item_style = style;
		itc.func.text_get = _gl_mtl_text_get;
		itc.func.content_get = _gl_mtl_content_get;
		itc.func.state_get = NULL;
		itc.func.del = NULL;

		elm_genlist_item_append(genlist, &itc, (void *) index, NULL, ELM_GENLIST_ITEM_NONE, _gl_sel, NULL);
	} else if (!strncmp(style, "multiline/3", 11)) {
		// Create genlist
		genlist = elm_genlist_add(ad->nf);
		evas_object_smart_callback_add(genlist, "unrealized", _unrealized_cb, NULL);

		// Set genlist block size.
		// Optimize your application with appropriate genlist block size.
		elm_genlist_block_count_set(genlist, 14);

		// Set multiline item class
		itc.item_style = style;
		itc.func.text_get = _gl_mtl_label2_get;
		itc.func.content_get = _gl_mtl_content_get;
		itc.func.state_get = NULL;
		itc.func.del = NULL;

		// Append items
		for (index = 0; index < 1; index++) {
			elm_genlist_item_append(genlist, &itc, (void *) index, NULL, ELM_GENLIST_ITEM_NONE, _gl_sel, NULL);
		}
	} else if (!strncmp(style, "contact/", 8)) {
		// Set item class for dialogue normal items
		itc.item_style = style+8;
		itc.func.text_get = _gl_app_text_get;
		itc.func.content_get = _gl_app_content_get;
		itc.func.state_get = NULL;
		itc.func.del = NULL;

		genlist = _create_app_genlist(style+8, ad);
	} else if (!strncmp(style, "convertor/", 10)) {
		itc.item_style = style+10;
		itc.func.text_get = _gl_app_text_get;
		itc.func.content_get = NULL;
		itc.func.state_get = NULL;
		itc.func.del = NULL;

		genlist = _create_app_genlist(style+10, ad);
	} else if (!strncmp(style, "email/", 6)) {
		itc.item_style = style+6;
		itc.func.text_get = _gl_app_text_get;
		itc.func.content_get = _gl_app_icon4_get;
		itc.func.state_get = NULL;
		itc.func.del = NULL;

		genlist = _create_app_genlist(style+6, ad);
	} else if (!strncmp(style, "setting/", 8)) {
		// Set item class for dialogue normal items
		itc.item_style = style+8;
		itc.func.text_get = _gl_app_text_get;
		itc.func.content_get = _gl_app_icon2_get;
		itc.func.state_get = NULL;
		itc.func.del = NULL;

		genlist = _create_app_genlist(style+8, ad);
	} else if (!strncmp(style, "samsungapps/", 12)) {
		itc.item_style = style+12;
		itc.func.text_get = _gl_app_text_get;
		itc.func.content_get = _gl_app_content_get;
		itc.func.state_get = NULL;
		itc.func.del = NULL;

		genlist = _create_app_genlist(style+12, ad);
	}

	elm_naviframe_item_push(ad->nf, _(style), NULL, NULL, genlist, NULL);
}

static void _do_save(void *data, int n)
{
	struct appdata *ad = (struct appdata *)data;
	Evas *e;
	Evas_Object *output;
	int w, h, i;
	int sx, sy;
	char file[256];
	Ecore_X_Image *img;
	Ecore_X_Window_Attributes att;
	int bpl = 0, rows = 0, bpp = 0;
	unsigned char *src;

/*
  sx = 0; sy = 0;
  w = 480; h = 800;
*/
	sx = 0; sy = 55;
	w = 480;
	char *style = menu_its[n];
	if (style[0] >= '0' && style[0] <= '9') h = 71;
	else if (!strncmp(style, "multiline/", 10)) h = 111;
	else if (!strncmp(style, "dialogue/", 9)) h = 127;
	else h = 163;

	sprintf(file, "./%s.png", menu_its[n]);
	for (i = 2; i < strlen(file); i++) {
		if (file[i] == '/')
			file[i] = '.';
	}

	memset(&att, 0, sizeof(Ecore_X_Window_Attributes));
	ecore_x_window_attributes_get(elm_win_xwindow_get(ad->win_main), &att);
	img = ecore_x_image_new(w, h, att.visual, att.depth);
	ecore_x_image_get(img, elm_win_xwindow_get(ad->win_main), sx, sy, 0, 0, w, h);
	src = ecore_x_image_data_get(img, &bpl, &rows, &bpp);

	e = evas_object_evas_get(ad->win_main);
	output = evas_object_image_add(e);

	evas_object_image_data_set(output, NULL);
	evas_object_image_size_set(output, w, h);
	evas_object_image_fill_set(output, 0, 0, w, h);
	evas_object_image_filled_set(output, EINA_TRUE);
	evas_object_image_alpha_set(output, EINA_TRUE);
	evas_object_image_data_copy_set(output, src);
	evas_object_image_data_update_add(output, 0, 0, w, h);
	evas_object_image_save(output, file, NULL, NULL);

	if (!src) free(src);
	ecore_x_image_free(img);

	fprintf(stderr, "## filename = %s is done\n", file);
}

static Eina_Bool _running_list(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	static int rcount = 0;
	static int push = 0;

	if ((!menu_its[rcount])) {
		return ECORE_CALLBACK_CANCEL;
	}
	if (!push) {
		_listing_style(ad, menu_its[rcount]);
		push = EINA_TRUE;
	}
	else {
		_do_save(ad, rcount);
		elm_naviframe_item_pop(ad->nf);
		push = EINA_FALSE;
		rcount++;
	}

	return ECORE_CALLBACK_RENEW;
}

static void _create_genlist(void *data, Evas_Object *obj, void *event_info)
{
	ecore_timer_add(1.0, _running_list, data);
}

void genlist_test_all_styles_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad;
	Evas_Object *list;

	ad = (struct appdata *) data;
	if (ad == NULL) return;

	list = elm_list_add(ad->nf);
	elm_list_mode_set(list, ELM_LIST_COMPRESS);
	evas_object_smart_callback_add(list, "selected", _gl_selected, NULL);

	elm_list_item_append(list, "All Item Styles", NULL, NULL, NULL, ad);
	elm_list_go(list);

	elm_naviframe_item_push(ad->nf, _("TEST All Item Styles"), NULL, NULL, list, NULL);

	_create_genlist(ad, obj, event_info);

	genlist_demo_names = genlist_get_demo_names();
	genlist_demo_country_names = genlist_get_demo_country_names();
}
