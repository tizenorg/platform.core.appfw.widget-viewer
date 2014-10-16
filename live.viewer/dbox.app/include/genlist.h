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

#ifndef __DEF_GENLIST_H__
#define __DEF_GENLIST_H__

#include <Elementary.h>

void genlist_cb(void *data, Evas_Object *obj, void *event_info);
void genlist_check_cb(void *data, Evas_Object *obj, void *event_info);
void genlist_edit_mode_cb(void *data, Evas_Object *obj, void *event_info);
void genlist_externals_cb(void *data, Evas_Object *obj, void *event_info);
void genlist_fastscroll_cb(void *data, Evas_Object *obj, void *event_info);
void genlist_index_fastscroll_cb(void *data, Evas_Object *obj, void *event_info);
void genlist_index_expandable_fastscroll_cb(void *data, Evas_Object *obj, void *event_info);
void genlist_music_player_style_cb(void *data, Evas_Object *obj, void *event_info);
void genlist_pinchzoom_cb(void *data, Evas_Object *obj, void *event_info);
void genlist_radio_cb(void *data, Evas_Object *obj, void *event_info);
void genlist_scroll_jump_cb(void *data, Evas_Object *obj, void *event_info);
void genlist_normal_cb(void *data, Evas_Object *obj, void *event_info);
void genlist_swipe_cb(void *data, Evas_Object *obj, void *event_info);
void genlist_variable_height_cb(void *data, Evas_Object *obj, void *event_info);
void genlist_variable_width_cb(void *data, Evas_Object *obj, void *event_info);
void genlist_color_cb(void *data, Evas_Object *obj, void *event_info);
void genlist_separator_cb(void *data, Evas_Object *obj, void *event_info);

void genlist_dialogue_cb(void *data, Evas_Object *obj, void *event_info);
void genlist_dialogue_item_cb(void *data, Evas_Object *obj, void *event_info);
void genlist_dialogue_appitem_cb(void *data, Evas_Object *obj, void *event_info);
void genlist_editfield_cb(void *data, Evas_Object *obj, void *event_info);
void genlist_collection_cb(void *data, Evas_Object *obj, void *event_info);

void genlist_dialogue_edit_cb(void *data, Evas_Object *obj, void *event_info);
void genlist_dialogue_contact_edit_cb(void *data, Evas_Object *obj, void *event_info);
void genlist_expandable_cb(void *data, Evas_Object *obj, void *event_info);
void genlist_ndepth_expandable_cb(void *data, Evas_Object *obj, void *event_info);

void genlist_theme_default_cb(void *data, Evas_Object *obj, void *event_info);
void genlist_theme_no_effect_cb(void *data, Evas_Object *obj, void *event_info);

void genlist_homogenous_cb(void *data, Evas_Object *obj, void *event_info);
void genlist_animator_cb(void *data, Evas_Object *obj, void *event_info);

void list_cb(void *data, Evas_Object *obj, void *event_info);

char** genlist_get_demo_names(void);
char** genlist_get_demo_country_names(void);

#define NUM_OF_GENLIST_DEMO_NAMES 100
#define NUM_OF_GENLIST_DEMO_COUNTRY_NAMES 50
#define NUM_OF_GENLIST_LONG_TEXTS 50
#define NUM_OF_GENLIST_TIMES 10

#endif /* __DEF_GENLIST_H__ */

