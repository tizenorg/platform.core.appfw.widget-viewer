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

#define NUM_OF_NAMES 50
#ifndef __UNUSED__
#define __UNUSED__ __attribute__((unused))
#endif

#define APPEND                  "Append Item"
#define PREPEND                 "Prepend Item"
#define INSERT_AFTER            "Insert After"
#define INSERT_BEFORE           "Insert Before"
#define DEL_ITEM                "Delete Item"
#define HIDE_ITEM               "Hide Item"
#define DISABLE_ITEM            "Disable Item"
#define CLEAR_ITEMS             "Clear Items"
#define CLEAR_SUB_ITEMS         "Clear Sub Items"
#define UPDATE_CLASS            "Update Item Class"
#define UPDATE_ITEM             "Update Item"
#define EXPAND_ITEM             "Expand Item"
#define CONTRACT_ITEM           "Contract Item"
#define SHOW_FIRST_ITEM         "Show First Item"
#define BRING_IN_LAST_ITEM      "Bring in Last Item"
#define BRING_IN_RANDOM_ITEM    "Bring in Random Item"

#define CHANGE_SELECT_MODE      "Change Select Mode"
#define MULTI_SELECT_MODE       "Multi Select Mode"
#define CHANGE_ITEM_SELECT_MODE "Change Item Select Mode"
#define DECORATE_MODE           "Decorate all Mode"
#define ITEM_DECORATE_MODE      "Item Decorate Mode"
#define REORDER_MODE            "Reorder Mode"
#define RANDOM_TEST             "Random Test"

const char *random_tests[] = {
	APPEND, PREPEND, INSERT_AFTER, INSERT_BEFORE,
	DEL_ITEM, HIDE_ITEM, DISABLE_ITEM, CLEAR_ITEMS, CLEAR_SUB_ITEMS,
	UPDATE_CLASS, UPDATE_ITEM, EXPAND_ITEM, CONTRACT_ITEM,
	SHOW_FIRST_ITEM, BRING_IN_LAST_ITEM, BRING_IN_RANDOM_ITEM,
	CHANGE_SELECT_MODE, MULTI_SELECT_MODE, CHANGE_SELECT_MODE,
	DECORATE_MODE, ITEM_DECORATE_MODE, REORDER_MODE
};

#define CHANGE_LIST_MODE        "Change List Mode"
#define MAX_LEN 128
#define NUM_OF_ITEMS 2000
#define ITC_MAX 10
#define WIDTH 600
#define HEIGHT 800
static char **genlist_demo_names;

typedef struct _View_Data {
	struct appdata *ad;

	Evas_Object *box;
	Evas_Object *select_all_layout;
	Evas_Object *genlist;
	Evas_Object *ctx;
	Elm_Object_Item *navi_it;
	int itc_cnt;
	Elm_Object_Select_Mode select_mode;
	Ecore_Timer *test_timer;
	Elm_List_Mode list_mode;
	Elm_Genlist_Item_Class *itc[ITC_MAX];
	int g_idx;
} View_Data;

typedef struct _Item_Data
{
	int idx;
} Item_Data;

static Eina_Bool rotate_flag = EINA_FALSE;

static void _print_item(Elm_Object_Item *it)
{
	if (!it) {
		printf("print_item: Item is NULL\n");
		return;
	}
	const char *txt = elm_object_item_part_text_get(it, "elm.text");
	Item_Data *id = elm_object_item_data_get(it);
	if (!txt)
		txt = elm_object_item_part_text_get(it, "elm.text.1");
	printf(" %p, text(%s), data(%d), index[%d]\n", it, txt, id->idx, elm_genlist_item_index_get(it));
}

static void _print_selected_items(Evas_Object *obj)
{
	Eina_List *l, *sel;
	Elm_Object_Item *it;

	sel = elm_genlist_selected_items_get(obj);
	printf("----- Selected items ------\n");
	EINA_LIST_FOREACH(sel, l, it) _print_item(it);
	printf("---------------------------\n");
}

static void _view_free_cb(void *data, Evas *e, Evas_Object *obj, void *ei)
{
	View_Data *vd = data;
	int i = 0;
	if (vd->test_timer) ecore_timer_del(vd->test_timer);

	for (i = 0; i < ITC_MAX; i++) {
		elm_genlist_item_class_free(vd->itc[i]);
	}

	elm_object_style_set(vd->ad->bg, "default");
	free(vd);
}

static void _gl_del(void *data, Evas_Object *obj)
{
	// FIXME: unrealized callback will be called after this.
	// accessing Item_Data can be dangerous on unrealized callback.
	Item_Data *id = data;
	if (id) free(id);
}

static void _selected(void *data, Evas_Object *obj, void *ei)
{
	printf("Selected, ");
	_print_item(ei);
	_print_selected_items(obj);
}

static void _unselected(void *data, Evas_Object *obj, void *ei)
{
	printf("Unselected, ");
	_print_item(ei);
	_print_selected_items(obj);
}

static void _longpressed(void *data, Evas_Object *obj, void *ei)
{
	printf("lonpressed, ");
	_print_item(ei);
	_print_selected_items(obj);
}

static void _con(void *data, Evas_Object *obj, void *ei)
{
	printf("Contracted, ");
	_print_item(ei);
	elm_genlist_item_subitems_clear(ei);
}

static void _gl_sel(void *data, Evas_Object *obj, void *ei)
{
	printf("Item Selected, ");
	_print_item(ei);
}

static void _exp(void *data, Evas_Object *genlist, void *ei)
{
	View_Data *vd = data;
	Item_Data *id;
	int depth = elm_genlist_item_expanded_depth_get(ei);
	printf("Expanded: depth[%d]\n",  depth);
	_print_item(ei);

	id = calloc(sizeof(Item_Data), 1);
	id->idx = vd->g_idx;
	vd->g_idx++;
	elm_genlist_item_append(genlist, vd->itc[0], id, ei,
			ELM_GENLIST_ITEM_TREE, _gl_sel, id);

	id = calloc(sizeof(Item_Data), 1);
	id->idx = vd->g_idx;
	vd->g_idx++;
	elm_genlist_item_append(genlist, vd->itc[0], id, ei,
			ELM_GENLIST_ITEM_NONE, _gl_sel, id);
}

static char *_gl_text_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024];
	Item_Data *id = data;
	snprintf(buf, 1023, "%d, %s", id->idx, part);
	printf("text_get: %s\n", buf);
	return strdup(buf);}

static Evas_Object *_gl_content_get(void *data, Evas_Object *obj, const char *part)
{
	if (!strcmp(part, "elm.slide.swallow.1")) {
		Evas_Object *button = elm_button_add(obj);
		elm_object_style_set(button, "sweep");
		elm_object_text_set(button, "Sweep");
		return button;
	}
	Evas_Object *icon = elm_image_add(obj);
	elm_image_file_set(icon, ICON_DIR"/genlist/00_brightness_right.png", NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	return icon;
}

static void
_move_ctx(void *data, Evas_Object *obj)
{
	if(!data) return;
	View_Data *vd = data;
	Evas_Object *ctx = obj;
	Evas_Coord w, h;
	int pos = -1;

	elm_win_screen_size_get(vd->ad->win_main, NULL, NULL, &w, &h);
	pos = elm_win_rotation_get(vd->ad->win_main);

	switch (pos) {
		case 0:
		case 180:
			evas_object_move(ctx, 0, h);
			break;
		case 90:
			evas_object_move(ctx, 0, w);
			break;
		case 270:
			evas_object_move(ctx, h, w);
		break;
	}
}

static void
_dismissed_cb(void *data __UNUSED__, Evas_Object *obj , void *event __UNUSED__)
{
	View_Data *vd = data;
	Evas_Object *ctx = obj;

	if (!rotate_flag) {
		evas_object_del(ctx);
		ctx = NULL;
	}
	else {
		_move_ctx(vd, ctx);
		evas_object_show(ctx);
		rotate_flag = EINA_FALSE;
	}
}

static void
_resize_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Object *ctx = (Evas_Object *)data;

	if (ctx)
		rotate_flag = EINA_TRUE;
	else
		rotate_flag = EINA_FALSE;
}

static void
_rotate_cb(void *data, Evas_Object *obj, void *event_info)
{
	View_Data *vd = data;
	Evas_Object *ctx = vd->ctx;
	_move_ctx(vd, ctx);
	evas_object_show(ctx);
}

static void _delete_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	View_Data *vd = data;
	Evas_Object *navi = vd->ad->nf;
	Evas_Object *ctx = obj;

	evas_object_event_callback_del_full(navi, EVAS_CALLBACK_RESIZE, _resize_cb, ctx);
	evas_object_smart_callback_del_full(elm_object_top_widget_get(ctx), "rotation,changed", _rotate_cb, vd);
	evas_object_smart_callback_del(ctx, "dismissed", _dismissed_cb);
	evas_object_event_callback_del_full(ctx, EVAS_CALLBACK_DEL, _delete_cb, navi);
}

static void
_genlist_test(const char *label, View_Data *vd)
{
	Elm_Object_Item *it;
	printf("============================\n");
	printf("%s: ", label);
	if (!strcmp(label, _(APPEND))) {
		Item_Data *id = calloc(sizeof(Item_Data), 1);
		id->idx = vd->g_idx;
		vd->g_idx++;
		it = elm_genlist_item_append(vd->genlist, vd->itc[0], id, NULL,
				ELM_GENLIST_ITEM_NONE, _gl_sel, id);
		_print_item(it);
	} else if (!strcmp(label, _(PREPEND))) {
		Item_Data *id = calloc(sizeof(Item_Data), 1);
		id->idx = vd->g_idx;
		vd->g_idx++;
		it = elm_genlist_item_prepend(vd->genlist, vd->itc[0], id, NULL,
				ELM_GENLIST_ITEM_NONE, _gl_sel, id);
		_print_item(it);
	} else if (!strcmp(label, _(INSERT_AFTER))) {
		Eina_List *list = elm_genlist_selected_items_get(vd->genlist);
		if (list) {
			Elm_Object_Item *before = eina_list_data_get(eina_list_last(list));
			Item_Data *id = calloc(sizeof(Item_Data), 1);
			id->idx = vd->g_idx;
			vd->g_idx++;
			it = elm_genlist_item_insert_after(vd->genlist, vd->itc[0], id,
					NULL , before, ELM_GENLIST_ITEM_NONE, _gl_sel, id);
			_print_item(it);
		}
	} else if (!strcmp(label, _(INSERT_BEFORE))) {
		Eina_List *list = elm_genlist_selected_items_get(vd->genlist);
		if (list) {
			Elm_Object_Item *before = eina_list_data_get(eina_list_last(list));
			Item_Data *id = calloc(sizeof(Item_Data), 1);
			id->idx = vd->g_idx;
			vd->g_idx++;
			it = elm_genlist_item_insert_before(vd->genlist, vd->itc[0], id,
					NULL , before, ELM_GENLIST_ITEM_NONE, _gl_sel, id);
			_print_item(it);
		}
	} else if (!strcmp(label, _(DEL_ITEM))) {
		Eina_List *list = elm_genlist_selected_items_get(vd->genlist);
		Eina_List *ll, *l;
		while (list) {
			it = eina_list_data_get(list);
			list = eina_list_next(list);
			printf("Item(%p) is going to be deleted\n", it);
			_print_item(it);
			elm_object_item_del(it);
		}
	} else if (!strcmp(label, _(HIDE_ITEM))) {
		Eina_List *list = elm_genlist_selected_items_get(vd->genlist);
		if (list) {
			it = eina_list_data_get(eina_list_last(list));
			_print_item(it);
			elm_genlist_item_hide_set(it, EINA_TRUE);
		}
	} else if (!strcmp(label, _(DISABLE_ITEM))) {
		Eina_List *list = elm_genlist_selected_items_get(vd->genlist);
		if (list) {
			it = eina_list_data_get(eina_list_last(list));
			_print_item(it);
			elm_object_item_disabled_set(it, EINA_TRUE);
			//printf("Item(%p) is disabled\n", it);
		}
	} else if (!strcmp(label, _(CLEAR_ITEMS))) {
		elm_genlist_clear(vd->genlist);
	} else if (!strcmp(label, _(CLEAR_SUB_ITEMS))) {
		Eina_List *list = elm_genlist_selected_items_get(vd->genlist);
		if (list) {
			it = eina_list_data_get(eina_list_last(list));
			elm_genlist_item_subitems_clear(it);
		}
	} else if (!strcmp(label, _(UPDATE_CLASS))) {
		Eina_List *list = elm_genlist_selected_items_get(vd->genlist);
		if (list) {
			it = eina_list_data_get(eina_list_last(list));
			vd->itc_cnt++;
			if (vd->itc_cnt >= ITC_MAX) vd->itc_cnt = 0;
			elm_genlist_item_item_class_update(it, vd->itc[vd->itc_cnt]);
			printf("New item style [%s]\n", vd->itc[vd->itc_cnt]->item_style);
			_print_item(it);
		}
	} else if (!strcmp(label, _(UPDATE_ITEM))) {
		Eina_List *list = elm_genlist_selected_items_get(vd->genlist);
		if (list) {
			it = eina_list_data_get(eina_list_last(list));
			Item_Data *id = elm_object_item_data_get(it);
			id->idx++;
			elm_genlist_item_update(it);
			_print_item(it);
		}
	} else if (!strcmp(label, _(EXPAND_ITEM))) {
		Eina_List *list = elm_genlist_selected_items_get(vd->genlist);
		if (list) {
			it = eina_list_data_get(eina_list_last(list));
			elm_genlist_item_expanded_set(it, EINA_TRUE);
			_print_item(it);
		}
	} else if (!strcmp(label, _(CONTRACT_ITEM))) {
		Eina_List *list = elm_genlist_selected_items_get(vd->genlist);
		if (list) {
			it = eina_list_data_get(eina_list_last(list));
			elm_genlist_item_expanded_set(it, EINA_FALSE);
			_print_item(it);
		}
	} else if (!strcmp(label, _(SHOW_FIRST_ITEM))) {
		Elm_Object_Item *it = elm_genlist_first_item_get(vd->genlist);
		elm_genlist_item_show(it, ELM_GENLIST_ITEM_SCROLLTO_TOP);
		_print_item(it);
	} else if (!strcmp(label, _(BRING_IN_LAST_ITEM))) {
		Elm_Object_Item *it = elm_genlist_last_item_get(vd->genlist);
		elm_genlist_item_bring_in(it, ELM_GENLIST_ITEM_SCROLLTO_MIDDLE);
		_print_item(it);
	} else if (!strcmp(label, _(BRING_IN_RANDOM_ITEM))) {
		int idx = 0, i = 0;
		int cnt = elm_genlist_items_count(vd->genlist);
		if (cnt) idx = rand() % cnt;
		int type = ELM_GENLIST_ITEM_SCROLLTO_IN << (rand() % 3);

		Elm_Object_Item *it = elm_genlist_first_item_get(vd->genlist);
		for (i = 0 ; i < idx ; i++) {
			it = elm_genlist_item_next_get(it);
		}
		elm_genlist_item_bring_in(it, type);
		_print_item(it);
	} else if (!strcmp(label, _(CHANGE_SELECT_MODE))) {
		Elm_Object_Select_Mode mode = elm_genlist_select_mode_get(vd->genlist);
		mode++;
		if (mode >ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY) mode = 0;
		elm_genlist_select_mode_set(vd->genlist, mode);
		printf("Select mode is changed to %d\n", mode);
	} else if (!strcmp(label, _(MULTI_SELECT_MODE))) {
		Eina_Bool multi = elm_genlist_multi_select_get(vd->genlist);
		elm_genlist_multi_select_set(vd->genlist, !multi);
	} else if (!strcmp(label, _(CHANGE_ITEM_SELECT_MODE))) {
		Eina_List *list = elm_genlist_selected_items_get(vd->genlist);
		if (list) {
			it = eina_list_data_get(eina_list_last(list));
			Elm_Object_Select_Mode mode = elm_genlist_item_select_mode_get(it);
			mode++;
			if (mode >ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY) mode = 0;
			elm_genlist_item_select_mode_set(it, mode);
			printf("New select mode: %d\n", mode);
			_print_item(it);
		}
	} else if (!strcmp(label, _(DECORATE_MODE))) {
		Eina_Bool mode = elm_genlist_decorate_mode_get(vd->genlist);
		elm_genlist_decorate_mode_set(vd->genlist, !mode);
		printf("Decorate mode is changed to %d\n", !mode);
	} else if (!strcmp(label, _(ITEM_DECORATE_MODE))) {
		Eina_List *list = elm_genlist_selected_items_get(vd->genlist);
		if (list) {
			it = eina_list_data_get(eina_list_last(list));
			const char *mode = elm_genlist_item_decorate_mode_get(it);
			Elm_Object_Item *it2 = elm_genlist_decorated_item_get(vd->genlist);
			if (it2 && (it == it2)) {
				elm_genlist_item_decorate_mode_set(it, NULL, EINA_FALSE);
				printf("Item[%p]: Decorate mode is disabled from %s\n",
						it, mode);
			} else {
				elm_genlist_item_decorate_mode_set(it, "slide", EINA_TRUE);
				printf("Item[%p]: Decorate mode is enabled slide from %s\n",
						it, mode);
			}
			_print_item(it);
		}
	} else if (!strcmp(label, _(REORDER_MODE))) {
		Eina_Bool mode = elm_genlist_reorder_mode_get(vd->genlist);
		elm_genlist_reorder_mode_set(vd->genlist, !mode);
		printf("Reorder is %d\n", mode);
	} else if (!strcmp(label, _(CHANGE_LIST_MODE))) {
		elm_genlist_mode_set(vd->genlist, vd->list_mode);
		printf("List mode is %d\n", vd->list_mode);
		vd->list_mode++;
		if (vd->list_mode >= ELM_LIST_LAST)
			vd->list_mode = ELM_LIST_COMPRESS;
	}
	printf("\n");
}

static Eina_Bool
_random_test(void *data)
{
	int len = sizeof(random_tests)/sizeof(random_tests[0]);
	View_Data *vd = data;
	int r = rand()%len;
	_genlist_test(random_tests[r], vd);
	return EINA_TRUE;
}

static void
_extpopup_cb(void *data, Evas_Object *obj, void *event)
{
	if ((!data) || (!obj)) return;
	View_Data *vd = data;
	const char *label = elm_object_item_text_get((Elm_Object_Item *) event);

	if (label) {
		if (!strcmp(label, _(RANDOM_TEST))) {
			if (vd->test_timer)	{
				ecore_timer_del(vd->test_timer);
				vd->test_timer = NULL;
			}
			else vd->test_timer = ecore_timer_add(0.1, _random_test, vd);
		} else _genlist_test(label, vd);
	}
	evas_object_del(obj);
}

static void
_menu_toolbar_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!data) return;
	Evas_Object *ctx;
	Evas_Coord x, y, w, h;
	View_Data *vd = data;
	double s = elm_config_scale_get();
	evas_object_geometry_get(obj, &x, &y, &w, &h);

	ctx = elm_ctxpopup_add(vd->ad->nf);
	evas_object_smart_callback_add(ctx,"dismissed", _dismissed_cb, NULL);
	elm_ctxpopup_item_append(ctx, _(APPEND), NULL, _extpopup_cb, vd);
	elm_ctxpopup_item_append(ctx, _(PREPEND), NULL, _extpopup_cb, vd);
	elm_ctxpopup_item_append(ctx, _(INSERT_AFTER), NULL, _extpopup_cb, vd);
	elm_ctxpopup_item_append(ctx, _(INSERT_BEFORE), NULL, _extpopup_cb, vd);
	elm_ctxpopup_item_append(ctx, _(DEL_ITEM), NULL, _extpopup_cb, vd);
	elm_ctxpopup_item_append(ctx, _(HIDE_ITEM), NULL, _extpopup_cb, vd);
	elm_ctxpopup_item_append(ctx, _(DISABLE_ITEM), NULL, _extpopup_cb, vd);
	elm_ctxpopup_item_append(ctx, _(UPDATE_CLASS), NULL, _extpopup_cb, vd);
	elm_ctxpopup_item_append(ctx, _(UPDATE_ITEM), NULL, _extpopup_cb, vd);
	elm_ctxpopup_item_append(ctx, _(EXPAND_ITEM), NULL, _extpopup_cb, vd);
	elm_ctxpopup_item_append(ctx, _(CONTRACT_ITEM), NULL, _extpopup_cb, vd);
	elm_ctxpopup_item_append(ctx, _(SHOW_FIRST_ITEM), NULL, _extpopup_cb, vd);
	elm_ctxpopup_item_append(ctx, _(BRING_IN_LAST_ITEM), NULL, _extpopup_cb, vd);
	elm_ctxpopup_item_append(ctx, _(BRING_IN_RANDOM_ITEM), NULL, _extpopup_cb, vd);
	elm_ctxpopup_item_append(ctx, _(CLEAR_ITEMS), NULL, _extpopup_cb, vd);
	elm_ctxpopup_item_append(ctx, _(CLEAR_SUB_ITEMS), NULL, _extpopup_cb, vd);
	elm_ctxpopup_item_append(ctx, _(CHANGE_SELECT_MODE), NULL, _extpopup_cb, vd);
	elm_ctxpopup_item_append(ctx, _(MULTI_SELECT_MODE), NULL, _extpopup_cb, vd);
	elm_ctxpopup_item_append(ctx, _(CHANGE_ITEM_SELECT_MODE), NULL, _extpopup_cb, vd);
	elm_ctxpopup_item_append(ctx, _(DECORATE_MODE), NULL, _extpopup_cb, vd);
	elm_ctxpopup_item_append(ctx, _(ITEM_DECORATE_MODE), NULL, _extpopup_cb, vd);
	elm_ctxpopup_item_append(ctx, _(REORDER_MODE), NULL, _extpopup_cb, vd);
	elm_ctxpopup_item_append(ctx, _(RANDOM_TEST), NULL, _extpopup_cb, vd);

	evas_object_size_hint_max_set(ctx, s*WIDTH, s*HEIGHT);
	evas_object_move(ctx, x+w/2, y+h/2);
	evas_object_show(ctx);
}

static void _menu_more_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!data) return;
	Evas_Object *ctx;
	View_Data *vd = data;
	struct appdata *ad;
	ad = vd->ad;
	double s = elm_config_scale_get();

	ctx = elm_ctxpopup_add(ad->nf);
	elm_object_style_set(ctx, "more/default");
	evas_object_smart_callback_add(ctx,"dismissed", _dismissed_cb, vd);
	evas_object_event_callback_add(ctx, EVAS_CALLBACK_DEL, _delete_cb, vd);
	evas_object_event_callback_add(ad->nf, EVAS_CALLBACK_RESIZE, _resize_cb, ctx);
	evas_object_smart_callback_add(elm_object_top_widget_get(ctx), "rotation,changed", _rotate_cb, vd);

	elm_ctxpopup_item_append(ctx, _(CHANGE_LIST_MODE), NULL, _extpopup_cb, vd);
	evas_object_size_hint_max_set(ctx, s*WIDTH, s*HEIGHT);
	elm_ctxpopup_direction_priority_set(ctx, ELM_CTXPOPUP_DIRECTION_UP,
									ELM_CTXPOPUP_DIRECTION_UNKNOWN,
									ELM_CTXPOPUP_DIRECTION_UNKNOWN,
									ELM_CTXPOPUP_DIRECTION_UNKNOWN);

	_move_ctx(vd, ctx);
	evas_object_show(ctx);
}

static Evas_Object *_create_menu_toolbar(View_Data *vd)
{
	Evas_Object *obj, *seg_btn;
	Elm_Object_Item *item = NULL;
	/*create toolbar */
	obj = elm_toolbar_add(vd->ad->nf);
	if(obj == NULL) return NULL;
	elm_object_style_set(obj, "default");
	elm_toolbar_shrink_mode_set(obj, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_transverse_expanded_set(obj, EINA_TRUE);
	elm_toolbar_select_mode_set(obj, ELM_OBJECT_SELECT_MODE_NONE);

	seg_btn = elm_button_add(obj);
	elm_object_style_set(seg_btn, "style1");
	elm_object_text_set(seg_btn, _("Menu"));
	item = elm_toolbar_item_append(obj, NULL, NULL, NULL, NULL);
	elm_object_item_part_content_set(item, "object", seg_btn);
	evas_object_smart_callback_add(seg_btn, "clicked", _menu_toolbar_cb, vd);
	return obj;
}

static Evas_Object *_create_menu_more_button(View_Data *vd)
{
	Evas_Object *button;

	button = elm_button_add(vd->ad->nf);
	elm_object_style_set(button, "naviframe/more/default");
	evas_object_size_hint_weight_set(button, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(button, EVAS_HINT_FILL, 0.5);
	elm_object_text_set(button, _("Menu"));
	evas_object_show(button);
	evas_object_smart_callback_add(button, "clicked", _menu_more_btn_clicked_cb, vd);
	return button;
}

static Evas_Object *_create_genlist(View_Data *vd, Evas_Object *parent)
{
	Evas_Object *genlist;
	Elm_Object_Item *it;
	int idx = 0;

	for (idx = 0; idx < ITC_MAX; idx++) {
		vd->itc[idx] = elm_genlist_item_class_new();
		vd->itc[idx]->func.text_get = _gl_text_get;
		vd->itc[idx]->func.content_get = _gl_content_get;
		vd->itc[idx]->decorate_item_style = "mode/slide";
		vd->itc[idx]->decorate_all_item_style = "edit";
		vd->itc[idx]->func.del = _gl_del;
	}

	vd->itc[0]->item_style = "1text";
	vd->itc[1]->item_style = "2text";
	vd->itc[2]->item_style = "1text.3icon";
	vd->itc[3]->item_style = "3text.1icon.2";
	vd->itc[4]->item_style = "4text.1icon.3";
	vd->itc[5]->item_style = "1text.2icon";
	vd->itc[6]->item_style = "dialogue/2text.5";
	vd->itc[7]->item_style = "dialogue/3text.2icon";
	vd->itc[8]->item_style = "dialogue/multiline/2text";
	vd->itc[9]->item_style = "dialogue/3text.1icon/expandable";

	// Create genlist
	genlist = elm_genlist_add(parent);

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

	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);

	evas_object_smart_callback_add(genlist, "selected", _selected, NULL);
	evas_object_smart_callback_add(genlist, "unselected", _unselected, NULL);
	evas_object_smart_callback_add(genlist, "longpressed", _longpressed, NULL);
	evas_object_smart_callback_add(genlist, "expanded", _exp, vd);
	evas_object_smart_callback_add(genlist, "contracted", _con, vd);
	evas_object_data_set(genlist, "view_data", vd);
	for (idx = 0 ; idx < 32; idx++) {
		Item_Data *id = calloc(sizeof(Item_Data), 1);
		id->idx = vd->g_idx;
		vd->g_idx++;
		it = elm_genlist_item_append(genlist, vd->itc[0], id, NULL,
					ELM_GENLIST_ITEM_TREE, _gl_sel, id);
	}
	it = elm_genlist_first_item_get(genlist);
	if (it) {
		printf("First Item, ");
		_print_item(it);
	}
	it = elm_genlist_last_item_get(genlist);
	if (it) {
		printf("Last Item, ");
		_print_item(it);
	}
	return genlist;
}

void genlist_collection_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!data) return;
	View_Data *vd;
	struct appdata *ad = data;

	genlist_demo_names = genlist_get_demo_names();
	// Create layout data for this view
	vd = calloc(1, sizeof(View_Data));
	vd->g_idx = 0;
	vd->ad = ad;
	vd->box = elm_box_add(ad->nf);
	vd->genlist = _create_genlist(vd, ad->nf);
	evas_object_show(vd->genlist);
	elm_box_pack_end(vd->box, vd->genlist);
	vd->navi_it = elm_naviframe_item_push(ad->nf, _("Collection"),
					NULL, NULL, vd->box, NULL);
	evas_object_event_callback_add(vd->box, EVAS_CALLBACK_FREE, _view_free_cb, vd);
	elm_object_item_part_content_set(vd->navi_it, "toolbar", _create_menu_toolbar(vd));
	elm_object_item_part_content_set(vd->navi_it, "toolbar_more_btn", _create_menu_more_button(vd));
}
