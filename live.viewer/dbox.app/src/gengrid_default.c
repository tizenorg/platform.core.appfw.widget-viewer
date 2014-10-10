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

#define IMAGE_MAX 79
#define VIEW_MODE 0
#define EDIT_MODE 1

typedef struct _Testitem
{
	Elm_Object_Item *item;
	const char *path;
	const char *text;
	int index;
	int checked;
} Testitem;

typedef struct View_Data
{
	struct appdata *ad;
	int w;
	int h;
	char *type;
	Evas_Object *ctx;
} View_Data;

static Elm_Win_Indicator_Opacity_Mode indi_o_mode;
static Elm_Win_Indicator_Mode indi_mode;
static Eina_Bool overlap_mode;

static int mode;
static int total_count;
static int checked_count;
static Eina_Bool rotate_flag = EINA_FALSE;
static Eina_Bool select_all_checked = EINA_FALSE;
static Eina_Bool longpressed = EINA_FALSE;
static Evas_Object *gengrid, *box;
static Evas_Object *select_all_layout, *select_all_checkbox;
static Elm_Gengrid_Item_Class *gic;
static void _edit_toolbar_cb(void *data, Evas_Object *obj, void *event_info);
static void _del_toolbar_cb(void *data, Evas_Object *obj, void *event_info);
static void _done_toolbar_cb(void *data, Evas_Object *obj, void *event_info);
static Evas_Object *create_edit_toolbar(struct appdata *ad);
static Evas_Object *create_del_done_toolbar(struct appdata *ad);
static void _normal_indi_cb(void *data);
static void _transparent_indi_cb(void *data);

static void
grid_moved(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *moved_item = (Elm_Object_Item *)event_info;

	Testitem *moved_ti = (Testitem *)elm_object_item_data_get(moved_item);
	printf("moved item index = %d", moved_ti->index);

	if (elm_gengrid_item_prev_get(moved_item)) {
		Testitem *prev_ti = (Testitem *)elm_object_item_data_get(elm_gengrid_item_prev_get(moved_item));
		printf(", prev index = %d", prev_ti->index);
	}
	if (elm_gengrid_item_next_get(moved_item)) {
		Testitem *next_ti = (Testitem *)elm_object_item_data_get(elm_gengrid_item_next_get(moved_item));
		printf(", next index = %d", next_ti->index);
	}
	printf("\n");

	// If you want change your data, you can here.
}

static void
grid_longpress(void *data, Evas_Object *obj, void *ei)
{
	if (mode == EDIT_MODE) {
		longpressed = EINA_TRUE;
		printf("Item(%p) Reorder enabled\n", ei);
	} else {
		longpressed = EINA_FALSE;
		printf("Item(%p) disabled\n", ei);
	}

	// If you need to cancel select status when longpress is called(e.g. popup)
	// set elm_gengrid_item_selected_set as EINA_FALSE.
	// elm_gengrid_item_selected_set(ei, EINA_FALSE);

}

static void
_item_check_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	Eina_Bool state;

	state = elm_check_state_get(obj);

	if (state) checked_count++;
	else checked_count--;

	if (select_all_layout) {
		if (total_count == checked_count)
			select_all_checked = EINA_TRUE;
		else
			select_all_checked = EINA_FALSE;
		elm_check_state_pointer_set(select_all_checkbox, &select_all_checked);
	}

}

static Evas_Object *
grid_content_get(void *data, Evas_Object *obj, const char *part)
{
	Testitem *ti = (Testitem *)data;
	Elm_Object_Item *it = elm_gengrid_last_item_get(obj);
	int last_idx = elm_gengrid_item_index_get(it);

	if (ti->index == last_idx) {
		if (!strcmp(part, "elm.swallow.end")) {
			Evas_Object *progressbar = elm_progressbar_add(obj);
			elm_object_style_set(progressbar, "process_medium");
			evas_object_size_hint_align_set(progressbar, EVAS_HINT_FILL, 0.5);
			//evas_object_size_hint_weight_set(progressbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			//evas_object_size_hint_min_set(progressbar, 150, 150);
			evas_object_show(progressbar);
			elm_progressbar_pulse(progressbar, EINA_TRUE);
			return progressbar;
		} else return NULL;
	} else if (!strcmp(part, "elm.swallow.icon")) {
		Evas_Object *icon = elm_image_add(obj);
		elm_image_file_set(icon, ti->path, NULL);
		elm_image_aspect_fixed_set(icon, EINA_FALSE);
		elm_image_preload_disabled_set(icon, EINA_FALSE);
		evas_object_show(icon);
		return icon;
	} else if (!strcmp(part, "elm.swallow.end") && mode == EDIT_MODE) {
		Evas_Object *ck = elm_check_add(obj);
		elm_object_style_set(ck, "grid");
		evas_object_propagate_events_set(ck, EINA_FALSE);
		elm_check_state_set(ck, ti->checked);
		evas_object_repeat_events_set(ck, EINA_TRUE);
		elm_access_object_unregister(ck);

		evas_object_show(ck);
		return ck;
	}

	return NULL;
}

static void
_show_selected_items(Testitem *ti)
{
	/*const Eina_List* list = elm_gengrid_selected_items_get(grid);
	const Eina_List* l = NULL;
	Elm_Object_Item *recv = NULL;
	printf("--------------------------\nSelected Items :");
	EINA_LIST_FOREACH(list, l, recv) {
		Testitem *ti = (Testitem *)elm_object_item_data_get(recv);
		printf("%d ",ti->index);
	}
	printf("\n--------------------------\n");*/
}

static void
_item_selected(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *ck;
	Testitem *ti = (Testitem *)data;
	ck = elm_object_item_part_content_get(event_info, "elm.swallow.end");

	printf("item selected: %p\n", event_info);

	_show_selected_items(ti);

	elm_gengrid_item_selected_set(ti->item, EINA_FALSE);

	if (longpressed) {
		longpressed = EINA_FALSE;
		return;
	}

	ti->checked = !(elm_check_state_get(ck));
	elm_check_state_set(ck, ti->checked);

	_item_check_changed_cb(data, ck, NULL);
}

static void _realized(void *data, Evas_Object *obj, void *ei)
{
	printf("[%s][%d]\n", __func__, __LINE__);
}

static void
_create_gengrid (void *data, char *type)
{
	struct View_Data *vd = data;
	struct appdata *ad = vd->ad;
	int i, j, n, w, h;
	char buf[PATH_MAX];
	static Testitem ti[IMAGE_MAX*25];

	gengrid = elm_gengrid_add(ad->nf);
	evas_object_size_hint_weight_set(gengrid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(gengrid, EVAS_HINT_FILL, EVAS_HINT_FILL);

	double scale = elm_config_scale_get();
	w = (int)(vd->w * scale); //177 as per UX ver 1.7.
	h = (int)(vd->h * scale);
	elm_gengrid_item_size_set(gengrid, w, h);
	elm_gengrid_align_set(gengrid, 0.5, 0.5);
	elm_gengrid_horizontal_set(gengrid, EINA_TRUE);
	elm_gengrid_multi_select_set(gengrid, EINA_TRUE);
	evas_object_smart_callback_add(gengrid, "moved", grid_moved, NULL);
	evas_object_smart_callback_add(gengrid, "longpressed", grid_longpress, NULL);
	evas_object_smart_callback_add(gengrid, "realized", _realized, NULL);

	gic = elm_gengrid_item_class_new();
	gic->item_style=type;

	gic->func.text_get = NULL;
	gic->func.content_get = grid_content_get;
	gic->func.state_get = NULL;
	gic->func.del = NULL;

	for (j = 0; j < 25; j++) {
		for (i = 0; i < IMAGE_MAX; i++) {
			n = i+(j*IMAGE_MAX);
			snprintf(buf, sizeof(buf), "%s/grid_image/%d_raw.jpg", ICON_DIR, i+1);
			ti[n].index = n;
			ti[n].path = eina_stringshare_add(buf);
			//TODO: check the func data
			ti[n].item = elm_gengrid_item_append(gengrid, gic, &(ti[n]), _item_selected, &(ti[n]));
			ti[n].checked = EINA_FALSE;
		}
	}
	total_count = n + 1;
}

static void
_check_field_update()
{
	Elm_Object_Item *it;
	Eina_List *realize_its;

	realize_its = elm_gengrid_realized_items_get(gengrid);

	EINA_LIST_FREE(realize_its, it) {
		elm_gengrid_item_fields_update(it, "elm.swallow.end", ELM_GENGRID_ITEM_FIELD_CONTENT);
	}
}

static void
_check_select_all()
{
	Elm_Object_Item *it;
	Testitem *ti;
	Eina_List *realize_its;

	if (select_all_checked) checked_count = total_count;
	else checked_count = 0;

	it = elm_gengrid_first_item_get(gengrid);
	while(it) {
		ti = elm_object_item_data_get(it);
		if (ti) ti->checked = select_all_checked;
		it = elm_gengrid_item_next_get(it);
	}
	realize_its = elm_gengrid_realized_items_get(gengrid);

	EINA_LIST_FREE(realize_its, it) {
		const char *type = NULL;
		Evas_Object *ck = elm_object_item_part_content_get(it, "elm.swallow.end");
		if (ck) type = elm_object_widget_type_get(ck);
		if (type && !strcmp(type, "elm_check")) {
			elm_check_state_set(ck, select_all_checked);
		}
	}
}

static void
_select_all_layout_mouse_down_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	select_all_checked = !select_all_checked;
	elm_check_state_pointer_set(select_all_checkbox, &select_all_checked);

	_check_select_all();
}

static void _select_all_check_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	_check_select_all(data);
}

static Evas_Object *create_edit_toolbar(struct appdata *ad)
{
	Evas_Object *obj;

	/* create toolbar */
	obj = elm_toolbar_add(ad->nf);
	if(obj == NULL) return NULL;
	elm_object_style_set(obj, "default");
	elm_toolbar_shrink_mode_set(obj, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_transverse_expanded_set(obj, EINA_TRUE);
	elm_toolbar_select_mode_set(obj, ELM_OBJECT_SELECT_MODE_NONE);

	elm_toolbar_item_append(obj, NULL, "Edit", _edit_toolbar_cb, ad);

	return obj;
}

static Evas_Object *create_del_done_toolbar(struct appdata *ad)
{
	Evas_Object *obj;

	/* create toolbar */
	obj = elm_toolbar_add(ad->nf);
	if(obj == NULL) return NULL;
	elm_object_style_set(obj, "default");
	elm_toolbar_shrink_mode_set(obj, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_transverse_expanded_set(obj, EINA_TRUE);
	elm_toolbar_select_mode_set(obj, ELM_OBJECT_SELECT_MODE_NONE);

	elm_toolbar_item_append(obj, NULL, "Delete", _del_toolbar_cb, ad);
	elm_toolbar_item_append(obj, NULL, "Done", _done_toolbar_cb, ad);

	return obj;
}

static void _edit_toolbar_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *toolbar;
	Elm_Object_Item *navi_it;
	struct appdata *ad;
	ad = (struct appdata *) data;
	if(ad == NULL) return;

	if (mode == VIEW_MODE) {
		mode = EDIT_MODE;
		select_all_layout = elm_layout_add(box);
		elm_layout_theme_set(select_all_layout, "genlist", "item", "select_all/default");
		evas_object_size_hint_weight_set(select_all_layout, EVAS_HINT_EXPAND, EVAS_HINT_FILL);
		evas_object_size_hint_align_set(select_all_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_event_callback_add(select_all_layout, EVAS_CALLBACK_MOUSE_DOWN, _select_all_layout_mouse_down_cb, NULL);

		select_all_checkbox = elm_check_add(select_all_layout);
		elm_check_state_pointer_set(select_all_checkbox, &select_all_checked);
		evas_object_smart_callback_add(select_all_checkbox, "changed", _select_all_check_changed_cb, NULL);
		evas_object_propagate_events_set(select_all_checkbox, EINA_FALSE);
		elm_object_part_content_set(select_all_layout, "elm.icon", select_all_checkbox);

		elm_object_part_text_set(select_all_layout, "elm.text", "Select All");
		elm_box_pack_end(box, select_all_layout);
		evas_object_show(select_all_layout);
		elm_gengrid_reorder_mode_set(gengrid, EINA_TRUE);
	}

	select_all_checked = EINA_FALSE;
	_check_select_all();
	_check_field_update();

	navi_it = elm_naviframe_top_item_get(ad->nf);
	toolbar = create_del_done_toolbar(ad);
	elm_object_item_part_content_set(navi_it, "toolbar", toolbar);
}

static void _del_toolbar_cb(void *data, Evas_Object *obj, void *envent_info)
{
	Testitem *ti;
	Elm_Object_Item *it;

	it = elm_gengrid_first_item_get(gengrid);
	while (it) {
		ti = (Testitem *)elm_object_item_data_get(it);
		it = elm_gengrid_item_next_get(it);
		if ((ti) && (ti->checked)) {
			elm_object_item_del(ti->item);
			total_count--;
			checked_count--;
		}
	}
}

static void _done_toolbar_cb(void *data, Evas_Object *obj, void *envent_info)
{
    Evas_Object *toolbar;
    Elm_Object_Item *navi_it;
    struct appdata *ad;
    ad = (struct appdata *) data;
    if(ad == NULL) return;

	if(mode == EDIT_MODE) {
		mode = VIEW_MODE;
		elm_box_unpack(box, select_all_layout);
		evas_object_del(select_all_layout);
		select_all_layout = NULL;
		elm_gengrid_reorder_mode_set(gengrid, EINA_FALSE);
	}

	select_all_checked = EINA_FALSE;
	_check_select_all();
	_check_field_update();

    navi_it = elm_naviframe_top_item_get(ad->nf);
    toolbar = create_edit_toolbar(ad);
    elm_object_item_part_content_set(navi_it, "toolbar", toolbar);
}

static Eina_Bool
_pop_cb(void *data, Elm_Object_Item *it)
{
	if (!data) return EINA_TRUE;
	View_Data *vd = data;
	_normal_indi_cb(vd->ad);

	if(vd->ctx) {
		evas_object_del(vd->ctx);
		vd->ctx = NULL;
	}

	elm_gengrid_clear(gengrid);
	evas_object_del(gengrid);
	gengrid = NULL;
	elm_object_style_set(vd->ad->bg, "default");

	if (vd) free(vd);

	return EINA_TRUE;
}

static void
_extpopup_cb(void *data, Evas_Object *obj, void *event)
{
	if ((!data) || (!obj)) return;
	View_Data *vd;
	struct appdata *ad;
	Elm_Object_Item *it = NULL;
	vd = (struct View_Data *) data;

	ad = vd->ad;
    Elm_Object_Item *navi_it;
	const char *label = elm_object_item_text_get((Elm_Object_Item *) event);

	if (label) {
		if (!strcmp(label, "Small Size")) {
			vd->w = 177;
			vd->h = 133;
		} else if (!strcmp(label, "Middle Size")) {
			vd->w = 238;
			vd->h = 178;
		} else  if (!strcmp(label, "Large Size")) {
			vd->w = 357;
			vd->h = 267;
		}
     }

	elm_gengrid_clear(gengrid);
	evas_object_del(gengrid);
	gengrid = NULL;

	_create_gengrid(vd, vd->type);
	it = elm_gengrid_last_item_get(gengrid);
	elm_gengrid_item_show(it, ELM_GENGRID_ITEM_SCROLLTO_IN);

	box = elm_box_add(ad->nf);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(box);
	elm_box_pack_end(box, gengrid);
	evas_object_show(gengrid);
	navi_it = elm_naviframe_top_item_get(ad->nf);
	elm_object_item_content_set(navi_it, box);
	evas_object_del(obj);
}

static void
_move_ctx(void *data, Evas_Object *obj)
{
	if(!data) return;
	View_Data *vd = data;
	Evas_Object *ctx = obj;
	Evas_Coord w, h;
	int pos = -1;

#if DESKTOP
	evas_object_geometry_get(vd->ad->win_main, NULL, NULL, &w, &h);
#else
	elm_win_screen_size_get(vd->ad->win_main, NULL, NULL, &w, &h);
#endif
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
_dismissed_cb(void *data, Evas_Object *obj , void *event_info)
{
	View_Data *vd = data;
	Evas_Object *ctx = obj;

		if (!rotate_flag) {
			evas_object_del(ctx);
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
_create_more_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!data) return;
	Evas_Object *ctx;
	View_Data *vd = data;
	struct appdata *ad;
	ad = vd->ad;
	double s = elm_config_scale_get();

	ctx = elm_ctxpopup_add(ad->nf);
	elm_object_style_set(ctx, "more/default");
	ea_object_event_callback_add(ctx, EA_CALLBACK_BACK, ea_ctxpopup_back_cb, NULL);
	evas_object_smart_callback_add(ctx,"dismissed", _dismissed_cb, vd);
	evas_object_event_callback_add(ctx, EVAS_CALLBACK_DEL, _delete_cb, vd);
	evas_object_event_callback_add(ad->nf, EVAS_CALLBACK_RESIZE, _resize_cb, ctx);
	evas_object_smart_callback_add(elm_object_top_widget_get(ctx), "rotation,changed", _rotate_cb, vd);
	elm_ctxpopup_item_append(ctx, "Small Size", NULL, _extpopup_cb, vd);
	elm_ctxpopup_item_append(ctx, "Middle Size", NULL, _extpopup_cb, vd);
	elm_ctxpopup_item_append(ctx, "Large Size", NULL, _extpopup_cb, vd);
	evas_object_size_hint_max_set(ctx, s*500, s*600);
	elm_ctxpopup_direction_priority_set(ctx, ELM_CTXPOPUP_DIRECTION_UP,
										ELM_CTXPOPUP_DIRECTION_UNKNOWN,
										ELM_CTXPOPUP_DIRECTION_UNKNOWN,
										ELM_CTXPOPUP_DIRECTION_UNKNOWN);

	_move_ctx(vd, ctx);
	evas_object_show(ctx);
}
void
gengrid_create_cb(void *data, char *type)
{
	struct appdata *ad;
	View_Data *vd;
	Evas_Object *toolbar;
	Evas_Object *btn;
	Elm_Object_Item *navi_it;
	Elm_Object_Item *it = NULL;

	vd = (struct View_Data *) data;
	if (vd == NULL) return;
	ad = vd->ad;

	if(!strcmp(type, "gallery")) _transparent_indi_cb(vd->ad);
	mode = VIEW_MODE;

	_create_gengrid(vd, type);
	it = elm_gengrid_last_item_get(gengrid);
	elm_gengrid_item_show(it, ELM_GENGRID_ITEM_SCROLLTO_IN);

	box = elm_box_add(ad->nf);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(box);
	elm_box_pack_end(box, gengrid);
	evas_object_show(gengrid);

	navi_it = elm_naviframe_item_push (ad->nf, _("Default") , NULL, NULL, box, NULL);

	toolbar = create_edit_toolbar(ad);
	elm_object_item_part_content_set(navi_it, "toolbar", toolbar);
	elm_naviframe_item_pop_cb_set(navi_it, _pop_cb, vd);
	btn = elm_button_add(ad->nf);
	elm_object_style_set(btn, "naviframe/more/default");
	evas_object_smart_callback_add(btn, "clicked", _create_more_btn_cb, vd);
	elm_object_item_part_content_set(navi_it, "toolbar_more_btn", btn);
}

static void
_transparent_indi_cb(void *data)
{
	struct appdata *ad = (struct appdata *) data;
	Evas_Object *win, *conform;

	if ((!ad->win_main) || (!ad->conform))
	{
		printf("[%s]:We can't get conformant\n", __FUNCTION__);
		return;
	}
	win     = ad->win_main;
	conform = ad->conform;

	/*App have to manage indicator type, layout, etc...
	save all value related with indicator,
	because we have to rollback if my view disappeared (like ug or etc)
	1. change indicator type into transparent indicator.
	2. change indicator bg into transparent.
	3. set layout start position to (0,0)
	*/

	//nooverlap mode is layout start <0,60(indicator height). This is default.
	//overlap mode is layout start <0,0>
	//if layout start (0,60), nooverlap_mode is null.

	elm_win_indicator_mode_set(win, ELM_WIN_INDICATOR_SHOW);
	elm_win_indicator_opacity_set(win, ELM_WIN_INDICATOR_TRANSPARENT);

	//if you change your layout to nooverlap_mode(to locate layout 0,0)
	//you should emit signal and set data of "nooverlap"
	elm_object_signal_emit(conform, "elm,state,indicator,overlap", "");
	evas_object_data_set(conform, "overlap", (void *)EINA_TRUE);
}

static void
_normal_indi_cb(void *data)
{
	struct appdata *ad = (struct appdata *) data;
	Evas_Object *win, *conform;

	if ((!ad->win_main) || (!ad->conform))
	{
		printf("[%s]:We can't get conformant\n", __FUNCTION__);
		return;
	}
	win     = ad->win_main;
	conform = ad->conform;

	//recover indicator mode and type when your view disappeared.
	elm_win_indicator_mode_set(win, ELM_WIN_INDICATOR_SHOW);
	elm_win_indicator_opacity_set(win, ELM_WIN_INDICATOR_OPAQUE);

	elm_object_signal_emit(conform, "elm,state,indicator,nooverlap", "");

	evas_object_data_set(conform, "overlap", NULL);
	printf("[%s][%d] trans indi mode=%d opacity=%d over(0,0)=%d\n", __FUNCTION__, __LINE__, indi_mode, indi_o_mode, (int)overlap_mode);
}


void
gengrid_gallerygrid_cb(void *data, Evas_Object *obj, void *event_info)
{
	View_Data *vd;
	vd = calloc(1, sizeof(View_Data));
	vd->ad = (struct appdata *)data;
	vd->w = 357;
	vd->h = 267;
	vd->type = "gallery";
	gengrid_create_cb(vd, vd->type);
}
