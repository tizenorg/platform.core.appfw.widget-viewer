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


#ifndef __DEF_elm_demo_H_
#define __DEF_elm_demo_H_

#include <Elementary.h>

#define _EDJ(x) elm_layout_edje_get(x)

typedef enum _MOVING_TYPE
{
	NONE,
	FLICK_L2R,
	FLICK_R2L,
} MOVING_TYPE;

struct _Touch_Info
{
   Evas_Coord cur_x, cur_y, cur_mx, cur_my;
   Evas_Coord prev_x, prev_y, prev_mx, prev_my;

   Eina_Bool multitouch_detected : 1;
   Eina_Bool touchtype_detected : 1;

   int moving_type;
   int magic_num;
};
typedef struct _Touch_Info Touch_Info;

struct appdata
{
	Evas_Coord root_w;
	Evas_Coord root_h;

	Evas *evas;
	Evas_Object *win_main;
	Evas_Object *bg;
	Evas_Object *conform;
	Evas_Object *layout_main;
	Evas_Object *nf;

	Touch_Info ti;
};

struct _menu_item {
	char *name;
	void (*func)(void *data, Evas_Object *obj, void *event_info);
};

struct _group_menu_item {
	Eina_Bool is_group;
	char *name;
	void (*func)(void *data, Evas_Object *obj, void *event_info);
};


int init_elm_demo(struct appdata *ad);

#endif /* __DEF_elm_demo_H__ */
