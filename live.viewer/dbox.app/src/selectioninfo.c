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

#define ITEM_STYLE_NUM	2
#define ITEM_COUNT	10

static void _selection_info_cb(void *data, Evas_Object *obj, void *event_info);
static void _selection_info_with_panes_cb(void *data, Evas_Object *obj, void *event_info);
static void _selection_info_with_center_align_cb(void *data, Evas_Object *obj, void *event_info);
static struct _menu_item menu_its[] = {
	{ "SelectionInfo", _selection_info_cb },
	{ "SelectionInfo with center align", _selection_info_with_center_align_cb },
	{ "SelectionInfo with Panes", _selection_info_with_panes_cb },
	/* do not delete below */
	{ NULL, NULL }
};
static char *styles[] = {
	"SelectionInfo",
	"SelectionInfa with center align",
	"SelectionInfo with Panes"
};
static char *countries[] = {
	"Argentina", "Australia", "Bangladesh", "Belgium", "Canada", "Chile",
	"Dominican Republic", "Egypt", "England", "Fiji"
};

typedef struct _SelInfo_Data SelInfo_Data;
struct _SelInfo_Data {
	struct appdata* ad;
	Evas_Object *selectioninfo_popup;
	int checked_count;
};

static SelInfo_Data *seld;
static Elm_Genlist_Item_Class itc;
static Eina_Bool state_pointer[ITEM_COUNT] = {0};

static void _create_selectioninfo(Evas_Object * data);

static int _checked_count_get()
{
	int i;
	int count = 0;

	for (i = 0 ; i < ITEM_COUNT ; i++)
		if (state_pointer[i])
			count++;
	return count;
}

static void _theme_set(Evas_Object * obj, Evas_Object *data)
{
	char text[128];
	const char *style = evas_object_data_get(data, "style");

	/* Set the layout theme */
	seld->checked_count = _checked_count_get();
	if (!seld->checked_count) {
		evas_object_hide(seld->selectioninfo_popup);
		return;
	}
	elm_layout_theme_set(obj, "standard", "selectioninfo", style);
	evas_object_show(seld->selectioninfo_popup);
	snprintf(text, sizeof(text), "%d %s", seld->checked_count, _("File(s) Selected"));
	/* Set the text */
	elm_object_part_text_set(obj, "elm.text", text);
}

static void _create_selectioninfo(Evas_Object *data)
{
	Evas_Object *selectioninfo_layout;
	if (!seld->selectioninfo_popup) {
		seld->selectioninfo_popup = elm_notify_add(data);
		elm_notify_align_set(seld->selectioninfo_popup, ELM_NOTIFY_ALIGN_FILL, 1.0);
		selectioninfo_layout = elm_layout_add(seld->selectioninfo_popup);
		elm_object_content_set(seld->selectioninfo_popup, selectioninfo_layout);
	}

	elm_notify_timeout_set(seld->selectioninfo_popup, 3.0);
	_theme_set(elm_object_content_get(seld->selectioninfo_popup), data);
}

static void _check_cb(void* data, Evas_Object *obj, void *event_info)
{
	_create_selectioninfo(data);
}

static char* _gl_text_get (void *data, Evas_Object *obj, const char *part)
{
	char buffer[PATH_MAX];
	int index = (int) data;

	if (strncmp("elm.text", part, strlen("elm.text")) == 0) {
		snprintf(buffer, sizeof(buffer), "%s", countries[index]);
		return strdup(buffer);
	}
	return NULL;
}

static Evas_Object *_gl_content_get (void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *check;
	int index = (int) data;

	check = elm_check_add(obj);
	elm_check_state_pointer_set(check, &state_pointer[index]);
	evas_object_smart_callback_add(check, "changed", _check_cb, obj);
	evas_object_show(check);
	return check;
}

static void _gl_sel(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *item = (Elm_Object_Item *) event_info;

	elm_genlist_item_selected_set(item, EINA_FALSE);
}

static void
_unpress(void *data, Evas_Object *obj, void *event_info)
{
	printf("unpress, size : %f\n", elm_panes_content_left_size_get(obj));
}

static Evas_Object *_create_genlist(const char *selectioninfo_style)
{
	int i;
	Elm_Object_Item *item;
	Evas_Object *genlist;

	itc.item_style = "1text.1icon.2";
	itc.func.text_get = _gl_text_get;
	itc.func.content_get = _gl_content_get;

	genlist = elm_genlist_add(seld->ad->nf);
	elm_object_style_set(genlist, UNCLIPPED_CONTENT);
	evas_object_data_set(genlist, "style", selectioninfo_style);

	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);

	for (i = 0; i < ITEM_COUNT; i++)
		item = elm_genlist_item_append(genlist, &itc, (void *) i,
		                               NULL,
		                               ELM_GENLIST_ITEM_NONE,
		                               _gl_sel,
		                               NULL);
	return genlist;
}

static Evas_Object *_create_panes()
{
	int i;
	Evas_Object *panes, *genlist, *btn;

	panes = elm_panes_add(seld->ad->nf);

	/*
	 * By default, pane's type is vertical.
	 * Setting the expansion weight hints for the pane.
	 */
	evas_object_size_hint_weight_set(panes, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(panes, EVAS_HINT_FILL, EVAS_HINT_FILL);

	evas_object_smart_callback_add(panes, "unpress", _unpress, panes);

	/* Creating a control to be set as one of the contents of the pane */
	btn = elm_button_add(panes);
	elm_object_text_set(btn, "Right");
	evas_object_size_hint_weight_set(btn, 1.0, 1.0);
	evas_object_size_hint_align_set(btn, -1.0, -1.0);
	/* Setting the relative size of the left view of the pane */
	elm_panes_content_left_size_set(panes, 1.0);
	/* Setting the left content of the pane */
	elm_object_part_content_set(panes, "right", btn);

	itc.item_style = "1text.1icon.2";
	itc.func.text_get = _gl_text_get;
	itc.func.content_get = _gl_content_get;

	genlist = elm_genlist_add(seld->ad->nf);
	elm_object_style_set(genlist, UNCLIPPED_CONTENT);
	evas_object_data_set(genlist, "style", "center_text");

	elm_object_part_content_set(panes, "left", genlist);

	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);

	for (i = 0; i < ITEM_COUNT; i++)
		elm_genlist_item_append(genlist, &itc, (void *) i, NULL, ELM_GENLIST_ITEM_NONE, _gl_sel, NULL);
	return panes;
}

static Eina_Bool _back_btn_cb(void *data, Elm_Object_Item *it)
{
	int i;

	for (i = 0; i < ITEM_COUNT; i++)
		state_pointer[i] = EINA_FALSE;

	if (seld->selectioninfo_popup)
		evas_object_del(seld->selectioninfo_popup);
	seld->selectioninfo_popup = NULL;
	return EINA_TRUE;
}

static Eina_Bool _pop_cb(void *data, Elm_Object_Item *it)
{
	if (seld) {
		free(seld);
		seld = NULL;
	}

	return EINA_TRUE;
}

static void _selection_info_cb(void *data, Evas_Object *obj, void *event_info)
{
	int index = (int) data;
	Elm_Object_Item *navi_it;
	Evas_Object *genlist;

	elm_list_item_selected_set((Elm_Object_Item *) event_info, EINA_FALSE);
	genlist = _create_genlist("default");
	navi_it = elm_naviframe_item_push(seld->ad->nf, styles[index] , NULL,
	                                  NULL, genlist, NULL);
	elm_naviframe_item_pop_cb_set(navi_it, _back_btn_cb, NULL);
}

static void _selection_info_with_center_align_cb(void *data, Evas_Object *obj, void *event_info)
{
	int index = (int) data;
	Elm_Object_Item *navi_it;
	Evas_Object *genlist;

	elm_list_item_selected_set((Elm_Object_Item *) event_info, EINA_FALSE);
	genlist = _create_genlist("center_text");
	navi_it = elm_naviframe_item_push(seld->ad->nf, styles[index] , NULL,
	                                  NULL, genlist, NULL);
	elm_naviframe_item_pop_cb_set(navi_it, _back_btn_cb, NULL);
}

static void _selection_info_with_panes_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *panes;
	int index = (int) data;
	Elm_Object_Item *navi_it;

	elm_list_item_selected_set((Elm_Object_Item *) event_info, EINA_FALSE);
	panes = _create_panes();
	navi_it = elm_naviframe_item_push(seld->ad->nf, styles[index] , NULL,
	                                  NULL, panes, NULL);
	elm_naviframe_item_pop_cb_set(navi_it, _back_btn_cb, NULL);
}

static Evas_Object *_create_list_menu(struct appdata *ad)
{
	Evas_Object *list;
	int idx = 0;

	if (ad == NULL)
		return NULL;
	list = elm_list_add(ad->nf);
	elm_list_mode_set(list, ELM_LIST_COMPRESS);
	while (menu_its[idx].name != NULL) {
		elm_list_item_append(
			list,
			menu_its[idx].name,
			NULL,
			NULL,
			menu_its[idx].func,
			(void *) idx);
		++idx;
	}
	elm_list_go(list);
	return list;
}

void selectioninfo_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *layout_inner;
	Elm_Object_Item *navi_it;
	struct appdata *ad = data;
	if (ad == NULL) return;

	seld = calloc(1, sizeof(SelInfo_Data));
	if (!seld) return;
	seld->ad = ad;

	layout_inner = _create_list_menu(ad);
	navi_it = elm_naviframe_item_push(ad->nf, _("Selection Info") , NULL, NULL, layout_inner, NULL);
	elm_naviframe_item_pop_cb_set(navi_it, _pop_cb, NULL);
}
