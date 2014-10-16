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
  Genlist Bubble
 ********************************************************/

#define NUM_OF_ITEMS 10
#define NUM_OF_TIMES 10
#define STYLE_COUNTER 6
//#define _ITEM_PROVIDER

static char *dates[] = {
	"2011.03.03", NULL, NULL, NULL, NULL, NULL,
	"2011.03.04", NULL, NULL, NULL
};

static char *times[] = {
	"10:00 AM", "10:01 AM", "10:02 AM", "10:03 AM", "10:04 AM", "10:05 AM",
	"3:50 PM", "3:51 PM", "3:52 PM", "3:53 PM"
};

static char *messages[] = {
	"Good morning!",
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

#ifdef _ITEM_PROVIDER
static char *items[] = {
	NULL, "attachment1", "attachment2", NULL, NULL, NULL,
	"attachment3", NULL, NULL, NULL
};
#else
static char *urls[] = {
	NULL,
	ICON_DIR"/Albums_Item/Albums_Item1.jpg",
	ICON_DIR"/Albums_Item/Albums_Item2.jpg", NULL, NULL, NULL,
	ICON_DIR"/Albums_Item/Albums_Item3.jpg", NULL, NULL, NULL
};
#endif

static Elm_Genlist_Item_Class itc, itc2, itc3, itc4;
static Ecore_Animator *animator;
static int gl_index;

static Eina_Bool _pop_cb(void *data, Elm_Object_Item *it)
{
	if (animator) ecore_animator_del(animator);
	animator = NULL;
	gl_index = 0;

	return EINA_TRUE;
}

static char* _gl_title_text_get(void *data, Evas_Object *obj, const char *part)
{
	int index = ((int)data) % NUM_OF_TIMES;

	if (dates[index]) return strdup(dates[index]);
	return NULL;
}

#ifdef _ITEM_PROVIDER
static Evas_Object *_item_provider(void *images, Evas_Object *entry, const char *item)
{
	Evas_Object *o = NULL;;

	if(!strcmp(item, "attachment1")) {
		o = evas_object_image_filled_add(evas_object_evas_get(entry));
		evas_object_image_file_set(o, ICON_DIR"/Albums_Item/Albums_Item1.jpg", NULL);
	} else if(!strcmp(item, "attachment2")) {
		o = evas_object_image_filled_add(evas_object_evas_get(entry));
		evas_object_image_file_set(o, ICON_DIR"/Albums_Item/Albums_Item2.jpg", NULL);
	} else if(!strcmp(item, "attachment3")) {
		o = evas_object_image_filled_add(evas_object_evas_get(entry));
		evas_object_image_file_set(o, ICON_DIR"/Albums_Item/Albums_Item3.jpg", NULL);
	}
	return o;
}
#endif

static Eina_Strbuf *_bubble_message_get(int index)
{
	Eina_Strbuf *buf = eina_strbuf_new();

	// message
	if (messages[index]) {
		if ((index < STYLE_COUNTER && index % 2 == 0) || (index >= STYLE_COUNTER && index % 2 == 1))
			eina_strbuf_append(buf, "<color=#4D3A17FF>");
		else
			eina_strbuf_append(buf, "<color=#000000FF>");
		eina_strbuf_append(buf, messages[index]);
		eina_strbuf_append(buf, "</color>");
	}

	// time
	if (times[index]) {
		if ((index < STYLE_COUNTER && index % 2 == 0) || (index >= STYLE_COUNTER && index % 2 == 1))
			eina_strbuf_append(buf, " <font_size=16><color=#8F783FFF>");
		else
			eina_strbuf_append(buf, " <font_size=16><color=#797364FF>");
		eina_strbuf_append(buf, times[index]);
		eina_strbuf_append(buf, "</color></font_size>");
	}

	// item
#ifdef _ITEM_PROVIDER
	if (items[index]) {
#else
	if (urls[index]) {
#endif
		// padding between text and item
		eina_strbuf_append(buf, "<br><font_size=6><br></font_size>");
		if ((index < STYLE_COUNTER && index % 2 == 0) || (index >= STYLE_COUNTER && index % 2 == 1))
			eina_strbuf_append(buf, "<item size=150x150 vsize=full href=");
		else
			eina_strbuf_append(buf, "<align=1.0><item size=150x150 vsize=full href=");
#ifdef _ITEM_PROVIDER
		eina_strbuf_append(buf, items[index]);
#else
		eina_strbuf_append(buf, "file://");
		eina_strbuf_append(buf, urls[index]);
#endif
		eina_strbuf_append(buf, "></item></align>");
		// padding between text and item
		eina_strbuf_append(buf, "<br><font_size=6><br></font_size>");
	}

	return buf;
}

static Evas_Object *_gl_content_get(void *data, Evas_Object *obj, const char *part)
{
	int index = (int)data;
	Evas_Object *entry;

	if (!strcmp(part, "elm.icon")) {
		Eina_Strbuf *buf;

		// add entry with bubble styles
		entry = elm_entry_add(obj);
		ea_entry_selection_back_event_allow_set(entry, EINA_TRUE);
		if ((index < STYLE_COUNTER && index % 2 == 0) || (index >= STYLE_COUNTER && index % 2 == 1))
			elm_object_style_set(entry, "readmessage");
		else
			elm_object_style_set(entry, "sentmessage");
		elm_entry_input_panel_enabled_set(entry, EINA_FALSE);
		elm_entry_editable_set(entry, EINA_FALSE);
#ifdef _ITEM_PROVIDER
		// to add an attachment, add item_provider
		if (items[index]) elm_entry_item_provider_append(entry, _item_provider, NULL);
#endif

		buf = _bubble_message_get(index % NUM_OF_TIMES);
		elm_entry_entry_set(entry, eina_strbuf_string_get(buf));
		eina_strbuf_free(buf);
		return entry;
	}
	return NULL;
}

static Eina_Bool _bubble_item_append_cb(void *data)
{
	Evas_Object *genlist = data;
	Elm_Object_Item *item = NULL;
	static Elm_Object_Item *git = NULL;
	int item_index = gl_index % NUM_OF_TIMES;

	if (gl_index >=  NUM_OF_ITEMS) {
		animator = NULL;
		return ECORE_CALLBACK_CANCEL;
	}

	// add a date group item and bubble item
	if ((item_index < STYLE_COUNTER && item_index % 2 == 0) || (item_index >= STYLE_COUNTER && item_index % 2 == 1)){
		if (dates[item_index])  {
			git = elm_genlist_item_append(genlist, &itc, (void *) gl_index, NULL, ELM_GENLIST_ITEM_GROUP, NULL, NULL);
			elm_genlist_item_select_mode_set(git, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		}
		item = elm_genlist_item_append(genlist, &itc2, (void *) gl_index, git, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	} else {
		if (dates[item_index])  {
			git = elm_genlist_item_append(genlist, &itc3, (void *) gl_index, NULL, ELM_GENLIST_ITEM_GROUP, NULL, NULL);
			elm_genlist_item_select_mode_set(git, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		}
		item = elm_genlist_item_append(genlist, &itc4, (void *) gl_index, git, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	}
	gl_index++;

	return ECORE_CALLBACK_RENEW;
}

void genlist_bubble_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	Evas_Object *genlist, *button;
	Elm_Object_Item *navi_it;

	// Create genlist
	genlist = elm_genlist_add(ad->nf);

	// To use multiline textblock/entry/editfield in genlist, set height_for_width mode
	// then the item's height is calculated while the item's width fits to genlist width.
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);

	// Set item class for bubble readmessage
	itc.item_style = "grouptitle.readmessage";
	itc.func.text_get = _gl_title_text_get;
	itc.func.content_get = NULL;
	itc.func.state_get = NULL;
	itc.func.del = NULL;

	itc2.item_style = "readmessage";
	itc2.func.text_get = NULL;
	itc2.func.content_get = _gl_content_get;
	itc2.func.state_get = NULL;
	itc2.func.del = NULL;

	// Set item class for bubble sentmessage
	itc3.item_style = "grouptitle.sentmessage";
	itc3.func.text_get = _gl_title_text_get;
	itc3.func.content_get = NULL;
	itc3.func.state_get = NULL;
	itc3.func.del = NULL;

	itc4.item_style = "sentmessage";
	itc4.func.text_get = NULL;
	itc4.func.content_get = _gl_content_get;
	itc4.func.state_get = NULL;
	itc4.func.del = NULL;

	// Append items using Ecore_Animator (recommend to use Ecore_Animator in multiline items)
	animator = ecore_animator_add(_bubble_item_append_cb, genlist);

	navi_it = elm_naviframe_item_push(ad->nf, _("Bubble"), NULL, NULL, genlist, NULL);
	elm_naviframe_item_pop_cb_set(navi_it, _pop_cb, NULL);
}
