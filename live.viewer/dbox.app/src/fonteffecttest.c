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
  Font Effect Test
 ********************************************************/

static Evas_Object* _create_scroller(Evas_Object* parent)
{
	Evas_Object* scroller = elm_scroller_add(parent);
	elm_scroller_bounce_set(scroller, EINA_FALSE, EINA_TRUE);
	elm_scroller_policy_set(scroller,ELM_SCROLLER_POLICY_OFF,ELM_SCROLLER_POLICY_AUTO);
	evas_object_show(scroller);

	return scroller;
}

static Evas_Object* _create_fonteffecttest(Evas_Object* parent)
{
	Evas_Object *layout = NULL;

	layout = elm_layout_add(parent);
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "group_fonteffecttest");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

#if 0 // TC 0: Normal case
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
#endif

#if 0 // TC 1: Same font and size, Regular in a layout
	// textobject
	edje_object_part_text_set(_EDJ(layout), "text1", _("Regular,Italic,28 TO"));
	edje_object_part_text_set(_EDJ(layout), "text2", _("Regular,Normal,28 TO"));
	edje_object_part_text_set(_EDJ(layout), "text3", _("Regular,Italic,28 TO"));

	// textblock
	edje_object_part_text_set(_EDJ(layout), "textblock1", _("Regular,Normal,28 TB"));
	edje_object_part_text_set(_EDJ(layout), "textblock2", _("Regular,Italic,28 TB"));
	edje_object_part_text_set(_EDJ(layout), "textblock3", _("Regular,Normal,28 TB"));
	edje_object_part_text_set(_EDJ(layout), "textblock4", _("Regular,Italic,28 TB"));
#endif

#if 0 // TC 2: Same font and size, Medium in a layout
	// textobject
	edje_object_part_text_set(_EDJ(layout), "text1", _("Medium,Italic,30 TO"));
	edje_object_part_text_set(_EDJ(layout), "text2", _("Medium,Normal,30 TO"));
	edje_object_part_text_set(_EDJ(layout), "text3", _("Medium,Italic,30 TO"));

	// textblock
	edje_object_part_text_set(_EDJ(layout), "textblock1", _("Medium,Normal,30 TB"));
	edje_object_part_text_set(_EDJ(layout), "textblock2", _("Medium,Italic,30 TB"));
	edje_object_part_text_set(_EDJ(layout), "textblock3", _("Medium,Normal,30 TB"));
	edje_object_part_text_set(_EDJ(layout), "textblock4", _("Medium,Italic,30 TB"));
#endif

#if 0 // TC 3: Same font and different size, Medium in a layout
	// textobject
	edje_object_part_text_set(_EDJ(layout), "text1", _("Medium,Italic,30 TO"));
	edje_object_part_text_set(_EDJ(layout), "text2", _("Medium,Normal,35 TO"));
	edje_object_part_text_set(_EDJ(layout), "text3", _("Medium,Italic,40 TO"));

	// textblock
	edje_object_part_text_set(_EDJ(layout), "textblock1", _("Medium,Normal,25 TB"));
	edje_object_part_text_set(_EDJ(layout), "textblock2", _("Medium,Italic,30 TB"));
	edje_object_part_text_set(_EDJ(layout), "textblock3", _("Medium,Normal,35 TB"));
	edje_object_part_text_set(_EDJ(layout), "textblock4", _("Medium,Italic,40 TB"));
#endif

#if 0 // TC 4: Same font and different size, Medium in a layout
	// textobject
	edje_object_part_text_set(_EDJ(layout), "text1", _("Medium,Italic,30 TO"));
	edje_object_part_text_set(_EDJ(layout), "text2", _("Regular,Italic,35 TO"));
	edje_object_part_text_set(_EDJ(layout), "text3", _("Bold,Italic,40 TO"));

	// textblock
	edje_object_part_text_set(_EDJ(layout), "textblock1", _("Medium,Italic,25 TB"));
	edje_object_part_text_set(_EDJ(layout), "textblock2", _("Regular,Italic,30 TB"));
	edje_object_part_text_set(_EDJ(layout), "textblock3", _("Bold,Italic,35 TB"));
	edje_object_part_text_set(_EDJ(layout), "textblock4", _("Medium,Normal,40 TB"));
#endif

#if 0 // TC 5: Different font, different size, and same Italic style, Regular in a layout
	// textobject
	edje_object_part_text_set(_EDJ(layout), "text1", _("Arial,Regular,30 TO"));
	edje_object_part_text_set(_EDJ(layout), "text2", _("Georgia,Regular,35 TO"));
	edje_object_part_text_set(_EDJ(layout), "text3", _("Helvetica,Regular,40 TO"));

	// textblock
	edje_object_part_text_set(_EDJ(layout), "textblock1", _("Arial,Regular,25 TB"));
	edje_object_part_text_set(_EDJ(layout), "textblock2", _("Georgia,Regular,30 TB"));
	edje_object_part_text_set(_EDJ(layout), "textblock3", _("Helvetica,Regualr,35 TB"));
	edje_object_part_text_set(_EDJ(layout), "textblock4", _("Times,Regular,40 TB"));
#endif

#if 1 // TC 6: Different font, different size, and same Italic style, Regular and Bold in a layout
	// textobject
	edje_object_part_text_set(_EDJ(layout), "text1", _("Arial,Regular,I,30 TO"));
	edje_object_part_text_set(_EDJ(layout), "text2", _("Georgia,Regular,I,35 TO"));
	edje_object_part_text_set(_EDJ(layout), "text3", _("Helvetica,Bold,I,40 TO"));

	// textblock
	edje_object_part_text_set(_EDJ(layout), "textblock1", _("Arial,Regular,I,25 TB"));
	edje_object_part_text_set(_EDJ(layout), "textblock2", _("Georgia,Regular,I,30 TB"));
	edje_object_part_text_set(_EDJ(layout), "textblock3", _("Helvetica,Bold,I,35 TB"));
	edje_object_part_text_set(_EDJ(layout), "textblock4", _("Times,Regular,I,40 TB"));
#endif


	// strike
	edje_object_part_text_set(_EDJ(layout), "strike1", _("<strikethrough=on strikethrough_color=#FF0000>strike test</strikethrough>"));
	edje_object_part_text_set(_EDJ(layout), "strike2", _("<strike>strike simple tag test</strike>"));

	return layout;
}

void fonteffecttest_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad;
	Evas_Object *layout, *scroller;

	ad = (struct appdata *) data;
	if (ad == NULL) return;

	scroller = _create_scroller(ad->nf);
	elm_naviframe_item_push(ad->nf, _("Font Effect Test"), NULL, NULL, scroller, NULL);

	layout = _create_fonteffecttest(ad->nf);
	elm_object_content_set(scroller, layout);
}
