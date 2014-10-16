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
#include "label_ellipsis.h"

/*********************************************************
 label
 ********************************************************/
static Evas_Object* _create_labels(Evas_Object* parent)
{
	Evas_Object* layout = elm_layout_add(parent);
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "elmdemo-test/label");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	Evas_Object *label;
	label = elm_label_add(layout);
	elm_object_part_content_set(layout, "label1", label);
	elm_object_text_set(label, _("<font_size=24>If the string length exceeds the width</font_size>"));
	elm_label_ellipsis_set(label, EINA_TRUE);
	elm_label_wrap_width_set(label, 300);

	Evas_Object *label2;
	label2 = elm_label_add(layout);
	elm_object_part_content_set(layout, "label2", label2);
	elm_object_text_set(label2, _("<font_size=24>If the string length exceeds the width of the</font_size>"));
	elm_label_ellipsis_set(label2, EINA_TRUE);
	elm_label_wrap_width_set(label2, 300);

	Evas_Object *label3;
	label3 = elm_label_add(layout);
	elm_object_part_content_set(layout, "label3", label3);
	elm_object_text_set(label3, _("<font_size=24>If the string length exceeds the width of the widget, then the string will be ellipsised by label. but it's a exprimental api and very slow function.</font_size>"));
	elm_label_wrap_width_set(label3, 300);
	elm_label_ellipsis_set(label3, EINA_TRUE);

	Evas_Object *label4;
	label4 = elm_label_add(layout);
	elm_object_part_content_set(layout, "label4", label4);
	elm_object_text_set(label4, _("<font_size=24>If the string length exceeds the width of the widget, then the string will be ellipsised by label. but it's a exprimental api and very slow function.</font_size>"));
	elm_label_wrap_width_set(label4, 300);
	elm_label_ellipsis_set(label4, EINA_TRUE);

	Evas_Object *label5;
	label5 = elm_label_add(layout);
	elm_object_part_content_set(layout, "label5", label5);
	elm_object_text_set(label5, _("If the string length exceeds the width of the widget, then the string will be ellipsised by label. However, it's a exprimental api and very slow function. Ellipsis feature supports multiline texts. Programmers should not misuse multiline ellipsis. It's slow a function."));
	elm_label_wrap_width_set(label5, 300);
	elm_label_ellipsis_set(label5, EINA_TRUE);

	/*
	elm_theme_extension_add(NULL, ELM_DEMO_EDJ");
	Evas_Object *label5;
	label5 = elm_label_add(layout);
	elm_object_style_set(label5, "extended/wrap_ellipsis");
	elm_object_part_content_set(layout, "label5", label5);
	elm_label_line_wrap_set(label5, 1);
	elm_object_text_set(label5, _("If the string length exceeds the width of the label, the font size is changed automatically. kdbGetKey(file/contact/display_order) failed(Resource temporarily unavailable) kdb Get(file/contact/display_order) failed ERR:e_dbus e_dbus_signal.c:71 cb_name_owner() Error: org.freedesktop.DBus.Errore [TAPI Library] [PID: 2384, tapi_send_request:170]  D-Bus API failure: errCode[0 [ 1198.769543] max8998 4-0066: Power Button Pushed (0x80) [keyrouter][DeliverDeviceKeyEvents] Non-grabbed key! Deliver KeyPress (keycode: [dix] Could not init font path element /usr/share/fonts/X11/cyrillic, removing from list!"));
	elm_label_wrap_width_set(label5, 300);
	elm_label_wrap_height_set(label5, 120);
	elm_label_ellipsis_set(label5, 1);
	//elm_object_style_set(label5, "extended/wrap_ellipsis");
	elm_label_background_color_set(label5, 220, 227, 229, 255);
	*/

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

void label_ellipsis_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *scroller, *layout_inner;
	struct appdata *ad = (struct appdata *) data;
	if (ad == NULL) return;

	scroller = _create_scroller(ad->nf);
	elm_naviframe_item_push(ad->nf, _("Ellipsis"), NULL, NULL, scroller, NULL);

	layout_inner = _create_labels(ad->nf);
	elm_object_content_set(scroller, layout_inner);
}
