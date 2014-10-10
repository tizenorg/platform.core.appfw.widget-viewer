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
#include "ctxpopup.h"

/*********************************************************
 contextpopup
 ********************************************************/
static Evas_Coord touch_x;
static Evas_Coord touch_y;
static Evas_Object* ctxpopup = NULL;
struct appdata *ad;
typedef struct _Ctxpopup_Data
{
	Evas_Object *ctx;
	Evas_Object *btn;
} Ctxpopup_Data;
Ctxpopup_Data ctx_data;


static Eina_Bool _pop_cb(void *data, Elm_Object_Item *it)
{
	if(ctxpopup) {
		evas_object_del(ctxpopup);
		ctxpopup = NULL;
	}

	elm_theme_extension_del(NULL, ELM_DEMO_EDJ);
	ecore_event_handler_del(data);

	return EINA_TRUE;
}

static void _move_ctxpopup(Evas_Object *ctxpopup, Evas_Object *btn)
{
   Evas_Coord x, y, w , h;
   evas_object_geometry_get(btn, &x, &y, &w, &h);
   evas_object_move(ctxpopup, x + (w / 2), y + (h /2));
}

static void _move_more_ctxpopup(Evas_Object *ctx)
{
	Evas_Coord w, h;
	int pos = -1;

	elm_win_screen_size_get(ad->win_main, NULL, NULL, &w, &h);
	pos = elm_win_rotation_get(ad->win_main);

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

static void _dismissed_cb(void *data, Evas_Object *obj, void *event_info)
{
	evas_object_smart_callback_del(ctxpopup,"dismissed", _dismissed_cb);
	evas_object_del(ctxpopup);
	ctxpopup = NULL;
}

static void _dismissed_more_ctxpopup_cb(void *data, Evas_Object *obj, void *event_info)
{
	evas_object_del(obj);
	obj = NULL;
}

static void _resize_more_ctxpopup_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	_move_more_ctxpopup(data);
}

static void _rotate_more_ctxpopup_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *ctx = (Evas_Object *)data;
	_move_more_ctxpopup(ctx);
}

static void _delete_more_ctxpopup_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Object *navi = (Evas_Object *)data;
	Evas_Object *ctx = obj;

	evas_object_event_callback_del_full(navi, EVAS_CALLBACK_RESIZE, _resize_more_ctxpopup_cb, ctx);
	evas_object_smart_callback_del_full(elm_object_top_widget_get(ctx), "rotation,changed", _rotate_more_ctxpopup_cb, ctx);
	evas_object_smart_callback_del(ctx, "dismissed", _dismissed_more_ctxpopup_cb);
	evas_object_event_callback_del_full(ctx, EVAS_CALLBACK_DEL, _delete_more_ctxpopup_cb, navi);
}

static void _dismissed_rotate_ctxpopup_cb(void *data, Evas_Object *obj, void *event_info)
{
	evas_object_del(ctx_data.ctx);
	ctx_data.ctx = NULL;
	ctxpopup = NULL;
}
static void _resize_rotate_ctxpopup_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	_move_ctxpopup(ctx_data.ctx, ctx_data.btn);
}

static void _rotate_ctxpopup_cb(void *data, Evas_Object *obj, void *event_info)
{
	_move_ctxpopup(ctx_data.ctx, ctx_data.btn);
}

static void _delete_rotate_ctxpopup_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Object *navi = (Evas_Object *)data;
	Evas_Object *ctx = ctx_data.ctx;

	evas_object_event_callback_del(navi, EVAS_CALLBACK_RESIZE, _resize_rotate_ctxpopup_cb);
	evas_object_smart_callback_del(elm_object_top_widget_get(ctx), "rotation,changed", _rotate_ctxpopup_cb);
	evas_object_smart_callback_del(ctx, "dismissed", _dismissed_rotate_ctxpopup_cb);
	ea_object_event_callback_del(ctxpopup, EA_CALLBACK_BACK, ea_ctxpopup_back_cb);
	ea_object_event_callback_del(ctxpopup, EA_CALLBACK_MORE, ea_ctxpopup_back_cb);
	evas_object_event_callback_del_full(ctx, EVAS_CALLBACK_DEL, _delete_rotate_ctxpopup_cb, navi);
}

static void _on_move(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Coord x, y, w, h;
	evas_object_geometry_get(obj, &x, &y, &w, &h);
	printf("[%s : %d] obj=%p x=%d y=%d w=%d h=%d\n", __func__, __LINE__, obj, x, y, w,h);
}

static void _ctxpopup_cb(void *data, Evas_Object *obj, void *event_info)
{
	const char *label = elm_object_item_text_get((Elm_Object_Item *) event_info);
	if(label) fprintf(stderr, "text(%s) is clicked\n", label);

	Evas_Object *icon = elm_object_item_content_get((Elm_Object_Item *) event_info);
	if(icon) fprintf(stderr, "icon is clicked\n");
}

/*********************************************************
Text Only
 ********************************************************/
static void _create_ctxpopup_text_only(void *data, Evas_Object *obj, void *event_info)
{
	if(ctxpopup) {
		evas_object_del(ctxpopup);
		ctxpopup = NULL;
	}

	ctxpopup = elm_ctxpopup_add(ad->nf);
	ea_object_event_callback_add(ctxpopup, EA_CALLBACK_BACK, ea_ctxpopup_back_cb, NULL);
	evas_object_smart_callback_add(ctxpopup,"dismissed", _dismissed_cb, NULL);

	elm_ctxpopup_item_append(ctxpopup, _("Message"), NULL, _ctxpopup_cb, ctxpopup);
	elm_ctxpopup_item_append(ctxpopup, _("Email"), NULL, _ctxpopup_cb, ctxpopup);
	elm_ctxpopup_item_append(ctxpopup, _("Facebook"), NULL, _ctxpopup_cb, ctxpopup);
	elm_ctxpopup_item_append(ctxpopup, _("Bluetooth"), NULL, _ctxpopup_cb, ctxpopup);
	elm_ctxpopup_item_append(ctxpopup, _("Flickr"), NULL, _ctxpopup_cb, ctxpopup);

	_move_ctxpopup(ctxpopup, obj);
	evas_object_show(ctxpopup);
}
/*********************************************************
Text Only End
 ********************************************************/


/*********************************************************
3 Icons
 ********************************************************/
static void _create_ctxpopup_3icon( void *data, Evas_Object *obj, void *event_info )
{
	char buf[255];
	Evas_Object* icon;

	if(ctxpopup) {
		evas_object_del(ctxpopup);
		ctxpopup = NULL;
	}

	ctxpopup = elm_ctxpopup_add(ad->nf);
	ea_object_event_callback_add(ctxpopup, EA_CALLBACK_BACK, ea_ctxpopup_back_cb, NULL);
	evas_object_smart_callback_add(ctxpopup,"dismissed", _dismissed_cb, NULL);
	elm_ctxpopup_horizontal_set(ctxpopup, EINA_TRUE);

	//Icon 1
	icon = elm_image_add(ctxpopup);
	snprintf(buf, sizeof(buf), "%s/00_popup_icon_call.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	elm_ctxpopup_item_append(ctxpopup, NULL, icon, _ctxpopup_cb, ctxpopup);

	//Icon 2
	icon = elm_image_add(ctxpopup);
	snprintf(buf, sizeof(buf), "%s/00_popup_icon_message.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	elm_ctxpopup_item_append(ctxpopup, NULL, icon, _ctxpopup_cb, ctxpopup);

	//Icon 3
	icon = elm_image_add(ctxpopup);
	snprintf(buf, sizeof(buf), "%s/00_popup_icon_video call.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	elm_ctxpopup_item_append(ctxpopup, NULL, icon, _ctxpopup_cb, ctxpopup);

	_move_ctxpopup(ctxpopup, obj);
	evas_object_show(ctxpopup);
}
/*********************************************************
3 Icons End
 ********************************************************/

/*********************************************************
Icon + Text
 ********************************************************/
static void _create_ctxpopup_icon_text(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object* icon;
	char buf[256];

	if(ctxpopup) {
		evas_object_del(ctxpopup);
		ctxpopup = NULL;
	}

	ctxpopup = elm_ctxpopup_add(ad->nf);
	ea_object_event_callback_add(ctxpopup, EA_CALLBACK_BACK, ea_ctxpopup_back_cb, NULL);
	evas_object_smart_callback_add(ctxpopup, "dismissed", _dismissed_cb, NULL);

	icon = elm_image_add(ctxpopup);
	snprintf(buf, sizeof(buf), "%s/icon1.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	elm_ctxpopup_item_append(ctxpopup, _("http://m.naver.com"), icon, _ctxpopup_cb, ctxpopup);

	icon = elm_image_add(ctxpopup);
	snprintf(buf, sizeof(buf), "%s/icon2.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	elm_ctxpopup_item_append(ctxpopup, _("http://m.telebox.net"), icon, _ctxpopup_cb, ctxpopup);

	icon = elm_image_add(ctxpopup);
	snprintf(buf, sizeof(buf), "%s/icon2.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	elm_ctxpopup_item_append(ctxpopup, _("http://m.samsung.com"), icon, _ctxpopup_cb, ctxpopup);

	icon = elm_image_add(ctxpopup);
	snprintf(buf, sizeof(buf), "%s/icon4.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	elm_ctxpopup_item_append(ctxpopup, _("http://m.yahoo.com"), icon, _ctxpopup_cb, ctxpopup);

	icon = elm_image_add(ctxpopup);
	snprintf(buf, sizeof(buf), "%s/icon3.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	elm_ctxpopup_item_append(ctxpopup, _("http://m.facebook.com"), icon, _ctxpopup_cb, ctxpopup);

	_move_ctxpopup(ctxpopup, obj);

	evas_object_show(ctxpopup);
}
/*********************************************************
Icon + Text End
 ********************************************************/
/*********************************************************
Ctxpop_on_roatation
 ********************************************************/

static void _create_ctxpopup_on_rotation(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object* icon;
	char buf[256];
	ctx_data.btn = obj;
	if(ctxpopup) {
		evas_object_del(ctxpopup);
		ctxpopup = NULL;
	}
	ctxpopup = elm_ctxpopup_add(ad->nf);
	ctx_data.ctx = ctxpopup;
	elm_ctxpopup_auto_hide_disabled_set(ctxpopup, EINA_TRUE);

	ea_object_event_callback_add(ctxpopup, EA_CALLBACK_BACK, ea_ctxpopup_back_cb, NULL);
	ea_object_event_callback_add(ctxpopup, EA_CALLBACK_MORE, ea_ctxpopup_back_cb, NULL);
	evas_object_smart_callback_add(ctxpopup, "dismissed", _dismissed_rotate_ctxpopup_cb, NULL);
	evas_object_event_callback_add(ctxpopup, EVAS_CALLBACK_DEL, _delete_rotate_ctxpopup_cb, ad->nf);
	evas_object_event_callback_add(ad->nf, EVAS_CALLBACK_RESIZE, _resize_rotate_ctxpopup_cb, NULL);
	evas_object_smart_callback_add(elm_object_top_widget_get(ctxpopup), "rotation,changed", _rotate_ctxpopup_cb, NULL);

	icon = elm_image_add(ctxpopup);
	snprintf(buf, sizeof(buf), "%s/icon1.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	elm_ctxpopup_item_append(ctxpopup, _("http://m.naver.com"), icon, _ctxpopup_cb, ctxpopup);

	icon = elm_image_add(ctxpopup);
	snprintf(buf, sizeof(buf), "%s/icon2.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	elm_ctxpopup_item_append(ctxpopup, _("http://m.telebox.net"), icon, _ctxpopup_cb, ctxpopup);

	icon = elm_image_add(ctxpopup);
	snprintf(buf, sizeof(buf), "%s/icon2.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	elm_ctxpopup_item_append(ctxpopup, _("http://m.samsung.com"), icon, _ctxpopup_cb, ctxpopup);

	icon = elm_image_add(ctxpopup);
	snprintf(buf, sizeof(buf), "%s/icon4.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	elm_ctxpopup_item_append(ctxpopup, _("http://m.yahoo.com"), icon, _ctxpopup_cb, ctxpopup);

	icon = elm_image_add(ctxpopup);
	snprintf(buf, sizeof(buf), "%s/icon3.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	elm_ctxpopup_item_append(ctxpopup, _("http://m.facebook.com"), icon, _ctxpopup_cb, ctxpopup);

	_move_ctxpopup(ctxpopup, obj);

	evas_object_show(ctxpopup);
}
/*********************************************************
Ctxpop_on_roatation_ends
 ********************************************************/
/*********************************************************
CopyPaste Sample
 ********************************************************/

const char *
_elm_theme_current_get(const char *theme_search_order)
{
   const char *ret;
   const char *p;

   if (!theme_search_order)
     return NULL;

   for (p = theme_search_order;; p++)
     {
        if ((*p == ':') || (!*p))
          {
             if (p > theme_search_order)
               {
                  char *n = malloc(p - theme_search_order + 1);
                  if (!n)
                    return NULL;

                  strncpy(n, theme_search_order, p - theme_search_order);
                  n[p - theme_search_order] = 0;
                  ret = eina_stringshare_add(n);
                  free(n);
                  break;
               }
          }
     }

   return ret;
}

static void _create_ctxpopup_copypaste_sample(void *data, Evas_Object *obj, void *event_info)
{
	char buf[255];
	char *theme;
	Evas_Object *icon;

	if(ctxpopup) {
		evas_object_del(ctxpopup);
		ctxpopup = NULL;
	}

	ctxpopup = elm_ctxpopup_add(ad->nf);
	ea_object_event_callback_add(ctxpopup, EA_CALLBACK_BACK, ea_ctxpopup_back_cb, NULL);
	evas_object_smart_callback_add(ctxpopup, "dismissed", _dismissed_cb, NULL);

	theme = elm_theme_list_item_path_get(_elm_theme_current_get(elm_theme_get(NULL)), NULL);
	icon = elm_image_add(ctxpopup);
	snprintf(buf, sizeof(buf), "elm/copypaste/select/default");
	elm_image_file_set(icon, theme, buf);
	elm_ctxpopup_item_append(ctxpopup, _("Select"), icon, _ctxpopup_cb, ctxpopup);

	icon = elm_image_add(ctxpopup);
	snprintf(buf, sizeof(buf), "elm/copypaste/select_all/default");
	elm_image_file_set(icon, theme, buf);
	elm_ctxpopup_item_append(ctxpopup, _("Select All"), icon, _ctxpopup_cb, ctxpopup);

	icon = elm_image_add(ctxpopup);
	snprintf(buf, sizeof(buf), "elm/copypaste/copy/default");
	elm_image_file_set(icon, theme, buf);
	elm_ctxpopup_item_append(ctxpopup, _("Copy"), icon, _ctxpopup_cb, ctxpopup);

	icon = elm_image_add(ctxpopup);
	snprintf(buf, sizeof(buf), "elm/copypaste/cut/default");
	elm_image_file_set(icon, theme, buf);
	elm_ctxpopup_item_append(ctxpopup, _("Cut"), icon, _ctxpopup_cb, ctxpopup);

	icon = elm_image_add(ctxpopup);
	snprintf(buf, sizeof(buf), "elm/copypaste/paste/default");
	elm_image_file_set(icon, theme, buf);
	elm_ctxpopup_item_append(ctxpopup, _("Paste"), icon, _ctxpopup_cb, ctxpopup);

	icon = elm_image_add(ctxpopup);
	snprintf(buf, sizeof(buf), "elm/copypaste/clipboard/default");
	elm_image_file_set(icon, theme, buf);
	elm_ctxpopup_item_append(ctxpopup, _("Clipboard"), icon, _ctxpopup_cb, ctxpopup);

	icon = elm_image_add(ctxpopup);
	snprintf(buf, sizeof(buf), "elm/copypaste/translate/default");
	elm_image_file_set(icon, theme, buf);
	elm_ctxpopup_item_append(ctxpopup, _("Translate"), icon, _ctxpopup_cb, ctxpopup);

	icon = elm_image_add(ctxpopup);
	snprintf(buf, sizeof(buf), "elm/copypaste/share/default");
	elm_image_file_set(icon, theme, buf);
	elm_ctxpopup_item_append(ctxpopup, _("Share"), icon, _ctxpopup_cb, ctxpopup);

	icon = elm_image_add(ctxpopup);
	snprintf(buf, sizeof(buf), "elm/copypaste/search/default");
	elm_image_file_set(icon, theme, buf);
	elm_ctxpopup_item_append(ctxpopup, _("Search"), icon, _ctxpopup_cb, ctxpopup);

	icon = elm_image_add(ctxpopup);
	snprintf(buf, sizeof(buf), "elm/copypaste/find/default");
	elm_image_file_set(icon, theme, buf);
	elm_ctxpopup_item_append(ctxpopup, _("Find"), icon, _ctxpopup_cb, ctxpopup);

	elm_ctxpopup_horizontal_set(ctxpopup, EINA_TRUE);
	elm_object_style_set(ctxpopup, "copypaste");
	_move_ctxpopup(ctxpopup, obj);

	evas_object_show(ctxpopup);
	if (theme) free(theme);
}
/*********************************************************
CopyPaste Sample End
 ********************************************************/

/*********************************************************
 Buttons Style
 ********************************************************/
static Evas_Object *_create_buttons(Evas_Object *parent)
{
	Evas_Object *table;
	Evas_Object *btn;
	Evas_Object *layout;

	layout = elm_layout_add(parent);
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "ctxpopup_buttons_nbeat");

	table = elm_table_add(layout);
	elm_table_padding_set(table, 10, 10);

	//cm
	btn = elm_button_add(table);
	elm_object_text_set(btn, "cm");
	evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(btn);
	elm_table_pack(table, btn, 0, 0, 1, 1);

	//m
	btn = elm_button_add(table);
	elm_object_text_set(btn, "m");
	evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(btn);
	elm_table_pack(table, btn, 1, 0, 1, 1);

	//km
	btn = elm_button_add(table);
	elm_object_text_set(btn, "km");
	evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(btn);
	elm_table_pack(table, btn, 2, 0, 1, 1);

	//inches
	btn = elm_button_add(table);
	elm_object_text_set(btn, "inches");
	evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(btn);
	elm_table_pack(table, btn, 0, 1, 1, 1);

	//gallons
	btn = elm_button_add(table);
	elm_object_text_set(btn, "gallons(us)");
	evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(btn);
	elm_table_pack(table, btn, 1, 1, 1, 1);

	//inches
	btn = elm_button_add(table);
	elm_object_text_set(btn, "inches³");
	evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(btn);
	elm_table_pack(table, btn, 2, 1, 1, 1);

	//inches
	btn = elm_button_add(table);
	elm_object_text_set(btn, "hectares");
	evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(btn);
	elm_table_pack(table, btn, 0, 2, 1, 1);

	//gallons
	btn = elm_button_add(table);
	elm_object_text_set(btn, "yards");
	evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(btn);
	elm_table_pack(table, btn, 1, 2, 1, 1);

	//inches
	btn = elm_button_add(table);
	elm_object_text_set(btn, "miles");
	evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(btn);
	elm_table_pack(table, btn, 2, 2, 1, 1);

	elm_object_part_content_set(layout, "elm.swallow.table", table);

	return layout;

}

static void _create_ctxpopup_buttons_style(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *buttons;

	if(ctxpopup) {
		evas_object_del(ctxpopup);
		ctxpopup = NULL;
	}

	ctxpopup = elm_ctxpopup_add(ad->nf);
	ea_object_event_callback_add(ctxpopup, EA_CALLBACK_BACK, ea_ctxpopup_back_cb, NULL);
	evas_object_smart_callback_add(ctxpopup, "dismissed", _dismissed_cb, NULL);

	buttons = _create_buttons(ad->nf);
	evas_object_show(buttons);
	elm_object_content_set(ctxpopup, buttons);

	_move_ctxpopup(ctxpopup, obj);
	evas_object_show(ctxpopup);
}
/*********************************************************
 Buttons Style End
 ********************************************************/

/*********************************************************
Imageeditor adjust
 ********************************************************/
static void _create_ctxpopup_imageeditor_adjust_style(void *data, Evas_Object *obj, void *event_info)
{
	if(ctxpopup) {
		evas_object_del(ctxpopup);
		ctxpopup = NULL;
	}

	ctxpopup = elm_ctxpopup_add(ad->nf);
	ea_object_event_callback_add(ctxpopup, EA_CALLBACK_BACK, ea_ctxpopup_back_cb, NULL);
	elm_object_style_set(ctxpopup, "elm_demo_tizen/imageeditor");
	evas_object_smart_callback_add(ctxpopup, "dismissed", _dismissed_cb, NULL);

	elm_ctxpopup_item_append(ctxpopup, _("option ment text"), NULL,_ctxpopup_cb, ctxpopup);
	elm_ctxpopup_item_append(ctxpopup, _("option ment text"), NULL,_ctxpopup_cb, ctxpopup);
	elm_ctxpopup_item_append(ctxpopup, _("option ment text"), NULL,_ctxpopup_cb, ctxpopup);
	elm_ctxpopup_item_append(ctxpopup, _("option ment text text text"), NULL,_ctxpopup_cb, ctxpopup);
	elm_ctxpopup_item_append(ctxpopup, _("option ment text"), NULL,_ctxpopup_cb, ctxpopup);

	elm_ctxpopup_direction_priority_set(ctxpopup, ELM_CTXPOPUP_DIRECTION_LEFT,
															ELM_CTXPOPUP_DIRECTION_RIGHT,
															ELM_CTXPOPUP_DIRECTION_DOWN,
															ELM_CTXPOPUP_DIRECTION_UP);
	elm_object_scroll_freeze_push(ctxpopup);
	_move_ctxpopup(ctxpopup, obj);

	evas_object_show(ctxpopup);
}
/*********************************************************
Imageeditor adjust End
 ********************************************************/
/*********************************************************
Camera Shooting mode
 ********************************************************/
static void _camera_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	elm_radio_value_set(data, (int) evas_object_data_get(data, "idx"));
}

static Evas_Object *_create_camera_shooting_item(Evas_Object *parent, char *icon_path, char *label)
{
	Evas_Object *layout;
	Evas_Object *icon;

	//layout
	layout = elm_layout_add(parent);
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "camera/radio_icon_text_item");
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL,  EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND,  EVAS_HINT_EXPAND);
	evas_object_show(layout);

	//icon
	icon = elm_image_add(layout);
	elm_image_file_set(icon, icon_path, NULL);
	evas_object_show(icon);
	elm_object_part_content_set(layout, "elm.swallow.icon", icon);

	//text
	edje_object_part_text_set(elm_layout_edje_get(layout), "elm.text", label);

	return layout;
}

static Evas_Object *_create_camera_shooting_content(Evas_Object *parent)
{
	Evas_Object *layout;
	Evas_Object *box;
	Evas_Object *item;
	Evas_Object *radio;
	Evas_Object *group;
	Eina_List *l = NULL;
	Evas_Object *ao;

	//Layout
	layout = elm_layout_add(ctxpopup);
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "camera/layout");
	evas_object_show(layout);

	//Box
	box = elm_box_add(layout);

	//Items : Single
	item = _create_camera_shooting_item(box, ICON_DIR"/11_camera_shootingmode_single.png", "Single");
	ao = elm_access_object_register(item, box);
	elm_access_info_set(ao, ELM_ACCESS_INFO, "Single");
	l = eina_list_append(l, ao);

	//radio
	group = radio = elm_radio_add(item);
	elm_object_style_set(radio, "elm_demo_tizen/camera");
	elm_radio_state_value_set(radio, 0);
	evas_object_data_set(radio, "idx", (void *)(0));
	evas_object_show(radio);
	elm_object_part_content_set(item, "elm.swallow.radio", radio);
	evas_object_event_callback_add(item, EVAS_CALLBACK_MOUSE_DOWN, _camera_cb, radio);
	elm_box_pack_end(box, item);

	//Items : Beauty
	item = _create_camera_shooting_item(box, ICON_DIR"/11_camera_shootingmode_beauty.png", "Beauty");
	ao = elm_access_object_register(item, box);
	elm_access_info_set(ao, ELM_ACCESS_INFO, "Beauty");
	l = eina_list_append(l, ao);

	//radio
	radio = elm_radio_add(item);
	elm_object_style_set(radio, "elm_demo_tizen/camera");
	elm_radio_state_value_set(radio, 1);
	evas_object_data_set(radio, "idx", (void *)(1));
	elm_radio_group_add(radio, group);
	evas_object_show(radio);
	elm_object_part_content_set(item, "elm.swallow.radio", radio);
	evas_object_event_callback_add(item, EVAS_CALLBACK_MOUSE_DOWN, _camera_cb, radio);
	elm_box_pack_end(box, item);

	//Items : Smile shot
	item = _create_camera_shooting_item(box, ICON_DIR"/11_camera_shootingmode_smileshot.png", "Smile shot");
	ao = elm_access_object_register(item, box);
	elm_access_info_set(ao, ELM_ACCESS_INFO, "Smile Shot");
	l = eina_list_append(l, ao);

	//radio
	radio = elm_radio_add(item);
	elm_object_style_set(radio, "elm_demo_tizen/camera");
	elm_radio_state_value_set(radio, 2);
	evas_object_data_set(radio, "idx", (void *)(2));
	elm_radio_group_add(radio, group);
	evas_object_show(radio);
	elm_object_part_content_set(item, "elm.swallow.radio", radio);
	evas_object_event_callback_add(item, EVAS_CALLBACK_MOUSE_DOWN, _camera_cb, radio);
	elm_box_pack_end(box, item);

	//Items : Continuous
	item = _create_camera_shooting_item(box, ICON_DIR"/11_camera_shootingmode_continuous.png", "Continuous");
	ao = elm_access_object_register(item, box);
	elm_access_info_set(ao, ELM_ACCESS_INFO, "Continuous");
	l = eina_list_append(l, ao);

	//radio
	radio = elm_radio_add(item);
	elm_object_style_set(radio, "elm_demo_tizen/camera");
	elm_radio_state_value_set(radio, 3);
	evas_object_data_set(radio, "idx", (void *)(3));
	elm_radio_group_add(radio, group);
	evas_object_show(radio);
	elm_object_part_content_set(item, "elm.swallow.radio", radio);
	evas_object_event_callback_add(item, EVAS_CALLBACK_MOUSE_DOWN, _camera_cb, radio);
	elm_box_pack_end(box, item);

	//Items : Panorama
	item = _create_camera_shooting_item(box, ICON_DIR"/11_camera_shootingmode_panorama.png", "Panorama");
	ao = elm_access_object_register(item, box);
	elm_access_info_set(ao, ELM_ACCESS_INFO, "Panorama");
	l = eina_list_append(l, ao);

	//radio
	radio = elm_radio_add(item);
	elm_object_style_set(radio, "elm_demo_tizen/camera");
	elm_radio_state_value_set(radio, 4);
	evas_object_data_set(radio, "idx", (void *)(4));
	elm_radio_group_add(radio, group);
	evas_object_show(radio);
	elm_object_part_content_set(item, "elm.swallow.radio", radio);
	evas_object_event_callback_add(item, EVAS_CALLBACK_MOUSE_DOWN, _camera_cb, radio);
	elm_box_pack_end(box, item);

	elm_object_part_content_set(layout, "elm.swallow.box", box);

	elm_object_focus_custom_chain_set(box, l);

	return layout;
}

static void _create_ctxpopup_camera_shooting_style(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *content;

	if(ctxpopup) {
		evas_object_del(ctxpopup);
		ctxpopup = NULL;
	}

	ctxpopup = elm_ctxpopup_add(ad->nf);
	ea_object_event_callback_add(ctxpopup, EA_CALLBACK_BACK, ea_ctxpopup_back_cb, NULL);
	elm_object_style_set(ctxpopup, "elm_demo_tizen/camera");
	evas_object_smart_callback_add(ctxpopup, "dismissed", _dismissed_cb, NULL);

	content = _create_camera_shooting_content(ctxpopup);
	elm_object_content_set(ctxpopup, content);

	_move_ctxpopup(ctxpopup, obj);
	evas_object_show(ctxpopup);
}
/*********************************************************
Camera Shooting mode End
 ********************************************************/

/*********************************************************
Icon + Text (More button style)
 ********************************************************/
static void _create_ctxpopup_more_button_style(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object* icon;
	char buf[256];

	if(ctxpopup) {
		evas_object_del(ctxpopup);
	}

	ctxpopup = elm_ctxpopup_add(ad->nf);
	ea_object_event_callback_add(ctxpopup, EA_CALLBACK_BACK, ea_ctxpopup_back_cb, NULL);
	elm_object_style_set(ctxpopup, "more/default");
	evas_object_smart_callback_add(ctxpopup, "dismissed", _dismissed_cb, NULL);

	icon = elm_image_add(ctxpopup);
	snprintf(buf, sizeof(buf), "%s/00_controlbar_icon_contacts.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	elm_ctxpopup_item_append(ctxpopup, _("Add contact"), icon, _ctxpopup_cb, ctxpopup);

	icon = elm_image_add(ctxpopup);
	snprintf(buf, sizeof(buf), "%s/00_controlbar_icon_logs.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	elm_ctxpopup_item_append(ctxpopup, _("Phone calls"), icon, _ctxpopup_cb, ctxpopup);

	icon = elm_image_add(ctxpopup);
	snprintf(buf, sizeof(buf), "%s/00_controlbar_icon_favorites.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	elm_ctxpopup_item_append(ctxpopup, _("Favorites"), icon, _ctxpopup_cb, ctxpopup);

	icon = elm_image_add(ctxpopup);
	snprintf(buf, sizeof(buf), "%s/00_controlbar_icon_playlist.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	elm_ctxpopup_item_append(ctxpopup, _("Search"), icon, _ctxpopup_cb, ctxpopup);

	icon = elm_image_add(ctxpopup);
	snprintf(buf, sizeof(buf), "%s/00_controlbar_icon_dialer.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	elm_ctxpopup_item_append(ctxpopup, _("Dialer"), icon, _ctxpopup_cb, ctxpopup);

	elm_ctxpopup_direction_priority_set(ctxpopup, ELM_CTXPOPUP_DIRECTION_UP,
										ELM_CTXPOPUP_DIRECTION_UNKNOWN,
										ELM_CTXPOPUP_DIRECTION_UNKNOWN,
										ELM_CTXPOPUP_DIRECTION_UNKNOWN);
	_move_ctxpopup(ctxpopup, obj);

	evas_object_show(ctxpopup);
}
/*********************************************************
Icon + Text (More button style) End
 ********************************************************/

/*********************************************************
2 Text & 2 Icons
 ********************************************************/
static void _create_ctxpopup_2text_2icon( void *data, Evas_Object *obj, void *event_info )
{
	char buf[255];
	Evas_Object* icon;

	if(ctxpopup) {
		evas_object_del(ctxpopup);
		ctxpopup = NULL;
	}

	ctxpopup = elm_ctxpopup_add(ad->nf);
	ea_object_event_callback_add(ctxpopup, EA_CALLBACK_BACK, ea_ctxpopup_back_cb, NULL);
	evas_object_smart_callback_add(ctxpopup,"dismissed", _dismissed_cb, NULL);
	elm_ctxpopup_horizontal_set(ctxpopup, EINA_TRUE);

	elm_ctxpopup_item_append(ctxpopup, _("Function"), NULL, _ctxpopup_cb, ctxpopup);
	elm_ctxpopup_item_append(ctxpopup, _("Text"), NULL, _ctxpopup_cb, ctxpopup);

	//Icon 1
	icon = elm_image_add(ctxpopup);
	snprintf(buf, sizeof(buf), "%s/00_popup_icon_call.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	elm_ctxpopup_item_append(ctxpopup, NULL, icon, _ctxpopup_cb, ctxpopup);

	//Icon 2
	icon = elm_image_add(ctxpopup);
	snprintf(buf, sizeof(buf), "%s/00_popup_icon_message.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	elm_ctxpopup_item_append(ctxpopup, NULL, icon, _ctxpopup_cb, ctxpopup);

	_move_ctxpopup(ctxpopup, obj);
	evas_object_show(ctxpopup);
}
/*********************************************************
2 Text & 2 Icons End
 ********************************************************/
/*********************************************************
Icon + Text (More button style : Naviframe Toolbar)
 ********************************************************/
static void _create_ctxpopup_more_button_2_style(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *icon, *more_ctxpopup = NULL;
	Evas_Object *it_obj;
	Elm_Object_Item *it;
	char buf[256];

	more_ctxpopup = elm_ctxpopup_add(ad->nf);
	ea_object_event_callback_add(more_ctxpopup, EA_CALLBACK_BACK, ea_ctxpopup_back_cb, NULL);
	ea_object_event_callback_add(more_ctxpopup, EA_CALLBACK_MORE, ea_ctxpopup_back_cb, NULL);
	elm_object_style_set(more_ctxpopup, "more/default");
	elm_ctxpopup_auto_hide_disabled_set(more_ctxpopup, EINA_TRUE);
	evas_object_smart_callback_add(more_ctxpopup, "dismissed", _dismissed_more_ctxpopup_cb, NULL);
	evas_object_event_callback_add(more_ctxpopup, EVAS_CALLBACK_DEL, _delete_more_ctxpopup_cb, ad->nf);
	evas_object_event_callback_add(ad->nf, EVAS_CALLBACK_RESIZE, _resize_more_ctxpopup_cb, more_ctxpopup);
	evas_object_smart_callback_add(elm_object_top_widget_get(more_ctxpopup), "rotation,changed", _rotate_more_ctxpopup_cb, more_ctxpopup);

	icon = elm_image_add(more_ctxpopup);
	snprintf(buf, sizeof(buf), "%s/00_controlbar_icon_contacts.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	it = elm_ctxpopup_item_append(more_ctxpopup, _("Add contact"), icon, _ctxpopup_cb, more_ctxpopup);

	it_obj = elm_object_item_track(it);
	evas_object_event_callback_add(it_obj, EVAS_CALLBACK_MOVE, _on_move, NULL);

	icon = elm_image_add(more_ctxpopup);
	snprintf(buf, sizeof(buf), "%s/00_controlbar_icon_logs.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	elm_ctxpopup_item_append(more_ctxpopup, _("Phone calls"), icon, _ctxpopup_cb, more_ctxpopup);

	icon = elm_image_add(more_ctxpopup);
	snprintf(buf, sizeof(buf), "%s/00_controlbar_icon_favorites.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	elm_ctxpopup_item_append(more_ctxpopup, _("Favorites"), icon, _ctxpopup_cb, more_ctxpopup);

	icon = elm_image_add(more_ctxpopup);
	snprintf(buf, sizeof(buf), "%s/00_controlbar_icon_playlist.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	elm_ctxpopup_item_append(more_ctxpopup, _("Search"), icon, _ctxpopup_cb, more_ctxpopup);

	icon = elm_image_add(more_ctxpopup);
	snprintf(buf, sizeof(buf), "%s/00_controlbar_icon_dialer.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	elm_ctxpopup_item_append(more_ctxpopup, _("Dialer"), icon, _ctxpopup_cb, more_ctxpopup);

	icon = elm_image_add(more_ctxpopup);
	snprintf(buf, sizeof(buf), "%s/00_controlbar_icon_contacts.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	elm_ctxpopup_item_append(more_ctxpopup, _("Add contact"), icon, _ctxpopup_cb, more_ctxpopup);

	icon = elm_image_add(more_ctxpopup);
	snprintf(buf, sizeof(buf), "%s/00_controlbar_icon_logs.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	elm_ctxpopup_item_append(more_ctxpopup, _("Phone calls"), icon, _ctxpopup_cb, more_ctxpopup);

	icon = elm_image_add(more_ctxpopup);
	snprintf(buf, sizeof(buf), "%s/00_controlbar_icon_favorites.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	elm_ctxpopup_item_append(more_ctxpopup, _("Favorites"), icon, _ctxpopup_cb, more_ctxpopup);

	icon = elm_image_add(more_ctxpopup);
	snprintf(buf, sizeof(buf), "%s/00_controlbar_icon_playlist.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	elm_ctxpopup_item_append(more_ctxpopup, _("Search"), icon, _ctxpopup_cb, more_ctxpopup);

	icon = elm_image_add(more_ctxpopup);
	snprintf(buf, sizeof(buf), "%s/00_controlbar_icon_dialer.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	elm_ctxpopup_item_append(more_ctxpopup, _("Dialer"), icon, _ctxpopup_cb, more_ctxpopup);

	icon = elm_image_add(more_ctxpopup);
	snprintf(buf, sizeof(buf), "%s/00_controlbar_icon_contacts.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	elm_ctxpopup_item_append(more_ctxpopup, _("Add contact"), icon, _ctxpopup_cb, more_ctxpopup);

	icon = elm_image_add(more_ctxpopup);
	snprintf(buf, sizeof(buf), "%s/00_controlbar_icon_logs.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	elm_ctxpopup_item_append(more_ctxpopup, _("Phone calls"), icon, _ctxpopup_cb, more_ctxpopup);

	icon = elm_image_add(more_ctxpopup);
	snprintf(buf, sizeof(buf), "%s/00_controlbar_icon_favorites.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	elm_ctxpopup_item_append(more_ctxpopup, _("Favorites"), icon, _ctxpopup_cb, more_ctxpopup);

	icon = elm_image_add(more_ctxpopup);
	snprintf(buf, sizeof(buf), "%s/00_controlbar_icon_playlist.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	elm_ctxpopup_item_append(more_ctxpopup, _("Search"), icon, _ctxpopup_cb, more_ctxpopup);

	icon = elm_image_add(more_ctxpopup);
	snprintf(buf, sizeof(buf), "%s/00_controlbar_icon_dialer.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	elm_ctxpopup_item_append(more_ctxpopup, _("Dialer"), icon, _ctxpopup_cb, more_ctxpopup);

	elm_ctxpopup_direction_priority_set(more_ctxpopup, ELM_CTXPOPUP_DIRECTION_UP,
										ELM_CTXPOPUP_DIRECTION_UNKNOWN,
										ELM_CTXPOPUP_DIRECTION_UNKNOWN,
										ELM_CTXPOPUP_DIRECTION_UNKNOWN);

	_move_more_ctxpopup(more_ctxpopup);
	evas_object_show(more_ctxpopup);
}
/*********************************************************
Icon + Text (More button style : Naviframe Toolbar) End
 ********************************************************/
 /*********************************************************
Split Window
 ********************************************************/
static void _create_ctxpopup_split_window_style(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *icon, *more_ctxpopup = NULL;
	char buf[256];

	more_ctxpopup = ea_menu_popup_add(ad->nf);

	icon = elm_image_add(more_ctxpopup);
	snprintf(buf, sizeof(buf), "%s/00_controlbar_icon_contacts.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	elm_ctxpopup_item_append(more_ctxpopup, _("Add contact"), icon, _ctxpopup_cb, more_ctxpopup);

	icon = elm_image_add(more_ctxpopup);
	snprintf(buf, sizeof(buf), "%s/00_controlbar_icon_logs.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	elm_ctxpopup_item_append(more_ctxpopup, _("Phone calls"), icon, _ctxpopup_cb, more_ctxpopup);

	icon = elm_image_add(more_ctxpopup);
	snprintf(buf, sizeof(buf), "%s/00_controlbar_icon_favorites.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	elm_ctxpopup_item_append(more_ctxpopup, _("Favorites"), icon, _ctxpopup_cb, more_ctxpopup);

	icon = elm_image_add(more_ctxpopup);
	snprintf(buf, sizeof(buf), "%s/00_controlbar_icon_playlist.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	elm_ctxpopup_item_append(more_ctxpopup, _("Search"), icon, _ctxpopup_cb, more_ctxpopup);

	icon = elm_image_add(more_ctxpopup);
	snprintf(buf, sizeof(buf), "%s/00_controlbar_icon_dialer.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	elm_ctxpopup_item_append(more_ctxpopup, _("Dialer"), icon, _ctxpopup_cb, more_ctxpopup);

	icon = elm_image_add(more_ctxpopup);
	snprintf(buf, sizeof(buf), "%s/00_controlbar_icon_contacts.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	elm_ctxpopup_item_append(more_ctxpopup, _("Add contact"), icon, _ctxpopup_cb, more_ctxpopup);

	icon = elm_image_add(more_ctxpopup);
	snprintf(buf, sizeof(buf), "%s/00_controlbar_icon_logs.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	elm_ctxpopup_item_append(more_ctxpopup, _("Phone calls"), icon, _ctxpopup_cb, more_ctxpopup);

	icon = elm_image_add(more_ctxpopup);
	snprintf(buf, sizeof(buf), "%s/00_controlbar_icon_favorites.png", ICON_DIR);
	elm_image_file_set(icon, buf, NULL);
	elm_ctxpopup_item_append(more_ctxpopup, _("Favorites"), icon, _ctxpopup_cb, more_ctxpopup);

	elm_ctxpopup_direction_priority_set(more_ctxpopup, ELM_CTXPOPUP_DIRECTION_UP,
										ELM_CTXPOPUP_DIRECTION_UNKNOWN,
										ELM_CTXPOPUP_DIRECTION_UNKNOWN,
										ELM_CTXPOPUP_DIRECTION_UNKNOWN);

	ea_menu_popup_move(more_ctxpopup);
	evas_object_show(more_ctxpopup);
}
/*********************************************************
Split Window End
 ********************************************************/
static Eina_Bool _mousedown_cb(void *data, int type, void *event)
{
	Ecore_Event_Mouse_Button *ev = event;
	touch_x = ev->x;
	touch_y = ev->y;
	return ECORE_CALLBACK_CANCEL;
}

void ctxpopup_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *layout;
	Evas_Object *btn;
	Ecore_Event_Handler* event;
	Evas_Object *scroller;
	Elm_Object_Item *navi_it;

	ad = (struct appdata*) data;
	if (ad == NULL) return;

	elm_theme_extension_add(NULL, ELM_DEMO_EDJ);
	event = ecore_event_handler_add(ECORE_EVENT_MOUSE_BUTTON_DOWN, _mousedown_cb, NULL);

	//scroller
	scroller = elm_scroller_add(ad->nf);
	elm_scroller_bounce_set(scroller, EINA_FALSE, EINA_TRUE);
	elm_scroller_policy_set(scroller,ELM_SCROLLER_POLICY_OFF,ELM_SCROLLER_POLICY_AUTO);

	//layout
	layout = elm_layout_add(ad->nf);
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "ctxpopup_layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	navi_it = elm_naviframe_item_push(ad->nf, _("CtxPopup"), NULL, NULL, scroller, NULL);
	elm_naviframe_item_pop_cb_set(navi_it, _pop_cb, event);

	elm_object_content_set(scroller, layout);

	//create more button
	btn = elm_button_add(ad->nf);
	elm_object_style_set(btn, "naviframe/more/default");
	evas_object_smart_callback_add(btn, "clicked", _create_ctxpopup_more_button_2_style, ad);
	elm_object_item_part_content_set(navi_it, "toolbar_more_btn", btn);

	//Icon + Text
	btn = elm_button_add(layout);
	elm_object_text_set(btn, _("Icon + Text") );
	evas_object_smart_callback_add(btn, "clicked", _create_ctxpopup_icon_text, ad);
	elm_object_part_content_set(layout, "btn1", btn);

	//Text Only
	btn = elm_button_add(layout);
	elm_object_text_set(btn, _("Text Only"));
	evas_object_smart_callback_add(btn, "clicked", _create_ctxpopup_text_only, ad);
	elm_object_part_content_set(layout, "btn2", btn);

	//Copy & Paste
	btn = elm_button_add(layout);
	elm_object_text_set(btn, _("CopyPaste"));
	evas_object_smart_callback_add(btn, "clicked", _create_ctxpopup_copypaste_sample, ad);
	elm_object_part_content_set(layout, "btn3", btn);

	//3 Icons
	btn = elm_button_add(layout);
	elm_object_text_set(btn, _("3 Icons") );
	evas_object_smart_callback_add(btn, "clicked", _create_ctxpopup_3icon, ad);
	elm_object_part_content_set(layout, "btn4", btn);

	//Buttons
	btn = elm_button_add(layout);
	elm_object_text_set(btn, _("Buttons"));
	evas_object_smart_callback_add(btn, "clicked", _create_ctxpopup_buttons_style, ad);
	elm_object_part_content_set(layout, "btn5", btn);

	//Camera Shooting Mode
	btn = elm_button_add(layout);
	elm_object_text_set(btn, _("Camera Shooting"));
	evas_object_smart_callback_add(btn, "clicked", _create_ctxpopup_camera_shooting_style, ad);
	elm_object_part_content_set(layout, "btn6", btn);

	//Rotation
	btn = elm_button_add(layout);
	elm_object_text_set(btn, _("Rotation"));
	evas_object_smart_callback_add(btn, "clicked", _create_ctxpopup_on_rotation, ad);
	elm_object_part_content_set(layout, "btn7", btn);
	//Image editor adjust
	btn = elm_button_add(layout);
	elm_object_text_set(btn, _("Image Option"));
	evas_object_smart_callback_add(btn, "clicked", _create_ctxpopup_imageeditor_adjust_style, ad);
	elm_object_part_content_set(layout, "btn8", btn);

	//More button
	btn = elm_button_add(layout);
	elm_object_text_set(btn, _("More..."));
	evas_object_smart_callback_add(btn, "clicked", _create_ctxpopup_more_button_style, ad);
	elm_object_part_content_set(layout, "btn9", btn);

	//2 Text & 2 Icons Horizontal
	btn = elm_button_add(layout);
	elm_object_text_set(btn, _("2Text&2Icons"));
	evas_object_smart_callback_add(btn, "clicked", _create_ctxpopup_2text_2icon, ad);
	elm_object_part_content_set(layout, "btn10", btn);

	//Split Window
	btn = elm_button_add(layout);
	elm_object_text_set(btn, _("Split Window"));
	evas_object_smart_callback_add(btn, "clicked", _create_ctxpopup_split_window_style, ad);
	elm_object_part_content_set(layout, "btn11", btn);
}
