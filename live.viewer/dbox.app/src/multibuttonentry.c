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
#include "multibuttonentry.h"

/*********************************************************
 multibuttonentry
 ********************************************************/

#define BTN_NUM 4

Evas_Object *multibuttonentry;
Elm_Object_Item *clicked_item;

static void _check_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *mbe = multibuttonentry;
   if (!mbe || !obj) return;

   if (!strcmp(elm_object_text_get(obj), "Editable"))
      elm_multibuttonentry_editable_set(mbe, elm_check_state_get(obj));
   else if (!strcmp(elm_object_text_get(obj), "Expanded"))
      elm_multibuttonentry_expanded_set(mbe, elm_check_state_get(obj));

   clicked_item = NULL;
}

static void _button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *mbe = multibuttonentry;
   if (!mbe || !obj) return;

   if (!strcmp(elm_object_text_get(obj), "Append"))
     {
        if (clicked_item)
           elm_multibuttonentry_item_insert_after(mbe, clicked_item, "Append", NULL, NULL);
        else
           elm_multibuttonentry_item_append(mbe, "Append", NULL, NULL);
     }
   else if (!strcmp(elm_object_text_get(obj), "Delete"))
     {
        if (clicked_item)
           elm_object_item_del(clicked_item);
     }
   else if (!strcmp(elm_object_text_get(obj), "Clear"))
     {
        elm_multibuttonentry_clear(mbe);
     }
   else if (!strcmp(elm_object_text_get(obj), "Item Disabled"))
     {
        const Eina_List *items, *l;
        Elm_Object_Item *it;

        items = elm_multibuttonentry_items_get(mbe);

        EINA_LIST_FOREACH(items, l, it)
          {
             if (elm_object_item_disabled_get(it))
               elm_object_item_disabled_set(it, EINA_FALSE);
             else
               elm_object_item_disabled_set(it, EINA_TRUE);
          }
     }
   else if (!strcmp(elm_object_text_get(obj), "MBE Disabled"))
     {
         if (elm_object_disabled_get(mbe))
          elm_object_disabled_set(mbe, EINA_FALSE);
        else
          elm_object_disabled_set(mbe, EINA_TRUE);
     }

   clicked_item = NULL;
}

static void _mouse_down_cb(void *data, Evas *evas, Evas_Object *o, void *ev)
{
   printf("%s\n", __func__);
}

static void _mouse_move_cb(void *data, Evas *evas, Evas_Object *o, void *ev)
{
   Evas_Coord x, y, w, h;

   Evas_Event_Mouse_Move *event = (Evas_Event_Mouse_Move *)ev;
   evas_object_geometry_get(data, &x, &y, &w, &h);
   evas_object_move(data, event->cur.canvas.x - (w / 2), event->cur.canvas.y - (h / 2));
}

static void _mouse_up_cb(void *data, Evas *evas, Evas_Object *o, void *ev)
{
   printf("%s\n", __func__);

   evas_object_event_callback_del_full(o, EVAS_CALLBACK_MOUSE_DOWN, _mouse_down_cb, data);
   evas_object_event_callback_del_full(o, EVAS_CALLBACK_MOUSE_UP, _mouse_up_cb, data);
   evas_object_event_callback_del_full(o, EVAS_CALLBACK_MOUSE_MOVE, _mouse_move_cb, data);

   evas_object_del(data);
}

static void _refresh_end_text(Evas_Object *obj)
{
   Evas_Object *end, *entry;
   const char *end_text, *entry_text;
   char buf[1024];

   if (!obj) return;

   end = (Evas_Object *)evas_object_data_get(obj, "end");
   entry = elm_multibuttonentry_entry_get(obj);
   if (end)
     {
        entry_text = elm_object_text_get(entry);
        end_text = elm_object_text_get(end);
        if (entry_text && strlen(entry_text))
          {
             if (end_text && strlen(end_text))
               snprintf(buf, sizeof(buf), "%s, %s", end_text, entry_text);
             else
               snprintf(buf, sizeof(buf), "%s", entry_text);
             elm_object_text_set(end, buf);
          }
     }
}

static void _item_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Object_Item *item = (Elm_Object_Item *)event_info;
   printf("\nselected item = %s\n", elm_object_item_text_get(item));
}

static void _item_added_cb(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Object_Item *item = (Elm_Object_Item *)event_info;
   printf("\nadded item = %s\n", elm_object_item_text_get(item));

   _refresh_end_text(obj);
}

static void _item_deleted_cb(void *data, Evas_Object *obj, void *event_info)
{
   printf("\ndeleted item!\n");

   _refresh_end_text(obj);
}

static void _item_clicked_cb( void *data, Evas_Object *obj, void *event_info)
{
   Elm_Object_Item *item = (Elm_Object_Item *)event_info;
   printf("\nclicked item = %s\n", elm_object_item_text_get(item));

   clicked_item = item;
}

static void _item_longpressed_cb(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Object_Item *item = (Elm_Object_Item *)event_info;
   printf("\npressed_item = %s\n", elm_object_item_text_get(item));

   Evas_Object *item_obj, *new_obj;
   Evas_Coord x, y, w, h;
   const char *buf;

   item_obj = elm_multibuttonentry_item_object_get(item);
   buf = elm_object_part_text_get(item_obj, "elm.btn.text");

   if (!item_obj) return;
   evas_object_geometry_get(item_obj, &x, &y, &w, &h);

   new_obj = elm_layout_add(data);
   elm_layout_theme_set(new_obj, "multibuttonentry", "btn", elm_object_style_get(item_obj));
   elm_object_part_text_set(new_obj, "elm.btn.text", buf);

   evas_object_smart_calculate(new_obj);
   elm_layout_signal_emit(new_obj, "elm,state,text,ellipsis", "");

   evas_object_event_callback_add(data, EVAS_CALLBACK_MOUSE_DOWN, _mouse_down_cb, new_obj);
   evas_object_event_callback_add(data, EVAS_CALLBACK_MOUSE_UP, _mouse_up_cb, new_obj);
   evas_object_event_callback_add(data, EVAS_CALLBACK_MOUSE_MOVE, _mouse_move_cb, new_obj);

   evas_object_move(new_obj, x, y);
   evas_object_resize(new_obj, w, h);
   evas_object_show(new_obj);
   elm_object_item_del(item);
}

static void _mbe_clicked_cb( void *data, Evas_Object *obj, void *event_info )
{
   printf("multibuttonentry is clicked!\n");
}

static void _mbe_focused_cb( void *data, Evas_Object *obj, void *event_info )
{
   printf("multibuttonentry is focused!\n");

   Evas_Object *end;

   end = (Evas_Object *)evas_object_data_get(obj, "end");
   if (end)
     elm_object_text_set(end, NULL);
}

static void _mbe_unfocused_cb( void *data, Evas_Object *obj, void *event_info )
{
   printf("multibuttonentry is unfocused!\n");

   _refresh_end_text(obj);
}

static void _expanded_cb(void *data, Evas_Object *obj, void *event_info)
{
   printf("multibuttonentry is expanded!\n");
}

static void _contracted_cb(void *data, Evas_Object *obj, void *event_info)
{
   printf("multibuttonentry is contracted!\n");
}

static void _expand_state_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
   if (elm_multibuttonentry_expanded_get(obj))
      printf("\nexpand state changed: expanded!!\n");
   else
      printf("\nexpand state changed: shrinked!!\n");
}

static Eina_Bool _item_filter_cb(Evas_Object *obj, const char* item_label, const void *item_data, const void *data)
{
   printf("\"%s\" item will be added\n", item_label);

   char *str = strstr(item_label, "S");

   Evas_Object *end = (Evas_Object *)evas_object_data_get(obj, "end");
   if (end)
     elm_object_text_set(end, NULL);

   if (str)
     return EINA_FALSE;
   else
     return EINA_TRUE;
}

static Evas_Object* _add_multibuttonentry(Evas_Object* parent)
{
   Evas_Object* scroller = NULL;
   Evas_Object* mbe = NULL;

   scroller = elm_scroller_add(parent);
   elm_scroller_bounce_set(scroller, EINA_FALSE, EINA_TRUE);
   elm_scroller_policy_set(scroller,ELM_SCROLLER_POLICY_OFF,ELM_SCROLLER_POLICY_AUTO);
   evas_object_show(scroller);

   // add a multibuttonentry object
   mbe = elm_multibuttonentry_add(parent);
   multibuttonentry = mbe;
   elm_object_text_set(mbe, "To: ");
   elm_object_part_text_set(mbe, "guide", "Tap to add recipient");
   evas_object_size_hint_weight_set(mbe, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(mbe, EVAS_HINT_FILL, 0.0);
   elm_object_content_set(scroller, mbe);

   // add item filter callback
   elm_multibuttonentry_item_filter_append(mbe, _item_filter_cb, NULL);

   // add state-related callbacks
   evas_object_smart_callback_add(mbe, "clicked", _mbe_clicked_cb, NULL);
   evas_object_smart_callback_add(mbe, "focused", _mbe_focused_cb, NULL);
   evas_object_smart_callback_add(mbe, "unfocused", _mbe_unfocused_cb, NULL);

   // add size-related callbacks
   evas_object_smart_callback_add(mbe, "expanded", _expanded_cb, NULL);
   evas_object_smart_callback_add(mbe, "contracted", _contracted_cb, NULL);
   evas_object_smart_callback_add(mbe, "expand,state,changed", _expand_state_changed_cb, NULL);

   // add item-related callbacks
   evas_object_smart_callback_add(mbe, "item,selected", _item_selected_cb, NULL);
   evas_object_smart_callback_add(mbe, "item,added", _item_added_cb, NULL);
   evas_object_smart_callback_add(mbe, "item,deleted", _item_deleted_cb, NULL);
   evas_object_smart_callback_add(mbe, "item,clicked", _item_clicked_cb, NULL);
   evas_object_smart_callback_add(mbe, "longpressed", _item_longpressed_cb, parent);

   elm_object_focus_set(mbe, EINA_TRUE);

   return scroller;
}

#if 0
static void _1000_items_test_cb(void *data, Evas_Object *obj, void *event_info)
{

   Evas_Object *mbe = multibuttonentry;
   Evas *e = evas_object_evas_get(mbe);
   int i = 0;
   char *str;
   double starttime = 0.0, endtime = 0.0;
   Elm_Object_Item* item = NULL;
   switch((int)data)
     {
        case 0:
           str = "111111111111";
           starttime = ecore_time_get();
           for (i = 0; i < 1000; i++)
             {
                item = elm_multibuttonentry_item_append(mbe, str, NULL, NULL);
                if (!item)
                  {
                     printf("item insert fail at %d\n", i);
                     break;
                  }
             }
           evas_render(e);
           endtime = ecore_time_get();
           break;
        case 1:
           str = "11111";
           starttime = ecore_time_get();
           for (i = 0; i < 1000; i++)
             {
                item = elm_multibuttonentry_item_append(mbe, str, NULL, NULL);
                if (!item)
                  {
                     printf("item insert fail at %d\n", i);
                     break;
                  }
             }
           evas_render(e);
           endtime = ecore_time_get();
           break;
        case 2:
           str = "1";
           starttime = ecore_time_get();
           for (i = 0; i < 1000; i++)
             {
                item = elm_multibuttonentry_item_append(mbe, str, NULL, NULL);
                if (!item)
                  {
                     printf("item insert fail at %d\n", i);
                     break;
                  }
             }
           evas_render(e);
           endtime = ecore_time_get();
           break;
        case 3:
           starttime = ecore_time_get();
           for (i = 0; i < 1000; i++)
             {
                item = elm_multibuttonentry_first_item_get(mbe);
                if (item)
                  elm_object_item_del(item);
                else
                  break;
             }
           evas_render(e);
           endtime = ecore_time_get();
           break;
     }
   printf("test %d items starttime: %02.06lf, endtime: %02.06lf, endtime - starttime: %02.06lf\n",
          i, starttime, endtime, endtime - starttime);
}


static Evas_Object* _add_1000_test_buttons(Evas_Object* parent)
{
   Evas_Object* bx = NULL;
   Evas_Object* btn[BTN_NUM] = {0,};
   char *label[BTN_NUM] = {
        "1000 items insert, 1 line by 2 items",
        "1000 items insert, 1 line by 5 items",
        "1000 Items insert, 1 line by 15 items",
        "Items remove all, one by one"
   };
   int i;

   bx = elm_box_add(parent);
   elm_box_horizontal_set(bx, EINA_FALSE);
   elm_box_homogeneous_set(bx, EINA_TRUE);

   for (i = 0; i < BTN_NUM; i++)
     {
        btn[i] = elm_button_add(parent);
        evas_object_smart_callback_add(btn[i], "clicked", _1000_items_test_cb, (void *)i);
        elm_object_text_set(btn[i], label[i]);
        evas_object_size_hint_weight_set(btn[i], EVAS_HINT_EXPAND, 0.0);
        evas_object_size_hint_align_set(btn[i], EVAS_HINT_FILL, EVAS_HINT_FILL);
        elm_box_pack_end(bx, btn[i]);
        evas_object_show(btn[i]);
     }
   return bx;
}
#endif

static Evas_Object *_add_check_buttons(Evas_Object *parent)
{
   Evas_Object *bx;
   Evas_Object *chk;

   bx = elm_box_add(parent);
   elm_box_horizontal_set(bx, EINA_TRUE);
   evas_object_show(bx);

   chk = elm_check_add(bx);
   elm_object_text_set(chk, "Editable");
   elm_check_state_set(chk, EINA_TRUE);
   evas_object_size_hint_weight_set(chk, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(chk, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(chk, "changed", _check_changed_cb, NULL);
   elm_box_pack_end(bx, chk);
   evas_object_show(chk);

   chk = elm_check_add(bx);
   elm_object_text_set(chk, "Expanded");
   elm_check_state_set(chk, EINA_TRUE);
   evas_object_size_hint_weight_set(chk, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(chk, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(chk, "changed", _check_changed_cb, NULL);
   elm_box_pack_end(bx, chk);
   evas_object_show(chk);

   return bx;
}

static Evas_Object *_add_buttons(Evas_Object *parent)
{
   Evas_Object *bx;
   Evas_Object *btn[BTN_NUM];
   char *text[BTN_NUM] = {"Unfocus", "Append", "Delete", "Clear"};
   int i;

   bx = elm_box_add(parent);
   elm_box_horizontal_set(bx, EINA_TRUE);
   evas_object_show(bx);

   for (i = 0; i < BTN_NUM; i++)
     {
        btn[i] = elm_button_add(parent);
        elm_object_text_set(btn[i], text[i]);
        evas_object_smart_callback_add(btn[i], "clicked", _button_clicked_cb, NULL);
        evas_object_size_hint_weight_set(btn[i], EVAS_HINT_EXPAND, 0.0);
        evas_object_size_hint_align_set(btn[i], EVAS_HINT_FILL, 0.0);
        elm_box_pack_end(bx, btn[i]);
        evas_object_show(btn[i]);
     }

   return bx;
}

void multibuttonentry_cb(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *ly = NULL;
   Evas_Object *sc = NULL;
   Evas_Object *bx = NULL;
   Evas_Object *btn1 = NULL;
   Evas_Object *btn2 = NULL;

   struct appdata *ad = (struct appdata *) data;
   if (ad == NULL) return;

   multibuttonentry = NULL;
   clicked_item = NULL;

   ly = elm_layout_add(ad->nf);
   evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_layout_file_set(ly, ELM_DEMO_EDJ, "multibuttonentry_test");
   elm_naviframe_item_push(ad->nf, _("Multibuttonentry"), NULL, NULL, ly, NULL);

   bx = _add_check_buttons(ly);
   elm_object_part_content_set(ly, "check_box", bx);

   bx = _add_buttons(ly);
   elm_object_part_content_set(ly, "button_box", bx);

   btn1 = elm_button_add(ly);
   elm_object_text_set(btn1, "Item Disabled");
   evas_object_smart_callback_add(btn1, "clicked", _button_clicked_cb, NULL);
   evas_object_size_hint_weight_set(btn1, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(btn1, EVAS_HINT_FILL, 0.0);
   elm_object_part_content_set(ly, "btn1", btn1);
   evas_object_show(btn1);

   btn2 = elm_button_add(ly);
   elm_object_text_set(btn2, "MBE Disabled");
   evas_object_smart_callback_add(btn2, "clicked", _button_clicked_cb, NULL);
   evas_object_size_hint_weight_set(btn2, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(btn2, EVAS_HINT_FILL, 0.0);
   elm_object_part_content_set(ly, "btn2", btn2);
   evas_object_show(btn2);

   sc = _add_multibuttonentry(ly);
   elm_object_part_content_set(ly, "multibuttonentry", sc);
}

