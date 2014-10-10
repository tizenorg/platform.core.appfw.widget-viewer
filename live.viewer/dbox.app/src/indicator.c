/*
 * Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *		http://www.apache.org/licenses/LICENSE-2.0
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
#include "indicator.h"

static void
_autohide_indi_cb(void *data, Evas_Object *obj, void *ev)
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

    elm_win_indicator_mode_set(win, ELM_WIN_INDICATOR_SHOW);
    elm_win_indicator_opacity_set(win, ELM_WIN_INDICATOR_TRANSPARENT);
}

static void
_headerbg_indi_cb(void *data, Evas_Object *obj, void *ev)
{

    struct appdata *ad = (struct appdata *) data;
    Evas_Object *win, *conform, *bg;

    if ((!ad->win_main) || (!ad->conform))
    {
       printf("[%s]:We can't get conformant\n", __FUNCTION__);
       return;
    }
    win     = ad->win_main;
    conform = ad->conform;

    elm_win_indicator_mode_set(win, ELM_WIN_INDICATOR_SHOW);
    elm_win_indicator_opacity_set(win, ELM_WIN_INDICATOR_OPAQUE);
    elm_object_signal_emit(conform, "elm,state,indicator,nooverlap", "elm");

    bg = elm_bg_add(conform);
    elm_object_style_set(bg, "indicator/headerbg");
    elm_object_part_content_set(conform, "elm.swallow.indicator_bg", bg);
    evas_object_show(bg);
}

static void
_translucent_indi_cb(void *data, Evas_Object *obj, void *ev)
{
    struct appdata *ad = (struct appdata *) data;
    Evas_Object *win, *conform, *bg;

    if ((!ad->win_main) || (!ad->conform))
    {
       printf("[%s]:We can't get conformant\n", __FUNCTION__);
       return;
    }
    win     = ad->win_main;
    conform = ad->conform;

    elm_win_indicator_mode_set(win, ELM_WIN_INDICATOR_SHOW);
    elm_win_indicator_opacity_set(win, ELM_WIN_INDICATOR_OPAQUE);
    elm_object_signal_emit(conform, "elm,state,indicator,overlap", "elm");

    bg = elm_bg_add(conform);
    elm_object_style_set(bg, "indicator/translucent");
    elm_object_part_content_set(conform, "elm.swallow.indicator_bg", bg);
    evas_object_show(bg);
}

static void
_transparent_indi_cb(void *data, Evas_Object *obj, void *ev)
{
    struct appdata *ad = (struct appdata *) data;
    Evas_Object *win, *conform, *bg;

    if ((!ad->win_main) || (!ad->conform))
    {
       printf("[%s]:We can't get conformant\n", __FUNCTION__);
       return;
    }
    win     = ad->win_main;
    conform = ad->conform;

    elm_win_indicator_mode_set(win, ELM_WIN_INDICATOR_SHOW);
    elm_win_indicator_opacity_set(win, ELM_WIN_INDICATOR_OPAQUE);
    elm_object_signal_emit(conform, "elm,state,indicator,overlap", "elm");

    bg = elm_bg_add(conform);
    elm_object_style_set(bg, "indicator/transparent");
    elm_object_part_content_set(conform, "elm.swallow.indicator_bg", bg);
    evas_object_show(bg);
}

//recover indicator when indicator start
static void
_default_indi_cb(void *data, Evas_Object *obj, void *ev)
{
    struct appdata *ad = (struct appdata *) data;
    Evas_Object *win, *conform, *bg;

    if ((!ad->win_main) || (!ad->conform))
    {
       printf("[%s]:We can't get conformant\n", __FUNCTION__);
       return;
    }
    win     = ad->win_main;
    conform = ad->conform;

    elm_win_indicator_mode_set(win, ELM_WIN_INDICATOR_SHOW);
    elm_win_indicator_opacity_set(win, ELM_WIN_INDICATOR_OPAQUE);
    elm_object_signal_emit(conform, "elm,state,indicator,nooverlap", "elm");

    bg = elm_bg_add(conform);
    elm_object_style_set(bg, "indicator/default");
    elm_object_part_content_set(conform, "elm.swallow.indicator_bg", bg);
    evas_object_show(bg);
}

static Evas_Object *
_create_box(Evas_Object *parent, void *data)
{
    struct appdata *ad = (struct appdata *) data;
    Evas_Object * bx;
    Evas_Object *bt;
    Evas_Object *win;
    win = ad->win_main;

    // Create box
    bx = elm_box_add(parent);

    bt = elm_button_add(bx);
    elm_object_text_set(bt, "Default indicator");
    evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0);
    evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0);
    evas_object_show(bt);
    elm_box_pack_end(bx, bt);
    evas_object_smart_callback_add(bt, "clicked", _default_indi_cb, data);

    bt = elm_button_add(bx);
    elm_object_text_set(bt, "Headerbg indicator");
    evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0);
    evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0);
    evas_object_show(bt);
    elm_box_pack_end(bx, bt);
    evas_object_smart_callback_add(bt, "clicked", _headerbg_indi_cb, data);

    bt = elm_button_add(bx);
    elm_object_text_set(bt, "Translucent indicator");
    evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0);
    evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0);
    evas_object_show(bt);
    elm_box_pack_end(bx, bt);
    evas_object_smart_callback_add(bt, "clicked", _translucent_indi_cb, data);

    bt = elm_button_add(bx);
    elm_object_text_set(bt, "Transparent indicator");
    evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0);
    evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0);
    evas_object_show(bt);
    elm_box_pack_end(bx, bt);
    evas_object_smart_callback_add(bt, "clicked", _transparent_indi_cb, data);

    bt = elm_button_add(bx);
    elm_object_text_set(bt, "Autohide indicator");
    evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0);
    evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0);
    evas_object_show(bt);
    elm_box_pack_end(bx, bt);
    evas_object_smart_callback_add(bt, "clicked", _autohide_indi_cb, data);
    return bx;
}

void indicator_cb(void *data, Evas_Object *obj, void *event_info)
{
    struct appdata *ad = (struct appdata *) data;
    Evas_Object *box;
    Evas_Object *win, *conform;
    win = ad->win_main;
    conform = ad->conform;

    box = _create_box(ad->nf, data);

    elm_naviframe_item_push(ad->nf, _("Indicator"), NULL, NULL, box, NULL);

}
