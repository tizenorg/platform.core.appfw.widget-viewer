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

/*********************************************************
  Font Test
 ********************************************************/

static Evas_Object* _create_scroller(Evas_Object* parent)
{
	Evas_Object* scroller = elm_scroller_add(parent);
	elm_scroller_bounce_set(scroller, EINA_FALSE, EINA_TRUE);
	elm_scroller_policy_set(scroller,ELM_SCROLLER_POLICY_OFF,ELM_SCROLLER_POLICY_AUTO);
	evas_object_show(scroller);

	return scroller;
}

static Evas_Object* _create_fonttest(Evas_Object* parent)
{
	Evas_Object *layout = NULL;
	double scale_factor = elm_config_scale_get();
	char buf[PATH_MAX] = {0, };

	layout = elm_layout_add(parent);
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "group_fonttest");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	// text
	edje_object_part_text_set(_EDJ(layout), "text1", _("Tizen:style=Bold TEXT"));
	edje_object_part_text_set(_EDJ(layout), "text2", _("Tizen:style=Roman TEXT"));
	edje_object_part_text_set(_EDJ(layout), "text3", _("Tizen:style=Medium TEXT"));

	// textblock
	edje_object_part_text_set(_EDJ(layout), "textblock1", _("Bold TEXTBLOCK"));
	edje_object_part_text_set(_EDJ(layout), "textblock2", _("<font color=#00FF00>Ro</font><font color=#0000FF>man</font> <font color=#FF0000><b>TEXT</b>BLOCK</font>"));
	snprintf(buf, sizeof(buf), "<font_size=35>Fo</font_size><font_size=45>nt Si</font_size><font_size=60>ze Test</font_size>");
	edje_object_part_text_set(_EDJ(layout), "textblock3", _(buf));
	snprintf(buf, sizeof(buf), "<font_size=%d>Fo</font_size><font_size=%d>nt Si</font_size><font_size=%d>ze Test</font_size>", (int)(35 * scale_factor), (int)(45 * scale_factor), (int)(60 * scale_factor));
	edje_object_part_text_set(_EDJ(layout), "textblock4", _(buf));

	// strike
	edje_object_part_text_set(_EDJ(layout), "strike1", _("<strikethrough=on strikethrough_color=#FF0000>strike test</strikethrough>"));
	edje_object_part_text_set(_EDJ(layout), "strike2", _("<strike>strike simple tag test</strike>"));

	return layout;
}

void fonttest_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad;
	Evas_Object *layout, *scroller;

	ad = (struct appdata *) data;
	if (ad == NULL) return;

	scroller = _create_scroller(ad->nf);
	elm_naviframe_item_push(ad->nf, _("Font Test"), NULL, NULL, scroller, NULL);

	layout = _create_fonttest(ad->nf);
	elm_object_content_set(scroller, layout);
}
