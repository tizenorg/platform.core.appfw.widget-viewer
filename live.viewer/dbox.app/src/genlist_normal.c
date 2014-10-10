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
  Genlist Styles
 ********************************************************/
#define COLOR_MAX 5
#define NUM_OF_ITEMS 8000
#define TEST_SIGNALS 0

typedef struct _Item_Data
{
	int index;
	int highlighted;
	Elm_Object_Item *item;
	Eina_Bool checked;
	Eina_Bool stared;
} Item_Data;

static char *menu_its[] = {
	"groupindex",
	"groupindex.icon",
	"1icon",
	"1icon/no_padding",
	"1icon/with_no_line",
	"1icon/no_padding_line",
	/*** 1line styles ***/
	"1text",
	"1text.tb",
	"2text",
	"1text.1icon",
	"1text.1icon.6",
	"1text.1icon.7",
	"1text.1icon.5",
	"1text.1icon.5.thumb.circle",
	"1text.1icon.5.thumb.square",
	"1text.2icon.3",
	"1text.2icon.3.tb",
	"1text.1icon.2",
	"1text.1icon.2.tb",
	"2text.1icon",
	"2text.1icon.tb",
	"1text.2icon",
	"1text.2icon.tb",
	"1text.2icon.6",
	"1text.2icon.6.tb",
	"1text.1icon.3",
	"1text.1icon.3.tb",
	"1text.2icon.4",
	"1text.2icon.4.tb",
	"1text.2icon.2",
	"1text.3icon",
	"1text.2icon.9",
	"1text.3icon.2",
	"1text.1icon.divider",
	"1text.2icon.divider",
	/*** 2line styles ***/
	"2text.2",
	"2text.2.tb",
	"2text.3",
	"3text",
	"3text.tb",
	"2text.1icon.2",
	"2text.1icon.2.tb",
	"2text.1icon.10",
	"2text.1icon.10.tb",
	"3text.1icon",
	"3text.1icon.tb",
	"3text.2icon",
	"3text.2icon.tb",
	"2text.1icon.4",
	"2text.1icon.4.thumb.circle",
	"2text.1icon.4.thumb.square",
	"2text.1icon.4.tb",
	"3text.1icon.2",
	"3text.1icon.2.tb",
	"2text.1icon.8",
	"2text.2icon.4",
	"2text.2icon.4.tb",
	"2text.2icon.6",
	"2text.2icon.6.tb",
	"2text.2icon.8",
	"2text.2icon.8.tb",
	"2text.2icon.progress",
	"3text.3icon.progress",
	"2text.1icon.divider",
	"2text.1icon.12",
	"2text.1icon.12.tb",
	"2text.2icon.7",
	"2text.2icon.5",
	"2text.3icon.4",
	"4text.1icon.1",
	"4text.1icon.1.tb",
	"4text.1icon.2",
	"4text.1icon.2.tb",
	"4text.1",
	"4text.1.tb",
	"4text.2",
	"3text.1icon.3",
	"3text.1icon.3.tb",
	"4text.1icon.3",
	"3text.1icon.1",
	"2text.2icon.1",
	"2text.1icon.1",
	"email.list.test",
	"email.list",
	"email.list.conversation",
	"email.outbox",
	"email.outbox.conversation",
	"email.list(lazy)",
	"multiline/2text.5icon",
	"1text/popup",
	"1text.1icon.5.thumb.circle/popup",
	"1text.1icon.5.thumb.square/popup",
	"2text.1icon.4.thumb.circle/popup",
	"2text.1icon.4.thumb.square/popup",
	"1text.1icon.divider/popup",
	"1text.2icon.divider/popup",
	/* do not delete below */
	NULL
};

extern char *genlist_demo_names[];
extern char *genlist_long_texts[];
extern char *genlist_times[];
extern char *genlist_messages[];
static Eina_Bool scrolling;
static Eina_Bool lazy;

static const char *color_set[COLOR_MAX] =
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
	// FIXME: Unrealized callback can be called after this.
	// Accessing Item_Data can be dangerous on unrealized callback.
	Item_Data *id = data;
	if (id) free(id);
}

static void _list_selected(void *data, Evas_Object *obj, void *ei)
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
_gl_text_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024];
	Item_Data *id = data;
	int index = id->index;

	if (!strcmp(part, "elm.text.2")) index++;

	snprintf(buf, 1023, "%s:%s", part, genlist_demo_names[index%NUM_OF_GENLIST_DEMO_NAMES]);
	return strdup(buf);
}

static char *
_gl_email_text_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024];
	Item_Data *id = data;
	int index = id->index;

	if (!strcmp(part, "elm.text.4")) {
		index = index%4;
		if (index == 0) snprintf(buf, 1023, "77");
		else if (index == 1) snprintf(buf, 1023, "9");
		else if (index == 2) snprintf(buf, 1023, "1");
		else  snprintf(buf, 1023, "...");
	} else if (!strcmp(part, "elm.text.1") || !strcmp(part, "elm.text.3")) {
		snprintf(buf, 1023, "<match>%s</match>:%s", part,
				genlist_long_texts[index%NUM_OF_GENLIST_LONG_TEXTS]);
	} else if (!strcmp(part, "elm.text.2")) {
		snprintf(buf, 1023, "<match>%s</match>:%s", part,
				genlist_demo_names[index%NUM_OF_GENLIST_DEMO_NAMES]);
	} else snprintf(buf, 1023, "%s:11:30 PM", part);

	return strdup(buf);
}

static char *_gl_text_get_textblock(void *data, Evas_Object *obj, const char *part)
{
	Item_Data *id = data;
	int index = id->index;
	char buf[PATH_MAX] = {0, };
	const char *color;
	int size;
	int val;

	if (!strcmp(part, "elm.text.4") || !strcmp(part, "elm.text"))
		val = index+2;
	else if (!strcmp(part, "elm.text.2"))
		val = index+4;
	else if (!strcmp(part, "elm.text.3"))
		val = index+6;
	else val = index+8;

	color = color_set[val % COLOR_MAX];
	size = (val%5)*20 + 32;

	if (index % 10 == 0) return _gl_text_get(data, obj, part);
	// if textblock is used, application should change color when item is highlighted.
	if (id->highlighted) // T023P is pressed color of list main text
		sprintf(buf, "<font_size=%d><color_class=T023P>%s:%s</color></font_size>",
				size, part, genlist_demo_names[index%NUM_OF_GENLIST_DEMO_NAMES]);
	else
		sprintf(buf, "<font_size=%d><color=%s>%s:%s</color></font_size>",
				size, color, part, genlist_demo_names[index%NUM_OF_GENLIST_DEMO_NAMES]);
	return strdup(buf);
}

static char *_gl_text_get_bubble_textblock(void *data, Evas_Object *obj, const char *part)
{
	if (!strcmp(part, "elm.text.4"))
		return _gl_text_get(data, obj, part);
	else return _gl_text_get_textblock(data, obj, part);
}

static char* _gl_multiline_text_get(void *data, Evas_Object *obj, const char *part)
{
	int index = ((int)data) % NUM_OF_GENLIST_TIMES;

	if (!strcmp(part, "elm.text.1") || !strcmp(part, "elm.title") ) {
		return strdup(genlist_times[index]);
	} else {
		return strdup(genlist_messages[index]);
	}
	return NULL;
}

static Evas_Object *
_gl_icon_colorbar_get(void *data, Evas_Object *obj, const char *part)
{
	Item_Data *id = data;
	int index = id->index;
	Evas_Object *icon = evas_object_rectangle_add(evas_object_evas_get(obj));
	evas_object_size_hint_align_set(icon, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	if (index%3 == 0) evas_object_color_set(icon, 80, 107, 207, 255);
	else if (index%3 == 1) evas_object_color_set(icon, 72, 136, 42, 255);
	else if (index%3 == 2) evas_object_color_set(icon, 204, 52, 52, 255);
	return icon;
}

static Evas_Object *
_gl_icon_progressbar_get(void *data, Evas_Object *obj, const char *part)
{
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
_icon_email_chk_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *icon = elm_check_add(obj);
	evas_object_propagate_events_set(icon, EINA_FALSE);
	return icon;
}

static Evas_Object *
_gl_icon_star_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *icon = elm_image_add(obj);
	elm_image_file_set(icon, ICON_DIR"/genlist/00_icon_favorite_off_45x45.png", NULL);
	evas_object_size_hint_align_set(icon, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	return icon;
}

static Evas_Object *_gl_content_get(void *data, Evas_Object *obj, const char *part);

static void _clicked(void *data, Evas_Object *obj, void *ei)
{
	printf("Button clicked\n");
	Item_Data *id = data;
	const Elm_Genlist_Item_Class *itc;
	itc = elm_genlist_item_item_class_get(id->item);

	if (itc && !strcmp(itc->item_style, "1text.1icon")) {
		printf("Item Class updated\n");
		// accessibility test
		elm_genlist_item_item_class_update(id->item, itc);

		Evas_Object *acc = elm_object_item_access_object_get(id->item);
		// because button is removed, set highlight the item itself.
		elm_access_highlight_set(acc);
	}
}

static Evas_Object *
_gl_icon_btn_text_get(void *data, Evas_Object *obj, const char *part)
{
	Item_Data *id = data;
	Evas_Object *icon = elm_button_add(obj);
	elm_object_text_set(icon, "Text button");
	evas_object_propagate_events_set(icon, EINA_FALSE);
	evas_object_smart_callback_add(icon, "clicked", _clicked, id);
	return icon;
}

static Evas_Object *
_gl_icon_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *icon = elm_image_add(obj);
	elm_image_file_set(icon, ICON_DIR"/genlist/iu.jpg", NULL);
	evas_object_size_hint_align_set(icon, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	return icon;
}

static Evas_Object *
_gl_icon_photo_get(void *data, Evas_Object *obj, const char *part)
{
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
	if (!strcmp(part, "elm.swallow.end")) {
		return _gl_icon_reveal_get(data, obj, part);
	} else if (!strcmp(part, "elm.swallow.colorbar")) {
		return _gl_icon_colorbar_get(data, obj, part);
	} else if (!strcmp(part, "elm.swallow.progress")) {
		return _gl_icon_progressbar_get(data, obj, part);
	}
	return _gl_icon_get(data, obj, part);
}

static Evas_Object *
_gl_2icon_btn_text_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *icon;
	if (!strcmp(part, "elm.icon") || !strcmp(part, "elm.icon.2"))
		icon = _gl_icon_btn_text_get(data, obj, part);
	else icon = _gl_content_get(data, obj, part);
	return icon;
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
	int index = (id->index)%7;
	Evas_Object *icon;
	if (0 == index) {
		icon = _gl_icon_progressbar_get(data, obj, part);
	} else if (1 == index) {
		icon = _gl_icon_reveal_get(data, obj, part);
	} else if (2 == index) {
		icon = _gl_icon_onoff_get(data, obj, part);
	} else if (3 == index) {
		icon = _gl_icon_chk_get(data, obj, part);
	} else if (4 == index) {
		icon = _gl_icon_btn_text_get(data, obj, part);
	} else {
		icon = _gl_icon_photo_get(data, obj, part);
	}
	return icon;
}

static void
_email_check_clicked(void *data, Evas* e, Evas_Object *obj, void *ei)
{
	printf("email check area clicked\n");
	Item_Data *id = data;
	id->checked = !id->checked;
	if (id->checked)
		elm_object_item_signal_emit(id->item, "elm,state,check,show", "");
	else
		elm_object_item_signal_emit(id->item, "elm,state,check,hide", "");
}

static void
_email_touch_clicked(void *data, Evas* e, Evas_Object *obj, void *ei)
{
	printf("email touch area clicked\n");
	Item_Data *id = data;
	id->stared = !id->stared;
	if (id->stared)
		elm_object_item_signal_emit(id->item, "elm,state,icon4,enable", "");
	else
		elm_object_item_signal_emit(id->item, "elm,state,icon4,disable", "");
}

static Evas_Object *
_gl_email_icon_test_get(void *data, Evas_Object *obj, const char *part)
{
	Item_Data *id = data;
	if (!strcmp(part, "elm.check.touch")) {
		Evas_Object *rect = evas_object_rectangle_add(evas_object_evas_get(obj));
		evas_object_size_hint_align_set(rect, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(rect, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_propagate_events_set(rect, EINA_FALSE);
		evas_object_color_set(rect, 64, 0, 0, 64);
		evas_object_event_callback_add(rect, EVAS_CALLBACK_MOUSE_UP, _email_check_clicked, id);
		return rect;
	} else if (!strcmp(part, "elm.icon.4.touch")) {
		Evas_Object *rect = evas_object_rectangle_add(evas_object_evas_get(obj));
		evas_object_size_hint_align_set(rect, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(rect, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_propagate_events_set(rect, EINA_FALSE);
		evas_object_color_set(rect, 64, 0, 0, 64);
		evas_object_event_callback_add(rect, EVAS_CALLBACK_MOUSE_UP, _email_touch_clicked, id);
		return rect;
	} else return _gl_content_get(data, obj, part);
	return NULL;
}


static Evas_Object *
_gl_email_icon_get(void *data, Evas_Object *obj, const char *part)
{
	Item_Data *id = data;
	int index = id->index;
	double scale = elm_config_scale_get();
	if (strstr(part, "touch")) {
		Evas_Object *rect = evas_object_rectangle_add(evas_object_evas_get(obj));
		evas_object_size_hint_align_set(rect, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(rect, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_event_callback_add(rect, EVAS_CALLBACK_MOUSE_UP, _email_touch_clicked, id);
		evas_object_propagate_events_set(rect, EINA_FALSE);
		evas_object_color_set(rect, 64, 0 ,0, 64);
		return rect;
	} else if (!strcmp(part, "elm.icon.1")) {
		return _icon_email_chk_get(data, obj, part);
	} else if (!strcmp(part, "elm.icon.2")) {
		if (index % 2 == 0) {
			Evas_Object *box = elm_box_add(obj);
			evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
			evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			elm_box_horizontal_set(box, EINA_TRUE);
			Evas_Object *icon = _gl_icon_star_get(data, obj, part);
			evas_object_size_hint_min_set(icon, 45*scale, 45*scale);
			evas_object_show(icon);
			elm_box_pack_end(box, icon);
			icon  = evas_object_rectangle_add(evas_object_evas_get(obj));
			evas_object_size_hint_min_set(icon, 16*scale, 16*scale);
			evas_object_color_set(icon, 0, 0, 0, 0);
			evas_object_show(icon);
			elm_box_pack_end(box, icon);
			return box;
		} else return NULL;
	} else if (!strcmp(part, "elm.icon.3")) {
		index = index % 3;
		if (index == 0) {
			Evas_Object *box = elm_box_add(obj);
			evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
			evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			elm_box_horizontal_set(box, EINA_TRUE);
			Evas_Object *icon = _gl_icon_star_get(data, obj, part);
			evas_object_size_hint_min_set(icon, 45*scale, 45*scale);
			evas_object_show(icon);
			elm_box_pack_start(box, icon);
			icon  = evas_object_rectangle_add(evas_object_evas_get(obj));
			evas_object_color_set(icon, 0, 0, 0, 0);
			evas_object_size_hint_min_set(icon, 16*scale, 16*scale);
			evas_object_show(icon);
			elm_box_pack_start(box, icon);
			return box;
		} else return NULL;
	} else if (!strcmp(part, "elm.icon.4")) {
		return _gl_icon_star_get(data, obj, part);
	} else return _gl_content_get(data, obj, part);
	return NULL;
}

static Evas_Object *
_gl_email_icon_lazy_get(void *data, Evas_Object *obj, const char *part)
{
	Item_Data *id = data;
	int index = id->index;
	double scale = elm_config_scale_get();
	if (strstr(part, "touch")) {
		Evas_Object *rect = evas_object_rectangle_add(evas_object_evas_get(obj));
		evas_object_size_hint_align_set(rect, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(rect, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_event_callback_add(rect, EVAS_CALLBACK_MOUSE_UP, _email_touch_clicked, NULL);
		evas_object_propagate_events_set(rect, EINA_FALSE);
		evas_object_color_set(rect, 64, 0 ,0, 64);
		return rect;
	} else if (!strcmp(part, "elm.icon.1")) {
		return _icon_email_chk_get(data, obj, part);
	} else if (!strcmp(part, "elm.icon.2")) {
		if (scrolling) return NULL;
		index = index % 2;
		if (index == 0) return NULL;
		else {
			Evas_Object *box = elm_box_add(obj);
			evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
			evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			elm_box_horizontal_set(box, EINA_TRUE);
			Evas_Object *icon = _gl_icon_photo_get(data, obj, part);
			evas_object_size_hint_min_set(icon, 45*scale, 45*scale);
			evas_object_show(icon);
			elm_box_pack_end(box, icon);
			icon  = evas_object_rectangle_add(evas_object_evas_get(obj));
			evas_object_size_hint_min_set(icon, 16*scale, 16*scale);
			evas_object_color_set(icon, 0, 0, 0, 0);
			evas_object_show(icon);
			elm_box_pack_end(box, icon);
			return box;
		}
	} else if (!strcmp(part, "elm.icon.4")) {
		if (scrolling) return NULL;
		Evas_Object *icon = _gl_icon_get(data, obj, part);
		evas_object_size_hint_min_set(icon, 35*scale, 35*scale);
		return icon;
	} else if (!strcmp(part, "elm.icon.3")) {
		if (scrolling) return NULL;
		index = index % 4;
		if (index == 0) return NULL;
		else if (index == 1) {
			Evas_Object *box = elm_box_add(obj);
			evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
			evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			elm_box_horizontal_set(box, EINA_TRUE);
			Evas_Object *icon = _gl_icon_photo_get(data, obj, part);
			evas_object_size_hint_min_set(icon, 45*scale, 45*scale);
			evas_object_show(icon);
			elm_box_pack_start(box, icon);
			icon  = evas_object_rectangle_add(evas_object_evas_get(obj));
			evas_object_color_set(icon, 0, 0, 0, 0);
			evas_object_size_hint_min_set(icon, 16*scale, 16*scale);
			evas_object_show(icon);
			elm_box_pack_start(box, icon);
			return box;
		} else if (index == 2) {
			Evas_Object *box = elm_box_add(obj);
			evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
			evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			elm_box_horizontal_set(box, EINA_TRUE);
			elm_box_padding_set(box, 10, 0);
			Evas_Object *icon = _gl_icon_photo_get(data, obj, part);
			evas_object_size_hint_min_set(icon, 45*scale, 45*scale);
			evas_object_show(icon);
			elm_box_pack_end(box, icon);
			icon = _gl_icon_photo_get(data, obj, part);
			evas_object_size_hint_min_set(icon, 35*scale, 35*scale);
			evas_object_show(icon);
			elm_box_pack_end(box, icon);
			icon  = evas_object_rectangle_add(evas_object_evas_get(obj));
			evas_object_size_hint_min_set(icon, 16*scale, 16*scale);
			evas_object_color_set(icon, 0, 0, 0, 0);
			evas_object_show(icon);
			elm_box_pack_start(box, icon);
			return box;
		} else if (index == 3) {
			Evas_Object *box = elm_box_add(obj);
			evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
			evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			elm_box_horizontal_set(box, EINA_TRUE);
			elm_box_padding_set(box, 10, 0);
			Evas_Object *icon = _gl_icon_photo_get(data, obj, part);
			evas_object_size_hint_min_set(icon, 50*scale, 50*scale);
			evas_object_show(icon);
			elm_box_pack_end(box, icon);
			icon = _gl_icon_photo_get(data, obj, part);
			evas_object_size_hint_min_set(icon, 35*scale, 35*scale);
			evas_object_show(icon);
			elm_box_pack_end(box, icon);
			icon = _gl_icon_photo_get(data, obj, part);
			evas_object_size_hint_min_set(icon, 45*scale, 45*scale);
			evas_object_show(icon);
			elm_box_pack_end(box, icon);
			icon  = evas_object_rectangle_add(evas_object_evas_get(obj));
			evas_object_size_hint_min_set(icon, 16*scale, 16*scale);
			evas_object_color_set(icon, 0, 0, 0, 0);
			evas_object_show(icon);
			elm_box_pack_start(box, icon);
			return box;
		}
	} else {
		if (scrolling) return NULL;
		return _gl_content_get(data, obj, part);
	}
	return NULL;
}


static Evas_Object *
_gl_email_outbox_get(void *data, Evas_Object *obj, const char *part)
{
	if (!strcmp(part, "elm.icon.5")) {
		return _gl_icon_btn_text_get(data, obj, part);
	} else
		return _gl_email_icon_get(data, obj, part);
}

static Evas_Object *
_gl_icon_highlight_get(void *data, Evas_Object *obj, const char *part)
{
	Item_Data *id = data;
	Evas_Object *icon = NULL;
	int index = id->index;
	if (id->highlighted) {
		printf("Change icon to hilighted icon [%s]\n", genlist_demo_names[index%NUM_OF_GENLIST_DEMO_NAMES]);
		icon = elm_image_add(obj);
		elm_image_file_set(icon, ICON_DIR"/genlist/00_brightness_right.png", NULL);
		evas_object_size_hint_align_set(icon, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	} else {
		icon = elm_image_add(obj);
		// Below should be called before elm_image_file_set
		elm_image_prescale_set(icon, 10);
		evas_object_color_set(icon, 128, 128, 128, 128);
		elm_image_file_set(icon, ICON_DIR"/genlist/iu.jpg", NULL);
		evas_object_size_hint_align_set(icon, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	}
	return icon;
}

static void _gl_sel(void *data, Evas_Object *obj, void *ei)
{
	if (!ei) return;
	Elm_Object_Item *item = ei;
	Item_Data *id = elm_object_item_data_get(item);
	int index = id->index;
	printf("Select callback: %s\n", genlist_demo_names[index%NUM_OF_GENLIST_DEMO_NAMES]);

	const Elm_Genlist_Item_Class *itc = elm_genlist_item_item_class_get(item);

	// accessibility test
	if (itc && !strcmp(itc->item_style, "1text.1icon")) {
		printf("Item Class updated\n");
		elm_genlist_item_item_class_update(item, itc);
	}

	// Make item as unselected
	elm_genlist_item_selected_set(item, EINA_FALSE);

	if (itc && !strcmp(itc->item_style, "groupindex")) {
		printf("slide start\n");
		elm_object_item_signal_emit(item, "elm,state,slide,start", "");
	}
	if (itc && (!strcmp(itc->item_style, "1text.1icon.3") ||
		!strcmp(itc->item_style, "1text.1icon.3.tb") ||
		!strcmp(itc->item_style, "1text.2icon.2") ||
		!strcmp(itc->item_style, "1text.3icon") ||
		!strcmp(itc->item_style, "1text.3icon.2") ||
		!strcmp(itc->item_style, "1text.2icon.9") ||
		!strcmp(itc->item_style, "1text.2icon.4") ||
		!strcmp(itc->item_style, "1text.2icon.4.tb") ||
		!strcmp(itc->item_style, "2text.2icon.7") ||
		!strcmp(itc->item_style, "2text.2icon.5") ||
		!strcmp(itc->item_style, "2text.3icon.4"))) {
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

static void _longpressed(void *data, Evas_Object *obj, void *ei)
{
	if (!ei) return;
	Elm_Object_Item *item = ei;
	Item_Data *id = elm_object_item_data_get(item);
	int index = id->index;
	printf("longpressed: [%s]\n", genlist_demo_names[index%NUM_OF_GENLIST_DEMO_NAMES]);
	// FIXME: after unrealized & realized, item will be back to default state (unread).
	elm_object_item_signal_emit(ei, "elm,state,read", "");
}

static void _scroll_anim_start(void *data, Evas_Object *obj, void *ei)
{
	scrolling = EINA_TRUE;
	printf("scroll,anim,start\n");
}

static void _scroll_anim_stop(void *data, Evas_Object *obj, void *ei)
{
	scrolling = EINA_FALSE;
	if (lazy) {
		Eina_List *l;
		Elm_Object_Item *it;
		l = elm_genlist_realized_items_get(obj);
		EINA_LIST_FREE(l, it) {
			elm_genlist_item_fields_update(it, "elm.icon.2", ELM_GENLIST_ITEM_FIELD_CONTENT);
			elm_genlist_item_fields_update(it, "elm.icon.3", ELM_GENLIST_ITEM_FIELD_CONTENT);
			elm_genlist_item_fields_update(it, "elm.icon.4", ELM_GENLIST_ITEM_FIELD_CONTENT);
		}
	}
	printf("scroll,anim,stop\n");
}

static void _scroll_drag_start(void *data, Evas_Object *obj, void *ei)
{
	scrolling = EINA_TRUE;
	printf("scroll,drag,start\n");
}

static void _scroll_drag_stop(void *data, Evas_Object *obj, void *ei)
{
	scrolling = EINA_FALSE;
	if (lazy) {
		Eina_List *l, *list;
		Elm_Object_Item *it;
		list = elm_genlist_realized_items_get(obj);
		EINA_LIST_FOREACH(list, l, it) {
			elm_genlist_item_fields_update(it, "elm.icon.2", ELM_GENLIST_ITEM_FIELD_CONTENT);
			elm_genlist_item_fields_update(it, "elm.icon.3", ELM_GENLIST_ITEM_FIELD_CONTENT);
			elm_genlist_item_fields_update(it, "elm.icon.4", ELM_GENLIST_ITEM_FIELD_CONTENT);
		}
	}

	printf("scroll,drag,stop\n");
}

static void _realized(void *data, Evas_Object *obj, void *ei)
{
	if (!ei) return;
	Item_Data *id = elm_object_item_data_get(ei);
	int index = id->index;

	const char *txt = elm_object_item_part_text_get(ei, "elm.text");
	if (!txt)
		txt = elm_object_item_part_text_get(ei, "elm.text.1");
	printf("realized: [%s]\n", txt); //genlist_demo_names[index%NUM_OF_GENLIST_DEMO_NAMES]);

	// ===== Accessibility ====
	if (elm_config_access_get()) {
		const Elm_Genlist_Item_Class *itc = elm_genlist_item_item_class_get(ei);
		if (!strcmp(itc->item_style, "1icon") ||
				!strcmp(itc->item_style, "1icon/no_padding") ||
				!strcmp(itc->item_style, "1icon/with_no_line") ||
				!strcmp(itc->item_style, "1icon/no_padding_line")) {

			// Unregister item to not be highlighted
			elm_object_item_access_unregister(ei);

			// if icon is not appended yet, register icon to be highglihted
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
	}

	// For email style ===================================================
	// email style implement icons as image part, not swallow part.
	if (id->checked)
		elm_object_item_signal_emit(id->item, "elm,state,check,show", "");
	else
		elm_object_item_signal_emit(id->item, "elm,state,check,hide", "");
	if (id->stared)
		elm_object_item_signal_emit(id->item, "elm,state,icon4,enable", "");
	else
		elm_object_item_signal_emit(id->item, "elm,state,icon4,disable", "");

	if (index%3 == 0)
		elm_object_item_signal_emit(ei, "elm,state,read", "");
	if (index%2 == 0)
		elm_object_item_signal_emit(ei, "elm,state,icon2,show", "");
	if (index%3 == 0)
		elm_object_item_signal_emit(ei, "elm,state,icon3,show", "");
	elm_object_item_signal_emit(ei, "elm,state,icon4,show", "");
	// ===================================================================
}

static void _loaded(void *data, Evas_Object *obj, void *ei)
{
	printf("loaded\n");
}

#if TEST_SIGNALS
static void _unrealized(void *data, Evas_Object *obj, void *ei)
{
	if (!ei) return;
	Elm_Object_Item *item = ei;
	Item_Data *id = elm_object_item_data_get(item);
	int index = id->index;
	printf("unrealized: [%s]\n", genlist_demo_names[index%NUM_OF_GENLIST_DEMO_NAMES]);
}

static void _selected(void *data, Evas_Object *obj, void *ei)
{
	if (!ei) return;
	Elm_Object_Item *item = ei;
	Item_Data *id = elm_object_item_data_get(item);
	int index = id->index;
	printf("selected: %s\n", genlist_demo_names[index%NUM_OF_GENLIST_DEMO_NAMES]);
}

static void _activated(void *data, Evas_Object *obj, void *ei)
{
	if (!ei) return;
	Elm_Object_Item *item = ei;
	Item_Data *id = elm_object_item_data_get(item);
	int index = id->index;
	printf("activated: [%s]\n", genlist_demo_names[index%NUM_OF_GENLIST_DEMO_NAMES]);
}

static void _clicked_double(void *data, Evas_Object *obj, void *ei)
{
	if (!ei) return;
	Elm_Object_Item *item = ei;
	Item_Data *id = elm_object_item_data_get(item);
	int index = id->index;
	printf("clicked,double: [%s]\n", genlist_demo_names[index%NUM_OF_GENLIST_DEMO_NAMES]);
}

static void _unselected(void *data, Evas_Object *obj, void *ei)
{
	if (!ei) return;
	Elm_Object_Item *item = ei;
	Item_Data *id = elm_object_item_data_get(item);
	int index = id->index;
	printf("unselected: [%s]\n", genlist_demo_names[index%NUM_OF_GENLIST_DEMO_NAMES]);
}

static void _drag_start_up(void *data, Evas_Object *obj, void *ei)
{
	if (!ei) return;
	Elm_Object_Item *item = ei;
	Item_Data *id = elm_object_item_data_get(item);
	int index = id->index;
	printf("drag,start,up: [%s]\n", genlist_demo_names[index%NUM_OF_GENLIST_DEMO_NAMES]);
}

static void _drag_start_down(void *data, Evas_Object *obj, void *ei)
{
	if (!ei) return;
	Elm_Object_Item *item = ei;
	Item_Data *id = elm_object_item_data_get(item);
	int index = id->index;
	printf("drag,start,down: [%s]\n", genlist_demo_names[index%NUM_OF_GENLIST_DEMO_NAMES]);
}

static void _drag_start_left(void *data, Evas_Object *obj, void *ei)
{
	if (!ei) return;
	Elm_Object_Item *item = ei;
	Item_Data *id = elm_object_item_data_get(item);
	int index = id->index;
	printf("drag,start,left: [%s]\n", genlist_demo_names[index%NUM_OF_GENLIST_DEMO_NAMES]);
}

static void _drag_start_right(void *data, Evas_Object *obj, void *ei)
{
	if (!ei) return;
	Elm_Object_Item *item = ei;
	Item_Data *id = elm_object_item_data_get(item);
	int index = id->index;
	printf("drag,start,right: [%s]\n", genlist_demo_names[index%NUM_OF_GENLIST_DEMO_NAMES]);
}

static void _drag_stop(void *data, Evas_Object *obj, void *ei)
{
	if (!ei) return;
	Elm_Object_Item *item = ei;
	Item_Data *id = elm_object_item_data_get(item);
	int index = id->index;
	printf("drag,stop: [%s]\n", genlist_demo_names[index%NUM_OF_GENLIST_DEMO_NAMES]);
}

static void _drag(void *data, Evas_Object *obj, void *ei)
{
	if (!ei) return;
	Elm_Object_Item *item = ei;
	Item_Data *id = elm_object_item_data_get(item);
	int index = id->index;
	printf("drag: [%s]\n", genlist_demo_names[index%NUM_OF_GENLIST_DEMO_NAMES]);
}

static void _edge_top(void *data, Evas_Object *obj, void *ei)
{
	printf("edge,top\n");
}

static void _edge_bottom(void *data, Evas_Object *obj, void *ei)
{
	printf("edge,bottom\n");
}

static void _edge_left(void *data, Evas_Object *obj, void *ei)
{
	printf("edge,left\n");
}

static void _edge_right(void *data, Evas_Object *obj, void *ei)
{
	printf("edge,right\n");
}

static void _multi_swipe_left(void *data, Evas_Object *obj, void *ei)
{
	if (!ei) return;
	Elm_Object_Item *item = ei;
	Item_Data *id = elm_object_item_data_get(item);
	int index = id->index;
	printf("multi,swipe,left: [%s]\n", genlist_demo_names[index%NUM_OF_GENLIST_DEMO_NAMES]);
}

static void _multi_swipe_right(void *data, Evas_Object *obj, void *ei)
{
	if (!ei) return;
	Elm_Object_Item *item = ei;
	Item_Data *id = elm_object_item_data_get(item);
	int index = id->index;
	printf("multi,swipe,right: [%s]\n", genlist_demo_names[index%NUM_OF_GENLIST_DEMO_NAMES]);
}

static void _multi_swipe_up(void *data, Evas_Object *obj, void *ei)
{
	if (!ei) return;
	Elm_Object_Item *item = ei;
	Item_Data *id = elm_object_item_data_get(item);
	int index = id->index;
	printf("multi,swipe,up: [%s]\n", genlist_demo_names[index%NUM_OF_GENLIST_DEMO_NAMES]);
}

static void _multi_swipe_down(void *data, Evas_Object *obj, void *ei)
{
	if (!ei) return;
	Elm_Object_Item *item = ei;
	Item_Data *id = elm_object_item_data_get(item);
	int index = id->index;
	printf("edge,bottom: [%s]\n", genlist_demo_names[index%NUM_OF_GENLIST_DEMO_NAMES]);
}

static void _multi_pinch_out(void *data, Evas_Object *obj, void *ei)
{
	if (!ei) return;
	Elm_Object_Item *item = ei;
	Item_Data *id = elm_object_item_data_get(item);
	int index = id->index;
	printf("multi,pinch,out: [%s]\n", genlist_demo_names[index%NUM_OF_GENLIST_DEMO_NAMES]);
}

static void _multi_pinch_in(void *data, Evas_Object *obj, void *ei)
{
	if (!ei) return;
	Elm_Object_Item *item = ei;
	Item_Data *id = elm_object_item_data_get(item);
	int index = id->index;
	printf("multi,pinch,in: [%s]\n", genlist_demo_names[index%NUM_OF_GENLIST_DEMO_NAMES]);
}

static void _swipe(void *data, Evas_Object *obj, void *ei)
{
	if (!ei) return;
	Elm_Object_Item *item = ei;
	Item_Data *id = elm_object_item_data_get(item);
	int index = id->index;
	printf("swipe: [%s]\n", genlist_demo_names[index%NUM_OF_GENLIST_DEMO_NAMES]);
}

static void _moved(void *data, Evas_Object *obj, void *ei)
{
	if (!ei) return;
	Elm_Object_Item *item = ei;
	Item_Data *id = elm_object_item_data_get(item);
	int index = id->index;
	printf("moved: [%s]\n", genlist_demo_names[index%NUM_OF_GENLIST_DEMO_NAMES]);
}

static void _moved_after(void *data, Evas_Object *obj, void *ei)
{
	if (!ei) return;
	Elm_Object_Item *item = ei;
	Item_Data *id = elm_object_item_data_get(item);
	int index = id->index;
	printf("moved,after: [%s]\n", genlist_demo_names[index%NUM_OF_GENLIST_DEMO_NAMES]);
}

static void _moved_before(void *data, Evas_Object *obj, void *ei)
{
	if (!ei) return;
	Elm_Object_Item *item = ei;
	Item_Data *id = elm_object_item_data_get(item);
	int index = id->index;
	printf("moved,before: [%s]\n", genlist_demo_names[index%NUM_OF_GENLIST_DEMO_NAMES]);
}

static void _index_update(void *data, Evas_Object *obj, void *ei)
{
	if (!ei) return;
	Elm_Object_Item *item = ei;
	Item_Data *id = elm_object_item_data_get(item);
	int index = id->index;
	printf("index,updated: [%s]\n", genlist_demo_names[index%NUM_OF_GENLIST_DEMO_NAMES]);
}

static void _expanded(void *data, Evas_Object *obj, void *ei)
{
	if (!ei) return;
	Elm_Object_Item *item = ei;
	Item_Data *id = elm_object_item_data_get(item);
	int index = id->index;
	printf("expanded: [%s]\n", genlist_demo_names[index%NUM_OF_GENLIST_DEMO_NAMES]);
}

static void _contracted(void *data, Evas_Object *obj, void *ei)
{
	if (!ei) return;
	Elm_Object_Item *item = ei;
	Item_Data *id = elm_object_item_data_get(item);
	int index = id->index;
	printf("contracted: [%s]\n", genlist_demo_names[index%NUM_OF_GENLIST_DEMO_NAMES]);
}

static void _expand_request(void *data, Evas_Object *obj, void *ei)
{
	if (!ei) return;
	Elm_Object_Item *item = ei;
	Item_Data *id = elm_object_item_data_get(item);
	int index = id->index;
	printf("expand,request: [%s]\n", genlist_demo_names[index%NUM_OF_GENLIST_DEMO_NAMES]);
}

static void _contract_request(void *data, Evas_Object *obj, void *ei)
{
	if (!ei) return;
	Elm_Object_Item *item = ei;
	Item_Data *id = elm_object_item_data_get(item);
	int index = id->index;
	printf("contract,request: [%s]\n", genlist_demo_names[index%NUM_OF_GENLIST_DEMO_NAMES]);
}
static void _tree_effect_finished(void *data, Evas_Object *obj, void *ei)
{
	printf("tree,effect,finished\n");
}
#endif

static void _highlighted(void *data, Evas_Object *obj, void *ei)
{
	if (!ei) return;
	Elm_Object_Item *item = ei;
	Item_Data *id = elm_object_item_data_get(item);
	int index = id->index;
	id->highlighted = EINA_TRUE;

	const Elm_Genlist_Item_Class *itc = elm_genlist_item_item_class_get(item);
	if (!strcmp(itc->item_style, "1text.1icon.7")) {
		Evas_Object *img = elm_object_item_part_content_get(item, "elm.icon");
		elm_image_file_set(img, ICON_DIR"/genlist/00_brightness_right_press.png", NULL);
	} else if (!strcmp(itc->item_style, "1text.tb")) {
		printf("highlighted: [%s]\n", genlist_demo_names[index%NUM_OF_GENLIST_DEMO_NAMES]);
		elm_genlist_item_fields_update(item, "elm.text", ELM_GENLIST_ITEM_FIELD_TEXT);
	}
}

static void _unhighlighted(void *data, Evas_Object *obj, void *ei)
{
	if (!ei) return;
	Elm_Object_Item *item = ei;
	Item_Data *id = elm_object_item_data_get(ei);
	int index = id->index;
	id->highlighted = EINA_FALSE;

	const Elm_Genlist_Item_Class *itc = elm_genlist_item_item_class_get(item);
	if (!strcmp(itc->item_style, "1text.1icon.7")) {
		Evas_Object *img = elm_object_item_part_content_get(item, "elm.icon");
		elm_image_file_set(img, ICON_DIR"/genlist/iu.jpg", NULL);
	} else if (!strcmp(itc->item_style, "1text.tb")) {
		printf("unhighlighted: [%s]\n", genlist_demo_names[index%NUM_OF_GENLIST_DEMO_NAMES]);
		elm_genlist_item_fields_update(item, "elm.text", ELM_GENLIST_ITEM_FIELD_TEXT);
	}
}

static void _index_clicked(void *data, Evas_Object *obj, const char *em, const char *src)
{
	printf("index clicked\n");
	elm_object_signal_emit(obj, "elm,state,slide,start", "");

}

static char *_access_info_cb(void *data, Evas_Object *obj)
{
    const char *txt = NULL;
	txt = elm_object_part_text_get(data, "elm.text");
	if (!txt) return NULL;

	return strdup(_(txt));
}

static void _mouse_down_cb(void *data, Evas *e, Evas_Object *obj, void *ei)
{
	Evas_Event_Mouse_Down *md = ei;
	int ret;
	Elm_Object_Item *it = elm_genlist_at_xy_item_get(obj, md->canvas.x, md->canvas.y, &ret);
	printf("down %d %d (%d) (%p)\n", md->canvas.x, md->canvas.y, elm_genlist_item_index_get(it), it);
}

static void _create_genlist(void *data, Evas_Object *obj, void *event_info)
{
	if (!data) return;
	struct appdata *ad = data;
	int index;
	Elm_Object_Item *item;
	Evas_Object *genlist;
	const char *style = elm_object_item_text_get(event_info);

	// Create item class
	Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
	itc->item_style = style;
	itc->func.text_get = _gl_text_get;
	itc->func.content_get = _gl_content_get;
	itc->func.del = _gl_del;

	// Create genlist
	genlist = elm_genlist_add(ad->nf);
	evas_object_event_callback_add(genlist, EVAS_CALLBACK_MOUSE_DOWN, _mouse_down_cb, NULL);
	if (strstr(style, "email.")) {
		elm_object_style_set(genlist, "handler");
	}
	// Optimize your application with appropriate genlist block size.
	elm_genlist_block_count_set(genlist, 14);

	// HOMOGENEOUS MODE
	// If item height is same when each style name is same,
	// Use homogeneous mode.
	if (!strstr(style, ".tb") && !strstr(style, "multiline")) {
		printf("Homogeneous mode enabled\n");
		elm_genlist_homogeneous_set(genlist, EINA_TRUE);
	}
	// COMPRESS MODE
	// If multiline text (multiline entry or multiline textblock or sliding mode)
	// is used, use compress mode for compressing width to fit the viewport width.
	// So genlist can calculate item's height correctly.
	if (strstr(style, "multiline") ||
		!strcmp(style, "groupindex") || !strstr(style, "email")) {
		printf("Compress mode enabled\n");
		elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	}

	evas_object_smart_callback_add(genlist, "scroll,anim,start", _scroll_anim_start, NULL);
	evas_object_smart_callback_add(genlist, "scroll,anim,stop", _scroll_anim_stop, NULL);
	evas_object_smart_callback_add(genlist, "scroll,drag,start", _scroll_drag_start, NULL);
	evas_object_smart_callback_add(genlist, "scroll,drag,stop", _scroll_drag_stop, NULL);
	evas_object_smart_callback_add(genlist, "realized", _realized, NULL);
	evas_object_smart_callback_add(genlist, "loaded", _loaded, NULL);

#if TEST_SIGNALS
	evas_object_smart_callback_add(genlist, "unrealized", _unrealized, NULL);
	evas_object_smart_callback_add(genlist, "activated", _activated, NULL);
	evas_object_smart_callback_add(genlist, "selected", _selected, NULL);
	evas_object_smart_callback_add(genlist, "unselected", _unselected, NULL);
	evas_object_smart_callback_add(genlist, "clicked,double", _clicked_double, NULL);
	evas_object_smart_callback_add(genlist, "drag,start,up", _drag_start_up, NULL);
	evas_object_smart_callback_add(genlist, "drag,start,down", _drag_start_down, NULL);
	evas_object_smart_callback_add(genlist, "drag,start,left", _drag_start_left, NULL);
	evas_object_smart_callback_add(genlist, "drag,start,right", _drag_start_right, NULL);
	evas_object_smart_callback_add(genlist, "drag,stop", _drag_stop, NULL);
	evas_object_smart_callback_add(genlist, "drag", _drag, NULL);
	evas_object_smart_callback_add(genlist, "edge,top", _edge_top, NULL);
	evas_object_smart_callback_add(genlist, "edge,bottom", _edge_bottom, NULL);
	evas_object_smart_callback_add(genlist, "edge,left", _edge_left, NULL);
	evas_object_smart_callback_add(genlist, "edge,left", _edge_right, NULL);
	evas_object_smart_callback_add(genlist, "multi,swipe,left", _multi_swipe_left, NULL);
	evas_object_smart_callback_add(genlist, "multi,swipe,right", _multi_swipe_right, NULL);
	evas_object_smart_callback_add(genlist, "multi,swipe,up", _multi_swipe_up, NULL);
	evas_object_smart_callback_add(genlist, "multi,swipe,down", _multi_swipe_down, NULL);
	evas_object_smart_callback_add(genlist, "multi,pinch,out", _multi_pinch_out, NULL);
	evas_object_smart_callback_add(genlist, "multi,pinch,in", _multi_pinch_in, NULL);
	evas_object_smart_callback_add(genlist, "swipe", _swipe, NULL);
	evas_object_smart_callback_add(genlist, "moved", _moved, NULL);
	evas_object_smart_callback_add(genlist, "moved,after", _moved_after, NULL);
	evas_object_smart_callback_add(genlist, "moved,before", _moved_before, NULL);
	evas_object_smart_callback_add(genlist, "index,update", _index_update, NULL);
	evas_object_smart_callback_add(genlist, "tree,effect,finished", _tree_effect_finished, NULL);
#endif
	evas_object_smart_callback_add(genlist, "highlighted", _highlighted, NULL);
	evas_object_smart_callback_add(genlist, "unhighlighted", _unhighlighted, NULL);
	evas_object_smart_callback_add(genlist, "longpressed", _longpressed, NULL);

	if (strstr(style, ".tb"))
		itc->func.text_get = _gl_text_get_textblock;
	else if (strstr(style, "multiline/2text.5icon"))
		itc->func.text_get = _gl_multiline_text_get;

	if (strstr(style, ".thumb."))
		itc->func.content_get = _gl_icon_photo_get;

	if (!strcmp(style, "1icon") || strstr(style, "1icon/")) {
		itc->func.content_get = _gl_icons_get;
	} else if (!strcmp(style, "1text.1icon.7")) {
		itc->func.content_get = _gl_icon_highlight_get;
	} else if (!strcmp(style, "2text.1icon.10") ||
			   !strcmp(style, "2text.1icon.10.tb")) {
		itc->func.content_get = _gl_icon_reveal_get;
	} else if (!strcmp(style, "1text.1icon.6") ||
			   !strcmp(style, "1text.1icon.divider") ||
			   !strcmp(style, "1text.2icon.divider") ||
			   !strcmp(style, "2text.1icon.divider")) {
		itc->func.content_get = _gl_2icon_onoff_get;
	} else if (!strcmp(style, "1text.1icon.3") ||
			   !strcmp(style, "1text.1icon.3.tb") ||
			   !strcmp(style, "1text.2icon.2") ||
			   !strcmp(style, "1text.2icon.2") ||
			   !strcmp(style, "2text.2icon.5") ||
			   !strcmp(style, "2text.2icon.5")) {
		itc->func.content_get = _gl_2icon_chk_get;
	} else if (!strcmp(style, "1text.1icon") ||
			   !strcmp(style, "1text.2icon") ||
			   !strcmp(style, "1text.2icon.tb") ||
			   !strcmp(style, "2text.1icon.2") ||
			   !strcmp(style, "2text.1icon.2.tb") ||
			   !strcmp(style, "2text.2icon.8") ||
			   !strcmp(style, "2text.2icon.8.tb") ||
			   !strcmp(style, "3text.3icon.progress")) {
		itc->func.content_get = _gl_2icon_btn_text_get;
	} else if (!strcmp(style, "1text.2icon.6") ||
			   !strcmp(style, "1text.2icon.6.tb") ||
			   !strcmp(style, "2text.2icon.4") |\
			   !strcmp(style, "2text.2icon.4.tb")) {
		itc->func.content_get = _gl_2icon_reveal_get;
	} else if (!strcmp(style, "1text.2icon.4") ||
			   !strcmp(style, "1text.2icon.4.tb") ||
			   !strcmp(style, "1text.2icon.9") ||
			   !strcmp(style, "2text.2icon.7")) {
		itc->func.content_get = _gl_2icon_chk_reveal_get;
	} else if (!strcmp(style, "1text.3icon") ||
			   !strcmp(style, "1text.3icon.2") ||
			   !strcmp(style, "2text.3icon.4")) {
		itc->func.content_get = _gl_3icon_chk_reveal_get;
	} else if (!strcmp(style, "4text.1icon.1.tb") ||
			   !strcmp(style, "4text.1.tb")) {
		itc->func.text_get = _gl_text_get_bubble_textblock;
	} else if (!strcmp(style, "email.list.test"))  {
		itc->func.text_get = _gl_email_text_get;
		itc->func.content_get = _gl_email_icon_test_get;
	} else if (!strcmp(style, "email.list") ||
			!strcmp(style, "email.list.conversation")) {
		itc->func.text_get = _gl_email_text_get;
		itc->func.content_get = _gl_email_icon_get;
	} else if (!strcmp(style, "email.outbox") ||
			!strcmp(style, "email.outbox.conversation")) {
		itc->func.text_get = _gl_email_text_get;
		itc->func.content_get = _gl_email_outbox_get;
	} else if (!strcmp(style, "email.list(lazy)")) {
		lazy = EINA_TRUE;
		itc->item_style = "email.list";
		itc->func.content_get = _gl_email_icon_lazy_get;
	}

	for (index = 0; index < NUM_OF_ITEMS; index++) {
		Item_Data *id = calloc(sizeof(Item_Data), 1);
		id->index = index;
		item = elm_genlist_item_append(
				genlist,			// genlist object
				itc,				// item class
				id,		            // data
				NULL,
				ELM_GENLIST_ITEM_NONE,
				_gl_sel,
				NULL);
		id->item = item;
		if (index ==0) elm_object_item_disabled_set(item, EINA_TRUE);
	}
	elm_genlist_item_class_free(itc);

	// Add select all layout for groupindex
	if (!strcmp(style, "groupindex")) {
		Evas_Object *bx = elm_box_add(ad->nf);
		evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);

		Evas_Object *ly = elm_layout_add(bx);
		elm_layout_theme_set(ly, "genlist/item", "groupindex", "default");
		evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, 0);
		evas_object_size_hint_align_set(ly, -1, -1);
		elm_object_part_text_set(ly, "elm.text", "This is a layout, not genlist item. This is a groupindex genlist item style. This is packed into box");
		elm_layout_signal_callback_add(ly, "mouse,clicked,1", "*", _index_clicked, NULL);
		evas_object_show(ly);
		elm_box_pack_end(bx, ly);

		// ============== Accessibility
		// Make layout accessible
		Evas_Object *ao = elm_access_object_register(ly, ly);
		elm_access_info_cb_set(ao, ELM_ACCESS_INFO, _access_info_cb, ly);
		elm_object_focus_custom_chain_append(ly, ao, NULL);

		evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_show(genlist);
		elm_box_pack_end(bx, genlist);
		evas_object_show(bx);
		elm_naviframe_item_push(ad->nf, _(style), NULL, NULL, bx, NULL);
	} else
		elm_naviframe_item_push(ad->nf, _(style), NULL, NULL, genlist, NULL);
}

void genlist_normal_cb(void *data, Evas_Object *obj, void *event_info)
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
		elm_list_item_append(list, menu_its[i], NULL, NULL,
				_create_genlist, ad);
	}
	elm_list_go(list);

	elm_naviframe_item_push(ad->nf, _("Item Styles"), NULL, NULL, list, NULL);
}

void genlist_homogenous_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!data) return;
	struct appdata *ad = data;
	int index;
	Elm_Object_Item *item;
	Evas_Object *genlist;

	// Create item class
	Elm_Genlist_Item_Class *itc1,*itc2, *itc3, *itc4, *itc5;
	itc1 = elm_genlist_item_class_new();
	itc2 = elm_genlist_item_class_new();
	itc3 = elm_genlist_item_class_new();
	itc4 = elm_genlist_item_class_new();
	itc5 = elm_genlist_item_class_new();

	// Set genlist item class
	itc1->item_style = "1text.1icon";
	itc1->func.text_get = _gl_text_get;
	itc1->func.content_get = _gl_content_get;
	itc1->func.del = _gl_del;

	itc2->item_style = "3text.1icon";
	itc2->func.text_get = _gl_text_get;
	itc2->func.content_get = _gl_content_get;
	itc2->func.del = _gl_del;

	itc3->item_style = "email.list";
	itc3->func.text_get = _gl_email_text_get;
	itc3->func.content_get = _gl_email_icon_get;
	itc3->func.del = _gl_del;

	itc4->item_style = "dialogue/1text.2icon";
	itc4->func.text_get = _gl_text_get;
	itc4->func.content_get = _gl_content_get;
	itc4->func.del = _gl_del;

	itc5->item_style = "dialogue/3text.2icon";
	itc5->func.text_get = _gl_text_get;
	itc5->func.content_get = _gl_content_get;
	itc5->func.del = _gl_del;

	// Create genlist
	genlist = elm_genlist_add(ad->nf);
	evas_object_smart_callback_add(genlist, "loaded", _loaded, NULL);

	// HOMOGENEOUS MODE
	// If item height is same when each style name is same,
	// Use homogeneous mode.
	elm_genlist_homogeneous_set(genlist, EINA_TRUE);
	printf("Homogeneous mode enabled\n");
	// COMPRESS MODE
	// If multiline text (multiline entry or multiline textblock or sliding mode)
	// is used, use compress mode for compressing width to fit the viewport width.
	// So genlist can calculate item's height correctly.
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	printf("Compress mode enabled\n");

	// Append items
	for (index = 0; index < NUM_OF_ITEMS; index++) {
		Elm_Genlist_Item_Class *itc;
		if (index%5 == 0) itc = itc1;
		else if (index%5 == 1) itc = itc2;
		else if (index%5 == 2) itc = itc3;
		else if (index%5 == 3) itc = itc4;
		else itc = itc5;

		Item_Data *id = calloc(sizeof(Item_Data), 1);
		id->index = index;
		item = elm_genlist_item_append(
				genlist,			// genlist object
				itc,				// item class
				id,		// data
				NULL,
				ELM_GENLIST_ITEM_NONE,
				_gl_sel,
				NULL
				);
		if (index ==0) elm_object_item_disabled_set(item, EINA_TRUE);
		id->item = item;
	}

	// Unref item class
	elm_genlist_item_class_free(itc1);
	elm_genlist_item_class_free(itc2);
	elm_genlist_item_class_free(itc3);
	elm_genlist_item_class_free(itc4);
	elm_genlist_item_class_free(itc5);

	elm_naviframe_item_push(ad->nf, "homogeneous", NULL, NULL, genlist, NULL);
}

static void _view_free_cb(void *data, Evas *e, Evas_Object *gen, void *ei)
{
	int *index;
	Ecore_Animator *anim;
	Elm_Genlist_Item_Class *itc1,*itc2, *itc3, *itc4, *itc5;

	index = evas_object_data_get(gen, "index");
	if (index) free(index);

	anim = evas_object_data_get(gen, "anim");
	if (anim) ecore_animator_del(anim);

	itc1 = evas_object_data_get(gen, "itc1");
	itc2 = evas_object_data_get(gen, "itc2");
	itc3 = evas_object_data_get(gen, "itc3");
	itc4 = evas_object_data_get(gen, "itc4");
	itc5 = evas_object_data_get(gen, "itc5");
	if (itc1) elm_genlist_item_class_free(itc1);
	if (itc2) elm_genlist_item_class_free(itc2);
	if (itc3) elm_genlist_item_class_free(itc3);
	if (itc4) elm_genlist_item_class_free(itc4);
	if (itc5) elm_genlist_item_class_free(itc5);
}

static Eina_Bool _item_append_cb(void *data)
{
	Evas_Object *genlist = data;
	int *index;
	Elm_Object_Item *item = NULL;
	Elm_Genlist_Item_Class *itc = NULL;

	index = evas_object_data_get(genlist, "index");
	if (*index >=  NUM_OF_ITEMS) {
		evas_object_data_set(genlist, "anim", NULL);
		return ECORE_CALLBACK_CANCEL;
	}

	if ((*index)%5 == 0)
		itc = evas_object_data_get(genlist, "itc1");
	else if (((*index)%5) == 1)
		itc = evas_object_data_get(genlist, "itc2");
	else if (((*index)%5) == 2)
		itc = evas_object_data_get(genlist, "itc3");
	else if (((*index)%5) == 3)
		itc = evas_object_data_get(genlist, "itc4");
	else
		itc = evas_object_data_get(genlist, "itc5");

	Item_Data *id = calloc(sizeof(Item_Data), 1);
	id->index = *index;
	item = elm_genlist_item_append(genlist, itc, id, NULL,
			ELM_GENLIST_ITEM_NONE, _gl_sel, NULL);
	id->item = item;
	if (*index == 0) elm_object_item_disabled_set(item, EINA_TRUE);
	(*index)++;

	return ECORE_CALLBACK_RENEW;
}

// FIXME: When item is added by animator, scroller is freezed.
void genlist_animator_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!data) return;
	struct appdata *ad = data;
	int *index;
	Evas_Object *genlist;

	// Create item class
	Elm_Genlist_Item_Class *itc1,*itc2, *itc3, *itc4, *itc5;
	itc1 = elm_genlist_item_class_new();
	itc2 = elm_genlist_item_class_new();
	itc3 = elm_genlist_item_class_new();
	itc4 = elm_genlist_item_class_new();
	itc5 = elm_genlist_item_class_new();

	// Set genlist item class
	itc1->item_style = "1text.1icon";
	itc1->func.text_get = _gl_text_get;
	itc1->func.content_get = _gl_content_get;
	itc1->func.del = _gl_del;

	itc2->item_style = "3text.1icon";
	itc2->func.text_get = _gl_text_get;
	itc2->func.content_get = _gl_content_get;
	itc2->func.del = _gl_del;

	itc3->item_style = "email.list";
	itc3->func.text_get = _gl_email_text_get;
	itc3->func.content_get = _gl_email_icon_get;
	itc3->func.del = _gl_del;

	itc4->item_style = "dialogue/1text.2icon";
	itc4->func.text_get = _gl_text_get;
	itc4->func.content_get = _gl_content_get;
	itc4->func.del = _gl_del;

	itc5->item_style = "dialogue/3text.2icon";
	itc5->func.text_get = _gl_text_get;
	itc5->func.content_get = _gl_content_get;
	itc5->func.del = _gl_del;

	// Create genlist
	genlist = elm_genlist_add(ad->nf);
	evas_object_smart_callback_add(genlist, "loaded", _loaded, NULL);

	// HOMOGENEOUS MODE
	// If item height is same when each style name is same,
	// Use homogeneous mode.
	printf("Homogeneous mode enabled\n");
	elm_genlist_homogeneous_set(genlist, EINA_TRUE);

	// COMPRESS MODE
	// If multiline text (multiline entry or multiline textblock or sliding mode)
	// is used, use compress mode for compressing width to fit the viewport width.
	// So genlist can calculate item's height correctly.
	printf("Compress mode enabled\n");
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);

	evas_object_data_set(genlist, "itc1", itc1);
	evas_object_data_set(genlist, "itc2", itc2);
	evas_object_data_set(genlist, "itc3", itc3);
	evas_object_data_set(genlist, "itc4", itc4);
	evas_object_data_set(genlist, "itc5", itc5);

	index = calloc(1, sizeof(int));
	*index = 0;
	evas_object_data_set(genlist, "index", index);

	// Append items using Ecore_Animator (recommend to use Ecore_Animator in multiline items)
	Ecore_Animator *anim = ecore_animator_add(_item_append_cb, genlist);
	evas_object_data_set(genlist, "anim", anim);

	evas_object_event_callback_add(genlist, EVAS_CALLBACK_FREE, _view_free_cb, NULL);
	elm_naviframe_item_push(ad->nf, "animator append", NULL, NULL, genlist, NULL);
}
