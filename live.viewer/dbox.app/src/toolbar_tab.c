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

Evas_Object *ly;
static Ecore_Timer *timer;
#define ELM_MAX(v1, v2)    (((v1) > (v2)) ? (v1) : (v2))

static void _2_items_style_cb(void *data, Evas_Object *obj, void *event_info);
static void _3_items_style_cb(void *data, Evas_Object *obj, void *event_info);
static void _scrollable_style_cb(void *data, Evas_Object *obj, void *event_info);
//static void _more_items_style_cb(void *data, Evas_Object *obj, void *event_info);
static void _2_items_with_title_style_cb(void *data, Evas_Object *obj, void *event_info);
static void _3_items_with_title_style_cb(void *data, Evas_Object *obj, void *event_info);
static void _4_items_with_title_style_cb(void *data, Evas_Object *obj, void *event_info);
//static void _icon_only_style_cb(void *data, Evas_Object *obj, void *event_info) ;

static Evas_Object* _create_list_winset(Evas_Object* parent, struct _menu_item *menu, struct appdata* ad);

static Evas_Object *create_2_items_tabbar(struct appdata *ad);
static Evas_Object *create_3_items_tabbar(struct appdata *ad);
static Evas_Object *create_scrollable_tabbar(struct appdata *ad);
//static Evas_Object *create_more_items_tabbar(struct appdata *ad);
static Evas_Object *create_text_only_2_items_tabbar(struct appdata *ad);
static Evas_Object *create_text_only_3_items_tabbar(struct appdata *ad);
static Evas_Object *create_text_only_4_items_tabbar(struct appdata *ad);
//static Evas_Object *create_icon_only_tabbar(struct appdata *ad);

static struct _menu_item menu_main[] = {
	{ "2 Items Style", _2_items_style_cb},
	{ "3 Items Style", _3_items_style_cb},
	{ "Scrollable Style", _scrollable_style_cb},
//	{ "More Items Style", _more_items_style_cb},
	{ "2 Items With Title Style", _2_items_with_title_style_cb},
	{ "3 Items With Title Style", _3_items_with_title_style_cb},
	{ "4 Items With Title Style", _4_items_with_title_style_cb},
//	{ "Icon Only Style", _icon_only_style_cb},
	/* do not delete below */
	{ NULL, NULL }
};

static struct _menu_item menu_first[] = {
	{ "Music", NULL},
	{ "Message", NULL},
	{ "Call", NULL},
	{ "Memo", NULL},
	{ "Alarm", NULL},
	{ "SNS", NULL},
	{ "Email", NULL},
	{ "Worldclock", NULL},
	{ "Converter", NULL},
	{ "Stopwatch", NULL},
	{ "Calculator", NULL},
	/* do not delete below */
	{ NULL, NULL }
};

static struct _menu_item menu_second[] = {
	{ "Tab Bar", NULL},
	{ "Navigation Bar", NULL},
	{ "Slider", NULL},
	{ "Genlist", NULL},
	{ "Tool Bar", NULL},
	{ "Color Picker", NULL},
	/* do not delete below */
	{ NULL, NULL }
};

static struct _menu_item menu_third[] = {
	{ "Eina", NULL},
	{ "Evas", NULL},
	{ "Ecore", NULL},
	{ "Elementary", NULL},
	{ "Edje", NULL},
	{ "Eet", NULL},
	/* do not delete below */
	{ NULL, NULL }
};

static struct _menu_item menu_fourth[] = {
	{ "Aquila", NULL},
	{ "Pantheon", NULL},
	{ "Kessler", NULL},
	{ "Neptune", NULL},
	{ "Aqua", NULL},
	/* do not delete below */
	{ NULL, NULL }
};

static void _2_items_style_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *tabbar;
	Elm_Object_Item *navi_it;
	struct appdata *ad;
	ad = (struct appdata *) data;
	if(ad == NULL) return;

	if (ly)
	{
		tabbar = elm_object_part_content_unset(ly, "elm.swallow.tabbar");
		if (tabbar) evas_object_del(tabbar);
		tabbar = create_2_items_tabbar(ad);
		elm_object_part_content_set(ly, "elm.swallow.tabbar", tabbar);
		navi_it = elm_naviframe_top_item_get(ad->nf);
	} else
	{
		ly = elm_layout_add(ad->nf);
		elm_layout_theme_set(ly, "layout", "tabbar", "default");
		tabbar = create_2_items_tabbar(ad);
		elm_object_part_content_set(ly, "elm.swallow.tabbar", tabbar);
		navi_it = elm_naviframe_top_item_get(ad->nf);
		elm_object_item_part_content_set(navi_it, NULL, ly);
	}
	elm_naviframe_item_title_enabled_set(navi_it, EINA_FALSE, EINA_FALSE);
}

static void _3_items_style_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *tabbar;
	Elm_Object_Item *navi_it;
	struct appdata *ad;
	ad = (struct appdata *) data;
	if(ad == NULL) return;

	if (ly)
	{
		tabbar = elm_object_part_content_unset(ly, "elm.swallow.tabbar");
		if (tabbar) evas_object_del(tabbar);
		tabbar = create_3_items_tabbar(ad);
		elm_object_part_content_set(ly, "elm.swallow.tabbar", tabbar);
		navi_it = elm_naviframe_top_item_get(ad->nf);
	} else
	{
		ly = elm_layout_add(ad->nf);
		elm_layout_theme_set(ly, "layout", "tabbar", "default");
		tabbar = create_3_items_tabbar(ad);
		elm_object_part_content_set(ly, "elm.swallow.tabbar", tabbar);
		navi_it = elm_naviframe_top_item_get(ad->nf);
		elm_object_item_part_content_set(navi_it, NULL, ly);
	}
	elm_naviframe_item_title_enabled_set(navi_it, EINA_FALSE, EINA_FALSE);
}

static void _scrollable_style_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *tabbar;
	Elm_Object_Item *navi_it;
	struct appdata *ad;
	ad = (struct appdata *) data;
	if(ad == NULL) return;

	if (ly)
	{
		tabbar = elm_object_part_content_unset(ly, "elm.swallow.tabbar");
		if (tabbar) evas_object_del(tabbar);
		tabbar = create_scrollable_tabbar(ad);
		elm_object_part_content_set(ly, "elm.swallow.tabbar", tabbar);
		navi_it = elm_naviframe_top_item_get(ad->nf);
	} else
	{
		ly = elm_layout_add(ad->nf);
		elm_layout_theme_set(ly, "layout", "tabbar", "default");
		tabbar = create_scrollable_tabbar(ad);
		elm_object_part_content_set(ly, "elm.swallow.tabbar", tabbar);
		navi_it = elm_naviframe_top_item_get(ad->nf);
		elm_object_item_part_content_set(navi_it, NULL, ly);
	}
	elm_naviframe_item_title_enabled_set(navi_it, EINA_FALSE, EINA_FALSE);
}

/*static void _more_items_style_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *tabbar;
	struct appdata *ad;
	ad = (struct appdata *) data;
	if(ad == NULL) return;

	tabbar = elm_object_part_content_unset(ly, "elm.swallow.tabbar");
	if (tabbar) evas_object_del(tabbar);
	tabbar = create_more_items_tabbar(ad);
	elm_object_part_content_set(ly, "elm.swallow.tabbar", tabbar);
}*/

static void _2_items_with_title_style_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *tabbar;
	Elm_Object_Item *navi_it;
	struct appdata *ad;
	ad = (struct appdata *) data;
	if(ad == NULL) return;

	if (ly)
	{
		tabbar = elm_object_part_content_unset(ly, "elm.swallow.tabbar");
		if (tabbar) evas_object_del(tabbar);
		evas_object_del(ly);
		ly = NULL;
	}

	navi_it = elm_naviframe_top_item_get(ad->nf);
	elm_object_item_part_text_set(navi_it, NULL, "2 Items with Title");

	tabbar = create_text_only_2_items_tabbar(ad);
	elm_object_item_part_content_set(navi_it, "tabbar", tabbar);

	elm_naviframe_item_title_enabled_set(navi_it, EINA_TRUE, EINA_FALSE);
}

static void _3_items_with_title_style_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *tabbar;
	Elm_Object_Item *navi_it;
	struct appdata *ad;
	ad = (struct appdata *) data;
	if(ad == NULL) return;

	if (ly)
	{
		tabbar = elm_object_part_content_unset(ly, "elm.swallow.tabbar");
		if(tabbar) evas_object_del(tabbar);
		evas_object_del(ly);
		ly = NULL;
	}

	navi_it = elm_naviframe_top_item_get(ad->nf);
	elm_object_item_part_text_set(navi_it, NULL, "3 Items with Title");

	tabbar = create_text_only_3_items_tabbar(ad);
	elm_object_item_part_content_set(navi_it, "tabbar", tabbar);

	elm_naviframe_item_title_enabled_set(navi_it, EINA_TRUE, EINA_FALSE);
}

static void _4_items_with_title_style_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *tabbar;
	Elm_Object_Item *navi_it;
	struct appdata *ad;
	ad = (struct appdata *) data;
	if(ad == NULL) return;

	if (ly)
	{
		tabbar = elm_object_part_content_unset(ly, "elm.swallow.tabbar");
		if (tabbar) evas_object_del(tabbar);
		evas_object_del(ly);
		ly = NULL;
	}

	navi_it = elm_naviframe_top_item_get(ad->nf);
	elm_object_item_part_text_set(navi_it, NULL, "4 Items with Title");

	tabbar = create_text_only_4_items_tabbar(ad);
	elm_object_item_part_content_set(navi_it, "tabbar", tabbar);

	elm_naviframe_item_title_enabled_set(navi_it, EINA_TRUE, EINA_FALSE);
}

/*static void _icon_only_style_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *tabbar;
	struct appdata *ad;
	ad = (struct appdata *) data;
	if(ad == NULL) return;

	tabbar = elm_object_part_content_unset(ly, "elm.swallow.tabbar");
	if (tabbar) evas_object_del(tabbar);
	tabbar = create_icon_only_tabbar(ad);
	elm_object_part_content_set(ly, "elm.swallow.tabbar", tabbar);
}*/

static void _list_click(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *it = (Elm_Object_Item *) elm_list_selected_item_get(obj);
	if(it == NULL) return;

	if (timer && strcmp(elm_object_item_text_get(it), "Scrollable Style"))
	{
		ecore_timer_del(timer);
		timer = NULL;
	}

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

static void view_change_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	Evas_Object *sub_view;
	Elm_Object_Item *it;
	const char *str = NULL;

	it = elm_toolbar_selected_item_get(obj);
	if (it)
		str = elm_object_item_text_get(it);

	sub_view = elm_object_part_content_unset(ly, "elm.swallow.content");
	evas_object_del(sub_view);

	if (!str || !strcmp(str, "Main"))
		sub_view = _create_list_winset(ad->layout_main, menu_main, ad);
	else if (!strcmp(str, "Playlist"))
		sub_view = _create_list_winset(ad->layout_main, menu_first, ad);
	else if (!strcmp(str, "Artists list"))
		sub_view = _create_list_winset(ad->layout_main, menu_second, ad);
	else if (!strcmp(str, "Songs"))
		sub_view = _create_list_winset(ad->layout_main, menu_third, ad);
	else if (!strcmp(str, "Dialer for call"))
		sub_view = _create_list_winset(ad->layout_main, menu_fourth, ad);
	else
		sub_view = _create_list_winset(ad->layout_main, menu_main, ad);

	elm_object_part_content_set(ly, "elm.swallow.content", sub_view);
	evas_object_show(sub_view);
}

static void view_change_cb2(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	Evas_Object *sub_view;
	Elm_Object_Item *it, *navi_it;
	const char *str = NULL;

	it = elm_toolbar_selected_item_get(obj);
	if (it)
		str = elm_object_item_text_get(it);

	if (!str || !strcmp(str, "Main"))
		sub_view = _create_list_winset(ad->layout_main, menu_main, ad);
	else if (!strcmp(str, "Playlist"))
		sub_view = _create_list_winset(ad->layout_main, menu_first, ad);
	else if (!strcmp(str, "Artists list"))
		sub_view = _create_list_winset(ad->layout_main, menu_second, ad);
	else if (!strcmp(str, "Songs"))
		sub_view = _create_list_winset(ad->layout_main, menu_third, ad);
	else if (!strcmp(str, "Dialer for call"))
		sub_view = _create_list_winset(ad->layout_main, menu_fourth, ad);
	else
		sub_view = _create_list_winset(ad->layout_main, menu_main, ad);

	navi_it = elm_naviframe_top_item_get(ad->nf);
	elm_object_item_part_content_set(navi_it, NULL, sub_view);
	evas_object_show(sub_view);
}
/*static void _toolbar_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *it = elm_toolbar_more_item_get(data);
	if (!strcmp(elm_object_item_text_get(it), "Open") && (it == elm_toolbar_selected_item_get(data)))
	{
		elm_toolbar_item_icon_set(it, "arrow_up");
		elm_object_item_text_set(it, "Close");
	}
	else if (!strcmp(elm_object_item_text_get(it), "Close"))
	{
		elm_toolbar_item_icon_set(it, "arrow_down");
		elm_object_item_text_set(it, "Open");
	}
}*/

static int _rotate_toolbar_cb(enum appcore_rm rotmode, void *data)
{
	switch (rotmode) {
		case APPCORE_RM_PORTRAIT_NORMAL:

		case APPCORE_RM_PORTRAIT_REVERSE:

		case APPCORE_RM_UNKNOWN:
			elm_object_signal_emit(ly, "elm,state,icon_text,tabbar", "elm");
			elm_object_style_set((Evas_Object *)data, "tabbar");
			break;

		case APPCORE_RM_LANDSCAPE_NORMAL:

		case APPCORE_RM_LANDSCAPE_REVERSE:
			elm_object_signal_emit(ly, "elm,state,horizontal_icon_text,tabbar", "elm");
			elm_object_style_set((Evas_Object *)data, "tabbar/item_horizontal");
			break;
	}
	return 0;
}

static Eina_Bool item_show_last(void *data)
{
	elm_toolbar_item_show(data, ELM_TOOLBAR_ITEM_SCROLLTO_LAST);

	return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool item_bring_in_first(void *data)
{
	timer = NULL;
	elm_toolbar_item_bring_in(data, ELM_TOOLBAR_ITEM_SCROLLTO_FIRST);

	return ECORE_CALLBACK_CANCEL;
}

static Evas_Object *create_2_items_tabbar(struct appdata *ad)
{
	Evas_Object *obj;
	Elm_Object_Item *item[3];

	/* create toolbar */
	obj = elm_toolbar_add(ad->nf);
	if(obj == NULL) return NULL;
	elm_toolbar_shrink_mode_set(obj, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_reorder_mode_set(obj, EINA_TRUE);
	elm_toolbar_transverse_expanded_set(obj, EINA_TRUE);

	elm_object_signal_emit(ly, "elm,state,icon_text,tabbar", "elm");
	elm_object_style_set(obj, "tabbar");

	item[0] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_favorites.png", "Main", view_change_cb, ad);
	item[1] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_artists.png", "Artists list", view_change_cb, ad);
	elm_object_item_part_text_set(item[1], "elm.text.badge", "24");

	elm_toolbar_select_mode_set(obj, ELM_OBJECT_SELECT_MODE_ALWAYS);
	elm_toolbar_item_selected_set(item[0], EINA_TRUE);

	return obj;
}

static Evas_Object *create_3_items_tabbar(struct appdata *ad)
{
	Evas_Object *obj;
	Elm_Object_Item *item[3];

	/* create toolbar */
	obj = elm_toolbar_add(ad->nf);
	if(obj == NULL) return NULL;
	elm_toolbar_shrink_mode_set(obj, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_reorder_mode_set(obj, EINA_TRUE);
	elm_toolbar_transverse_expanded_set(obj, EINA_TRUE);

	elm_object_signal_emit(ly, "elm,state,icon_text,tabbar", "elm");
	elm_object_style_set(obj, "tabbar");

	item[0] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_favorites.png", "Main", view_change_cb, ad);
	item[1] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_playlist.png", "Playlist", view_change_cb, ad);
	item[2] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_artists.png", "Artists list", view_change_cb, ad);
	elm_object_item_part_text_set(item[2], "elm.text.badge", "7");

	elm_toolbar_select_mode_set(obj, ELM_OBJECT_SELECT_MODE_ALWAYS);
	elm_toolbar_item_selected_set(item[0], EINA_TRUE);

	return obj;
}

static Evas_Object *create_scrollable_tabbar(struct appdata *ad)
{
	Evas_Object *obj;
	Elm_Object_Item *item[8];

	/* create toolbar */
	obj = elm_toolbar_add(ad->nf);
	if(obj == NULL) return NULL;
	elm_toolbar_shrink_mode_set(obj, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_reorder_mode_set(obj, EINA_TRUE);
	elm_toolbar_transverse_expanded_set(obj, EINA_TRUE);

	elm_object_signal_emit(ly, "elm,state,icon_text,tabbar", "elm");
	elm_object_style_set(obj, "tabbar");

	item[0] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_favorites.png", "Main", view_change_cb, ad);
	item[1] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_playlist.png", "Playlist", view_change_cb, ad);
	elm_object_item_part_text_set(item[1], "elm.text.badge", "99");
	item[2] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_artists.png", "Artists list", view_change_cb, ad);
	elm_object_item_disabled_set(item[2], EINA_TRUE);
	elm_object_item_part_text_set(item[2], "elm.text.badge", "8");
	item[3] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_songs.png", "Songs", view_change_cb, ad);
	item[4] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_dialer.png", "Dialer for call", view_change_cb, ad);
	item[5] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_playlist.png", "Playlist", view_change_cb, ad);
	elm_object_item_part_text_set(item[5], "elm.text.badge", "999");
	item[6] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_artists.png", "Artists list", view_change_cb, ad);
	item[7] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_songs.png", "Songs", view_change_cb, ad);
	elm_object_item_part_text_set(item[7], "elm.text.badge", "9999");

	elm_toolbar_select_mode_set(obj, ELM_OBJECT_SELECT_MODE_ALWAYS);
	elm_toolbar_item_selected_set(item[0], EINA_TRUE);

	ecore_idler_add(item_show_last, item[7]);
	timer = ecore_timer_add(0.5, item_bring_in_first, item[0]);

	set_rotate_cb_for_winset(_rotate_toolbar_cb, obj);

	return obj;
}

/*static Evas_Object *create_more_items_tabbar(struct appdata *ad)
{
	Evas_Object *obj;
	Elm_Object_Item *item[14];

	// create toolbar //
	obj = elm_toolbar_add(ad->nf);
	if(obj == NULL) return NULL;
	elm_toolbar_shrink_mode_set(obj, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_reorder_mode_set(obj, EINA_TRUE);
	elm_toolbar_transverse_expanded_set(obj, EINA_TRUE);

	elm_toolbar_standard_priority_set(obj, 1);

	elm_object_signal_emit(ly, "elm,state,icon_text,tabbar", "elm");
	elm_object_style_set(obj, "tabbar");

	item[0] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_favorites.png", "Main", view_change_cb, ad);
	elm_toolbar_item_priority_set(item[0], 100);
	item[1] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_playlist.png", "Playlist", view_change_cb, ad);
	elm_toolbar_item_priority_set(item[1], 100);
	item[2] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_artists.png", "Artists list", view_change_cb, ad);
	elm_toolbar_item_priority_set(item[2], 100);
	item[3] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_songs.png", "Songs", view_change_cb, ad);
	elm_toolbar_item_priority_set(item[3], 100);
	item[4] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_dialer.png", "Dialer for call", view_change_cb, ad);

	item[5] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_favorites.png", "Main", view_change_cb, ad);
	item[6] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_playlist.png", "Playlist", view_change_cb, ad);
	item[7] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_artists.png", "Artists list", view_change_cb, ad);
	item[8] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_songs.png", "Songs", view_change_cb, ad);
	item[9] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_dialer.png", "Dialer for call", view_change_cb, ad);

	item[10] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_playlist.png", "Playlist", view_change_cb, ad);
	item[11] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_artists.png", "Artists list", view_change_cb, ad);
	item[12] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_songs.png", "Songs", view_change_cb, ad);
	item[13] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_dialer.png", "Dialer for call", view_change_cb, ad);

	elm_toolbar_select_mode_set(obj, ELM_OBJECT_SELECT_MODE_ALWAYS);
	elm_toolbar_item_selected_set(item[0], EINA_TRUE);

	elm_toolbar_item_icon_set(elm_toolbar_more_item_get(obj), "arrow_down");
	elm_object_item_text_set(elm_toolbar_more_item_get(obj), "Open");

	evas_object_smart_callback_add(obj, "clicked", _toolbar_clicked_cb, obj);

	return obj;
}*/

static Evas_Object *create_text_only_2_items_tabbar(struct appdata *ad)
{
	Evas_Object *obj;
	Elm_Object_Item *item[5];

	/* create toolbar */
	obj = elm_toolbar_add(ad->nf);
	if(obj == NULL) return NULL;
	elm_toolbar_shrink_mode_set(obj, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_reorder_mode_set(obj, EINA_TRUE);
	elm_toolbar_transverse_expanded_set(obj, EINA_TRUE);
	elm_object_style_set(obj, "tabbar/item_with_title");

	item[0] = elm_toolbar_item_append(obj, NULL, "Main", view_change_cb2, ad);
	item[1] = elm_toolbar_item_append(obj, NULL, "Playlist", view_change_cb2, ad);

	elm_toolbar_select_mode_set(obj, ELM_OBJECT_SELECT_MODE_ALWAYS);
	elm_toolbar_item_selected_set(item[0], EINA_TRUE);

	return obj;
}

static Evas_Object *create_text_only_3_items_tabbar(struct appdata *ad)
{
	Evas_Object *obj;
	Elm_Object_Item *item[5];

	/* create toolbar */
	obj = elm_toolbar_add(ad->nf);
	if(obj == NULL) return NULL;
	elm_toolbar_shrink_mode_set(obj, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_reorder_mode_set(obj, EINA_TRUE);
	elm_toolbar_transverse_expanded_set(obj, EINA_TRUE);
	elm_object_style_set(obj, "tabbar/item_with_title");

	item[0] = elm_toolbar_item_append(obj, NULL, "Main", view_change_cb2, ad);
	item[1] = elm_toolbar_item_append(obj, NULL, "Playlist", view_change_cb2, ad);
	item[2] = elm_toolbar_item_append(obj, NULL, "Artists list", view_change_cb2, ad);

	elm_toolbar_select_mode_set(obj, ELM_OBJECT_SELECT_MODE_ALWAYS);
	elm_toolbar_item_selected_set(item[0], EINA_TRUE);

	return obj;
}

static Evas_Object *create_text_only_4_items_tabbar(struct appdata *ad)
{
	Evas_Object *obj;
	Elm_Object_Item *item[5];

	/* create toolbar */
	obj = elm_toolbar_add(ad->nf);
	if(obj == NULL) return NULL;
	elm_toolbar_shrink_mode_set(obj, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_reorder_mode_set(obj, EINA_TRUE);
	elm_toolbar_transverse_expanded_set(obj, EINA_TRUE);
	elm_object_style_set(obj, "tabbar/item_with_title");

	item[0] = elm_toolbar_item_append(obj, NULL, "Main", view_change_cb2, ad);
	item[1] = elm_toolbar_item_append(obj, NULL, "Playlist", view_change_cb2, ad);
	item[2] = elm_toolbar_item_append(obj, NULL, "Artists list", view_change_cb2, ad);
	item[3] = elm_toolbar_item_append(obj, NULL, "Composers", view_change_cb2, ad);

	elm_toolbar_select_mode_set(obj, ELM_OBJECT_SELECT_MODE_ALWAYS);
	elm_toolbar_item_selected_set(item[0], EINA_TRUE);

	return obj;
}

/*static Evas_Object *create_icon_only_tabbar(struct appdata *ad)
{
	Evas_Object *obj;
	Elm_Object_Item *item[5];

	// create toolbar //
	obj = elm_toolbar_add(ad->nf);
	if(obj == NULL) return NULL;
	elm_toolbar_shrink_mode_set(obj, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_reorder_mode_set(obj, EINA_TRUE);
	elm_toolbar_transverse_expanded_set(obj, EINA_TRUE);

	elm_object_signal_emit(ly, "elm,state,default,tabbar", "elm");
	elm_object_style_set(obj, "tabbar");

	item[0] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_favorites.png", NULL, view_change_cb, ad);
	item[1] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_playlist.png", NULL, view_change_cb, ad);
	item[2] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_artists.png", NULL, view_change_cb, ad);
	item[3] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_songs.png", NULL, view_change_cb, ad);
	item[4] = elm_toolbar_item_append(obj, ICON_DIR"/00_controlbar_icon_dialer.png", NULL, view_change_cb, ad);
	elm_object_item_part_text_set(item[4], "elm.text.badge", "999");

	elm_toolbar_select_mode_set(obj, ELM_OBJECT_SELECT_MODE_ALWAYS);
	elm_toolbar_item_selected_set(item[0], EINA_TRUE);

	return obj;
}*/

void tab_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad;
	Evas_Object *tabbar;
	Elm_Object_Item *nf_it;

	ad = (struct appdata *) data;
	if(ad == NULL) return;

	ly = elm_layout_add(ad->nf);
	elm_layout_theme_set(ly, "layout", "tabbar", "default");

	tabbar = create_scrollable_tabbar(ad);
	elm_object_part_content_set(ly, "elm.swallow.tabbar", tabbar);
	nf_it = elm_naviframe_item_push(ad->nf, _("Tabbar"), NULL, NULL, ly, "tabbar");
	elm_naviframe_item_title_enabled_set(nf_it, EINA_FALSE, EINA_FALSE);
}
