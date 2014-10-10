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

#define FONT_SIZE_INDEX_SMALL 24
#define FONT_SIZE_INDEX_NORMAL 35
#define FONT_SIZE_INDEX_LARGE 55
#define FONT_SIZE_INDEX_HUGE 75
#define FONT_SIZE_INDEX_GIANT 92
#define BASE_GENGRID_HEIGHT 76 //As per reference of UX of Genlist Group Index.

typedef struct _Testitem
{
	Elm_Object_Item *item;
	const char *text;
	const char *path;
	int index;
	int checked;
} Testitem;

typedef enum {
	SIZE_INDEX_SMALL = 0,
	SIZE_INDEX_NORMAL,
	SIZE_INDEX_LARGE,
	SIZE_INDEX_HUGE,
	SIZE_INDEX_GIANT
} font_size_index;

static int mode;
static int total_count;
static int checked_count;
static Eina_Bool select_all_checked = EINA_FALSE;
static Eina_Bool longpressed = EINA_FALSE;
static Evas_Object *gengrid, *box;
static Evas_Object *select_all_layout, *select_all_checkbox;
static Elm_Gengrid_Item_Class *gic, *ngic, *vgic;
static Elm_Gengrid_Item_Class ggic;
static void _edit_toolbar_cb(void *data, Evas_Object *obj, void *event_info);
static void _del_toolbar_cb(void *data, Evas_Object *obj, void *event_info);
static void _done_toolbar_cb(void *data, Evas_Object *obj, void *event_info);
static Evas_Object *create_edit_toolbar(struct appdata *ad);
static Evas_Object *create_del_done_toolbar(struct appdata *ad);

char *groupindex_names[] = {"A", "B", "C", "g", "y", NULL};

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
grid_longpress(void *data, Evas_Object *obj, void *event_info)
{
	longpressed = (mode == EDIT_MODE ? EINA_TRUE : EINA_FALSE);

	// If you need to cancel select status when longpress is called(e.g. popup)
	// set elm_gengrid_item_selected_set as EINA_FALSE.
	// elm_gengrid_item_selected_set(event_info, EINA_FALSE);
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

static void
_text_part_mouse_clicked_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	printf("text_clicked!!\n");
}

static char *
grid_text_get(void *data, Evas_Object *obj, const char *part)
{
	Testitem *ti = (Testitem *)data;

	if (!strcmp(part, "elm.text"))
		return strdup(ti->text);

	return NULL;
}

static char *
grid_text_video_text_get(void *data, Evas_Object *obj, const char *part)
{
	Testitem *ti = (Testitem *)data;

	if (!strcmp(part, "elm.text"))
		return strdup(ti->text);
	else if (!strcmp(part, "elm.text.video"))
		return strdup("00:00:00");
	return NULL;
}

static char *
grid_video_text_get(void *data, Evas_Object *obj, const char *part)
{
	if (!strcmp(part, "elm.text.video"))
		return strdup("00:00:00");
	return NULL;
}

static char *
grid_groupindex_text_get(void *data, Evas_Object *obj, const char *part)
{
	int index = (int)data;
	if (!strcmp(part, "elm.text")) {
		return strdup(groupindex_names[index]);
	}
	return NULL;
}

static void
_size_set_on_font_resize() {
	int x, h, w;
	font_size_index font_index;
	vconf_get_int(VCONFKEY_SETAPPL_ACCESSIBILITY_FONT_SIZE, &x);
	double scale = elm_config_scale_get();
	font_index = x;

	switch (font_index) {
		case SIZE_INDEX_SMALL :
			h = (int)((BASE_GENGRID_HEIGHT - FONT_SIZE_INDEX_NORMAL + FONT_SIZE_INDEX_SMALL + 1 /*extra pixel for overlapp check*/) * scale);
			break;
		case SIZE_INDEX_NORMAL :
			h = (int)(BASE_GENGRID_HEIGHT * scale);
			break;
		case SIZE_INDEX_LARGE :
			h = (int)((BASE_GENGRID_HEIGHT - FONT_SIZE_INDEX_NORMAL + FONT_SIZE_INDEX_LARGE) * scale);
			break;
		case SIZE_INDEX_HUGE :
			h = (int)((BASE_GENGRID_HEIGHT - FONT_SIZE_INDEX_NORMAL + FONT_SIZE_INDEX_HUGE) * scale);
			break;
		case SIZE_INDEX_GIANT :
			h = (int)((BASE_GENGRID_HEIGHT - FONT_SIZE_INDEX_NORMAL + FONT_SIZE_INDEX_GIANT) * scale);
			break;
		default:
			h = (int)(BASE_GENGRID_HEIGHT * scale ); //If no font size obtained, set the height to maximum
	}
	w = (int)(720 * scale);
	elm_gengrid_group_item_size_set(gengrid, w, h);
}

static void
_vconf_size_key_changed_cb()
{
	_size_set_on_font_resize();
}

static Evas_Object *
grid_content_get(void *data, Evas_Object *obj, const char *part)
{
	Testitem *ti = (Testitem *)data;

	if (ti->index == 1) {
		if (!strcmp(part, "elm.swallow.end")) {
			Evas_Object *progressbar = elm_progressbar_add(obj);
			elm_object_style_set(progressbar, "process_medium");
			evas_object_size_hint_align_set(progressbar, EVAS_HINT_FILL, 0.5);
			evas_object_size_hint_weight_set(progressbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			evas_object_show(progressbar);
			elm_progressbar_pulse(progressbar, EINA_TRUE);
			return progressbar;
		} else return NULL;
	} else	if (!strcmp(part, "elm.swallow.icon")) {
		Evas_Object *icon = elm_image_add(obj);
		elm_image_file_set(icon, ti->path, NULL);
		elm_image_aspect_fixed_set(icon, EINA_FALSE);
		elm_image_preload_disabled_set(icon, EINA_FALSE);
		evas_object_show(icon);
		return icon;
	} else if (!strcmp(part, "elm.swallow.block") && mode == EDIT_MODE) {
		Evas_Object *layout = elm_layout_add(obj);
		elm_layout_theme_set(layout, "gengrid", "item", "block/default");
		evas_object_propagate_events_set(layout, 0);
		evas_object_event_callback_add(layout, EVAS_CALLBACK_MOUSE_DOWN, _text_part_mouse_clicked_cb, ti);
		return layout;
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

static Evas_Object *
grid_video_content_get(void *data, Evas_Object *obj, const char *part)
{
	Testitem *ti = (Testitem *)data;

	if (ti->index == 1) {
		if (!strcmp(part, "elm.swallow.end")) {
			elm_object_item_signal_emit(ti->item, "elm,state,end,visible", "elm");
			Evas_Object *progressbar = elm_progressbar_add(obj);
			elm_object_style_set(progressbar, "process_medium");
			evas_object_size_hint_align_set(progressbar, EVAS_HINT_FILL, 0.5);
			evas_object_size_hint_weight_set(progressbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			evas_object_show(progressbar);
			elm_progressbar_pulse(progressbar, EINA_TRUE);
			return progressbar;
		} else return NULL;
	} else	if (!strcmp(part, "elm.swallow.icon")) {
		Evas_Object *icon = elm_image_add(obj);
		elm_image_file_set(icon, ti->path, NULL);
		elm_image_aspect_fixed_set(icon, EINA_FALSE);
		elm_image_preload_disabled_set(icon, EINA_FALSE);
		evas_object_show(icon);
		return icon;
	} else if (!strcmp(part, "elm.swallow.block") && mode == EDIT_MODE) {
		Evas_Object *layout = elm_layout_add(obj);
		elm_layout_theme_set(layout, "gengrid", "item", "block/default");
		evas_object_propagate_events_set(layout, 0);
		evas_object_event_callback_add(layout, EVAS_CALLBACK_MOUSE_DOWN, _text_part_mouse_clicked_cb, ti);
		return layout;
	}  else if (!strcmp(part, "elm.swallow.end") && mode == EDIT_MODE) {
		Evas_Object *ck = elm_check_add(obj);
		elm_object_style_set(ck, "grid");
		evas_object_propagate_events_set(ck, EINA_FALSE);
		elm_check_state_set(ck, ti->checked);
		evas_object_repeat_events_set(ck, EINA_TRUE);
		elm_access_object_unregister(ck);

		evas_object_show(ck);
		return ck;
	} else if (!strcmp(part, "elm.swallow.video")) {
		Evas_Object *video_play = elm_image_add(obj);
        elm_image_file_set(video_play, ICON_DIR"/00_video_play.png", NULL);
		elm_image_aspect_fixed_set(video_play, EINA_TRUE);
		elm_image_resizable_set(video_play, EINA_FALSE, EINA_FALSE);
		elm_image_preload_disabled_set(video_play, EINA_FALSE);
		evas_object_show(video_play);
		return video_play;
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

static void
_create_gengrid (void *data, char *type)
{
	struct appdata *ad = (struct appdata *)data;
	int i, j, n, x, w, h;
	char buf[PATH_MAX];
	static Testitem ti[IMAGE_MAX*25];

	vconf_get_int(VCONFKEY_SETAPPL_ACCESSIBILITY_FONT_SIZE, &x);
	gengrid = elm_gengrid_add(ad->nf);
	evas_object_size_hint_weight_set(gengrid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(gengrid, EVAS_HINT_FILL, EVAS_HINT_FILL);
	w = h = 0;

	double scale = elm_config_scale_get();
	if (!strcmp(type, "myfile/text")) {
		w = (int)(176 * scale); //176 as per UX ver 0.6
		h = (int)((176 + 64) * scale); //64 for textas per UX ver 0.6
	}
	else if (!strcmp(type, "myfile")) {
		w = h = (int)(176 * scale); //176 as per UX ver 0.6
	}
	elm_gengrid_item_size_set(gengrid, w, h);
	_size_set_on_font_resize();
	elm_gengrid_align_set(gengrid, 0.5, 0.0);
	elm_gengrid_horizontal_set(gengrid, EINA_FALSE);
	elm_gengrid_multi_select_set(gengrid, EINA_TRUE);
	evas_object_smart_callback_add(gengrid, "moved", grid_moved, NULL);
	evas_object_smart_callback_add(gengrid, "longpressed", grid_longpress, NULL);

	/******text myfile type******/
	gic = elm_gengrid_item_class_new();
	gic->item_style = type;
	gic->func.text_get = grid_text_get;
	gic->func.content_get = grid_content_get;
	gic->func.state_get = NULL;
	gic->func.del = NULL;

	/*****no text myfile type****/
	ngic = elm_gengrid_item_class_new();
	ngic->item_style = type;
	ngic->func.text_get = NULL;
	ngic->func.content_get = grid_content_get;
	ngic->func.state_get = NULL;
	ngic->func.del = NULL;

	/****video play grid type****/
	vgic = elm_gengrid_item_class_new();
	if (!strcmp(type, "myfile"))
	{
		vgic->item_style = "myfile_video";
		vgic->func.text_get = grid_video_text_get;
	} else if (!strcmp(type, "myfile/text")){
		vgic->item_style = "myfile_video/text";
		vgic->func.text_get = grid_text_video_text_get;
	}
	vgic->func.content_get = grid_video_content_get;
	vgic->func.state_get = NULL;
	vgic->func.del = NULL;

	ggic.item_style = "group_index";
	ggic.func.text_get = grid_groupindex_text_get;
	ggic.func.content_get = NULL;
	ggic.func.state_get = NULL;
	ggic.func.del = NULL;

	for (j = 0; j < 25; j++) {
		for (i = 0; i < IMAGE_MAX; i++) {
			n = i+(j*IMAGE_MAX);
			snprintf(buf, sizeof(buf), "%s/grid_image/%d_raw.jpg", ICON_DIR, i+1);
			ti[n].index = n;
			ti[n].path = eina_stringshare_add(buf);
			if (i == 0 || i == 11 || i == 22 || i == 33 || i == 44)
			{
				ti[i].item = elm_gengrid_item_append(gengrid, &ggic, (void *)(i%10), NULL, NULL);
				elm_gengrid_item_select_mode_set(ti[i].item, ELM_OBJECT_SELECT_MODE_NONE);
			} else if(!(n % 7))
			{
				ti[n].item = elm_gengrid_item_append(gengrid, vgic, &(ti[n]), _item_selected, &(ti[n]));
				ti[n].text = strdup("video.mp4");
			} else if((!(n % 6)) && !strcmp(type, "myfile"))
			{
				ti[n].item = elm_gengrid_item_append(gengrid, ngic, &(ti[n]), _item_selected, &(ti[n]));
			} else
			{
				ti[n].item = elm_gengrid_item_append(gengrid, gic, &(ti[n]), _item_selected, &(ti[n]));
				if (n%4 == 0)
					ti[n].text = strdup("DavidRobinson.jpg");
				else if (n%4 == 1)
					ti[n].text = strdup("CaptainFantasticFasterThanSupermanSpidermanBatmanWolverineHulkAndTheFlashCombined.jpg");
				else if (n%4 == 2)
					ti[n].text = strdup("1.jpg");
				else
				ti[n].text = strdup("2.jpg");
			}
			ti[n].checked = EINA_FALSE;
		}
	}

	total_count = n + 1;
	if (vconf_notify_key_changed(VCONFKEY_SETAPPL_ACCESSIBILITY_FONT_SIZE, _vconf_size_key_changed_cb, NULL) < 0) {
		printf("\nFail to register VCONFKEY_SETAPPL_SOUND_STATUS_BOOL key callback");
	}
}

static void
_check_fields_update()
{
	const Elm_Gengrid_Item_Class *itc;
	Elm_Object_Item *it;
	Eina_List *realize_its;

	realize_its = elm_gengrid_realized_items_get(gengrid);

	EINA_LIST_FREE(realize_its, it) {
		itc = elm_gengrid_item_item_class_get(it);
		if(strcmp(itc->item_style, "group_index"))
			elm_gengrid_item_fields_update(it, "elm.swallow.end", ELM_GENGRID_ITEM_FIELD_CONTENT);
	}
}

static void
_check_select_all()
{
	Elm_Object_Item *it;
	Testitem *ti;
	const Elm_Gengrid_Item_Class *itc;
	Eina_List *realize_its;

	if (select_all_checked) checked_count = total_count;
	else checked_count = 0;
	it = elm_gengrid_first_item_get(gengrid);
	while(it) {
		itc = elm_gengrid_item_item_class_get(it);
		if(strcmp(itc->item_style,"group_index")){
			ti = elm_object_item_data_get(it);
			if (ti) ti->checked = select_all_checked;
		}
		it = elm_gengrid_item_next_get(it);
	}
	realize_its = elm_gengrid_realized_items_get(gengrid);

	EINA_LIST_FREE(realize_its, it) {
		itc = elm_gengrid_item_item_class_get(it);
		if(strcmp(itc->item_style,"group_index")){
			const char *type = NULL;
			Evas_Object *ck = elm_object_item_part_content_get(it, "elm.swallow.end");
			if (ck) type = elm_object_widget_type_get(ck);
			if (type && !strcmp(type, "elm_check")) {
				elm_check_state_set(ck, select_all_checked);
			}
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
		elm_box_pack_start(box, select_all_layout);
		evas_object_show(select_all_layout);
		elm_gengrid_reorder_mode_set(gengrid, EINA_TRUE);
	}

	select_all_checked = EINA_FALSE;
	_check_select_all();
    _check_fields_update();

	navi_it = elm_naviframe_top_item_get(ad->nf);
	toolbar = create_del_done_toolbar(ad);
	elm_object_item_part_content_set(navi_it, "toolbar", toolbar);
}

static void _del_toolbar_cb(void *data, Evas_Object *obj, void *envent_info)
{
	Testitem *ti;
	Elm_Object_Item *it;
	const Elm_Gengrid_Item_Class *itc;

	it = elm_gengrid_first_item_get(gengrid);
	while (it) {
		itc = elm_gengrid_item_item_class_get(it);
		ti = (Testitem *)elm_object_item_data_get(it);
		it = elm_gengrid_item_next_get(it);
		if((strcmp(itc->item_style, "group_index") && ((ti) && (ti->checked)))) {
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

	if(mode == EDIT_MODE){
		mode = VIEW_MODE;
		elm_box_unpack(box, select_all_layout);
		evas_object_del(select_all_layout);
		select_all_layout = NULL;

		elm_gengrid_reorder_mode_set(gengrid, EINA_FALSE);
	}
	select_all_checked = EINA_FALSE;
	_check_select_all();
    _check_fields_update();

	navi_it = elm_naviframe_top_item_get(ad->nf);
	toolbar = create_edit_toolbar(ad);
	elm_object_item_part_content_set(navi_it, "toolbar", toolbar);
}

static Eina_Bool
_pop_cb(void *data, Elm_Object_Item *it)
{
	elm_gengrid_clear(gengrid);
	evas_object_del(gengrid);
	gengrid = NULL;

	if (vconf_ignore_key_changed(VCONFKEY_SETAPPL_ACCESSIBILITY_FONT_SIZE, _vconf_size_key_changed_cb) < 0) {
		printf("\nFail to unregister VCONFKEY_SETAPPL_ACCESSIBILITY_FONT_SIZE key callback");
	}

	return EINA_TRUE;
}

static void
gengrid_create_view(void *data, char *type)
{
	struct appdata *ad;
	Evas_Object *toolbar;
	Elm_Object_Item *navi_it;

	ad = (struct appdata *)data;
	if (ad == NULL) return;

	mode = VIEW_MODE;

	_create_gengrid(ad, type);

	box = elm_box_add(ad->nf);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(box);
	elm_box_pack_end(box, gengrid);
	evas_object_show(gengrid);

	navi_it = elm_naviframe_item_push (ad->nf, _("Myfile") , NULL, NULL, box, NULL);

	toolbar =  create_edit_toolbar(ad);
	elm_object_item_part_content_set(navi_it, "toolbar", toolbar);
	elm_naviframe_item_pop_cb_set(navi_it, _pop_cb, NULL);
}

void
gengrid_myfilegrid_cb(void *data, Evas_Object *obj, void *event_info)
{
	gengrid_create_view(data, "myfile");
}

void
gengrid_myfilegridtext_cb(void *data, Evas_Object *obj, void *event_info)
{
	gengrid_create_view(data, "myfile/text");
}
