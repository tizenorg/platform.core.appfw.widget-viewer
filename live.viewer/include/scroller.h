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

extern Evas_Object *scroller_create(Evas_Object *parent);
extern Evas_Object *scroller_peek_by_idx(Evas_Object *sc, int idx);
extern int scroller_peek_by_obj(Evas_Object *sc, Evas_Object *obj);
extern int scroller_append(Evas_Object *sc, Evas_Object *child);
extern int scroller_get_current_idx(Evas_Object *sc);
extern int scroller_peek_by_obj(Evas_Object *sc, Evas_Object *obj);
extern Evas_Object *scroller_get_page(Evas_Object *sc, int idx);
extern int scroller_is_scrolling(Evas_Object *sc);

extern int scroller_add_stop_cb(Evas_Object *scroller, int (*cb)(Evas_Object *sc, void *data), void *data);
extern void scroller_del_stop_cb(Evas_Object *scroller, int (*cb)(Evas_Object *sc, void *data), void *data);

extern int scroller_get_page_index(Evas_Object *sc, Evas_Object *page);

extern void scroller_unlock(Evas_Object *sc);
extern void scroller_lock(Evas_Object *sc);

extern int scroller_get_page_count(Evas_Object *sc);
extern int scroller_scroll_to(Evas_Object *sc, int idx);
extern int scroller_jump_to(Evas_Object *sc, int idx);

extern int scroller_destroy(Evas_Object *sc);
extern int scroller_update(Evas_Object *sc, void *data);
extern int scroller_fast_scroll(Evas_Object *sc, int idx);
extern void scroller_loop_set(Evas_Object *sc, Eina_Bool val);
extern void scroller_quick_navi(Evas_Object *sc, Eina_Bool val);

/* End of a file */
