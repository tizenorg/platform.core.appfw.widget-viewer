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

#include <stdlib.h>
#include <Elementary.h>
#ifndef DESKTOP
	#include <location.h>
	#include <vconf.h>
	#include <utilX.h>
	#include <sensor.h>
#endif
#include "elmdemo_util.h"
#include "elmdemo_test.h"

// FIXME: This is temporary. Each App. should make it's own UA.
#define UA_TIZEN_WEB \
	"Mozilla/5.0 (Linux; Tizen 2.1; SAMSUNG GT-I8800) AppleWebKit/537.3 (KHTML, like Gecko) Version/2.1 Mobile Safari/537.3"

#define ENGINE_CHANGE               "Change engine"
#define SOURCE_CHANGE               "Change source"
#define GET_ADDRESS                 "Get address"
#define ICON_ADD                    "Icon to center"
#define CONTENT_ADD                 "Content to center"
#define GOTO_ADDRESS                "Go to address"
#define SHOW_SAMSUNG                "Show Samsung HQ"
#define BRINGIN_SEOUL               "Bring in Seoul"

#define NAVI_MY_POSITION            "My Position"
#define NAVI_COMPASS                "Compass"

#define MAP_MAPNIK                  "Mapnik"
#define MAP_OSMARENDER              "Osmarender"
#define MAP_CYCLEMAP                "CycleMap"
#define MAP_MAPQUEST                "MaqQuest"
#define MAP_MAPQUESTAERIAL          "MaqQuest Open Aerial"
#define MAP_MODULE_GOOGLE_STREET    "Google Street"
#define MAP_MODULE_GOOGLE_SATELLITE "Google Satellite"
#define MAP_MODULE_DECARTA_NORMAL   "Decarta Normal"
#define MAP_MODULE_BING_ROAD        "Bing Road"
#define MAP_MODULE_BING_AERIAL      "Bing Aerial"
#define MAP_MODULE_BING_HYBRID      "Bing Hybrid"

#define NAME_ENTRY_TEXT             "Enter freeform address"

#ifndef __UNUSED__
#define __UNUSED__ __attribute__((unused))
#endif

typedef struct _Map_App_Data
{
   struct appdata *ad;

   Evas_Object *box;
#ifndef DESKTOP
   LocationObject *loc;
#endif
   Evas_Object *txt_popup;
   Evas_Object *capture_popup;
   Evas_Object *list_popup;

   Evas_Object *map;
   Elm_Map_Name *name;
   Elm_Map_Overlay *ovl;
   Elm_Map_Overlay *route_from, *route_to;
   Elm_Map_Route *route;

   Evas_Object *sav_obj;
   int sensor_handle;
   Ecore_Timer *sensor_timer;
   unsigned long seid;
   unsigned long suid;
   double from_degree;
   double old_to_degree;
   double to_degree;
   Ecore_Animator *rotate_animator;
   Eina_Bool loaded;
} Mad;

Elm_Map_Overlay *clas;

static void
_dismissed_cb(void *data __UNUSED__, Evas_Object *obj , void *event __UNUSED__)
{
   if (!obj) return;
   evas_object_del(obj);
}

#ifndef DESKTOP
static Eina_Bool _add_proxy (void)
{
   char *proxy = vconf_get_str(VCONFKEY_NETWORK_PROXY);
   if (!proxy || (0 == strcmp(proxy, "")))
     {
        fprintf(stdout, "No Proxy\n");
        return EINA_TRUE;
     }
   fprintf(stdout, "Current proxy: %s\n", proxy);

   return EINA_TRUE;
}

static Eina_Bool
_get_network_state()
{
   int net_conf;
   int net_status;
   vconf_get_int(VCONFKEY_NETWORK_CONFIGURATION_CHANGE_IND, &net_conf);
   vconf_get_int(VCONFKEY_NETWORK_STATUS, &net_status);
   fprintf(stdout, "current network configuration (%d), Network status (%d)\n",
           net_conf, net_status);

   if (net_conf == 0 && net_status == VCONFKEY_NETWORK_OFF)
     {
        fprintf(stderr, "No wifi and No 3G\n");
        return EINA_FALSE;
     }
   else
     {
        fprintf(stdout, "Network is available\n");
        char *ip = vconf_get_str(VCONFKEY_NETWORK_IP);
        fprintf(stdout, "Current ip: %s\n", ip);
        if (!_add_proxy())
          {
             fprintf(stderr, "Add proxy failed\n");
             return EINA_FALSE;
          }
     }
   return EINA_TRUE;
}

static void
_capture_popup_end_cb(void *data, Evas_Object *obj, void *event_info)
{
   Mad *mad = data;
   evas_object_hide(obj);
   if (mad->sav_obj)
     {
        evas_object_del(mad->sav_obj);
        mad->sav_obj = NULL;
     }
}

static void
_capture(void *data, Evas_Object *obj, void *event_info)
{
   int x;
   int y;
   int width;
   int height;
   Ecore_X_Window x_win;
   XImage *img;
   Display *display;
   char buf[PATH_MAX];
   Evas_Object *popup;

   Mad *mad = data;
   if (!mad->capture_popup) mad->capture_popup = elm_popup_add(mad->map);
   popup = mad->capture_popup;
   if (mad->loaded)
     {
        if (!mad->sav_obj)
           mad->sav_obj = evas_object_image_add(evas_object_evas_get(mad->map));
        x_win = elm_win_xwindow_get(mad->sav_obj);
        if (!x_win) return;

        evas_object_geometry_get(mad->map, &x, &y, &width, &height);
        printf("\nMap Geometry: x = %d y = %d w1 =%d h1 =%d\n", x, y, width, height);

        display = XOpenDisplay(NULL);
        img = XGetImage(display, x_win, x, y, width, height, XAllPlanes(), ZPixmap);
        if (!img) return;
        if (!img->data) return;

        evas_object_image_size_set(mad->sav_obj, width, height);
        evas_object_image_data_set(mad->sav_obj, img->data);
        evas_object_image_filled_set(mad->sav_obj, EINA_TRUE);
        snprintf(buf, sizeof(buf), "%s/capturemap.png", ICON_DIR);

        if (!evas_object_image_save(mad->sav_obj, buf, NULL, "quality=100 compress=1"))
           printf("failed to save image \n");
        else
           printf("Saved scene as '%s'\n", buf);
        elm_object_style_set(popup, "content_expand");
        elm_object_part_text_set(popup, "title,text", "Image saved");
        elm_object_content_set(popup, mad->sav_obj);
        evas_object_resize(mad->sav_obj, 500, 500);
        evas_object_size_hint_min_set(mad->sav_obj, 500, 500);
        evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_smart_callback_add(popup, "block,clicked",
                                       _capture_popup_end_cb, mad);
     }
   else
     {
        evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        elm_object_text_set(popup, "map not loaded yet,try again !");
        elm_popup_timeout_set(popup, 3.0);
        evas_object_smart_callback_add(popup, "timeout",
                                       _capture_popup_end_cb, mad);
        evas_object_smart_callback_add(popup, "block,clicked",
                                       _capture_popup_end_cb, mad);
     }
   evas_object_show(popup);
}

#else
static void _capture(void *data, Evas_Object *obj, void *event_info)
{
	fprintf(stderr, "ERR: Cannot support capturing\n");
	return;
}
#endif

static void
_map_save_state(void* data)
{
   if (!data) return;

   //int zoom;
   double lon, lat;
   //const char *src_name;
   Mad *mad = data;
   if (!mad->map) return;

   elm_map_region_get(mad->map, &lon, &lat);
   //zoom = elm_map_zoom_get(mad->map);
   elm_map_zoom_get(mad->map);
   //src_name = elm_map_source_get(mad->map, ELM_MAP_SOURCE_TYPE_TILE);
   elm_map_source_get(mad->map, ELM_MAP_SOURCE_TYPE_TILE);

   //FIXME: using vconf to save!!!
   //printf("Saved, Zoom: %d, Longitude: %lf, Latitude:%lf\n", zoom, lon, lat);
}

static void
_map_load_state(void *data)
{
   if (!data) return;

#if 0 // FIXME: using vconf to revoke
   Mad *mad = data;
   int zoom = 15;
   double lon = 127.05286;
   double lat = 37.25768;
   char *src_name;


     {
        elm_map_region_show(mad->map, lon, lat);
        printf(" lon: %lf, lat: %lf", lon, lat);
     }

     {
        if (src_name)
          {
             elm_map_source_set(mad->map, ELM_MAP_SOURCE_TYPE_TILE, src_name);
             printf(" source name: %s", src_name);
             free(src_name);
          }
        else fprintf(stderr, "src_name is NULL\n");
     }
   printf(", State is Loded\n");
#endif
}

static Evas_Object *
_group_icon_get(Evas_Object *obj)
{
   Evas_Object *icon = elm_image_add(obj);
   elm_image_file_set(icon, ICON_DIR"/00_brightness_right.png", NULL);
   evas_object_show(icon);
   return icon;
}

static Evas_Object *
_icon_get(Evas_Object *obj)
{
   Evas_Object *icon = elm_image_add(obj);
   elm_image_file_set(icon, ICON_DIR"/logo.png", NULL);
   evas_object_show(icon);
   return icon;
}

static void
_response_cb(void *data, Evas_Object *obj __UNUSED__, void *event __UNUSED__)
{
   evas_object_del(data);
}

static void
_txt_popup_show(Mad *mad, const char *text)
{
   if (!mad->txt_popup)
     {
        Evas_Object *btn;
        mad->txt_popup = elm_popup_add(mad->box);
        evas_object_size_hint_weight_set(mad->txt_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        btn = elm_button_add(mad->txt_popup);
        elm_object_text_set(btn, "Close");
        elm_object_part_content_set(mad->txt_popup, "button1", btn);
        evas_object_smart_callback_add(btn, "clicked", _response_cb, mad->txt_popup);
     }
   elm_object_text_set(mad->txt_popup, text);
   evas_object_show(mad->txt_popup);
}

static void
_route_loaded(void *data, Evas_Object *obj __UNUSED__, void *event __UNUSED__)
{
   if (!data) return;

   Mad *mad = data;

   double d;
   const char *w, *n;
   char buf[PATH_MAX];

   d = elm_map_route_distance_get(mad->route);
   sprintf(buf, "route distance = %lf km", d);

   w = elm_map_route_waypoint_get(mad->route);
   if (w) fprintf(stdout, "[waypoints]\n%s\n", w);

   n = elm_map_route_node_get(mad->route);
   if (n) fprintf(stdout, "[nodes]\n%s\n", n);

   _txt_popup_show(mad, buf);
}

static void
_name_loaded(void *data, Evas_Object *obj __UNUSED__, void *event __UNUSED__)
{
   if (!data) return;
   Mad *mad = data;
   if (!mad->name) return;

   double lon, lat;
   const char *addr;

   elm_map_name_region_get(mad->name, &lon, &lat);
   elm_map_zoom_set(mad->map, 18);
   elm_map_region_show(mad->map, lon, lat);

   addr = elm_map_name_address_get(mad->name);
   _txt_popup_show(mad, addr);
}

static void
_clicked_double(void *data, Evas_Object *obj, void *event)
{
   if (!data) return;

   double lon, lat;
   Mad *mad = data;
   if (elm_map_zoom_get(mad->map) < 5) return;

   Evas_Event_Mouse_Down *down = event;
   elm_map_canvas_to_region_convert(obj, down->canvas.x, down->canvas.y,
                                    &lon, &lat);
   elm_map_region_bring_in(obj, lon, lat);
   int zoom = elm_map_zoom_get(obj);
   elm_map_zoom_set(obj, zoom+1);
   printf("clicked,double, lon:%lf lat:%lf (%d %d), zoom: %d\n", lon, lat,
		   down->canvas.x, down->canvas.y, zoom+1);
}

static void _on_load(void *data, Evas_Object *obj, void *event_info)
{
   printf("Map loaded\n");
   Mad *mad = data;
   if (!mad->loaded) mad->loaded = EINA_TRUE;
}

static void
_longpressed(void *data, Evas_Object *obj, void *ei)
{
   Mad *mad = data;
   Evas_Event_Mouse_Down *down = ei;
   double lon, lat, flon, flat, tlon, tlat;
   elm_map_canvas_to_region_convert(obj, down->canvas.x, down->canvas.y,
                                    &lon, &lat);
   if (!clas)
     {
        clas = elm_map_overlay_class_add(obj);
        elm_map_overlay_icon_set(clas, _group_icon_get(obj));
        elm_map_overlay_displayed_zoom_min_set(clas, 5);
     }
   if (mad->route_from && mad->route_to)
     {
        elm_map_overlay_del(mad->route_from);
        elm_map_overlay_del(mad->route_to);
        elm_map_route_del(mad->route);
        mad->route_from = NULL;
        mad->route_to = NULL;
        mad->route = NULL;
     }
   if (!mad->route_from)
     {
        mad->route_from = elm_map_overlay_add(obj, lon, lat);
        elm_map_overlay_class_append(clas, mad->route_from);
        printf("route from: %lf %lf\n", lon, lat);
     }
   else if (!mad->route_to)
     {
        mad->route_to = elm_map_overlay_add(obj, lon, lat);
        elm_map_overlay_class_append(clas, mad->route_to);
        printf("route to: %lf %lf\n", lon, lat);
     }
   if ((mad->route_from) && (mad->route_to))
     {
        elm_map_overlay_region_get(mad->route_from, &flon, &flat);
        elm_map_overlay_region_get(mad->route_to, &tlon, &tlat);
        mad->route = elm_map_route_add(obj, ELM_MAP_ROUTE_TYPE_MOTOCAR,
                                  ELM_MAP_ROUTE_METHOD_FASTEST, flon,
                                  flat, tlon, tlat, NULL, NULL);
        printf("Route from: %lf, %lf to: %lf, %lf\n", flon, flat, tlon, tlat);
     }
}

static void
_scroll_start(void *data, Evas_Object *obj __UNUSED__, void *event __UNUSED__)
{
   if (!data) return;

   Mad *mad = data;
   evas_object_smart_callback_del(mad->map, "longpressed", _longpressed);
}

static void
_scroll_stop(void *data, Evas_Object *obj __UNUSED__, void *event __UNUSED__)
{
   if (!data) return;

   Mad *mad = data;
   evas_object_smart_callback_add(mad->map, "longpressed", _longpressed, mad);
}

static void
_block_clicked_cb(void *data, Evas_Object *obj, void *ei)
{
   evas_object_del(data);
}

static void
_src_select_cb(void *data, Evas_Object *obj, void *ei)
{
   Mad *mad = data;
   const char *txt = elm_object_item_text_get(ei);
   elm_map_source_set(mad->map, ELM_MAP_SOURCE_TYPE_TILE, txt);
   printf("Source [%s] is selected\n", txt);
   evas_object_del(mad->list_popup);
   mad->list_popup = NULL;
}

static void
_source_change_cb(void *data, Evas_Object *obj, void *ei)
{
   Evas_Object *list, *popup;
   int idx = 0;
   Mad *mad = data;
   popup = elm_popup_add(mad->map);
   evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_object_style_set(popup, "menustyle");
   elm_object_part_text_set(popup, "title,text", "Select Source");
   evas_object_smart_callback_add(popup, "block,clicked", _block_clicked_cb, NULL);
   evas_object_show(popup);

   list = elm_list_add(mad->map);
   evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(list, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(list, "selected", _src_select_cb, mad);

   const char **sources = elm_map_sources_get(mad->map, ELM_MAP_SOURCE_TYPE_TILE);
   while(sources[idx])
     {
        elm_list_item_append(list, sources[idx], NULL, NULL, NULL, NULL);
        idx++;
     }
   elm_object_content_set(popup, list);
   mad->list_popup = popup;
   evas_object_del(obj);
}

static void
_engine_select_cb(void *data, Evas_Object *obj, void *ei)
{
   Mad *mad = data;
   const char *txt = elm_object_item_text_get(ei);
   elm_map_engine_set(mad->map, txt);
   printf("Engine [%s] is selected\n", txt);
   evas_object_del(mad->list_popup);
   mad->list_popup = NULL;
}

static void
_engine_change_cb(void *data, Evas_Object *obj, void *ei)
{
   Evas_Object *list, *popup;
   int idx = 0;
   Mad *mad = data;
   popup = elm_popup_add(mad->map);
   evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_object_style_set(popup, "menustyle");
   elm_object_part_text_set(popup, "title,text", "Select Engine");
   evas_object_smart_callback_add(popup, "block,clicked", _block_clicked_cb, NULL);
   evas_object_show(popup);

   list = elm_list_add(popup);
   evas_object_show(popup);
   evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(list, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(list, "selected", _engine_select_cb, mad);

   const char **engines = elm_map_engines_get(mad->map);
   while(engines[idx])
     {
        elm_list_item_append(list, engines[idx], NULL, NULL, NULL, NULL);
        idx++;
     }
   elm_object_content_set(popup, list);
   mad->list_popup = popup;
   evas_object_del(obj);
}

static void
_address_cb(void *data, Evas_Object *obj, void *ei)
{
   Mad *mad = data;
   double lon, lat;
   elm_map_region_get(mad->map, &lon, &lat);

   if (mad->name) elm_map_name_del(mad->name);
   mad->name = elm_map_name_add(mad->map, NULL, lon, lat, NULL, NULL);
   evas_object_del(obj);
}

static void
_icon_overlay_add_cb(void *data, Evas_Object *obj, void *ei)
{
   Mad *mad = data;
   double lon, lat;
   elm_map_region_get(mad->map, &lon, &lat);

   if (mad->ovl) elm_map_overlay_del(mad->ovl);
   mad->ovl = elm_map_overlay_add(mad->map, lon, lat);
   elm_map_overlay_icon_set(mad->ovl, _icon_get(mad->map));
   evas_object_del(obj);
}

static void
_content_overlay_add_cb(void *data, Evas_Object *obj, void *ei)
{
   Mad *mad = data;
   double lon, lat;
   Evas_Object *content;
   elm_map_region_get(mad->map, &lon, &lat);

   if (mad->ovl) elm_map_overlay_del(mad->ovl);
   mad->ovl = elm_map_overlay_add(mad->map, lon, lat);
   content = _icon_get(mad->map);
   evas_object_resize(content, 120, 120);
   elm_map_overlay_content_set(mad->ovl, content);
   evas_object_del(obj);
}

static void
_show_cb(void *data, Evas_Object *obj, void *ei)
{
   Mad *mad = data;
   elm_map_region_show(mad->map, 127.052888, 37.257516);
}

static void
_bringin_cb(void *data, Evas_Object *obj, void *ei)
{
   Mad *mad = data;
   elm_map_region_bring_in(mad->map, 126.962305, 37.563927);
}

static void
_menu_clicked(void *data, Evas_Object *obj, void *ei)
{
   if (!data) return;
   Mad *mad = data;

   Evas_Object *ctx;
   Evas_Coord x, y;
   evas_object_geometry_get(obj, &x, &y, NULL, NULL);

   ctx = elm_ctxpopup_add(mad->ad->nf);
   evas_object_smart_callback_add(ctx,"dismissed", _dismissed_cb, NULL);
   elm_ctxpopup_item_append(ctx, _(ENGINE_CHANGE), NULL, _engine_change_cb, mad);
   elm_ctxpopup_item_append(ctx, _(SOURCE_CHANGE), NULL, _source_change_cb, mad);
   elm_ctxpopup_item_append(ctx, _(GET_ADDRESS), NULL, _address_cb, mad);
   elm_ctxpopup_item_append(ctx, _(ICON_ADD), NULL, _icon_overlay_add_cb, mad);
   elm_ctxpopup_item_append(ctx, _(CONTENT_ADD), NULL, _content_overlay_add_cb, mad);
   elm_ctxpopup_item_append(ctx, _(SHOW_SAMSUNG), NULL, _show_cb, mad);
   elm_ctxpopup_item_append(ctx, _(BRINGIN_SEOUL), NULL, _bringin_cb, mad);
   evas_object_size_hint_max_set(ctx, 400, 600);
   evas_object_move(ctx, x, y);
   evas_object_show(ctx);
}

static void
_overlay_cb(void *data, Evas_Object *map, void *ev)
{
	Elm_Map_Overlay *overlay = ev;
	Elm_Map_Overlay_Type type = elm_map_overlay_type_get(overlay);
	if (type == ELM_MAP_OVERLAY_TYPE_GROUP) {
		Eina_List *l;
		Elm_Map_Overlay *memb;
		Eina_List *members = elm_map_overlay_group_members_get(overlay);
		EINA_LIST_FOREACH(members, l, memb) {
			printf("\nmemb=%p\n", memb);
		}
	}
	if (elm_map_overlay_data_get(overlay)) {
		printf("\n%s\n", (char *)elm_map_overlay_data_get(overlay));
		elm_map_overlay_paused_set(overlay, EINA_TRUE);
	}
	printf("Overlay clicked\n");
}

static Evas_Object *
_label_get(Evas_Object *obj)
{
	Evas_Object *label;
	label = elm_label_add(obj);
	elm_object_text_set(label, "Frankfurt International Airport");
	evas_object_show(label);
	return label;
}

static void
_get_cb(void *data __UNUSED__, Evas_Object *map, Elm_Map_Overlay *ovl)
{
	printf("\n\nIn get cb function\n");
}

static Evas_Object *
_map_add(Mad *mad)
{
   Evas_Object *map, *txt;
   Elm_Map_Overlay *grp1;
   Elm_Map_Overlay *ovl_1, *ovl_2, *ovl_3, *ovl_7, *ovl_8, *ovl_9, *ovl_10;
   Elm_Map_Overlay *ovl_circle, *ovl_bubble;

   if (!mad) return NULL;
   map = elm_map_add(mad->box);
   if (!map) return map;
   evas_object_show(map);

   elm_map_user_agent_set(map, UA_TIZEN_WEB);
   elm_map_engine_key_set(map, "Tizen Nokia Map",
                          "SpMLqvWrvuFmp27kV4e6/Md_LeHoPn0b4LPOxB60p4A");
   if (!elm_map_engine_set(map, "Tizen Nokia Map")) {
	   _txt_popup_show(mad, "Nokia Map engine set failed");
   }

   // Scale
   elm_map_overlay_scale_add(map, 30, 100);

   // Show Tour eiffel in Paris
   elm_map_region_show(map, 2.29435, 48.85814);
   elm_map_zoom_set(map, 10);
   ovl_1 = elm_map_overlay_add(map, 2.29435, 48.85814);
   elm_map_overlay_color_set(ovl_1, 0x00, 0xfa, 0x9a, 0xff);
   elm_map_overlay_displayed_zoom_min_set(ovl_1, 2);

   // Bubble on Paris
   ovl_bubble = elm_map_overlay_bubble_add(map);
   elm_map_overlay_bubble_follow(ovl_bubble, ovl_1);
   txt = elm_label_add(map);
   elm_object_text_set(txt, "It's Paris");
   evas_object_show(txt);
   elm_map_overlay_bubble_content_append(ovl_bubble, txt);

   // Circle on Tokyo
   ovl_circle = elm_map_overlay_circle_add(map, 139.794846, 35.541306, 5);
   elm_map_overlay_color_set(ovl_circle, 64, 16, 16, 64);

   // Frankfurt International Airport (content)
   ovl_2 = elm_map_overlay_add(map, 8.5604, 50.0382);
   elm_map_overlay_data_set(ovl_2, "data");
   elm_map_overlay_content_set(ovl_2, _label_get(map));
   elm_map_overlay_displayed_zoom_min_set(ovl_2, 2);

   // Tower of Pisa (icon)
   ovl_3 = elm_map_overlay_add(map, 10.396661, 43.723027);
   elm_map_overlay_get_cb_set(ovl_3, _get_cb, NULL);
   elm_map_overlay_icon_set(ovl_3, _icon_get(map));
   elm_map_overlay_displayed_zoom_min_set(ovl_3, 2);

   // Group Overlay (Rome)
   grp1 = elm_map_overlay_class_add(map);
   elm_map_overlay_icon_set(grp1, _group_icon_get(map));
   // Colosseo
   ovl_7 = elm_map_overlay_add(map, 12.49331, 41.88997);
   elm_map_overlay_class_append(grp1, ovl_7);
   // Sole al Pantheon Hotel
   ovl_8 = elm_map_overlay_add(map, 12.47684, 41.89853);
   elm_map_overlay_class_append(grp1, ovl_8);
   // Gardens of Vatican City
   ovl_9 = elm_map_overlay_add(map, 12.45016, 41.90360);
   elm_map_overlay_class_append(grp1, ovl_9);

   // Barcelonas Stadium in Spain
   ovl_10 = elm_map_overlay_add(map, 2.12174, 41.38067);
   elm_map_overlay_class_append(grp1, ovl_10);

   printf("\ngroup type = %d\n", elm_map_overlay_type_get(grp1));
   printf("\ndefault type = %d\n", elm_map_overlay_type_get(ovl_7));

   evas_object_smart_callback_add(map, "overlay,clicked", _overlay_cb, map);
   evas_object_smart_callback_add(map, "longpressed", _longpressed, mad);
   evas_object_smart_callback_add(map, "clicked,double", _clicked_double, mad);
   evas_object_smart_callback_add(map, "route,loaded", _route_loaded, mad);
   evas_object_smart_callback_add(map, "name,loaded", _name_loaded, mad);
   evas_object_smart_callback_add(map, "scroll,drag,start", _scroll_start, mad);
   evas_object_smart_callback_add(map, "scroll,drag,stop", _scroll_stop, mad);
   evas_object_smart_callback_add(map, "loaded", _on_load, mad);

   return map;
}

static Eina_Bool
_pop_cb(void *data, Elm_Object_Item *it)
{
   if (!data) return EINA_TRUE;

   Mad *mad = data;

   _map_save_state(mad);
#ifndef DESKTOP
   if (mad->loc)
     {
        location_stop(mad->loc);
        location_free(mad->loc);
     }
   if (mad->sensor_handle >= 0)
     {
        sf_disconnect(mad->sensor_handle);
        sf_stop(mad->sensor_handle);
     }
#endif
   if (mad->rotate_animator) ecore_animator_del(mad->rotate_animator);
   if (mad->sensor_timer) ecore_timer_del(mad->sensor_timer);
   if (mad->txt_popup) evas_object_del(mad->txt_popup);
   if (mad->capture_popup) evas_object_del(mad->capture_popup);
   if (mad->list_popup) evas_object_del(mad->list_popup);
   if (mad) free(mad);

   return EINA_TRUE;
}

static Evas_Object *
_box_add(Evas_Object *parent)
{
   if (!parent) return NULL;

   Evas_Object *box = elm_box_add(parent);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(box);
   return box;
}

static void
_nf_push(Evas_Object *nf, Evas_Object *item, Mad *mad)
{
   if (!nf || !item) return;

   Evas_Object *btn;
   Elm_Object_Item *it = elm_naviframe_item_push(nf, _("Map"), NULL, NULL, item, NULL);
   elm_naviframe_item_pop_cb_set(it, _pop_cb, mad);

   btn = elm_button_add(nf);
   elm_object_style_set(btn, "style1");
   elm_object_text_set(btn, "Capture");
   elm_object_item_part_content_set(it, "toolbar_button1", btn);
   evas_object_smart_callback_add(btn, "clicked", _capture, mad);

   btn = elm_button_add(nf);
   elm_object_style_set(btn, "style1");
   elm_object_text_set(btn, "Menu");
   elm_object_item_part_content_set(it, "toolbar_button2", btn);
   evas_object_smart_callback_add(btn, "clicked", _menu_clicked, mad);
}

void
map_cb(void *data, Evas_Object *obj, void *event)
{
   if (!data) return;
   fprintf(stdout, "\nMap test started\n");

   struct appdata *ad = data;
   Mad *mad = calloc(1, sizeof(Mad));

   mad->ad = ad;
   mad->sensor_handle = -1;

   mad->box = _box_add(mad->ad->nf);
   _nf_push(ad->nf, mad->box, mad);
#ifndef DESKTOP
   if (!_get_network_state())
     {
        _txt_popup_show(mad, "Network is not available");
        Elm_Object_Item *it = elm_naviframe_top_item_get(ad->nf);
        Evas_Object *btn =
           elm_object_item_part_content_get(it, "toolbar_button1");
        elm_object_disabled_set(btn, EINA_TRUE);
        btn = elm_object_item_part_content_get(it, "toolbar_button2");
        elm_object_disabled_set(btn, EINA_TRUE);
        return;
     }

   location_init();
#endif
   mad->map = _map_add(mad);
   if (!mad->map) {
	   _txt_popup_show(mad, "elm_map_add() failed");
   }
   _map_load_state(mad);

   evas_object_size_hint_weight_set(mad->map, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(mad->map, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(mad->box, mad->map);
}
