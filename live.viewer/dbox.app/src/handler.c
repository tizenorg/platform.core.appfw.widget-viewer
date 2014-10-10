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
#include "handler.h"

/*********************************************************
  Handler
 ********************************************************/

static void _handler_h_cb(void *data, Evas_Object *obj, void *event_info);
static void _handler_v_cb(void *data, Evas_Object *obj, void *event_info);

#define NUM_OF_ITEMS 50
static Elm_Genlist_Item_Class itc;
static char *country_names[] = {
	"Algeria", "Andorra",
	"Argentina", "Australia", "Bahamas", "Bangladesh", "belgium", "Benin",
	"Bosnia", "Canada", "Chad", "Chile", "Cuba ", "Denmark",
	"Dominican Republic", "Egypt", "England", "Europa Island", "china",
	"Ethiopia", "Fiji", "Finland", "France", "Germany", "Ghana", "Greece",
	"Hong Kong", "Hungary", "India", "Iran", "Iraq", "Hafiz", "Italy",
	"Jordan", "kuwait", "Macau", "Mexico ", "Panama", "Poland", "Peru",
	"Russia", "Scotland", "Slovakia", "South Korea", "Sri Lanka", "Sweden",
	"Taiwan", "Tanzania", "Uganda", "Zimbabwe"
};
static struct _menu_item menu_its[] = {
   {"Vert Handler", _handler_v_cb },
   {"Horz Handler", _handler_h_cb },

   /* do not delete below */
   {NULL, NULL}
};

static char *_gl_text_get(void *data, Evas_Object *obj, const char *part)
{
	if (!strcmp(part, "elm.text")) {
		return strdup(country_names[(int)data % NUM_OF_ITEMS]);
	}

	return NULL;
}

static Evas_Object *_gl_content_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *icon = NULL;
	char buf[PATH_MAX];

	if (!strcmp(part, "elm.icon")) {
		icon = elm_image_add(obj);
		snprintf(buf, sizeof(buf), ICON_DIR"/g%d.png", ((int)data)%6);
		elm_image_file_set(icon, buf, NULL);
		evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	}

	return icon;
}

static Eina_Bool _gl_state_get(void *data, Evas_Object *obj, const char *part)
{
	return EINA_FALSE;
}

static void _gl_del(void *data, Evas_Object *obj)
{
	return;
}

static void _gl_sel(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *item = (Elm_Object_Item *)event_info;
	int index = (int)elm_object_item_data_get(item);

	if (item != NULL) {
		fprintf(stdout, "selected text %s\n", country_names[index]);
	}
}

static void _handler_v_cb(void *data, Evas_Object *obj, void *event_info)
{
	int index;
	Evas_Object *genlist;
	struct appdata *ad = (struct appdata *) data;

	if (ad == NULL) return;

	itc.item_style = "1text.1icon.2";
	itc.func.text_get = _gl_text_get;
	itc.func.content_get = _gl_content_get;
	itc.func.state_get = _gl_state_get;
	itc.func.del = _gl_del;

	genlist = elm_genlist_add(ad->nf);
	elm_object_style_set(genlist, "handler");

	for (index = 0; index < 4*NUM_OF_ITEMS; index++) {
		elm_genlist_item_append(genlist, &itc, (void *) index, NULL, ELM_GENLIST_ITEM_NONE, _gl_sel, NULL);
	}
	elm_naviframe_item_push(ad->nf,  _("Vert-Handler"), NULL, NULL, genlist, NULL);
}

static Evas_Object* _create_horz_handler(Evas_Object* parent)
{
	Evas_Object * in_layout;
	Evas_Object *image;
	Evas_Coord w, h;

	in_layout = elm_layout_add(parent);
	elm_layout_file_set(in_layout, ELM_DEMO_EDJ, "elmdemo-test/horz_handler");

	Evas_Object* scroller = elm_scroller_add(parent);
	elm_scroller_bounce_set(scroller, EINA_FALSE, EINA_FALSE);
	elm_object_style_set(scroller, "handler");
	elm_scroller_policy_set(scroller,ELM_SCROLLER_POLICY_AUTO, ELM_SCROLLER_POLICY_AUTO);
	evas_object_show(scroller);

	image = elm_image_add(parent);
	elm_image_file_set(image, ICON_DIR"/horz_scrollbar.jpg", NULL);
	elm_image_resizable_set(image, EINA_TRUE, EINA_FALSE);
	elm_image_object_size_get(image, &w, &h);
	evas_object_show(image);

	elm_object_content_set(scroller, image);
	elm_object_part_content_set(in_layout, "scroller", scroller);

	return in_layout;
}

static void _handler_h_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad;
	Evas_Object *layout_inner;

	ad = (struct appdata *)data;
	if (ad == NULL) return;

	layout_inner = _create_horz_handler (ad->nf);
	elm_naviframe_item_push (ad->nf, _("Horz-Handler") , NULL, NULL, layout_inner, NULL);
}

static void
_list_click(void *data, Evas_Object * obj, void *event_info)
{
   Elm_Object_Item *it = (Elm_Object_Item *) elm_list_selected_item_get(obj);

   if (it == NULL)
   {
	   fprintf((LOG_PRI(LOG_ERR) == LOG_ERR?stderr:stdout), "List item is NULL.\n");
	   return;
   }

   elm_list_item_selected_set(it, EINA_FALSE);
}

static Evas_Object *
_create_list_winset(struct appdata *ad)
{
   Evas_Object *li;

   if (ad == NULL) return NULL;

   li = elm_list_add(ad->nf);
   elm_list_mode_set(li, ELM_LIST_COMPRESS);
   evas_object_smart_callback_add(li, "selected", _list_click, NULL);

   int idx = 0;

   while (menu_its[idx].name != NULL)
     {
        elm_list_item_append(li, menu_its[idx].name, NULL, NULL, menu_its[idx].func, ad);
        ++idx;
     }
   elm_list_go(li);

   return li;
}

void handler_cb(void *data, Evas_Object * obj, void *event_info)
{
   struct appdata *ad;
   Evas_Object *list;

   ad = (struct appdata *)data;
   if (!ad) return;

   list = _create_list_winset(ad);
   elm_naviframe_item_push(ad->nf, _("Handler"), NULL, NULL, list, NULL);
}
