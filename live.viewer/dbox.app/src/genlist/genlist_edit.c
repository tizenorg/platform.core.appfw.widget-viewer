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
#include "efl_assist.h"

/*********************************************************
  Genlist Edit
 ********************************************************/

#define NUM_OF_ITEMS 1000
#define EDITMODE "Edit Mode"
#define REORDER  "Reorder"
#define EDIT_REORDER "Edit Reorder"
#define WIDTH 500
#define HEIGHT 600
#define ZOOM_TOLERANCE 0.4

static char *photo_path[] = {
	"00_list_photo_default.png", "iu.jpg", "iu2.jpg", "koo.jpg", "top.jpg", "boa.jpg",
	"kimtaehee.jpg", "moon.jpg", "taeyon.jpg"
};


typedef struct _View_Data {
	struct appdata *ad;
	char** genlist_demo_country_names;

	Evas_Object *genlist;
	Evas_Object *box;
	Elm_Object_Item *navi_it;

	Evas_Object *select_all_layout;

	Elm_Object_Item *renamed_it;
	int checked_count;
	Evas_Object *ctx;
	Elm_Object_Item *pinch_zoom_it;
	Eina_Bool pinch_zoomed;
} View_Data;

typedef struct _Item_Data {
	View_Data *vd;
	int index;
	Elm_Object_Item *it;  // Genlist Item pointer
	char *label;
	Eina_Bool checked;     // Check status
} Item_Data;

static Eina_Bool rotate_flag = EINA_FALSE;
static Evas_Object *create_del_done_toolbar(View_Data *vd);
static Evas_Object *create_done_toolbar(View_Data *vd);
static void _del_toolbar_cb(void *data, Evas_Object *obj, void *event_info);
static void _done_toolbar_cb(void *data, Evas_Object *obj, void *event_info);
static void _maxlength_reached_cb(void *data, Evas_Object *obj, void *event_info)
{
	printf("Maxlength reached!\n");
}

static void _view_free_cb(void *data, Evas *e, Evas_Object *obj, void *ei)
{
	View_Data *vd = data;
	if(vd && vd->ad) elm_object_style_set(vd->ad->bg, "default");
	if (vd) free(vd);
}

static void _gl_del(void *data, Evas_Object *obj)
{
	// FIXME: unrealized callback will be called after this.
	// accessing Item_Data can be dangerous on unrealized callback.
	Item_Data *id = data;
	if (id && id->label) free(id->label);
	if (id) free(id);
}

static void _changed_cb(void *data, Evas_Object *obj, void *event_info) // This callback is for showing(hiding) X marked button.
{
	Item_Data *id = data;
	if (id->label) free(id->label);
	id->label = strdup(elm_entry_entry_get(obj));
}

static void _unset_rename(Evas_Object *genlist)
{
	View_Data *vd = evas_object_data_get(genlist, "view_data");

	if ((vd) && (vd->renamed_it) && (elm_genlist_item_flip_get(vd->renamed_it))) {
		printf("rename stop\n");
		//elm_genlist_decorate_mode_set(vd->genlist, EINA_FALSE);
		elm_genlist_item_flip_set(vd->renamed_it, EINA_FALSE);
		elm_genlist_item_select_mode_set(vd->renamed_it, ELM_OBJECT_SELECT_MODE_DEFAULT);
		vd->renamed_it = NULL;
	}
}

static void _rename_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!data) return;
	Evas_Object *genlist = elm_object_item_widget_get(data);
	View_Data *vd = evas_object_data_get(genlist, "view_data");

	if (vd) {
		printf("rename start\n");
		_unset_rename(genlist);
		vd->renamed_it = data;
		elm_genlist_item_flip_set(vd->renamed_it, EINA_TRUE);
		elm_genlist_item_select_mode_set(vd->renamed_it, ELM_OBJECT_SELECT_MODE_NONE);
	}
}

static void
_navi_text_update(View_Data *vd)
{
	char buf[1024];
	snprintf(buf, 1023, "%d selectded", vd->checked_count);
	elm_object_item_part_text_set(vd->navi_it, NULL, buf);
}

static void
_cancel_clicked(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *genlist = data;
	_unset_rename(genlist);
}

static void _check_change_cb(void *data, Evas_Object *obj, void *event_info)
{
	// For accessibility, because accessbility activate events cannot be propagated.
	if (elm_config_access_get()) {
		Item_Data *it_data = elm_object_item_data_get(data);
		Eina_Bool state = elm_check_state_get(obj);
		if (state) it_data->vd->checked_count++;
		else it_data->vd->checked_count--;
		printf("check changed, count: %d\n", it_data->vd->checked_count);
		if (it_data->vd->select_all_layout) {
			Evas_Object *check =
				elm_object_part_content_get(it_data->vd->select_all_layout,
						"elm.icon");
			if (elm_genlist_items_count(it_data->vd->genlist) ==
					it_data->vd->checked_count)
				elm_check_state_set(check, EINA_TRUE);
			else elm_check_state_set(check, EINA_FALSE);
		}
		_navi_text_update(it_data->vd);
	}
}

static void _gl_sel(void *data, Evas_Object *obj, void *ei)
{
	Elm_Object_Item *item = ei;
	Item_Data *it_data = data;
	Eina_Bool state = EINA_FALSE;

	_unset_rename(obj);

	elm_genlist_item_selected_set(item, EINA_FALSE);
	printf("item(%p) selected\n", item);

	if (elm_genlist_item_flip_get(item)) return;
	if (!elm_genlist_decorate_mode_get(obj)) return;

	Evas_Object *ck = elm_object_item_part_content_get(ei, "elm.edit.icon.1");
	if (ck) {
		state = elm_check_state_get(ck);
		elm_check_state_set(ck, !state);
		if (!state) it_data->vd->checked_count++;
		else it_data->vd->checked_count--;
		printf("check changed, count: %d\n", it_data->vd->checked_count);
		if (it_data->vd->select_all_layout) {
			Evas_Object *check =
				elm_object_part_content_get(it_data->vd->select_all_layout,
						"elm.icon");
			if (elm_genlist_items_count(it_data->vd->genlist) ==
					it_data->vd->checked_count)
				elm_check_state_set(check, EINA_TRUE);
			else elm_check_state_set(check, EINA_FALSE);
		}
		_navi_text_update(it_data->vd);
	}
}

static char *
_create_label(View_Data *vd, int index)
{
	int num = index % NUM_OF_GENLIST_DEMO_COUNTRY_NAMES;
	return strdup(vd->genlist_demo_country_names[num]);
}

static char *_gl_text_get(void *data, Evas_Object *obj, const char *part)
{
	Item_Data *id = data;
	return strdup(id->label);
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

static void
_entry_edit_mode_show_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	evas_object_event_callback_del(obj, EVAS_CALLBACK_SHOW, _entry_edit_mode_show_cb);
	elm_object_focus_set(obj, EINA_TRUE);
}

static Evas_Object *_gl_content_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *content = NULL;
	Item_Data *id = data;

	// "edit_default" EDC layout is like below. each part is swallow part.
	// the existing item is swllowed to  elm.swallow.decorate.content part.
	// --------------------------------------------------------------------
	// | elm.edit.icon.1 | elm.swallow.decorate.content | elm.edit.icon,2 |
	// --------------------------------------------------------------------
	// if flip mode is enabled, genlist shows below swallow part
	// ------------------------------------------------------
	// | elm.flip.content |  elm.flip.icon |
	// ------------------------------------------------------

	if (elm_genlist_decorate_mode_get(obj)) {
		if (!strcmp(part, "elm.flip.content")) {
			static Elm_Entry_Filter_Limit_Size limit_filter_data;
			Evas_Object *entry;
			entry = ea_editfield_add(obj, EA_EDITFIELD_SCROLL_SINGLELINE);
			evas_object_smart_callback_add(entry, "changed", _changed_cb, id);
			evas_object_smart_callback_add(entry, "preedit,changed", _changed_cb, id);
			evas_object_smart_callback_add(entry, "maxlength,reached", _maxlength_reached_cb, NULL);

			// the below is sample code for control entry. It is not mandatory.
			limit_filter_data.max_char_count = 0;
			limit_filter_data.max_byte_count = 30;
			elm_entry_markup_filter_append(entry, elm_entry_filter_limit_size, &limit_filter_data);

			// Set entry's text as saved text
			elm_entry_entry_set(entry, id->label);
			elm_entry_cursor_end_set(entry);
			evas_object_event_callback_add(entry, EVAS_CALLBACK_SHOW, _entry_edit_mode_show_cb, NULL);
			return entry;
		} else if (!strcmp(part, "elm.flip.icon")) {
			content = elm_button_add(obj);
			elm_object_text_set(content, _("Done"));
			evas_object_smart_callback_add(content, "clicked", _cancel_clicked, obj);
			evas_object_propagate_events_set(content, EINA_FALSE);
		} else if (!strcmp(part, "elm.edit.icon.1")) {
			// swallow checkbox or radio button
			content = elm_check_add(obj);
			elm_object_style_set(content, "default/genlist");
			// keep & revoke the state from integer pointer
			elm_check_state_pointer_set(content, &id->checked);
			// Repeat events to below (genlist)
			// So that if check is clicked, genlist can be clicked.
			evas_object_repeat_events_set(content, EINA_TRUE);
			evas_object_propagate_events_set(content, EINA_FALSE);
			if (id->it) evas_object_smart_callback_add(content, "changed",
					_check_change_cb, id->it);
		} else if (!strcmp(part, "elm.edit.icon.2")) {
			// swallow rename button if need
			content = elm_button_add(obj);
			elm_object_style_set(content, "rename");
			evas_object_propagate_events_set(content, EINA_FALSE);
			if (id->it) evas_object_smart_callback_add(content, "clicked",
													_rename_button_cb, id->it);
		}
		else return _gl_icon_photo_get(data, obj, part);
	} else return _gl_icon_photo_get(data, obj, part);
	return content;
}

static void _realized_cb(void *data, Evas_Object *obj, void *ei)
{
	Item_Data *id = elm_object_item_data_get(ei);
	printf("Realized (%d)\n", id->index);
}

static void _unrealized_cb(void *data, Evas_Object *obj, void *ei)
{
	// FIXME: _gl_del callback will be called before this.
	// So id already be freed
	Item_Data *id = elm_object_item_data_get(ei);
	printf("Unrealized (%d)\n", id->index);
}

static void _item_down_cb(void *data, Evas_Object *obj, void *ei)
{
	View_Data *vd = data;
	vd->pinch_zoom_it = ei;
}

static void _items_hide(Evas_Object *obj, Eina_Bool enabled)
{
	View_Data *vd = (View_Data*)obj;
	Elm_Object_Item *it = elm_genlist_first_item_get(vd->genlist);
	while (it) {
		if (elm_genlist_item_type_get(it) != ELM_GENLIST_ITEM_GROUP) {
			if (enabled)
				elm_genlist_item_hide_set(it, EINA_TRUE);
			else
				elm_genlist_item_hide_set(it, EINA_FALSE);
		}

		it = elm_genlist_item_next_get(it);
	}
}

static Evas_Event_Flags
_pinch_zoom_cb(void *data, void *event_info)
{
   View_Data *vd = data;
   Elm_Gesture_Zoom_Info *p = (Elm_Gesture_Zoom_Info *) event_info;

   if (p->zoom > 1.0 + ZOOM_TOLERANCE)
     {
		if (!vd->pinch_zoomed) return EVAS_EVENT_FLAG_NONE;

		printf("PINCH ZOOM OUT \n");

		vd->pinch_zoomed = EINA_FALSE;
		_items_hide(data, EINA_FALSE);
		elm_genlist_item_show(vd->pinch_zoom_it, ELM_GENLIST_ITEM_SCROLLTO_TOP);

     }
   else if (p->zoom < 1.0 - ZOOM_TOLERANCE)
     {
		if (vd->pinch_zoomed) return EVAS_EVENT_FLAG_NONE;

		printf("PINCH ZOOM IN \n");

		vd->pinch_zoomed = EINA_TRUE;
		_items_hide(data, EINA_TRUE);
     }

   return EVAS_EVENT_FLAG_NONE;
}

// Create genlist and append items
static Evas_Object *_create_genlist(View_Data *vd, Evas_Object *parent)
{
	Item_Data *id;
	Elm_Object_Item *pit = NULL;
	int index = 0;
	Evas_Object *genlist;

	Elm_Theme *th = elm_theme_new();
	elm_theme_ref_set(th, NULL);
	elm_theme_extension_add(th, ELM_DEMO_EDJ);

	vd->genlist_demo_country_names = genlist_get_demo_country_names();

#define ITC_MAX 4
	Elm_Genlist_Item_Class *itc[ITC_MAX], *itch;
	for (index = 0; index < ITC_MAX ; index++) {
		itc[index] = elm_genlist_item_class_new();
	}
	itch = elm_genlist_item_class_new();

	itc[0]->item_style = "1text";
	itc[0]->func.text_get = _gl_text_get;
	itc[0]->func.content_get = _gl_content_get;
	itc[0]->func.del = _gl_del;
	//itc[0]->decorate_all_item_style = "edit_default";

	itc[1]->item_style = "1text.1icon.5.thumb.circle";
	itc[1]->func.text_get = _gl_text_get;
	itc[1]->func.content_get = _gl_content_get;
	itc[1]->func.del = _gl_del;
	itc[1]->decorate_all_item_style = "edit_default";

	itc[2]->item_style = "1text.1icon.2.thumb.square";
	itc[2]->func.text_get = _gl_text_get;
	itc[2]->func.content_get = _gl_content_get;
	itc[2]->func.del = _gl_del;
	itc[2]->decorate_all_item_style = "edit_default";

	itc[3]->item_style = "1text.1icon.3";
	itc[3]->func.text_get = _gl_text_get;
	itc[3]->func.content_get = _gl_content_get;
	itc[3]->func.del = _gl_del;
	itc[3]->decorate_all_item_style = "edit_default";

	itch->item_style = "groupindex";
	itch->func.text_get = _gl_text_get;

	// Create genlist
	genlist = elm_genlist_add(parent);
	elm_object_theme_set(genlist, th);
	// HOMOGENEOUS MODE
	// If item height is same when each style name is same,
	// Use homogeneous mode.
	elm_genlist_homogeneous_set(genlist, EINA_TRUE);
	printf("Homogeneous mode enabled\n");
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_smart_callback_add(genlist, "realized", _realized_cb, vd);
	evas_object_smart_callback_add(genlist, "unrealized", _unrealized_cb, vd);
	evas_object_smart_callback_add(genlist, "pressed", _item_down_cb, vd);
	evas_object_data_set(genlist, "view_data", vd);

	for (index = 0; index < NUM_OF_ITEMS; index++) {
		// Create Item_Data
		id = calloc(1, sizeof(Item_Data));
		id->vd = vd;
		id->index = index;
		id->label = _create_label(vd, index);
		if ((index % 10) == 0) {
			id->it = elm_genlist_item_append(genlist, itch, id, NULL,
					ELM_GENLIST_ITEM_GROUP, NULL, id);
			elm_genlist_item_select_mode_set(id->it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
			pit = id->it;
		} else {
			id->it = elm_genlist_item_append(genlist, itc[index%ITC_MAX], id, pit,
					ELM_GENLIST_ITEM_NONE, _gl_sel, id);
		}
	}
	evas_object_show(genlist);

	for (index = 0; index < ITC_MAX ; index++) {
		elm_genlist_item_class_free(itc[index]);
	}
	elm_genlist_item_class_free(itch);

	return genlist;
}

static void _select_all_chk_changed_cb(void *data, Evas_Object *obj, void *ei)
{
	View_Data *vd = data;
	Eina_Bool state = elm_check_state_get(obj);
	int cnt = 0;

	Elm_Object_Item *it = elm_genlist_first_item_get(vd->genlist);
	while(it) {
		Item_Data *id = elm_object_item_data_get(it);
		const Elm_Genlist_Item_Class *itc = elm_genlist_item_item_class_get(it);
		if (itc->decorate_all_item_style) {
			// For realized items, set state of real check object
			Evas_Object *ck = elm_object_item_part_content_get(it, "elm.edit.icon.1");
			if (ck) elm_check_state_set(ck, state);
			// For all items (include unrealized), just set pointer state
			id->checked = state;
			if (state) cnt++;
		}
		it = elm_genlist_item_next_get(it);
	}
	vd->checked_count = cnt;
	_navi_text_update(vd);
}

static void _select_all_layout_down_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	printf("select all layout clicked\n");
	View_Data *vd = data;
	Evas_Object *check = elm_object_part_content_get(vd->select_all_layout, "elm.icon");
	Eina_Bool state = elm_check_state_get(check);
	elm_check_state_set(check, !state);
	_select_all_chk_changed_cb(data, check, NULL);
}

static void
_navi_btn_clicked_cb(void *data, Evas_Object *obj, void *ei)
{
	printf("Naviframe button clicked\n");
	View_Data *vd = data;
	Evas_Object *check = elm_object_part_content_get(vd->select_all_layout, "elm.icon");
	Eina_Bool state = elm_check_state_get(check);
	elm_check_state_set(check, !state);
	_select_all_chk_changed_cb(data, check, NULL);

}

static char *_access_info_cb(void *data, Evas_Object *obj)
{
    const char *txt = NULL;
	txt = elm_object_part_text_get(data, "elm.text");
	if (!txt) return NULL;

	return strdup(_(txt));
}

static Eina_Bool _access_activate_cb(void *data, Evas_Object *obj, Elm_Access_Action_Info *ai)
{
	_select_all_layout_down_cb(data, NULL, NULL, NULL);
	return EINA_TRUE;
}

static Evas_Object *
_create_select_all_layout(View_Data *vd)
{
	Evas_Object *layout = elm_layout_add(vd->box);
	elm_layout_theme_set(layout, "genlist", "item", "select_all/default");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_FILL);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_event_callback_add(layout, EVAS_CALLBACK_MOUSE_DOWN, _select_all_layout_down_cb, vd);
	evas_object_propagate_events_set(layout, EINA_FALSE);
	evas_object_show(layout);

	Evas_Object *check = elm_check_add(layout);
	evas_object_propagate_events_set(check, EINA_FALSE);
	evas_object_smart_callback_add(check, "changed", _select_all_chk_changed_cb, vd);
	elm_object_part_content_set(layout, "elm.icon", check);
	elm_object_part_text_set(layout, "elm.text", "Select All");

	// ============== Accessibility
	Evas_Object *ao = elm_access_object_register(layout, layout);
	elm_access_info_cb_set(ao, ELM_ACCESS_INFO, _access_info_cb, layout);
    elm_access_action_cb_set(ao, ELM_ACCESS_ACTION_ACTIVATE, _access_activate_cb, vd);
	elm_object_focus_custom_chain_append(layout, ao, NULL);

	return layout;
}

static Evas_Object *create_del_done_toolbar(View_Data *vd)
{
	Evas_Object *obj;

	/*create toolbar */
	obj = elm_toolbar_add(vd->ad->nf);
	if(obj == NULL) return NULL;
	elm_object_style_set(obj, "default");
	elm_toolbar_shrink_mode_set(obj, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_transverse_expanded_set(obj, EINA_TRUE);
	elm_toolbar_select_mode_set(obj, ELM_OBJECT_SELECT_MODE_NONE);

	elm_toolbar_item_append(obj, NULL, "Delete", _del_toolbar_cb, vd);
	elm_toolbar_item_append(obj, NULL, "Done", _done_toolbar_cb, vd);
	return obj;
}

static Evas_Object *create_done_toolbar(View_Data *vd)
{
	Evas_Object *obj;
	struct appdata *ad;
	ad = vd->ad;

	/*create toolbar */
	obj = elm_toolbar_add(ad->nf);
	if(obj == NULL) return NULL;
	elm_object_style_set(obj, "default");
	elm_toolbar_shrink_mode_set(obj, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_transverse_expanded_set(obj, EINA_TRUE);
	elm_toolbar_select_mode_set(obj, ELM_OBJECT_SELECT_MODE_NONE);

	elm_toolbar_item_append(obj, NULL, "Done", _done_toolbar_cb, vd);
	return obj;
}

static Evas_Object *_create_navi_title_btn(View_Data *vd)
{
	Evas_Object *ic;
	Evas_Object *btn = elm_button_add(vd->ad->nf);
	elm_object_style_set(btn, "naviframe/title_icon");
	ic = elm_image_add(btn);
	elm_image_file_set(ic, ICON_DIR"/00_icon_select_all.png", NULL);
	evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(ic, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(btn, "icon", ic);
	evas_object_smart_callback_add(btn, "clicked", _navi_btn_clicked_cb, vd);
	return btn;
}

void _del_toolbar_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!data) return;
	View_Data *vd =data;

	Elm_Object_Item *it = elm_genlist_first_item_get(vd->genlist);
	while (it) {
		Item_Data *id = elm_object_item_data_get(it);
		// For unrealized items, use check pointer
		if (id->checked) {
			// Before delete, get next item.
			Elm_Object_Item *del_it = it;
			it = elm_genlist_item_next_get(it);

			elm_object_item_del(del_it);
			vd->checked_count--;
		} else it = elm_genlist_item_next_get(it);
	}
}

void _done_toolbar_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!data) return;
	View_Data *vd = data;
	Evas_Object *tmp;

	struct appdata *ad;
	ad = vd->ad;
	if (!ad) return;

	// Delete select all layout
	if (vd->select_all_layout) {
		elm_box_unpack(vd->box, vd->select_all_layout);
		evas_object_del(vd->select_all_layout);
		vd->select_all_layout = NULL;
	}

	// Delete naviframe button
	elm_object_item_part_text_set(vd->navi_it, NULL, "Edit Mode");
	tmp = elm_object_item_part_content_unset(vd->navi_it, "title_right_btn");
	if (tmp) evas_object_del(tmp);

    elm_genlist_decorate_mode_set(vd->genlist, EINA_FALSE);
	elm_genlist_reorder_mode_set(vd->genlist, EINA_FALSE);

	tmp = elm_object_item_part_content_unset(vd->navi_it, "toolbar");
	if(tmp) evas_object_del(tmp);
}

static void
_edit_on(void *data)
{
	if (!data) return;
	View_Data *vd = data;
	struct appdata *ad = vd->ad;
	Evas_Object *tmp;

	if (elm_genlist_decorate_mode_get(vd->genlist) &&
		!elm_genlist_reorder_mode_get(vd->genlist)) return;

	if (elm_genlist_reorder_mode_get(vd->genlist))
		elm_genlist_reorder_mode_set(vd->genlist, EINA_FALSE);

	// Change bg style as editmode
	elm_object_style_set(ad->bg, "edit_mode");

	// Create select all layout
	if (!vd->select_all_layout) {
		vd->select_all_layout = _create_select_all_layout(vd);
		elm_box_pack_start(vd->box, vd->select_all_layout);
	}

	// Create toolbars
	tmp = elm_object_item_part_content_unset(vd->navi_it, "toolbar");
	if (tmp) evas_object_del(tmp);
	elm_object_item_part_content_set(vd->navi_it, "toolbar",
			create_del_done_toolbar(vd));

	// Create naviframe select all button
	if (!elm_object_item_part_content_get(vd->navi_it, "title_right_btn")) {
		elm_object_item_part_text_set(vd->navi_it, NULL, "Select Items");
		elm_object_item_part_content_set(vd->navi_it, "title_right_btn",
				_create_navi_title_btn(vd));
	}

	// Set decoreate mode
	elm_genlist_decorate_mode_set(vd->genlist, EINA_TRUE);
	printf("Edit Mode on\n");
}

static void
_edit_reorder_on(void *data)
{
	if (!data) return;
	View_Data *vd = data;
	Evas_Object *tmp;
	struct appdata *ad = vd->ad;

	if (elm_genlist_decorate_mode_get(vd->genlist) &&
		elm_genlist_reorder_mode_get(vd->genlist)) return;

	elm_object_style_set(ad->bg, "edit_mode");

	// Create select all layout
	if (!vd->select_all_layout) {
		vd->select_all_layout = _create_select_all_layout(vd);
		elm_box_pack_start(vd->box, vd->select_all_layout);
	}

	// Create toolbars
	tmp = elm_object_item_part_content_unset(vd->navi_it, "toolbar");
	if (tmp) evas_object_del(tmp);
	elm_object_item_part_content_set(vd->navi_it, "toolbar",
			create_del_done_toolbar(vd));

	// Create naviframe select all button
	if (!elm_object_item_part_content_get(vd->navi_it, "title_right_btn")) {
		elm_object_item_part_text_set(vd->navi_it, NULL, "Select Items");
		elm_object_item_part_content_set(vd->navi_it, "title_right_btn",
				_create_navi_title_btn(vd));
	}

	if (!elm_genlist_decorate_mode_get(vd->genlist))
		elm_genlist_decorate_mode_set(vd->genlist, EINA_TRUE);

	if (!elm_genlist_reorder_mode_get(vd->genlist))
		elm_genlist_reorder_mode_set(vd->genlist, EINA_TRUE);
	printf("Edit Reorder Mode on\n");
}

static void
_reorder_on(void *data)
{
	if (!data) return;
	View_Data *vd = data;
	Evas_Object *tmp;

	if (!elm_genlist_decorate_mode_get(vd->genlist) &&
		elm_genlist_reorder_mode_get(vd->genlist)) return;

	if (elm_genlist_decorate_mode_get(vd->genlist))
		elm_genlist_decorate_mode_set(vd->genlist, EINA_FALSE);

	// Delete select all layout
	if (vd->select_all_layout) {
		elm_box_unpack(vd->box, vd->select_all_layout);
		evas_object_del(vd->select_all_layout);
		vd->select_all_layout = NULL;
	}

	// Create toolbars
	tmp = elm_object_item_part_content_unset(vd->navi_it, "toolbar");
	if (tmp) evas_object_del(tmp);
	elm_object_item_part_content_set(vd->navi_it, "toolbar",
									create_done_toolbar(vd));

	// Delete naviframe button
	elm_object_item_part_text_set(vd->navi_it, NULL, "Edit Mode");
	tmp = elm_object_item_part_content_unset(vd->navi_it, "title_right_btn");
	if (tmp) evas_object_del(tmp);

	elm_genlist_reorder_mode_set(vd->genlist, EINA_TRUE);
	printf("Reorder Mode on\n");
}

static void
_extpopup_cb(void *data, Evas_Object *obj, void *event)
{
	if ((!data) || (!obj)) return;
	View_Data *vd = data;
	const char *label = elm_object_item_text_get((Elm_Object_Item *) event);

	if (label) {
		if (!strcmp(label, EDITMODE)) {
			_edit_on(vd);
		} else if (!strcmp(label, REORDER)) {
			_reorder_on(vd);
		} else  if (!strcmp(label, EDIT_REORDER)) {
			_edit_reorder_on(vd);
		}
     }
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
_menu_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
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
	elm_ctxpopup_item_append(ctx, EDITMODE, NULL, _extpopup_cb, vd);
	elm_ctxpopup_item_append(ctx, REORDER, NULL, _extpopup_cb, vd);
	elm_ctxpopup_item_append(ctx, EDIT_REORDER, NULL, _extpopup_cb, vd);
	evas_object_size_hint_max_set(ctx, s*WIDTH, s*HEIGHT);
	elm_ctxpopup_direction_priority_set(ctx, ELM_CTXPOPUP_DIRECTION_UP,
										ELM_CTXPOPUP_DIRECTION_UNKNOWN,
										ELM_CTXPOPUP_DIRECTION_UNKNOWN,
										ELM_CTXPOPUP_DIRECTION_UNKNOWN);

	_move_ctx(vd, ctx);
	evas_object_show(ctx);
}

static Evas_Object *_create_menu_button(View_Data *vd)
{
	Evas_Object *button;

	button = elm_button_add(vd->ad->nf);
	elm_object_style_set(button, "naviframe/more/default");
	evas_object_size_hint_weight_set(button, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(button, EVAS_HINT_FILL, 0.5);
	elm_object_text_set(button, _("Menu"));
	evas_object_show(button);
	evas_object_smart_callback_add(button, "clicked", _menu_btn_clicked_cb, vd);
	return button;
}

void genlist_edit_mode_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!data) return;
	View_Data *vd;
	struct appdata *ad = data;
	Evas_Object *g_layer;

	// Create layout data for this view
	vd = calloc(1, sizeof(View_Data));
	vd->ad = ad;

	vd->box = elm_box_add(vd->ad->nf);
	evas_object_size_hint_weight_set(vd->box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(vd->box, EVAS_HINT_FILL, EVAS_HINT_FILL);
	vd->genlist = _create_genlist(vd, vd->box);
	elm_box_pack_end(vd->box, vd->genlist);
	evas_object_show(vd->box);

	g_layer = elm_gesture_layer_add(vd->genlist);
	if (g_layer) {
	   elm_gesture_layer_attach(g_layer, vd->genlist);
	   elm_gesture_layer_cb_set
	      (g_layer, ELM_GESTURE_ZOOM, ELM_GESTURE_STATE_MOVE, _pinch_zoom_cb, vd);
	}

	vd->navi_it = elm_naviframe_item_push(ad->nf, _("Edit Mode"), NULL, NULL, vd->box, NULL);
	evas_object_event_callback_add(vd->box, EVAS_CALLBACK_FREE, _view_free_cb, vd);
	elm_object_item_part_content_set(vd->navi_it, "toolbar_more_btn",
					_create_menu_button(vd));
}
