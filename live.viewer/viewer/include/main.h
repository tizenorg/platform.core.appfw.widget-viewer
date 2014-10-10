/*
 * Copyright 2012  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.tizenopensource.org/license
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#ifndef __MAIN_H__
#define __MAIN_H__

#if !defined(PKGNAME)
#  define PKGNAME "com.samsung.w-add-viewer"
#endif

#define S_(str) dgettext("sys_string", str)
#define T_(str) dgettext(PACKAGE, str)
#undef N_
#define N_(str) (str)
#undef _
#define _(str) gettext(str)

#define _EDJ(x) elm_layout_edje_get(x)
#define _X(x) (x*elm_config_scale_get())

/* Build */
#define HAPI __attribute__((visibility("hidden")))

struct appdata
{
	Evas_Object *win;
	Evas_Object *layout;
	Evas_Object *scroller_box;
	Ecore_Event_Handler *handler;
	int box_created;
};

#endif /* __MAIN_H__ */

