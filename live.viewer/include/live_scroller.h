/*
 * Copyright 2013  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.tizenopensource.org/license
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

struct live_sc_event_info {
	int curidx;
	int toidx;
};

struct live_sc_drag_info {
	int dx;
	int dy;
};

struct live_sc_move_info {
	Evas_Object *item;
	Evas_Coord x;
	Evas_Coord y;
	Evas_Coord w;
	Evas_Coord h;

	double relx;
	double rely;
};

extern Evas_Object *live_scroller_add(Evas_Object *parent);
extern int live_scroller_append(Evas_Object *scroller, Evas_Object *item);
extern Evas_Object *live_scroller_remove(Evas_Object *scroller, int idx);
extern Evas_Object *live_scroller_get_item(Evas_Object *scroller, int idx);
extern int live_scroller_get_current(Evas_Object *scroller);
extern int live_scroller_loop_set(Evas_Object *scroller, int is_loop);

extern int live_scroller_freeze(Evas_Object *scroller);
extern int live_scroller_thaw(Evas_Object *scroller);

extern int live_scroller_anim_to(Evas_Object *scroller, double fps, int offset);
extern int live_scroller_go_to(Evas_Object *scroller, int idx);

extern int live_scroller_update(Evas_Object *scroller);

extern int live_scroller_remove_by_obj(Evas_Object *scroller, Evas_Object *obj);
extern int live_scroller_get_item_index(Evas_Object *scroller, Evas_Object *item);
extern int live_scroller_get_item_count(Evas_Object *scroller);

/* End of a file */
