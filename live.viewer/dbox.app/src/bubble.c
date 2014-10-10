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
#include "bubble.h"

/*********************************************************
 bubble
********************************************************/

static char *g_messages[] = {
	"Good morning, Princess?",
	"It's you again. How do you do this?",
	"This is the princess who fell from the sky into my arms.",
	"Yes. We keep meeting like this. You just suddenly show up.",
	"We could make plans to meet.",
	"No, it's nicer this way. I hope we meet again suddenly.",
	"Don't you know that it takes so little to make me happy?",
	"Good morning, Princess!  ",
	"This is incredible. You owe me an explanation.",
	"No, you're the one who owes me an explanation. You've really got a crush on me. Where shall we go, Princess?",
	NULL
};

static char *g_urls[] = {
	NULL,
	ICON_DIR"/Albums_Item/Albums_Item1.jpg",
	ICON_DIR"/Albums_Item/Albums_Item2.jpg", NULL, NULL, NULL,
	ICON_DIR"/Albums_Item/Albums_Item3.jpg", NULL,
	ICON_DIR"/Albums_Item/Albums_Item4.jpg", NULL
};

static char *g_times[] = {
	"10:00 AM", "10:01 AM", "10:02 AM", "10:03 AM", "10:04 AM", "10:05 AM",
	"3:50 PM", "3:51 PM", "3:52 PM", "3:53 PM", NULL
};

static Ecore_Animator *g_ani = NULL;
static int g_msgcnt = 0;
static Evas_Object *g_scroller = NULL;

static Evas_Object *_create_scroller(Evas_Object *parent);

static Eina_Strbuf *_bubble_message_get(int index)
{
	Eina_Strbuf *buf = eina_strbuf_new();

	// message
	if (g_messages[index])
		eina_strbuf_append(buf, g_messages[index]);

	if (g_urls[index]) {
		// padding between text and item
		eina_strbuf_append(buf, "<br><font_size=6><br></font_size>");
		eina_strbuf_append(buf, "<item size=150x150 vsize=full href=file://");
		eina_strbuf_append(buf, g_urls[index]);
		eina_strbuf_append(buf, "></item></align>");
	}

	switch (index)
	{
		case 6:
			eina_strbuf_append(buf, "<br><font_size=6><br></font_size>");
			eina_strbuf_append(buf, "File01.jpg(20KB)");
			break;
		case 8:
			eina_strbuf_append(buf, "<br><font_size=6><br></font_size>");
			eina_strbuf_append(buf, "File02.jpg(30KB)");
			break;
		default:
			break;
	}

	return buf;
}


static Eina_Bool _bubble_add(void *data)
{
	Evas_Object *box, *layout, *entry;
	Evas_Object *cicon, *check, *btn1, *pgbar;
	Eina_Strbuf *buf;

	if (!g_messages[g_msgcnt]) {
		g_msgcnt = 0;
		g_ani = NULL;
		return ECORE_CALLBACK_CANCEL;
	}
	box = data;

	/* Create Layout */
	layout = elm_layout_add(box);
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(layout);

	if (g_msgcnt % 2)
		elm_layout_theme_set(layout, "layout", "bubble", "sentmessage/default");
	else
		elm_layout_theme_set(layout, "layout", "bubble", "readmessage/default");

	/* Create Entry */
	entry = elm_entry_add(box);
	ea_entry_selection_back_event_allow_set(entry, EINA_TRUE);
	if (g_msgcnt % 2)
		elm_object_style_set(entry, "sentmessage");
	else
		elm_object_style_set(entry, "readmessage");

	elm_entry_input_panel_enabled_set(entry, EINA_FALSE);
	elm_entry_editable_set(entry, EINA_FALSE);
	buf = _bubble_message_get(g_msgcnt);
	elm_entry_entry_set(entry, eina_strbuf_string_get(buf));
	eina_strbuf_free(buf);
	elm_object_part_text_set(entry, "elm.text.time", g_times[g_msgcnt]);
	evas_object_show(entry);

	switch (g_msgcnt % 9)
	{
		case 1:
			/* group chat name */
			elm_object_signal_emit(entry, "elm,state,groupchat,enabled", "elm");
			elm_object_part_text_set(entry, "elm.text.groupchat", "Josh");
			break;
		case 2:
			/* caller ID */
			cicon = elm_image_add(box);
			elm_image_file_set(cicon, ICON_DIR"/genlist/iu.jpg", NULL);
			elm_object_signal_emit(entry, "elm,state,callerid,enabled", "elm");
			elm_object_part_content_set(entry, "elm.swallow.callerid", cicon);

			/* status text*/
			elm_object_part_text_set(entry, "elm.text.status", "Unread");

			/* icon */
			elm_object_signal_emit(entry, "elm,state,contents,enabled", "elm");
			btn1 = elm_button_add(entry);
			elm_object_style_set(btn1, "reveal");
			elm_object_part_content_set(entry, "elm.swallow.icon1", btn1);
			break;
		case 3:
			/* group chat name */
			elm_object_signal_emit(entry, "elm,state,groupchat,enabled", "elm");
			elm_object_part_text_set(entry, "elm.text.groupchat", "Josh");

			/* caller ID */
			cicon = elm_image_add(box);
			elm_image_file_set(cicon, ICON_DIR"/genlist/top.jpg", NULL);
			elm_object_signal_emit(entry, "elm,state,callerid,enabled", "elm");
			elm_object_part_content_set(entry, "elm.swallow.callerid", cicon);

			/* status icon*/
			elm_object_signal_emit(entry, "elm,state,content,status,enabled", "elm");
			pgbar = elm_progressbar_add(box);
			elm_object_style_set(pgbar, "list_process_small");
			evas_object_size_hint_align_set(pgbar, EVAS_HINT_FILL, 0.5);
			evas_object_size_hint_weight_set(pgbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			elm_progressbar_pulse(pgbar, EINA_TRUE);
			elm_object_part_content_set(entry, "elm.swallow.status", pgbar);
			break;
		case 4:
			/* group chat name */
			elm_object_signal_emit(entry, "elm,state,groupchat,enabled", "elm");
			elm_object_part_text_set(entry, "elm.text.groupchat", "Ellie");

			/* status text*/
			elm_object_part_text_set(entry, "elm.text.status", "Unread");
			break;
		case 5:
			/* caller ID */
			cicon = elm_image_add(box);
			elm_image_file_set(cicon, ICON_DIR"/genlist/top.jpg", NULL);
			elm_object_signal_emit(entry, "elm,state,callerid,enabled", "elm");
			elm_object_part_content_set(entry, "elm.swallow.callerid", cicon);

			/* status text*/
			elm_object_part_text_set(entry, "elm.text.status", "Sent");
			break;
		case 6:
			/* group chat name */
			elm_object_signal_emit(entry, "elm,state,groupchat,enabled", "elm");
			elm_object_part_text_set(entry, "elm.text.groupchat", "Ellie");

			/* caller ID */
			cicon = elm_image_add(box);
			elm_image_file_set(cicon, ICON_DIR"/genlist/iu.jpg", NULL);
			elm_object_signal_emit(entry, "elm,state,callerid,enabled", "elm");
			elm_object_part_content_set(entry, "elm.swallow.callerid", cicon);
			break;
		case 7:
			/* icon */
			elm_object_signal_emit(entry, "elm,state,contents,enabled", "elm");
			btn1 = elm_button_add(entry);
			elm_object_style_set(btn1, "cancel");
			elm_object_part_content_set(entry, "elm.swallow.icon1", btn1);
			break;
		case 8:
			/* select icon */
			check = elm_check_add(box);
			elm_check_state_set(check, EINA_TRUE);
			elm_layout_signal_emit(layout, "elm,state,select,enable", "elm");
			elm_object_part_content_set(layout, "elm.swallow.select_icon", check);
			break;
		default:
			break;
	}

	elm_object_part_content_set(layout, "elm.icon", entry);
	elm_box_pack_end(box, layout);

	g_msgcnt++;
	return ECORE_CALLBACK_RENEW;
}

static Evas_Object *
_create_bubbles(Evas_Object *parent)
{
	Evas_Object *box, *rect;

	box = elm_box_add(parent);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, 0.0);
	evas_object_show(box);

	/* Add top padding */
	rect = evas_object_rectangle_add(evas_object_evas_get(box));
	evas_object_size_hint_min_set(rect, 0, 30*elm_config_scale_get());
	evas_object_show(rect);
	elm_box_pack_end(box, rect);

	g_ani = ecore_animator_add(_bubble_add, box);

	return box;
}

static Evas_Object *
_create_scroller(Evas_Object *parent)
{
	Evas_Object *scroller = elm_scroller_add(parent);

	elm_scroller_bounce_set(scroller, EINA_FALSE, EINA_TRUE);
	elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	evas_object_show(scroller);

	return scroller;
}

static Eina_Bool
_pop_cb(void *data, Elm_Object_Item *it)
{
	if (g_ani) ecore_animator_del(g_ani);
	g_ani = NULL;
	return EINA_TRUE;
}

void
bubble_cb(void *data, Evas_Object * obj, void *event_info)
{
	struct appdata *ad;
	Elm_Object_Item *navi_it;

	ad = (struct appdata *)data;
	if (ad == NULL) return;

	Evas_Object *scroller, *layout_inner;

	scroller = _create_scroller(ad->nf);
	navi_it = elm_naviframe_item_push(ad->nf, _("Bubble"), NULL, NULL, scroller, NULL);
	elm_naviframe_item_pop_cb_set(navi_it, _pop_cb, NULL);
	g_scroller = scroller;

	layout_inner = _create_bubbles(ad->nf);
	elm_object_content_set(scroller, layout_inner);
}
