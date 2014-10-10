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

#include "elmdemo_util.h"
#include "elmdemo_test.h"
#include "colorclass.h"

struct ccdata {
	struct appdata *ad;
	char *name;
	Evas_Object *rect;
	Evas_Object *box;
	int r, g, b, a;
};

static void _button_clicked(void *data, Evas_Object *obj, void *event_info)
{
	struct ccdata *cd = (struct ccdata *)data;

	elm_config_color_overlay_set(cd->name, cd->r, cd->g, cd->b, cd->a, 0, 0, 0, 0, 0, 0, 0, 0);
	elm_config_color_overlay_apply();
	elm_config_all_flush();
	elm_config_save();

	elm_naviframe_item_pop(cd->ad->nf);
	free(cd->name);
	free(cd);
}

static void _slider_changed(void *data, Evas_Object *obj, void *event_info)
{
	struct ccdata *cd = (struct ccdata *)data;
	char *color = evas_object_data_get(obj, "color");
	Evas_Object *lb = evas_object_data_get(obj, "label");
	char buf[256] = {0,};
	int value;

	value = (int)elm_slider_value_get(obj);
	if (!strcmp(color, "red"))
	{
		cd->r = value;
		sprintf(buf, "<color=#FF0000> R: %d</color>", value);
	}
	else if (!strcmp(color, "green"))
	{
		cd->g = value;
		sprintf(buf, "<color=#00FF00> G: %d</color>", value);
	}
	else if (!strcmp(color, "blue"))
	{
		cd->b = value;
		sprintf(buf, "<color=#0000FF> B: %d</color>", value);
	}
	else if (!strcmp(color, "alpha"))
	{
		cd->a = value;
		sprintf(buf, "<color=#FFFFFF> A: %d</color>", value);
	}
	elm_object_text_set(lb, buf);
	evas_object_color_set(cd->rect, cd->r, cd->g, cd->b, cd->a);
}

void _color_class_changed(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	int r, g, b, a;
	edje_color_class_get(source, &r, &g, &b, &a, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
	printf("[%s][%d] name: %s, color: %d, %d, %d, %d\n", __FUNCTION__, __LINE__, source, r, g, b, a);
}

static void _create_color_slider(struct ccdata *cd, const char *color)
{
	Evas_Object *sl, *lb;
	char buf[256] = {0,};

	if (!strcmp(color, "red"))
		sprintf(buf, "<color=#FF0000> R: %d</color>", cd->r);
	else if (!strcmp(color, "green"))
		sprintf(buf, "<color=#00FF00> G: %d</color>", cd->g);
	else if (!strcmp(color, "blue"))
		sprintf(buf, "<color=#0000FF> B: %d</color>", cd->b);
	else if (!strcmp(color, "alpha"))
		sprintf(buf, "<color=#FFFFFF> A: %d</color>", cd->a);

	lb = elm_label_add(cd->box);
	elm_object_text_set(lb, buf);
	evas_object_size_hint_weight_set(lb, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(lb, EVAS_HINT_FILL, 0.5);
	elm_box_pack_end(cd->box, lb);
	evas_object_show(lb);

	sl = elm_slider_add(cd->box);
	elm_slider_indicator_show_set(sl, EINA_FALSE);
	evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
	evas_object_data_set(sl, "color", color);
	evas_object_data_set(sl, "label", lb);
	elm_slider_min_max_set(sl, 0, 255);
	evas_object_smart_callback_add(sl, "changed", _slider_changed, cd);
	elm_box_pack_end(cd->box, sl);
	evas_object_show(sl);

	if (!strcmp(color, "red"))
		elm_slider_value_set(sl, cd->r);
	else if (!strcmp(color, "green"))
		elm_slider_value_set(sl, cd->g);
	else if (!strcmp(color, "blue"))
		elm_slider_value_set(sl, cd->b);
	else if (!strcmp(color, "alpha"))
		elm_slider_value_set(sl, cd->a);
}

static Evas_Object *_create_sliders(struct ccdata *cd)
{
	Evas_Object *box, *sc, *lb, *rect;

	sc = elm_scroller_add(cd->ad->nf);
	elm_scroller_bounce_set(sc, EINA_FALSE, EINA_TRUE);
	elm_scroller_policy_set(sc, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	evas_object_show(sc);

	box = elm_box_add(sc);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_content_set(sc, box);
	evas_object_show(box);
	cd->box = box;

	lb = elm_label_add(box);
	elm_object_text_set(lb, " Current Color");
	evas_object_size_hint_weight_set(lb, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(lb, EVAS_HINT_FILL, 0.5);
	elm_box_pack_end(box, lb);
	evas_object_show(lb);

	// set an edje signal and get the current color using elm_layout object 
	elm_layout_signal_callback_add(lb, "color_class,set", "", _color_class_changed, NULL);
	edje_object_color_class_get(elm_layout_edje_get(lb), cd->name,
			&cd->r, &cd->g, &cd->b, &cd->a, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

	rect = evas_object_rectangle_add(evas_object_evas_get(box));
	evas_object_size_hint_weight_set(rect, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(rect, EVAS_HINT_FILL, 0.5);
	evas_object_size_hint_min_set(rect, 0, 150);
	evas_object_color_set(rect, cd->r, cd->g, cd->b, cd->a);
	elm_box_pack_end(box, rect);
	evas_object_show(rect);
	cd->rect = rect;

	_create_color_slider(cd, "red");
	_create_color_slider(cd, "green");
	_create_color_slider(cd, "blue");
	_create_color_slider(cd, "alpha");

	return sc;
}

static void _item_clicked(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *it = (Elm_Object_Item *)event_info;
        Elm_Object_Item *navi_it;
	struct appdata *ad = (struct appdata *)data;
	struct ccdata *cd = NULL;
	Evas_Object *ly, *tb;
	char *text;

	if (ad == NULL) return;

	cd = calloc(1, sizeof(struct ccdata));
	if (!cd) return;

	text = strdup(elm_object_item_text_get(it));
	if (!text)
	{
		free(cd);
		return;
	}

	cd->ad = ad;
	cd->name = strdup(strtok(text, " :"));

	ly = _create_sliders(cd);
	navi_it = elm_naviframe_item_push(ad->nf, cd->name, NULL, NULL, ly, NULL);

	tb = elm_toolbar_add(ad->nf);
	elm_toolbar_shrink_mode_set(tb, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_transverse_expanded_set(tb, EINA_TRUE);
	elm_toolbar_select_mode_set(tb, ELM_OBJECT_SELECT_MODE_NONE);
	elm_toolbar_item_append(tb, NULL, "Set", _button_clicked, cd);
	elm_object_item_part_content_set(navi_it, "toolbar", tb);

	free(text);
}

static void _list_click(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *it = (Elm_Object_Item *) elm_list_selected_item_get(obj);

	if (it == NULL)
	{
		fprintf((LOG_PRI(LOG_ERR) == LOG_ERR?stderr:stdout), "List item is NULL.\n");
		return;
	}

	elm_list_item_selected_set(it, EINA_FALSE);
}

static Evas_Object* _create_list_winset(struct appdata* ad)
{
	Evas_Object *li;
	Eina_List *cl, *l;
	Elm_Color_Class *cc;
	char buf[128];

	li = elm_list_add(ad->nf);
	elm_list_mode_set(li, ELM_LIST_COMPRESS);
	evas_object_smart_callback_add(li, "selected", _list_click, NULL);

	cl = elm_config_color_classes_list_get();
	EINA_LIST_FOREACH(cl, l, cc)
	{
		snprintf(buf, 128, "%s : %s", cc->name, cc->desc);
		elm_list_item_append(li, buf, NULL, NULL,
				_item_clicked, ad);
	}
	elm_config_color_classes_list_free(cl);

	elm_list_go(li);

	return li;
}

void colorclass_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *list;
	struct appdata *ad = (struct appdata *) data;
	if (ad == NULL) return;

	list = _create_list_winset(ad);
	elm_naviframe_item_push(ad->nf, _("Color Class"), NULL, NULL, list, NULL);
}
