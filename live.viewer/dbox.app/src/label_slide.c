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
#include "label.h"
#include "label_slide.h"

/*********************************************************
 label
 ********************************************************/


static void
_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *label = (Evas_Object *)data;
	const char* text;
	text = elm_object_text_get(obj);
	if (strcmp(text,"Start Sliding") == 0)
	{
		while (label != NULL)
			{
				label = evas_object_data_get(label, "next_label");
				elm_label_slide_mode_set(label, ELM_LABEL_SLIDE_MODE_AUTO);
				elm_label_slide_go(label);
			}
		elm_object_text_set(obj, "Stop Sliding");
	}
	else
	{
		while (label != NULL)
			{
				label = evas_object_data_get(label, "next_label");
				elm_label_slide_mode_set(label, ELM_LABEL_SLIDE_MODE_NONE);
				elm_label_slide_go(label);
			}
		elm_object_text_set(obj, "Start Sliding");
	}
}

static Evas_Object* _create_labels(Evas_Object* parent)
{
	Evas_Object* layout = elm_layout_add(parent);
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "elmdemo-test/label_slide");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	Evas_Object *label_long_5sec;
	label_long_5sec = elm_label_add(layout);
	elm_object_style_set(label_long_5sec, "slide_long");
	elm_object_text_set(label_long_5sec, "<color=#4D554BFF>This is a sample long text for sliding demo in elm label.</color>");
	elm_object_part_content_set(layout, "swallow_label_1", label_long_5sec);
	elm_label_wrap_width_set(label_long_5sec, 100);
	elm_label_slide_duration_set(label_long_5sec, 5);
	elm_label_slide_mode_set(label_long_5sec, ELM_LABEL_SLIDE_MODE_ALWAYS);

	Evas_Object *label_long_10sec;
	label_long_10sec = elm_label_add(layout);
	elm_object_style_set(label_long_10sec, "slide_long");
	elm_object_text_set(label_long_10sec, "<color=#4D554BFF>This is a sample long text for sliding demo in elm label.</color>");
	elm_object_part_content_set(layout, "swallow_label_2", label_long_10sec);
	elm_label_wrap_width_set(label_long_10sec, 100);
	elm_label_slide_duration_set(label_long_10sec, 10);
	elm_label_slide_mode_set(label_long_10sec, ELM_LABEL_SLIDE_MODE_ALWAYS);

	Evas_Object *label_short;
	label_short = elm_label_add(layout);
	elm_object_style_set(label_short, "slide_short");
	elm_object_text_set(label_short, "<color=#4D554BFF>This is a sample long text for sliding demo in elm label.</color>");
	elm_object_part_content_set(layout, "swallow_label_3", label_short);
	elm_label_wrap_width_set(label_short, 100);
	elm_label_slide_duration_set(label_short, 5);
	elm_label_slide_mode_set(label_short, ELM_LABEL_SLIDE_MODE_ALWAYS);

	Evas_Object *label_bounce;
	label_bounce = elm_label_add(layout);
	elm_object_style_set(label_bounce, "slide_bounce");
	elm_object_text_set(label_bounce, "<color=#4D554BFF>This is a sample long text for sliding demo in elm label.</color>");
	elm_object_part_content_set(layout, "swallow_label_4", label_bounce);
	elm_label_wrap_width_set(label_bounce, 100);
	elm_label_slide_duration_set(label_bounce, 3);
	elm_label_slide_mode_set(label_bounce, ELM_LABEL_SLIDE_MODE_ALWAYS);

	Evas_Object *label_rollShort;
	label_rollShort = elm_label_add(layout);
	elm_object_style_set(label_rollShort, "slide_roll");
	elm_object_text_set(label_rollShort, "<color=#4D554BFF>This is a sample roll style.</color>");
	elm_object_part_content_set(layout, "swallow_label_5", label_rollShort);
	elm_label_wrap_width_set(label_rollShort, 100);
	elm_label_slide_mode_set(label_rollShort, ELM_LABEL_SLIDE_MODE_ALWAYS);

	Evas_Object *label_rollLong;
	label_rollLong = elm_label_add(layout);
	elm_object_style_set(label_rollLong, "slide_roll");
	elm_object_text_set(label_rollLong, "<color=#4D554BFF>This is a sample long text for slide roll style demo in elm label.</color>");
	elm_object_part_content_set(layout, "swallow_label_6", label_rollLong);
	elm_label_wrap_width_set(label_rollLong, 100);
	elm_label_slide_mode_set(label_rollLong, ELM_LABEL_SLIDE_MODE_ALWAYS);

	evas_object_data_set(label_long_5sec, "next_label", label_long_10sec);
	evas_object_data_set(label_long_10sec, "next_label", label_short);
	evas_object_data_set(label_short, "next_label", label_bounce);
	evas_object_data_set(label_bounce, "next_label", label_rollShort);
	evas_object_data_set(label_rollShort, "next_label", label_rollLong);

	Evas_Object *btn;
	btn = elm_button_add(layout);
	elm_object_text_set(btn, "Start Sliding");
	evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_smart_callback_add(btn, "clicked", _btn_clicked_cb, label_long_5sec);
	elm_object_part_content_set(layout, "swallow_btn", btn);

	return layout;
}

static Evas_Object* _create_scroller(Evas_Object* parent)
{
	Evas_Object* scroller = elm_scroller_add(parent);
	elm_scroller_bounce_set(scroller, EINA_FALSE, EINA_TRUE);
	elm_scroller_policy_set(scroller,ELM_SCROLLER_POLICY_OFF,ELM_SCROLLER_POLICY_AUTO);
	evas_object_show(scroller);

	return scroller;
}

void label_slide_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *scroller, *layout_inner;
	struct appdata *ad = (struct appdata *) data;
	if (ad == NULL) return;

	scroller = _create_scroller(ad->nf);
	elm_naviframe_item_push(ad->nf, _("Slide Text"), NULL, NULL, scroller, NULL);

	layout_inner = _create_labels(ad->nf);
	elm_object_content_set(scroller, layout_inner);
}
