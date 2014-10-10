/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
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

#include "elmdemo_util.h"
#include "elmdemo_test.h"
#include "wallpaper.h"

#define IMAGE_MAX 28
#define EDIT_MODE 1

typedef struct _Testitem
{
	Elm_Object_Item *item;
	const char *path;
	const char *text;
	int index;
} Testitem;

typedef struct View_Data
{
	struct appdata *ad;
	int w;
	int h;
	char *type;
} View_Data;

static Eina_Bool longpressed = EINA_FALSE;
static Evas_Object *gengrid, *box;
static Elm_Gengrid_Item_Class *gic;


static Evas_Object *
grid_content_get(void *data, Evas_Object *obj, const char *part)
{
	Testitem *ti = (Testitem *)data;
	Elm_Object_Item *it = elm_gengrid_last_item_get(obj);
	int last_idx = elm_gengrid_item_index_get(it);

	if (ti->index == last_idx) {
		if (!strcmp(part, "elm.swallow.end")) {
			Evas_Object *progressbar = elm_progressbar_add(obj);
			elm_object_style_set(progressbar, "process_medium");
			evas_object_size_hint_align_set(progressbar, EVAS_HINT_FILL, 0.5);
			evas_object_show(progressbar);
			elm_progressbar_pulse(progressbar, EINA_TRUE);
			return progressbar;
		} else return NULL;
	} else if (!strcmp(part, "elm.swallow.icon")) {
		Evas_Object *icon = elm_image_add(obj);
		elm_image_file_set(icon, ti->path, NULL);
		elm_image_aspect_fixed_set(icon, EINA_FALSE);
		elm_image_preload_disabled_set(icon, EINA_FALSE);
		evas_object_show(icon);
		return icon;
	}

	return NULL;
}

static void
_item_selected(void *data, Evas_Object *obj, void *event_info)
{
	Testitem *ti = (Testitem *)data;
	int theme_index;

	elm_gengrid_item_selected_set(ti->item, EINA_FALSE);

	if (longpressed) {
		longpressed = EINA_FALSE;
		return;
	}

	theme_index = ea_theme_suitable_theme_get_from_image(ti->path);

	vconf_set_int(VCONFKEY_SETAPPL_CHANGE_UI_THEME_INT, theme_index);

	Eina_Bool result = ea_theme_input_colors_set(theme_index);
	if( result == EINA_TRUE)
	{
		ea_theme_system_colors_apply();
		change_config_owner();
	}
}

static void _realized(void *data, Evas_Object *obj, void *ei)
{
	printf("[%s][%d]\n", __func__, __LINE__);
}

static void
_create_gengrid (void *data, char *type)
{
	struct View_Data *vd = data;
	struct appdata *ad = vd->ad;
	int i, n, w, h;
	char buf[PATH_MAX];
	static Testitem ti[IMAGE_MAX+1];
	Evas_Object *win, *conform;

	gengrid = elm_gengrid_add(ad->nf);
	evas_object_size_hint_weight_set(gengrid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(gengrid, EVAS_HINT_FILL, EVAS_HINT_FILL);

	double scale = elm_config_scale_get();
	w = (int)(vd->w * scale); //177 as per UX ver 1.7.
	h = (int)(vd->h * scale);
	elm_gengrid_item_size_set(gengrid, w, h);
	elm_gengrid_align_set(gengrid, 0.5, 0.5);
	elm_gengrid_horizontal_set(gengrid, EINA_TRUE);
	elm_gengrid_multi_select_set(gengrid, EINA_TRUE);
	evas_object_smart_callback_add(gengrid, "realized", _realized, NULL);

	gic = elm_gengrid_item_class_new();
	gic->item_style=type;

	gic->func.text_get = NULL;
	gic->func.content_get = grid_content_get;
	gic->func.state_get = NULL;
	gic->func.del = NULL;

	if ((!ad->win_main) || (!ad->conform))
	{
		printf("[%s]:We can't get conformant\n", __FUNCTION__);
		return;
	}
	win     = ad->win_main;
	conform = ad->conform;

	elm_win_indicator_mode_set(win, ELM_WIN_INDICATOR_SHOW);
	elm_win_indicator_opacity_set(win, ELM_WIN_INDICATOR_TRANSPARENT);

	//if you change your layout to nooverlap_mode(to locate layout 0,0)
	//you should emit signal and set data of "nooverlap"
	elm_object_signal_emit(conform, "elm,state,indicator,overlap", "");
	evas_object_data_set(conform, "overlap", (void *)EINA_TRUE);

	for (i = 0; i <= IMAGE_MAX; i++) {
		n = i;
		snprintf(buf, sizeof(buf), "%s/wallpaper_image/%d_raw.jpg", ICON_DIR, i+1);
		ti[n].index = n;
		ti[n].path = eina_stringshare_add(buf);
		//TODO: check the func data
		ti[n].item = elm_gengrid_item_append(gengrid, gic, &(ti[n]), _item_selected, &(ti[n]));
	}
}

static Eina_Bool
_pop_cb(void *data, Elm_Object_Item *it)
{
	if (!data) return EINA_TRUE;
	View_Data *vd = data;

	elm_gengrid_clear(gengrid);
	evas_object_del(gengrid);
	gengrid = NULL;

	if (vd) free(vd);

	return EINA_TRUE;
}

void
wallpaper_gengrid_create_cb(void *data, char *type)
{
	struct appdata *ad;
	View_Data *vd;
	Elm_Object_Item *navi_it;
	Elm_Object_Item *it = NULL;

	vd = (struct View_Data *) data;
	if (vd == NULL) return;
	ad = vd->ad;

	_create_gengrid(vd, type);
	it = elm_gengrid_last_item_get(gengrid);
	elm_gengrid_item_show(it, ELM_GENGRID_ITEM_SCROLLTO_IN);

	box = elm_box_add(ad->nf);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(box);
	elm_box_pack_end(box, gengrid);
	evas_object_show(gengrid);

	navi_it = elm_naviframe_item_push (ad->nf, _("[Theme Change Wallpaper]") , NULL, NULL, box, NULL);
	elm_naviframe_item_pop_cb_set(navi_it, _pop_cb, vd);
}

void
wallpaper_theme_cb(void *data, Evas_Object *obj, void *event_info)
{
	View_Data *vd;
	vd = calloc(1, sizeof(View_Data));
	vd->ad = (struct appdata *)data;
	vd->w = 357;
	vd->h = 267;
	vd->type = "gallery";
	wallpaper_gengrid_create_cb(vd, vd->type);
}
