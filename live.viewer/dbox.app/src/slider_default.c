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
#include "slider.h"

/*********************************************************
   Slider_Default
 ********************************************************/

#define SLIDER_POPUP_X 438
#define SLIDER_POPUP_Y 96
#define NUM_OF_ITEMS 14

static Elm_Genlist_Item_Class itc, itc2;

static char *slider_itemlist[] = {
	"Slider Normal Style",
	"Text-Slider-Icon",
	"Icon-Slider-Icon",
	"Icon-Text-Slider-Text",
	"Text Slider Text",
	"Number Slider Number",
	"Slider for ebook",
	NULL
};

static Evas_Object *slider_popup;

static void _slider_ebook_drag_start_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *slider = (Evas_Object*)data;
	char buf[50];
	Evas_Coord x, y, w, h;
	int pos_x, pos_y;
	double min, max;

	evas_object_geometry_get(slider, &x, &y, &w, &h);
	pos_x = x + (w / 2) - (int)(((double)SLIDER_POPUP_X * elm_config_scale_get()) / 2);
	pos_y = y - (int)((double)SLIDER_POPUP_Y * elm_config_scale_get());
	evas_object_move(slider_popup, pos_x, pos_y);
	elm_slider_min_max_get(slider, &min, &max);
	sprintf(buf, "%.0lf of %.0lf", elm_slider_value_get(slider), max);
	edje_object_part_text_set(_EDJ(slider_popup), "page", buf);
	evas_object_show(slider_popup);
}

static void _slider_ebook_change_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *slider = (Evas_Object*)data;
	char buf[50];
	Evas_Coord x, y, w, h;
	int pos_x, pos_y;
	double min, max;

	evas_object_geometry_get(slider, &x, &y, &w, &h);
	pos_x = x + (w / 2) - (int)(((double)SLIDER_POPUP_X * elm_config_scale_get()) / 2);
	pos_y = y - (int)((double)SLIDER_POPUP_Y * elm_config_scale_get());
	evas_object_move(slider_popup, pos_x, pos_y);
	elm_slider_min_max_get(slider, &min, &max);
	sprintf(buf, "%.0lf of %.0lf", elm_slider_value_get(slider), max);
	edje_object_part_text_set(_EDJ(slider_popup), "page", buf);
}

static void _slider_ebook_drag_stop_cb(void *data, Evas_Object *obj, void *event_info)
{
	evas_object_hide(slider_popup);
}

static Eina_Bool _pop_cb(void *data, Elm_Object_Item *it)
{
	evas_object_del(slider_popup);
	return EINA_TRUE;
}

static char *_gl_text_get(void *data, Evas_Object *obj, const char *part)
{
	int index = ((int)(long)data) / 2;
	return strdup(slider_itemlist[index]);
}

static void _parse_negative_zero(char *str)
{
        char *ptr, *str_temp;

        ptr = strstr(str, "-0");
        if (ptr) {
                str_temp = ptr++;
                while (*ptr++) {
                        if (*ptr == '0' || *ptr == '.')
                                continue;
                        else if (*ptr >='1' && *ptr <='9')
                                break;
                        else {
                                while (*str_temp) {
                                        *str_temp = *(str_temp+1);
					str_temp++;
				}
                                break;
                        }
                }
        }
}

static char* _indicator_format(double val)
{
        char *indicator = malloc(sizeof(char) * 64);
        if (!indicator) return NULL;
        snprintf(indicator, 64, "%1.0f", val);

        // Check and remove -0 state
        _parse_negative_zero(indicator);

        return indicator;
}

static void _indicator_free(char *str)
{
        free(str);
}

static double _step_size_calculate(Evas_Object *obj, double min, double max)
{
	double step = 0.0;
	int steps = 0;

	steps = max - min;
	if (steps) step = (1.0 / steps);
	return step;
}

static Evas_Object *_gl_content_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *slider, *icon, *icon_end;
	int index = ((int)(long)data);
	double step;

	slider = elm_slider_add(obj);
	elm_slider_indicator_show_set(slider, EINA_TRUE);
	evas_object_size_hint_weight_set(slider, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(slider, EVAS_HINT_FILL, 0.5);
	elm_slider_indicator_format_set(slider, "%1.0f");

	if (index == 1) {
		elm_slider_indicator_format_function_set(slider, _indicator_format, _indicator_free);
		elm_object_style_set(slider, "center_point");
		elm_slider_min_max_set(slider, -5, 5);
		step = _step_size_calculate(slider, -5, 5);
		elm_slider_step_set(slider, step);
		elm_slider_value_set(slider, 0);
	} else if (index == 3) {
		elm_slider_min_max_set(slider, 0, 9);
		step = _step_size_calculate(slider, 0, 9);
		elm_slider_step_set(slider, step);
		elm_slider_value_set(slider, 7);
		icon = elm_image_add(obj);
		elm_image_file_set(icon, ICON_DIR"/00_slider_button_brightness_01.png", NULL);
		evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
		elm_object_content_set(slider,icon);
		icon_end = elm_image_add(obj);
		elm_image_file_set(icon_end, ICON_DIR"/00_slider_button_brightness_02.png", NULL);
		evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
		elm_object_part_content_set(slider, "end", icon_end);
	} else if (index == 5) {
		icon = elm_image_add(obj);
		elm_image_file_set(icon, ICON_DIR"/00_slider_button_volume_01.png", NULL);
		evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
		icon_end = elm_image_add(obj);
		elm_image_file_set(icon_end, ICON_DIR"/00_slider_button_volume_02.png", NULL);
		evas_object_size_hint_aspect_set(icon_end, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
		elm_object_content_set(slider,icon);
		elm_object_part_content_set(slider, "end", icon_end);
		elm_slider_min_max_set(slider, 0, 9);
		step = _step_size_calculate(slider, 0, 9);
		elm_slider_step_set(slider, step);
		elm_slider_value_set(slider, 2);
	} else if (index == 7) {
		icon = elm_image_add(obj);
		elm_image_file_set(icon, ICON_DIR"/00_slider_button_volume_01.png", NULL);
		evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
		icon_end = elm_image_add(obj);
		elm_image_file_set(icon_end, ICON_DIR"/00_slider_button_volume_02.png", NULL);
		evas_object_size_hint_aspect_set(icon_end, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
		elm_object_content_set(slider,icon);
		elm_object_text_set(slider, "Volume");
		elm_object_part_content_set(slider, "end", icon_end);
		elm_slider_min_max_set(slider, 0, 9);
		step = _step_size_calculate(slider, 0, 9);
		elm_slider_step_set(slider, step);
		elm_slider_value_set(slider, 5);
	} else if (index == 9) {
		elm_object_style_set(slider, "textstyle");
		elm_object_text_set(slider,"Minimum");
		elm_object_part_text_set(slider,"elm.text.end","Maximum");
		elm_slider_indicator_show_set(slider, EINA_TRUE);
		elm_slider_min_max_set(slider, 0, 9);
		step = _step_size_calculate(slider, 0, 9);
		elm_slider_step_set(slider, step);
		elm_slider_value_set(slider, 5);
	} else if (index == 11) {
		elm_object_style_set(slider, "numberstyle");
		elm_object_text_set(slider,"0");
		elm_object_part_text_set(slider,"elm.text.end","20");
		elm_slider_indicator_show_set(slider, EINA_TRUE);
		elm_slider_min_max_set(slider, 0, 20);
		step = _step_size_calculate(slider, 0, 20);
		elm_slider_step_set(slider, step);
		elm_slider_value_set(slider, 5);
	} else if (index == 13) {
		elm_slider_indicator_show_set(slider, EINA_FALSE);
		elm_layout_signal_emit(slider, "slider,center,point,show", "");
		elm_slider_min_max_set(slider, 1, 427);
		elm_object_style_set(slider, "ebook");
		evas_object_smart_callback_add(slider, "slider,drag,start", _slider_ebook_drag_start_cb, slider);
		evas_object_smart_callback_add(slider, "slider,drag,stop", _slider_ebook_drag_stop_cb, slider);
		evas_object_smart_callback_add(slider, "changed", _slider_ebook_change_cb, slider);
	}

	return slider;
}

static void _realized(void *data, Evas_Object *obj, void *ei)
{
	Eina_List *items = NULL;

	if (!ei) return;
	Elm_Object_Item *item = ei;

	const Elm_Genlist_Item_Class *itc = elm_genlist_item_item_class_get(item);
	if (!strcmp(itc->item_style, "dialogue/1icon")) {
		/* unregister item itself */
		elm_object_item_access_unregister(item);

		/* convey highlight to its content */
		Evas_Object *content;
		content = elm_object_item_part_content_get(item, "elm.icon");
		items = eina_list_append(items, content);
		elm_object_item_access_order_set(item, items);
	}
}

static Evas_Object* _create_genlist(Evas_Object* parent)
{;
	Evas_Object *genlist;
	Elm_Object_Item *gl_item = NULL;
	int index;

	genlist = elm_genlist_add(parent);

	itc.item_style = "dialogue/1text";
	itc.func.text_get = _gl_text_get;
	itc.func.content_get = NULL;
	itc.func.state_get = NULL;
	itc.func.del = NULL;

	itc2.item_style = "dialogue/1icon";
	itc2.func.text_get = NULL;
	itc2.func.content_get = _gl_content_get;
	itc2.func.state_get = NULL;
	itc2.func.del = NULL;

	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, 0.0);
	evas_object_show(genlist);

	for (index = 0; index < NUM_OF_ITEMS; index++) {
		if (index % 2 == 0) {
			gl_item = elm_genlist_item_append(genlist, &itc, (void *) index, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
			elm_genlist_item_select_mode_set(gl_item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		} else {
			gl_item = elm_genlist_item_append(genlist, &itc2, (void *) index, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
			elm_genlist_item_select_mode_set(gl_item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		}
	}

	evas_object_smart_callback_add(genlist, "realized", _realized, NULL);
	return genlist;
}

void slider_default_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *genlist;
	struct appdata *ad = (struct appdata *) data;
	Elm_Object_Item *it;
	if (ad == NULL) return;

	genlist = _create_genlist(ad->nf);
	it = elm_naviframe_item_push(ad->nf, _("Default Style"), NULL, NULL, genlist, NULL);
	elm_naviframe_item_pop_cb_set(it, _pop_cb, NULL);
	// make a popup for showing the page change of ebook style slider
	slider_popup = elm_layout_add(ad->nf);
	elm_layout_file_set(slider_popup, ELM_DEMO_EDJ, "slider_popup");
	evas_object_resize(slider_popup,
			(int)((double)SLIDER_POPUP_X * elm_config_scale_get()),
			(int)((double)SLIDER_POPUP_Y * elm_config_scale_get()));
	edje_object_part_text_set(_EDJ(slider_popup), "title", _("Chapter 5. Here is my secret story"));
}

