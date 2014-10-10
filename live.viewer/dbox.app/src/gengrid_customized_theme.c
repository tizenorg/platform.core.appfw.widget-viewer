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

#define IMAGE_MAX    16
#define VIEW_MODE    0
#define EDIT_MODE    1
#define ITEM_SIZE_W  158
#define ITEM_SIZE_H  158
#define PHOTO_INNER  2
#define PHOTO_REAL_W (ITEM_SIZE_W-(2*PHOTO_INNER))
#define PHOTO_REAL_H (ITEM_SIZE_H-(2*PHOTO_INNER))

typedef struct _Testitem
{
	Elm_Object_Item *item;
	const char *path;
	int index;
	int checked;
} Testitem;

static int mode;
static int total_count;
static int checked_count;
static Eina_Bool select_all_checked = EINA_FALSE;
static Eina_Bool longpressed = EINA_FALSE;
static Evas_Object *gengrid, *toolbar, *edit_btn, *box;
static Evas_Object *select_all_layout, *select_all_checkbox;
static Elm_Object_Item *del_btn_item;
static Elm_Gengrid_Item_Class *gic_default, *gic_photo;

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
}

static void
_item_check_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	Testitem *ti = (Testitem *)data;

	if (longpressed) {
		ti->checked = !elm_check_state_get(obj);
		elm_check_state_set(obj, ti->checked);
		longpressed = EINA_FALSE;
	} else {
		ti->checked = elm_check_state_get(obj);

		if (ti->checked) checked_count++;
		else checked_count--;

		if (select_all_layout) {
			if (total_count == checked_count)
				select_all_checked = EINA_TRUE;
			else
				select_all_checked = EINA_FALSE;

			elm_check_state_pointer_set(select_all_checkbox, &select_all_checked);
		}
	}

	elm_gengrid_item_selected_set(ti->item, ti->checked);
}

static Evas_Object *
grid_default_content_get(void *data, Evas_Object *obj, const char *part)
{
	Testitem *ti = (Testitem *)data;
	if (ti->index == 0) {
		if (!strcmp(part, "elm.swallow.end")) {
			Evas_Object *progressbar = elm_progressbar_add(obj);
			elm_object_style_set(progressbar, "process_medium");
			evas_object_size_hint_align_set(progressbar, EVAS_HINT_FILL, 0.5);
			evas_object_size_hint_weight_set(progressbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			evas_object_show(progressbar);
			elm_progressbar_pulse(progressbar, EINA_TRUE);
			return progressbar;
		} else return NULL;
	} else if (!strcmp(part, "elm.swallow.icon")) {
		Evas_Object *icon = elm_image_add(obj);
		elm_image_file_set(icon, ti->path, NULL);
	    elm_image_aspect_fixed_set(icon, EINA_FALSE);
		evas_object_show(icon);
		return icon;
	} else if (!strcmp(part, "elm.swallow.end") && mode == EDIT_MODE) {
		Evas_Object *ck = elm_check_add(obj);
		elm_object_style_set(ck, "grid");
		evas_object_propagate_events_set(ck, 1);
		elm_check_state_set(ck, ti->checked);
		elm_gengrid_item_selected_set(ti->item, ti->checked);
		evas_object_smart_callback_add(ck, "changed", _item_check_changed_cb, data);
		evas_object_show(ck);
		return ck;
	}
	return NULL;
}

static Evas_Object *
grid_photo_content_get(void *data, Evas_Object *obj, const char *part)
{
	Testitem *ti = (Testitem *)data;

	if (ti->index == 0) {
		if (!strcmp(part, "elm.swallow.end")) {
			Evas_Object *progressbar = elm_progressbar_add(obj);
			elm_object_style_set(progressbar, "process_medium");
			evas_object_size_hint_align_set(progressbar, EVAS_HINT_FILL, 0.5);
			evas_object_size_hint_weight_set(progressbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			evas_object_show(progressbar);
			elm_progressbar_pulse(progressbar, EINA_TRUE);
			return progressbar;
		} else return NULL;
	} else if (!strcmp(part, "elm.swallow.icon")) {
		int w, h, iw, ih;

		Evas_Object *image = evas_object_image_add(evas_object_evas_get(obj));
		evas_object_image_load_size_set(image, PHOTO_REAL_W, PHOTO_REAL_H);
		evas_object_image_file_set(image, ti->path, NULL);
		evas_object_image_size_get(image, &w, &h);

		double scale = elm_config_scale_get();
		if (w > PHOTO_REAL_W || h > PHOTO_REAL_H)
		{
			if (w >= h)
			{
				iw = PHOTO_REAL_W;
				ih = (int)((h * iw) / w);
			}
			else
			{
				ih = PHOTO_REAL_H;
				iw = (int)((w * ih) / h);
			}
		}
		else
		{
			iw = w;
			ih = h;
		}

		iw = iw * scale;
		ih = ih * scale;

		evas_object_image_fill_set(image, 0, 0, iw, ih);
		evas_object_resize(image, iw, ih);
		evas_object_size_hint_min_set(image, iw, ih);

		return image;
	} else if (!strcmp(part, "elm.swallow.end") && mode == EDIT_MODE) {
		Evas_Object *ck = elm_check_add(obj);
		elm_object_style_set(ck, "grid");
		evas_object_propagate_events_set(ck, 1);
		elm_check_state_set(ck, ti->checked);
		elm_gengrid_item_selected_set(ti->item, ti->checked);
		evas_object_smart_callback_add(ck, "changed", _item_check_changed_cb, data);
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
	Testitem *ti = (Testitem *)data;

	_show_selected_items(ti);

	if (mode == VIEW_MODE)
		elm_gengrid_item_selected_set(ti->item, EINA_FALSE);
}

static void _on_rotation_changed(void *data, Evas_Object *obj, void *event_info )
{
	int rot = -1;
	Evas_Object *win = obj;
	Evas_Object *gengrid = data;
	Elm_Object_Item *it;

	it = elm_gengrid_first_item_get(gengrid);
	rot = elm_win_rotation_get(win);
	while(it) {
		if (rot == 90 || rot == 270)
			elm_gengrid_item_item_class_update(it, gic_default);
		else
			elm_gengrid_item_item_class_update(it, gic_photo);
		it = elm_gengrid_item_next_get(it);
	}
}

static void
_create_gengrid (void *data, Elm_Theme *th)
{
	struct appdata *ad = (struct appdata *)data;
	int i;
	char buf[PATH_MAX];
	static Testitem ti[IMAGE_MAX+6];

	gengrid = elm_gengrid_add(ad->nf);
	elm_object_theme_set(gengrid, th);
	evas_object_size_hint_weight_set(gengrid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(gengrid, EVAS_HINT_FILL, EVAS_HINT_FILL);

	double scale = elm_config_scale_get();
	int w = (int)((10*scale)+(158*scale)+(10*scale)); // width 10+158+10
	int h = (int)((24*scale)+(158*scale)); // height 24+158
	elm_gengrid_item_size_set(gengrid, w, h);
	elm_gengrid_align_set(gengrid, 0.0, 0.0);
	elm_gengrid_horizontal_set(gengrid, EINA_FALSE);
	elm_gengrid_multi_select_set(gengrid, EINA_TRUE);
	evas_object_smart_callback_add(gengrid, "moved", grid_moved, NULL);
	evas_object_smart_callback_add(gengrid, "longpressed", grid_longpress, NULL);
	evas_object_smart_callback_add(elm_object_top_widget_get(gengrid), "rotation,changed", _on_rotation_changed, gengrid);

	gic_default = elm_gengrid_item_class_new();
	gic_default->item_style = "elm_demo_tizen/customized_default_style";
	gic_default->func.text_get = NULL;
	gic_default->func.content_get = grid_default_content_get;
	gic_default->func.state_get = NULL;
	gic_default->func.del = NULL;

	gic_photo = elm_gengrid_item_class_new();
	gic_photo->item_style = "elm_demo_tizen/customized_photo_style";
	gic_photo->func.text_get = NULL;
	gic_photo->func.content_get = grid_photo_content_get;
	gic_photo->func.state_get = NULL;
	gic_photo->func.del = NULL;

	for (i = 0; i < IMAGE_MAX; i++) {
		snprintf(buf, sizeof(buf), "%s/Albums_Item/Albums_Item%d.jpg", ICON_DIR, i+1);
		ti[i].index = i;
		ti[i].path = eina_stringshare_add(buf);
		ti[i].item = elm_gengrid_item_append(gengrid, gic_photo, &(ti[i]), _item_selected, &(ti[i]));
		ti[i].checked = EINA_FALSE;
	}

	i++;
	snprintf(buf, sizeof(buf), "%s/grid_image/1_raw.jpg", ICON_DIR);
	ti[i].index = i;
	ti[i].path = eina_stringshare_add(buf);
	ti[i].item = elm_gengrid_item_append(gengrid, gic_photo, &(ti[i]), _item_selected, &(ti[i]));
	ti[i].checked = EINA_FALSE;
	i++;
        snprintf(buf, sizeof(buf), "%s/genlist/taeyon50.jpg", ICON_DIR);
        ti[i].index = i;
        ti[i].path = eina_stringshare_add(buf);
        ti[i].item = elm_gengrid_item_append(gengrid, gic_photo, &(ti[i]), _item_selected, &(ti[i]));
        ti[i].checked = EINA_FALSE;
	i++;
        snprintf(buf, sizeof(buf), "%s/image_slider.png", ICON_DIR);
        ti[i].index = i;
        ti[i].path = eina_stringshare_add(buf);
        ti[i].item = elm_gengrid_item_append(gengrid, gic_photo, &(ti[i]), _item_selected, &(ti[i]));
        ti[i].checked = EINA_FALSE;

	i++;
        snprintf(buf, sizeof(buf), "%s/grid_image/3_raw.jpg", ICON_DIR);
        ti[i].index = i;
        ti[i].path = eina_stringshare_add(buf);
        ti[i].item = elm_gengrid_item_append(gengrid, gic_default, &(ti[i]), _item_selected, &(ti[i]));
        ti[i].checked = EINA_FALSE;

	total_count = i + 1;
}

static void
_check_select_all()
{
	Elm_Object_Item *it;
	Testitem *ti;

	if (select_all_checked) checked_count = total_count;
	else checked_count = 0;

	it = elm_gengrid_first_item_get(gengrid);
	while(it) {
		ti = elm_object_item_data_get(it);
		if (ti) ti->checked = select_all_checked;
		elm_gengrid_item_selected_set(it, select_all_checked);
		elm_gengrid_item_update (it);
		it = elm_gengrid_item_next_get(it);
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

static void _delete_button_cb(void *data, Evas_Object *obj, void *event_info)
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

static void
_edit_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *del_btn;

	if (mode == VIEW_MODE) {
		mode = EDIT_MODE;
		elm_object_text_set(edit_btn, _("Done"));

		del_btn = elm_button_add(toolbar);
		elm_object_style_set(del_btn, "style1");
		evas_object_size_hint_weight_set(del_btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(del_btn, EVAS_HINT_FILL, 0.5);
		elm_object_text_set(del_btn, _("Delete"));
		evas_object_smart_callback_add(del_btn, "clicked", _delete_button_cb, NULL);
		evas_object_show(del_btn);

		del_btn_item = elm_toolbar_item_prepend(toolbar, NULL, NULL, NULL, NULL);
		elm_object_item_part_content_set(del_btn_item, "object", del_btn);

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
	} else {
		mode = VIEW_MODE;
		elm_object_text_set(edit_btn, _("Edit"));

		elm_object_item_del(del_btn_item);

		elm_box_unpack(box, select_all_layout);
		evas_object_del(select_all_layout);
		select_all_layout = NULL;

		elm_gengrid_reorder_mode_set(gengrid, EINA_FALSE);
	}

	select_all_checked = EINA_FALSE;
	_check_select_all();
}

static Eina_Bool
_pop_cb(void *data, Elm_Object_Item *it)
{
	Elm_Theme *th = data;
	// Free customized theme
	elm_theme_free(th);

	elm_gengrid_clear(gengrid);
	evas_object_del(gengrid);
	gengrid = NULL;

	return EINA_TRUE;
}

void
gengrid_theme_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad;
	Elm_Object_Item *item;
	Elm_Object_Item *navi_it;

	ad = (struct appdata *)data;
	if (ad == NULL) return;

	mode = VIEW_MODE;

	toolbar = elm_toolbar_add(ad->nf);
	elm_toolbar_shrink_mode_set(toolbar, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_object_style_set(toolbar, "naviframe");

	edit_btn = elm_button_add(toolbar);
	evas_object_smart_callback_add(edit_btn, "clicked", _edit_btn_cb, ad);
	elm_object_style_set(edit_btn, "style1");
	evas_object_size_hint_weight_set(edit_btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(edit_btn, EVAS_HINT_FILL, 0.5);
	elm_object_text_set(edit_btn, _("Edit"));
	evas_object_show(edit_btn);

	item = elm_toolbar_item_append(toolbar, NULL, NULL, NULL, NULL);
	elm_object_item_part_content_set(item, "object", edit_btn);

	// Add customizaed theme
	Elm_Theme *th = elm_theme_new();
	elm_theme_ref_set(th, NULL); // refer to default theme
	elm_theme_extension_add(th, ELM_DEMO_EDJ);

	_create_gengrid(ad, th);

	box = elm_box_add(ad->nf);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(box);
	elm_box_pack_end(box, gengrid);
	evas_object_show(gengrid);

	navi_it = elm_naviframe_item_push (ad->nf, _("Customized Style Sample") , NULL, NULL, box, NULL);

	elm_naviframe_item_pop_cb_set(navi_it, _pop_cb, th);
}
