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

#define IMAGE_MAX 79
#define FONT_SIZE_INDEX_SMALL 24
#define FONT_SIZE_INDEX_NORMAL 35
#define FONT_SIZE_INDEX_LARGE 55
#define FONT_SIZE_INDEX_HUGE 75
#define FONT_SIZE_INDEX_GIANT 92
#define BASE_GENGRID_HEIGHT 76 //As per reference of UX of Genlist Group Index.
typedef struct _Testitem
{
	Elm_Object_Item *item;
	const char *path;
	int index;
	int checked;
} Testitem;

typedef enum {
	SIZE_INDEX_SMALL = 0,
	SIZE_INDEX_NORMAL,
	SIZE_INDEX_LARGE,
	SIZE_INDEX_HUGE,
	SIZE_INDEX_GIANT
} font_size_index;

static int total_count;
static Evas_Object *gengrid, *box;
static Elm_Gengrid_Item_Class *gic;
static Elm_Gengrid_Item_Class ggic;

char *group_names[] = {"A", "B", "C", "g", "y", NULL};

static Evas_Object *
grid_content_get(void *data, Evas_Object *obj, const char *part)
{
	Testitem *ti = (Testitem *)data;

	if (ti->index == 1) {
		if (!strcmp(part, "elm.swallow.end")) {
			Evas_Object *progressbar = elm_progressbar_add(obj);
			elm_object_style_set(progressbar, "process_medium");
			evas_object_size_hint_align_set(progressbar, EVAS_HINT_FILL, 0.5);
			evas_object_size_hint_weight_set(progressbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			evas_object_show(progressbar);
			elm_progressbar_pulse(progressbar, EINA_TRUE);
			return progressbar;
		} else return NULL;
	} else if (!strcmp(part, "elm.swallow.icon")) {
		Evas_Object *icon = elm_image_add(obj);
		elm_image_file_set(icon, ti->path, NULL);
		elm_image_aspect_fixed_set(icon, EINA_FALSE);
		evas_object_show(icon);
		return icon;
	}

	return NULL;
}

static void
_item_selected(void *data, Evas_Object *obj, void *event_info)
{
	printf("--------------------------\nSelected Items :");
}


static char *
grid_text_get(void *data, Evas_Object *obj, const char *part)
{
	int index = (int)data;
	if (!strcmp(part, "elm.text")) {
		return strdup(group_names[index]);
	}
	return NULL;
}

static void
_size_set_on_font_resize() {
	int x, h, w;
	font_size_index font_index;
	vconf_get_int(VCONFKEY_SETAPPL_ACCESSIBILITY_FONT_SIZE, &x);
	double scale = elm_config_scale_get();
	font_index = x;

	switch (font_index) {
		case SIZE_INDEX_SMALL :
			h = (int)((BASE_GENGRID_HEIGHT - FONT_SIZE_INDEX_NORMAL + FONT_SIZE_INDEX_SMALL + 1 /*extra pixel for overlapp check*/) * scale);
			break;
		case SIZE_INDEX_NORMAL :
			h = (int)(BASE_GENGRID_HEIGHT * scale);
			break;
		case SIZE_INDEX_LARGE :
			h = (int)((BASE_GENGRID_HEIGHT - FONT_SIZE_INDEX_NORMAL + FONT_SIZE_INDEX_LARGE) * scale);
			break;
		case SIZE_INDEX_HUGE :
			h = (int)((BASE_GENGRID_HEIGHT - FONT_SIZE_INDEX_NORMAL + FONT_SIZE_INDEX_HUGE) * scale);
			break;
		case SIZE_INDEX_GIANT :
			h = (int)((BASE_GENGRID_HEIGHT - FONT_SIZE_INDEX_NORMAL + FONT_SIZE_INDEX_GIANT) * scale);
			break;
		default:
			h = (int)(BASE_GENGRID_HEIGHT * scale ); //If no font size obtained, set the height to maximum
	}
	w = (int)(720 * scale);
	elm_gengrid_group_item_size_set(gengrid, w, h);
}

static void
_vconf_size_key_changed_cb()
{
	_size_set_on_font_resize();
}

static void
_create_gengrid (void *data)
{
	struct appdata *ad = (struct appdata *)data;
	int i, j, n, x, w, h;
	char buf[PATH_MAX];
	static Testitem ti[IMAGE_MAX*25];

	vconf_get_int(VCONFKEY_SETAPPL_ACCESSIBILITY_FONT_SIZE, &x);
	gengrid = elm_gengrid_add(ad->nf);
	evas_object_size_hint_weight_set(gengrid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(gengrid, EVAS_HINT_FILL, EVAS_HINT_FILL);

	double scale = elm_config_scale_get();
	w = h = (int)(177 * scale); //177 as per UX ver 1.7.
	elm_gengrid_item_size_set(gengrid, w, h);
	_size_set_on_font_resize();
	elm_gengrid_align_set(gengrid, 0.0, 0.0);
	elm_gengrid_horizontal_set(gengrid, EINA_FALSE);

	elm_gengrid_multi_select_set(gengrid, EINA_TRUE);

	gic = elm_gengrid_item_class_new();
	gic->item_style="default_grid";

	gic->func.text_get = NULL;
	gic->func.content_get = grid_content_get;
	gic->func.state_get = NULL;
	gic->func.del = NULL;

	ggic.item_style = "group_index";
	ggic.func.text_get = grid_text_get;
	ggic.func.content_get = NULL;
	ggic.func.state_get = NULL;
	ggic.func.del = NULL;

	for (j = 0; j < 25; j++) {
		for (i = 0; i < IMAGE_MAX; i++) {
			n = i+(j*IMAGE_MAX);
			snprintf(buf, sizeof(buf), "%s/grid_image/%d_raw.jpg", ICON_DIR, i+1);
			ti[n].index = n;
			ti[n].path = eina_stringshare_add(buf);
			if (i == 0 || i == 11 || i == 22 || i == 33 || i == 44)
			  ti[i].item = elm_gengrid_item_append(gengrid, &ggic, (void *)(i%10), NULL, NULL);
			else
			  ti[i].item = elm_gengrid_item_append(gengrid, gic, &(ti[n]), _item_selected, &(ti[i]));
			ti[n].checked = EINA_FALSE;
		}
	}

	total_count = n + 1;
	if (vconf_notify_key_changed(VCONFKEY_SETAPPL_ACCESSIBILITY_FONT_SIZE, _vconf_size_key_changed_cb, NULL) < 0) {
		printf("\nFail to register VCONFKEY_SETAPPL_SOUND_STATUS_BOOL key callback");
	}
}

static Eina_Bool
_pop_cb(void *data, Elm_Object_Item *it)
{
	elm_gengrid_clear(gengrid);
	evas_object_del(gengrid);
	gengrid = NULL;

	if (vconf_ignore_key_changed(VCONFKEY_SETAPPL_ACCESSIBILITY_FONT_SIZE, _vconf_size_key_changed_cb) < 0) {
		printf("\nFail to unregister VCONFKEY_SETAPPL_ACCESSIBILITY_FONT_SIZE key callback");
	}

	return EINA_TRUE;
}

void
gengrid_groupindex_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad;
	Elm_Object_Item *navi_it;

	ad = (struct appdata *)data;
	if (ad == NULL) return;

	_create_gengrid(ad);

	box = elm_box_add(ad->nf);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(box);
	elm_box_pack_end(box, gengrid);
	evas_object_show(gengrid);

	navi_it = elm_naviframe_item_push (ad->nf, _("Group Index Sample") , NULL, NULL, box, NULL);
	elm_naviframe_item_pop_cb_set(navi_it, _pop_cb, NULL);
}

