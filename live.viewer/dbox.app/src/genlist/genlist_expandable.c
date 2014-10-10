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
  Genlist Expandable List
 ********************************************************/
#define IDX_TOP     -1
#define IDX_BOTTOM  -99

typedef struct _Item_Data
{
	Eina_Bool top;
	Eina_Bool bottom;
	int index;
	Elm_Object_Item *item;
} Item_Data;

const char * mails[] = {
	"Gmail", "Hotmail", "Hanmail", "Yahoo", "Naver", "Empas", "Samsung", "Daum", NULL
};

const char * mailbox[] = {
	"Inbox", "Outbox", "Sent", "Favorite", "Trash", "Spam", "Spam2", NULL
};

const char * mailbox2[] = {
	"Private", "Friends", "Work", "TODO", NULL
};

const char longtxt[] = "This is very very veryyyyyy long longggggggg multiline text";

static void _print_item(Elm_Object_Item *it)
{
	const char *txt = elm_object_item_part_text_get(it, "elm.text");
	int data = (int)elm_object_item_data_get(it);
	if (!txt)
		txt = elm_object_item_part_text_get(it, "elm.text.1");
	printf(" %p, text(%s), data(%d), index[%d]\n", it, txt, data, elm_genlist_item_index_get(it));
}

static Evas_Object *_gl_icon_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *icon;
	Item_Data *id = data;
	icon = elm_image_add(obj);
	if (id->item && elm_genlist_item_expanded_get(id->item))
		elm_image_file_set(icon, ICON_DIR"/genlist/00_icon_favorite_on_45x45.png", NULL);
	else
		elm_image_file_set(icon, ICON_DIR"/genlist/00_icon_favorite_off_45x45.png", NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	return icon;
}

static Evas_Object *_gl_content_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *icon;

	if (!strcmp(part, "elm.swallow.colorbar")) {
		icon = evas_object_rectangle_add(evas_object_evas_get(obj));
		evas_object_size_hint_align_set(icon, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_color_set(icon, 80, 107, 207, 255);
		return icon;
	} else
		return _gl_icon_get(data, obj, part);

	return NULL;
}

static Evas_Object *
_gl_icon_chk_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *icon = elm_check_add(obj);
	evas_object_propagate_events_set(icon, EINA_FALSE);
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

static char *_gl_label_get(void *data, Evas_Object *obj, const char *part)
{
	Item_Data *id = data;
	int idx = id->index;
	//printf("text_get: label1: %s\n", mails[idx]);
	if (!mails[idx]) return strdup("NUUUUUUL");
	return strdup(mails[idx]);
}

static char *_gl_label2_get(void *data, Evas_Object *obj, const char *part)
{
	Item_Data *id = data;
	int idx = id->index;
	//printf("text_get: label2: %s\n", mailbox[idx]);
	if (!mailbox[idx]) return strdup("NUUUUUUL");
	return strdup(mailbox[idx]);
}

static char *_gl_label3_get(void *data, Evas_Object *obj, const char *part)
{
	Item_Data *id = data;
	int idx = id->index;
	printf("text_get: label3: %s\n", mailbox2[idx]);
	if (!mailbox2[idx]) return strdup("NUUUUUUL");
	return strdup(mailbox2[idx]);
}

static char *_gl_multiline_label_get(void *data, Evas_Object *obj, const char *part)
{
	//printf("text_get: label: %s\n", longtxt);
	return strdup(longtxt);
}

static void _gl_sel(void *data, Evas_Object *obj, void *ei)
{
	if (!ei) return;
	Eina_Bool expanded = EINA_FALSE;

	elm_genlist_item_selected_set(ei, EINA_FALSE);

	expanded = elm_genlist_item_expanded_get(ei);
	elm_genlist_item_expanded_set(ei, !expanded);

	const Elm_Genlist_Item_Class *itc = elm_genlist_item_item_class_get(ei);
	if (itc->item_style &&
		(!strcmp(itc->item_style, "1text.1icon.2") ||
		 !strcmp(itc->item_style, "2text.1icon.6")))
		elm_genlist_item_fields_update(ei, "elm.icon", ELM_GENLIST_ITEM_FIELD_CONTENT);
}

static void _gl_exp(void *data, Evas_Object *obj, void *event_info)
{
	Item_Data *id;
	Elm_Object_Item *it;
	Elm_Object_Item *parent = event_info;
	Evas_Object *gl = elm_object_item_widget_get(parent);
	int depth = 0;

	Elm_Genlist_Item_Class *itc1, *itc2, *itc3, *itc4, *itc5, *itc6, *itc7;
	itc1 = elm_genlist_item_class_new();
	itc2 = elm_genlist_item_class_new();
	itc3 = elm_genlist_item_class_new();
	itc4 = elm_genlist_item_class_new();
	itc5 = elm_genlist_item_class_new();
	itc6 = elm_genlist_item_class_new();
	itc7 = elm_genlist_item_class_new();

	itc1->item_style = "dialogue/1text";
	itc1->func.text_get = _gl_label2_get;

	itc2->item_style = "dialogue/1text.2icon";
	itc2->func.text_get = _gl_label2_get;
	itc2->func.content_get = _gl_icon_get;

	itc3->item_style = "dialogue/1text/expandable";
	itc3->func.text_get = _gl_label2_get;

	itc4->item_style = "dialogue/1text.2icon.divider";
	itc4->func.text_get = _gl_label2_get;
	itc4->func.content_get = _gl_icon_get;

	itc5->item_style = "dialogue/1text.1icon.3";
	itc5->func.text_get = _gl_label3_get;
	itc5->func.content_get = _gl_icon_get;

	itc6->item_style = "dialogue/1text.1icon.2";
	itc6->func.text_get = _gl_label3_get;
	itc6->func.content_get = _gl_icon_get;

	itc7->item_style = "dialogue/multiline/2text/expandable";
	itc7->func.text_get = _gl_multiline_label_get;

	Item_Data *pid = elm_object_item_data_get(parent);
	depth = elm_genlist_item_expanded_depth_get(parent);
	printf("Expanded: depth[%d]\n",  depth);
	if (depth == 0) {
		id = calloc(sizeof(Item_Data), 1);
		id->index = 0;
		it = elm_genlist_item_append(gl, itc1, id, parent, ELM_GENLIST_ITEM_TREE, _gl_sel, NULL);
		id->item = it;

		id = calloc(sizeof(Item_Data), 1);
		id->index = 1;
		it = elm_genlist_item_append(gl, itc2, id, parent, ELM_GENLIST_ITEM_NONE, _gl_sel, NULL);
		id->item = it;

		id = calloc(sizeof(Item_Data), 1);
		id->index = 2;
		it = elm_genlist_item_append(gl, itc3, id, parent, ELM_GENLIST_ITEM_TREE, _gl_sel, NULL);
		id->item = it;

		id = calloc(sizeof(Item_Data), 1);
		id->index = 3;
		it = elm_genlist_item_append(gl, itc4, id, parent, ELM_GENLIST_ITEM_NONE, _gl_sel, NULL);
		id->item = it;

		id = calloc(sizeof(Item_Data), 1);
		id->index = 4;
		it = elm_genlist_item_append(gl, itc1, id, parent, ELM_GENLIST_ITEM_NONE, _gl_sel, NULL);

		id = calloc(sizeof(Item_Data), 1);
		id->index = 5;
		it = elm_genlist_item_append(gl, itc2, id, parent, ELM_GENLIST_ITEM_NONE, _gl_sel, NULL);

		id = calloc(sizeof(Item_Data), 1);
		id->index = 6;
		it = elm_genlist_item_append(gl, itc3, id, parent, ELM_GENLIST_ITEM_NONE, _gl_sel, NULL);

		id = calloc(sizeof(Item_Data), 1);
		id->index = 7;
		it = elm_genlist_item_append(gl, itc4, id, parent, ELM_GENLIST_ITEM_NONE, _gl_sel, NULL);

		id = calloc(sizeof(Item_Data), 1);
		id->index = 8;
		if (pid->bottom) id->bottom = EINA_TRUE;
		it = elm_genlist_item_append(gl, itc7, id, parent, ELM_GENLIST_ITEM_NONE, _gl_sel, NULL);
		id->item = it;
	} else if (depth == 1) {
		id = calloc(sizeof(Item_Data), 1);
		id->index = 0;
		it = elm_genlist_item_append(gl, itc5, id, parent, ELM_GENLIST_ITEM_NONE, _gl_sel, NULL);
		id->item = it;

		id = calloc(sizeof(Item_Data), 1);
		id->index = 1;
		it = elm_genlist_item_append(gl, itc6, id, parent, ELM_GENLIST_ITEM_NONE, _gl_sel, NULL);
		id->item = it;

		id = calloc(sizeof(Item_Data), 1);
		id->index = 2;
		it = elm_genlist_item_append(gl, itc5, id, parent, ELM_GENLIST_ITEM_NONE, _gl_sel, NULL);
		id->item = it;

		id = calloc(sizeof(Item_Data), 1);
		id->index = 3;
		if (pid->bottom) id->bottom = EINA_TRUE;
		it = elm_genlist_item_append(gl, itc6, id, parent, ELM_GENLIST_ITEM_NONE, _gl_sel, NULL);
		id->item = it;
	}

	elm_genlist_item_class_free(itc1);
	elm_genlist_item_class_free(itc2);
	elm_genlist_item_class_free(itc3);
	elm_genlist_item_class_free(itc4);
	elm_genlist_item_class_free(itc5);
	elm_genlist_item_class_free(itc6);
	elm_genlist_item_class_free(itc7);
}

static void _gl_con(void *data, Evas_Object *obj, void *ei)
{
	elm_genlist_item_subitems_clear(ei);
	Item_Data *id = elm_object_item_data_get(ei);
	if (id->bottom) {
		printf("Last parent: bottom\n");
		elm_object_item_signal_emit(ei, "elm,state,bottom", "");
	}
}

static void _gl_exp_ndepth(void *data, Evas_Object *obj, void *event_info)
{
	Item_Data *id;
	Elm_Object_Item *it;
	Elm_Object_Item *parent = event_info;
	Evas_Object *gl = elm_object_item_widget_get(parent);
	int depth = 0;

	depth = elm_genlist_item_expanded_depth_get(parent);
	printf("Contracted: depth[%d]\n",  depth);

	Elm_Genlist_Item_Class *itc1, *itc2;
	itc1 = elm_genlist_item_class_new();
	itc2 = elm_genlist_item_class_new();

	itc1->item_style = "1text.1icon.2";
	itc1->func.text_get = _gl_label_get;
	itc1->func.content_get = _gl_icon_get;

	itc2->item_style = "2text.1icon.6";
	itc2->func.text_get = _gl_label_get;
	itc2->func.content_get = _gl_icon_get;

	id = calloc(sizeof(Item_Data), 1);
	id->index = 0;
	it = elm_genlist_item_append(gl, itc1, id, parent, ELM_GENLIST_ITEM_TREE, _gl_sel, NULL);
	id->item = it;

	id = calloc(sizeof(Item_Data), 1);
	id->index = 1;
	it = elm_genlist_item_append(gl, itc2, id, parent, ELM_GENLIST_ITEM_NONE, _gl_sel, NULL);
	id->item = it;

	id = calloc(sizeof(Item_Data), 1);
	id->index = 2;
	it = elm_genlist_item_append(gl, itc2, id, parent, ELM_GENLIST_ITEM_TREE, _gl_sel, NULL);
	id->item = it;

	id = calloc(sizeof(Item_Data), 1);
	id->index = 2;
	it = elm_genlist_item_append(gl, itc1, id, parent, ELM_GENLIST_ITEM_TREE, _gl_sel, NULL);
	id->item = it;

	elm_genlist_item_class_free(itc1);
	elm_genlist_item_class_free(itc2);
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

static void _access_activate_cb(void *data, Evas_Object *part_obj, Elm_Object_Item *item)
{
	elm_genlist_item_selected_set(data, EINA_TRUE);
}

static void _realized(void *data, Evas_Object *obj, void *ei)
{
	if (!ei) return;
	Elm_Object_Item *parent = elm_genlist_item_parent_get(ei);
	Item_Data *id = elm_object_item_data_get(ei);
	//printf("Realized: data[%d], index[%d],", idx, elm_genlist_item_index_get(ei));

	if (id->top) {
		printf("top\n");
		elm_object_item_signal_emit(ei, "elm,state,top", "");
	} else if (id->bottom) {
		printf("bottom\n");
		if (!elm_genlist_item_expanded_get(ei))	{
			elm_object_item_signal_emit(ei, "elm,state,bottom", "");
			if (parent)
				elm_object_item_signal_emit(parent, "elm,state,center", "");
		}
	} else {
		printf("center\n");
		elm_object_item_signal_emit(ei, "elm,state,center", "");
	}

	// ===== Accessibility ====
	if (elm_config_access_get()) {
		// Register bg_dialogue image to be highlighted instead of whole item
		Evas_Object *acc = elm_object_item_part_access_register(ei, "bg_dialogue");
		elm_access_info_cb_set(acc, ELM_ACCESS_INFO, _accinfo_cb, ei);
		elm_access_activate_cb_set(acc, _access_activate_cb, ei);
	}
}

static void _selected(void *data, Evas_Object *obj, void *ei)
{
	printf("Selected, ");
	_print_item(ei);
}

void genlist_expandable_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad;
	Evas_Object *genlist;
	Elm_Object_Item *it;
	Item_Data *id;

	ad = (struct appdata *) data;
	if (ad == NULL) return;

	Elm_Genlist_Item_Class *itc, *itc2, *itc3, *itc4, *itc5, *itc6, *itc7;
	itc = elm_genlist_item_class_new();
	itc2 = elm_genlist_item_class_new();
	itc3 = elm_genlist_item_class_new();
	itc4 = elm_genlist_item_class_new();
	itc5 = elm_genlist_item_class_new();
	itc6 = elm_genlist_item_class_new();
	itc7 = elm_genlist_item_class_new();

	// Expandable item styles
	itc->item_style = "dialogue/2text/expandable";
	itc->func.text_get = _gl_label_get;

	itc2->item_style = "dialogue/2text.2/expandable";
	itc2->func.text_get = _gl_label_get;

	itc3->item_style = "dialogue/2text.3/expandable";
	itc3->func.text_get = _gl_label_get;

	itc4->item_style = "dialogue/2text.1icon/expandable";
	itc4->func.text_get = _gl_label_get;
	itc4->func.content_get = _gl_2icon_chk_get;

	itc5->item_style = "dialogue/3text.1icon/expandable";
	itc5->func.text_get = _gl_label_get;
	itc5->func.content_get = _gl_content_get;

	itc6->item_style = "dialogue/1text/expandable";
	itc6->func.text_get = _gl_label_get;

	itc7->item_style = "dialogue/multiline/2text/expandable";
	itc7->func.text_get = _gl_multiline_label_get;

	genlist = elm_genlist_add(ad->nf);
	// COMPRESS MODE
	// If multiline text (multiline entry or multiline textblock or sliding mode)
	// is used, use compress mode for compressing width to fit the viewport width.
	// So genlist can calculate item's height correctly.
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	printf("Compress mode enabled\n");

	id = calloc(sizeof(Item_Data), 1);
	id->top = EINA_TRUE;
	id->index = 0;
	it = elm_genlist_item_append(genlist, itc, id, NULL, ELM_GENLIST_ITEM_TREE, _gl_sel, NULL);
	id->item = it;

	id = calloc(sizeof(Item_Data), 1);
	id->index = 1;
	it = elm_genlist_item_append(genlist, itc2, id, NULL, ELM_GENLIST_ITEM_TREE, _gl_sel, NULL);
	id->item = it;

	id = calloc(sizeof(Item_Data), 1);
	id->index = 2;
	it = elm_genlist_item_append(genlist, itc3, id, NULL, ELM_GENLIST_ITEM_TREE, _gl_sel, NULL);
	id->item = it;

	id = calloc(sizeof(Item_Data), 1);
	id->index = 3;
	it = elm_genlist_item_append(genlist, itc4, id, NULL, ELM_GENLIST_ITEM_TREE, NULL, NULL);
	elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
	id->item = it;

	id = calloc(sizeof(Item_Data), 1);
	id->index = 4;
	it = elm_genlist_item_append(genlist, itc5, id, NULL, ELM_GENLIST_ITEM_TREE, _gl_sel, NULL);
	id->item = it;

	id = calloc(sizeof(Item_Data), 1);
	id->index = 5;
	it = elm_genlist_item_append(genlist, itc6, id, NULL, ELM_GENLIST_ITEM_TREE, _gl_sel, NULL);
	id->item = it;

	id = calloc(sizeof(Item_Data), 1);
	id->index = 6;
	id->bottom = EINA_TRUE;
	it = elm_genlist_item_append(genlist, itc7, id, NULL, ELM_GENLIST_ITEM_TREE, _gl_sel, NULL);
	id->item = it;

	elm_genlist_item_class_free(itc);
	elm_genlist_item_class_free(itc2);
	elm_genlist_item_class_free(itc3);
	elm_genlist_item_class_free(itc4);
	elm_genlist_item_class_free(itc5);
	elm_genlist_item_class_free(itc6);
	elm_genlist_item_class_free(itc7);

	evas_object_smart_callback_add(genlist, "expanded", _gl_exp, genlist);
	evas_object_smart_callback_add(genlist, "contracted", _gl_con, genlist);
	evas_object_smart_callback_add(genlist, "realized", _realized, genlist);
	evas_object_smart_callback_add(genlist, "selected", _selected, NULL);

	elm_naviframe_item_push(ad->nf, _("Expandable"), NULL, NULL, genlist, NULL);
}

void genlist_ndepth_expandable_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad;
	Evas_Object *genlist;
	Item_Data *id;
	Elm_Object_Item *it;

	ad = (struct appdata *) data;
	if (ad == NULL) return;

	Elm_Genlist_Item_Class *itc, *itc2;
	itc = elm_genlist_item_class_new();
	itc2 = elm_genlist_item_class_new();

	itc->item_style = "1text.1icon.2";
	itc->func.text_get = _gl_label_get;
	itc->func.content_get = _gl_icon_get;

	itc2->item_style = "2text.1icon.6";
	itc2->func.text_get = _gl_label_get;
	itc2->func.content_get = _gl_icon_get;

	genlist = elm_genlist_add(ad->nf);
	// HOMOGENEOUS MODE
	// If item height is same when each style name is same,
	// Use homogeneous mode.
	elm_genlist_homogeneous_set(genlist, EINA_TRUE);
	printf("Homogeneous mode enabled\n");

	id = calloc(sizeof(Item_Data), 1);
	id->index = 0;
	it = elm_genlist_item_append(genlist, itc, id, NULL, ELM_GENLIST_ITEM_TREE, _gl_sel, NULL);
	id->item = it;

	id = calloc(sizeof(Item_Data), 1);
	id->index = 1;
	it = elm_genlist_item_append(genlist, itc2, id, NULL, ELM_GENLIST_ITEM_TREE, _gl_sel, NULL);
	id->item = it;

	id = calloc(sizeof(Item_Data), 1);
	id->index = 1;
	it = elm_genlist_item_append(genlist, itc, id, NULL, ELM_GENLIST_ITEM_TREE, _gl_sel, NULL);
	id->item = it;

	id = calloc(sizeof(Item_Data), 1);
	id->index = 1;
	it = elm_genlist_item_append(genlist, itc2, id, NULL, ELM_GENLIST_ITEM_TREE, _gl_sel, NULL);
	id->item = it;

	elm_genlist_item_class_free(itc);
	elm_genlist_item_class_free(itc2);

	evas_object_smart_callback_add(genlist, "expanded", _gl_exp_ndepth, genlist);
	evas_object_smart_callback_add(genlist, "contracted", _gl_con, genlist);
	evas_object_smart_callback_add(genlist, "selected", _selected, NULL);
	elm_naviframe_item_push(ad->nf, _("Expandable"), NULL, NULL, genlist, NULL);
}
