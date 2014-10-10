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
#include <math.h>

//Evas_Object *sub_view[10];

#define ELM_MAX(v1, v2)    (((v1) > (v2)) ? (v1) : (v2))

static void _1_items_style_cb(void *data, Evas_Object *obj, void *event_info);
static void _2_items_style_cb(void *data, Evas_Object *obj, void *event_info);
static void _3_items_style_cb(void *data, Evas_Object *obj, void *event_info);
static void _1_icons_style_cb(void *data, Evas_Object *obj, void *event_info);
static void _2_icons_style_cb(void *data, Evas_Object *obj, void *event_info);
static void _3_icons_style_cb(void *data, Evas_Object *obj, void *event_info);
static void _4_icons_style_cb(void *data, Evas_Object *obj, void *event_info);
static void _5_icons_style_cb(void *data, Evas_Object *obj, void *event_info);
static void _6_icons_style_cb(void *data, Evas_Object *obj, void *event_info);
static void _2_icons_gap_style_cb(void *data, Evas_Object *obj, void *event_info);
static void _3_icons_gap_style_cb(void *data, Evas_Object *obj, void *event_info);
//static void _segment_style_cb(void *data, Evas_Object *obj, void *event_info);

static Evas_Object* _create_list_winset(Evas_Object* parent, struct _menu_item *menu, struct appdata* ad);

static Evas_Object *create_1_items_toolbar(struct appdata *ad);
static Evas_Object *create_2_items_toolbar(struct appdata *ad);
static Evas_Object *create_3_items_toolbar(struct appdata *ad);
static Evas_Object *create_1_icons_toolbar(struct appdata *ad);
static Evas_Object *create_2_icons_toolbar(struct appdata *ad);
static Evas_Object *create_3_icons_toolbar(struct appdata *ad);
static Evas_Object *create_4_icons_toolbar(struct appdata *ad);
static Evas_Object *create_5_icons_toolbar(struct appdata *ad);
static Evas_Object *create_6_icons_toolbar(struct appdata *ad);
static Evas_Object *create_2_icons_gap_toolbar(struct appdata *ad);
static Evas_Object *create_3_icons_gap_toolbar(struct appdata *ad);
//static Evas_Object *create_segment_items_toolbar(struct appdata *ad);

static struct _menu_item menu_main[] = {
	{ "1 Items Style", _1_items_style_cb},
	{ "2 Items Style", _2_items_style_cb},
	{ "3 Items Style", _3_items_style_cb},
	{ "1 Icons Style", _1_icons_style_cb},
	{ "2 Icons Style", _2_icons_style_cb},
	{ "3 Icons Style", _3_icons_style_cb},
	{ "4 Icons Style", _4_icons_style_cb},
	{ "5 Icons Style", _5_icons_style_cb},
	{ "6 Icons Style", _6_icons_style_cb},
	{ "2 Icons Gap Style", _2_icons_gap_style_cb},
	{ "3 Icons Gap Style", _3_icons_gap_style_cb},
//	{ "Segment Style", _segment_style_cb},
	/* do not delete below */
	{ NULL, NULL }
};

static void _response_cb(void *data, Evas_Object *obj, void *event_info)
{
	evas_object_del(data);
}

static void _toolbar_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	Evas_Object *popup, *btn1, *btn2;

	popup = elm_popup_add(ad->win_main);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_text_set(popup,"Toolbar item was clicked. if you want to go back, press any button.");
	btn1 = elm_button_add(popup);
	elm_object_style_set(btn1, "popup");
	elm_object_text_set(btn1, "OK");
	elm_object_part_content_set(popup, "button1", btn1);
	evas_object_smart_callback_add(btn1, "clicked", _response_cb, popup);
	btn2 = elm_button_add(popup);
	elm_object_style_set(btn2, "popup");
	elm_object_text_set(btn2, "Cancel");
	elm_object_part_content_set(popup, "button2", btn2);
	evas_object_smart_callback_add(btn2, "clicked", _response_cb, popup);
	evas_object_show(popup);
}

static void _1_items_style_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *toolbar;
	Elm_Object_Item *navi_it;
	struct appdata *ad;
	ad = (struct appdata *) data;
	if(ad == NULL) return;

	navi_it = elm_naviframe_top_item_get(ad->nf);
	toolbar = create_1_items_toolbar(ad);
	elm_object_item_part_content_set(navi_it, "toolbar", toolbar);
}

static void _2_items_style_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *toolbar;
	Elm_Object_Item *navi_it;
	struct appdata *ad;
	ad = (struct appdata *) data;
	if(ad == NULL) return;

	navi_it = elm_naviframe_top_item_get(ad->nf);
	toolbar = create_2_items_toolbar(ad);
	elm_object_item_part_content_set(navi_it, "toolbar", toolbar);
}

static void _3_items_style_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *toolbar;
	Elm_Object_Item *navi_it;
	struct appdata *ad;
	ad = (struct appdata *) data;
	if(ad == NULL) return;

	navi_it = elm_naviframe_top_item_get(ad->nf);
	toolbar = create_3_items_toolbar(ad);
	elm_object_item_part_content_set(navi_it, "toolbar", toolbar);
}

static void _1_icons_style_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *toolbar;
	Elm_Object_Item *navi_it;
	struct appdata *ad;
	ad = (struct appdata *) data;
	if(ad == NULL) return;

	navi_it = elm_naviframe_top_item_get(ad->nf);
	toolbar = create_1_icons_toolbar(ad);
	elm_object_item_part_content_set(navi_it, "toolbar", toolbar);
}

static void _2_icons_style_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *toolbar;
	Elm_Object_Item *navi_it;
	struct appdata *ad;
	ad = (struct appdata *) data;
	if(ad == NULL) return;

	navi_it = elm_naviframe_top_item_get(ad->nf);
	toolbar = create_2_icons_toolbar(ad);
	elm_object_item_part_content_set(navi_it, "toolbar", toolbar);
}

static void _3_icons_style_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *toolbar;
	Elm_Object_Item *navi_it;
	struct appdata *ad;
	ad = (struct appdata *) data;
	if(ad == NULL) return;

	navi_it = elm_naviframe_top_item_get(ad->nf);
	toolbar = create_3_icons_toolbar(ad);
	elm_object_item_part_content_set(navi_it, "toolbar", toolbar);
}

static void _4_icons_style_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *toolbar;
	Elm_Object_Item *navi_it;
	struct appdata *ad;
	ad = (struct appdata *) data;
	if(ad == NULL) return;

	navi_it = elm_naviframe_top_item_get(ad->nf);
	toolbar = create_4_icons_toolbar(ad);
	elm_object_item_part_content_set(navi_it, "toolbar", toolbar);
}

static void _5_icons_style_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *toolbar;
	Elm_Object_Item *navi_it;
	struct appdata *ad;
	ad = (struct appdata *) data;
	if(ad == NULL) return;

	navi_it = elm_naviframe_top_item_get(ad->nf);
	toolbar = create_5_icons_toolbar(ad);
	elm_object_item_part_content_set(navi_it, "toolbar", toolbar);
}

static void _6_icons_style_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *toolbar;
	Elm_Object_Item *navi_it;
	struct appdata *ad;
	ad = (struct appdata *) data;
	if(ad == NULL) return;

	navi_it = elm_naviframe_top_item_get(ad->nf);
	toolbar = create_6_icons_toolbar(ad);
	elm_object_item_part_content_set(navi_it, "toolbar", toolbar);
}

static void _2_icons_gap_style_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *toolbar;
	Elm_Object_Item *navi_it;
	struct appdata *ad;
	ad = (struct appdata *) data;
	if(ad == NULL) return;

	navi_it = elm_naviframe_top_item_get(ad->nf);
	toolbar = create_2_icons_gap_toolbar(ad);
	elm_object_item_part_content_set(navi_it, "toolbar", toolbar);
}

static void _3_icons_gap_style_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *toolbar;
	Elm_Object_Item *navi_it;
	struct appdata *ad;
	ad = (struct appdata *) data;
	if(ad == NULL) return;

	navi_it = elm_naviframe_top_item_get(ad->nf);
	toolbar = create_3_icons_gap_toolbar(ad);
	elm_object_item_part_content_set(navi_it, "toolbar", toolbar);
}

/*
static void _segment_style_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *toolbar;
	Elm_Object_Item *navi_it;
	struct appdata *ad;
	ad = (struct appdata *) data;
	if(ad == NULL) return;

	navi_it = elm_naviframe_top_item_get(ad->nf);
	toolbar = create_segment_items_toolbar(ad);
	elm_object_item_part_content_set(navi_it, "toolbar", toolbar);
}*/

static void _list_click(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *it = (Elm_Object_Item *) elm_list_selected_item_get(obj);
	if(it == NULL) return;

	elm_list_item_selected_set(it, EINA_FALSE);
}

static Evas_Object* _create_list_winset(Evas_Object* parent, struct _menu_item *menu, struct appdata *ad)
{
	Evas_Object *li;

	if(parent == NULL || ad == NULL) return NULL;

	li = elm_list_add(parent);
	elm_list_mode_set(li, ELM_LIST_COMPRESS);
	evas_object_smart_callback_add(li, "selected", _list_click, NULL);

	int idx = 0;

	while(menu[ idx ].name != NULL) {

		elm_list_item_append(
				li,
				menu[ idx ].name,
				NULL,
				NULL,
				menu[ idx ].func,
				ad);
		++idx;
	}

	elm_list_go(li);

	return li;
}
/*
static void _move_ctxpopup(Evas_Object *ctxpopup, Evas_Object *btn)
{
   Evas_Coord x, y, w , h;
   evas_object_geometry_get(btn, &x, &y, &w, &h);
   evas_object_move(ctxpopup, x + (w / 2), y + (h /2));
}

static void _dismissed_cb(void *data, Evas_Object *obj, void *event_info)
{
	evas_object_smart_callback_del(obj ,"dismissed", _dismissed_cb);
	evas_object_del(obj);
	obj = NULL;
}

static void _create_ctxpopup_toolbar_segmentedControl(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *ctxpopup;
	struct appdata *ad = (struct appdata *) data;

	ctxpopup = elm_ctxpopup_add(ad->nf);
	elm_object_style_set(ctxpopup, "toolbar_segmentedcontrol");
	evas_object_smart_callback_add(ctxpopup,"dismissed", _dismissed_cb, NULL);

	elm_ctxpopup_item_append(ctxpopup, _("Group1"), NULL, NULL, ctxpopup);
	elm_ctxpopup_item_append(ctxpopup, _("Group2"), NULL, NULL, ctxpopup);
	elm_ctxpopup_item_append(ctxpopup, _("Group3"), NULL, NULL, ctxpopup);
	elm_ctxpopup_item_append(ctxpopup, _("Group4"), NULL, NULL, ctxpopup);
	_move_ctxpopup(ctxpopup, obj);
	evas_object_show(ctxpopup);
}*/

static Evas_Object *create_1_items_toolbar(struct appdata *ad)
{
	Evas_Object *obj;
	Elm_Object_Item *item[10];

	/* create toolbar */
	obj = elm_toolbar_add(ad->nf);
	if(obj == NULL) return NULL;
	elm_object_style_set(obj, "default");
	elm_toolbar_shrink_mode_set(obj, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_transverse_expanded_set(obj, EINA_TRUE);
	elm_toolbar_select_mode_set(obj, ELM_OBJECT_SELECT_MODE_NONE);

	item[0] = elm_toolbar_item_append(obj, NULL, "Main", _toolbar_clicked_cb, ad);

	return obj;
}

static Evas_Object *create_2_items_toolbar(struct appdata *ad)
{
	Evas_Object *obj;
	Elm_Object_Item *item[10];

	/* create toolbar */
	obj = elm_toolbar_add(ad->nf);
	if(obj == NULL) return NULL;
	elm_object_style_set(obj, "default");
	elm_toolbar_shrink_mode_set(obj, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_transverse_expanded_set(obj, EINA_TRUE);
	elm_toolbar_select_mode_set(obj, ELM_OBJECT_SELECT_MODE_NONE);

	item[0] = elm_toolbar_item_append(obj, NULL, "Main", _toolbar_clicked_cb, ad);
	elm_object_item_disabled_set(item[0], EINA_TRUE);
	item[1] = elm_toolbar_item_append(obj, NULL, "Playlist", _toolbar_clicked_cb, ad);
	return obj;
}

static Evas_Object *create_3_items_toolbar(struct appdata *ad)
{
	Evas_Object *obj;
	Elm_Object_Item *item[10];

	/* create toolbar */
	obj = elm_toolbar_add(ad->nf);
	if(obj == NULL) return NULL;
	elm_object_style_set(obj, "default");
	elm_toolbar_shrink_mode_set(obj, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_transverse_expanded_set(obj, EINA_TRUE);
	elm_toolbar_select_mode_set(obj, ELM_OBJECT_SELECT_MODE_NONE);

	item[0] = elm_toolbar_item_append(obj, NULL, "Main", _toolbar_clicked_cb, ad);
	item[1] = elm_toolbar_item_append(obj, NULL, "Playlist", _toolbar_clicked_cb, ad);
	item[2] = elm_toolbar_item_append(obj, NULL, "Artists list", _toolbar_clicked_cb, ad);
	return obj;
}

static Evas_Object *create_1_icons_toolbar(struct appdata *ad)
{
	Evas_Object *obj;
	Elm_Object_Item *item[10];

	/* create toolbar */
	obj = elm_toolbar_add(ad->nf);
	if(obj == NULL) return NULL;
	elm_object_style_set(obj, "default");
	elm_toolbar_shrink_mode_set(obj, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_transverse_expanded_set(obj, EINA_TRUE);
	elm_toolbar_select_mode_set(obj, ELM_OBJECT_SELECT_MODE_NONE);

	item[0] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_favorites.png", "Main", _toolbar_clicked_cb, ad);
	return obj;
}

static Evas_Object *create_2_icons_toolbar(struct appdata *ad)
{
	Evas_Object *obj;
	Elm_Object_Item *item[10];

	/* create toolbar */
	obj = elm_toolbar_add(ad->nf);
	if(obj == NULL) return NULL;
	elm_object_style_set(obj, "default");
	elm_toolbar_shrink_mode_set(obj, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_transverse_expanded_set(obj, EINA_TRUE);
	elm_toolbar_select_mode_set(obj, ELM_OBJECT_SELECT_MODE_NONE);

	item[0] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_favorites.png", "Main", _toolbar_clicked_cb, ad);
	item[1] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_artists.png", "Artists list", _toolbar_clicked_cb, ad);
	return obj;
}

static Evas_Object *create_3_icons_toolbar(struct appdata *ad)
{
	Evas_Object *obj;
	Elm_Object_Item *item[10];

	/* create toolbar */
	obj = elm_toolbar_add(ad->nf);
	if(obj == NULL) return NULL;
	elm_object_style_set(obj, "default");
	elm_toolbar_shrink_mode_set(obj, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_transverse_expanded_set(obj, EINA_TRUE);
	elm_toolbar_select_mode_set(obj, ELM_OBJECT_SELECT_MODE_NONE);

	item[0] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_favorites.png", "Main", _toolbar_clicked_cb, ad);
	item[1] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_artists.png", "Artists list", _toolbar_clicked_cb, ad);
	item[2] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_playlist.png", "Playlist", _toolbar_clicked_cb, ad);
	return obj;
}

static Evas_Object *create_4_icons_toolbar(struct appdata *ad)
{
	Evas_Object *obj;
	Elm_Object_Item *item[10];

	/* create toolbar */
	obj = elm_toolbar_add(ad->nf);
	if(obj == NULL) return NULL;
	elm_object_style_set(obj, "default");
	elm_toolbar_shrink_mode_set(obj, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_transverse_expanded_set(obj, EINA_TRUE);
	elm_toolbar_select_mode_set(obj, ELM_OBJECT_SELECT_MODE_NONE);

	item[0] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_favorites.png", "Main", _toolbar_clicked_cb, ad);
	item[1] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_artists.png", "Artists list", _toolbar_clicked_cb, ad);
	item[2] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_playlist.png", "Playlist", _toolbar_clicked_cb, ad);
	item[3] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_songs.png", "Songs", _toolbar_clicked_cb, ad);
	return obj;
}

static Evas_Object *create_5_icons_toolbar(struct appdata *ad)
{
	Evas_Object *obj;
	Elm_Object_Item *item[10];

	/* create toolbar */
	obj = elm_toolbar_add(ad->nf);
	if(obj == NULL) return NULL;
	elm_object_style_set(obj, "default");
	elm_toolbar_shrink_mode_set(obj, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_transverse_expanded_set(obj, EINA_TRUE);
	elm_toolbar_select_mode_set(obj, ELM_OBJECT_SELECT_MODE_NONE);

	item[0] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_favorites.png", "Main", _toolbar_clicked_cb, ad);
	item[1] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_artists.png", "Artists list", _toolbar_clicked_cb, ad);
	item[2] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_playlist.png", "Playlist", _toolbar_clicked_cb, ad);
	item[3] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_songs.png", "Songs", _toolbar_clicked_cb, ad);
	item[4] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_dialer.png", "Dialer for call", _toolbar_clicked_cb, ad);
	return obj;
}

static Evas_Object *create_6_icons_toolbar(struct appdata *ad)
{
	Evas_Object *obj;
	Elm_Object_Item *item[10];

	/* create toolbar */
	obj = elm_toolbar_add(ad->nf);
	if(obj == NULL) return NULL;
	elm_object_style_set(obj, "default");
	elm_toolbar_shrink_mode_set(obj, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_transverse_expanded_set(obj, EINA_TRUE);
	elm_toolbar_select_mode_set(obj, ELM_OBJECT_SELECT_MODE_NONE);

	item[0] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_favorites.png", "Main", _toolbar_clicked_cb, ad);
	item[1] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_artists.png", "Artists list", _toolbar_clicked_cb, ad);
	item[2] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_playlist.png", "Playlist", _toolbar_clicked_cb, ad);
	item[3] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_songs.png", "Songs", _toolbar_clicked_cb, ad);
	item[4] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_dialer.png", "Dialer for call", _toolbar_clicked_cb, ad);
	item[5] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_playlist.png", "Playlist", _toolbar_clicked_cb, ad);
	return obj;
}

static Evas_Object *create_2_icons_gap_toolbar(struct appdata *ad)
{
	Evas_Object *obj;
	Elm_Object_Item *item[10];

	/* create toolbar */
	obj = elm_toolbar_add(ad->nf);
	if(obj == NULL) return NULL;
	elm_object_style_set(obj, "default");
	elm_toolbar_shrink_mode_set(obj, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_transverse_expanded_set(obj, EINA_TRUE);
	elm_toolbar_select_mode_set(obj, ELM_OBJECT_SELECT_MODE_NONE);

	item[0] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_favorites.png", "Main", _toolbar_clicked_cb, ad);
	item[1] = elm_toolbar_item_append(obj, NULL, NULL, NULL, NULL);
	elm_object_item_disabled_set(item[1], EINA_TRUE);
	item[2] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_artists.png", "Artists list", _toolbar_clicked_cb, ad);
	return obj;
}

static Evas_Object *create_3_icons_gap_toolbar(struct appdata *ad)
{
	Evas_Object *obj;
	Elm_Object_Item *item[10];

	/* create toolbar */
	obj = elm_toolbar_add(ad->nf);
	if(obj == NULL) return NULL;
	elm_object_style_set(obj, "default");
	elm_toolbar_shrink_mode_set(obj, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_transverse_expanded_set(obj, EINA_TRUE);
	elm_toolbar_select_mode_set(obj, ELM_OBJECT_SELECT_MODE_NONE);

	item[0] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_favorites.png", "Main", _toolbar_clicked_cb, ad);
	item[1] = elm_toolbar_item_append(obj, NULL, NULL, NULL, NULL);
	elm_object_item_disabled_set(item[1],EINA_TRUE);
	item[2] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_artists.png", "Artists list", _toolbar_clicked_cb, ad);
	item[3] = elm_toolbar_item_append(obj, NULL, NULL, NULL, NULL);
	elm_object_item_disabled_set(item[3], EINA_TRUE);
	item[4] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_artists.png", "Artists list", _toolbar_clicked_cb, ad);
	return obj;
}

/*
static Evas_Object *create_segment_items_toolbar(struct appdata *ad)
{
	Evas_Object *obj, *segmentedBtn;
	Elm_Object_Item *item[10];

	// create toolbar //
	obj = elm_toolbar_add(ad->nf);
	if(obj == NULL) return NULL;
	elm_object_style_set(obj, "default");
	elm_toolbar_shrink_mode_set(obj, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_transverse_expanded_set(obj, EINA_TRUE);

	segmentedBtn = elm_button_add(obj);
	elm_object_style_set(segmentedBtn, "style1");
	elm_object_text_set(segmentedBtn, _("Toolbar Segmented"));
	evas_object_smart_callback_add(segmentedBtn, "clicked", _create_ctxpopup_toolbar_segmentedControl, ad);

	item[0] = elm_toolbar_item_append(obj, NULL, NULL, NULL, NULL);
	elm_object_item_part_content_set(item[0], "object", segmentedBtn);

	return obj;
}*/

void tool_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad;
	Evas_Object *sub_view;
	Evas_Object *toolbar;
	Elm_Object_Item *navi_it;

	ad = (struct appdata *) data;
	if(ad == NULL) return;

	sub_view = _create_list_winset(obj, menu_main, ad);
	navi_it = elm_naviframe_item_push(ad->nf, _("Toolbar"), NULL, NULL, sub_view, NULL);
	toolbar = create_3_items_toolbar(ad);
	elm_object_item_part_content_set(navi_it, "toolbar", toolbar);
}
