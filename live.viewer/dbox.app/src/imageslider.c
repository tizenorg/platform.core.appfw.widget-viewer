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
#include "imageslider.h"

#define IMAGE_COUNT 22

static char *imageslider_photos_path[IMAGE_COUNT] = {
	ICON_DIR "/Albums_Item/Albums_Item1.jpg",
	ICON_DIR "/Albums_Item/Albums_Item2.jpg",
	ICON_DIR "/Albums_Item/Albums_Item3.jpg",
	ICON_DIR "/Albums_Item/Albums_Item4.jpg",
	ICON_DIR "/Albums_Item/Albums_Item5.jpg",
	ICON_DIR "/Albums_Item/Albums_Item6.jpg",
	ICON_DIR "/Albums_Item/Albums_Item7.jpg",
	ICON_DIR "/Albums_Item/Albums_Item8.jpg",
	ICON_DIR "/Albums_Item/Albums_Item9.jpg",
	ICON_DIR "/Albums_Item/Albums_Item10.jpg",
	ICON_DIR "/Albums_Item/Albums_Item11.jpg",
	ICON_DIR "/Albums_Item/Albums_Item12.jpg",
	ICON_DIR "/Albums_Item/Albums_Item13.jpg",
	ICON_DIR "/Albums_Item/Albums_Item14.jpg",
	ICON_DIR "/Albums_Item/Albums_Item15.jpg",
	ICON_DIR "/Albums_Item/Albums_Item16.jpg",
	ICON_DIR "/Albums_Item/Albums_Item17.jpg",
	ICON_DIR "/Albums_Item/Albums_Item18.jpg",
	ICON_DIR "/Albums_Item/Albums_Item19.jpg",
	ICON_DIR "/Albums_Item/Albums_Item20.jpg",
	ICON_DIR "/Albums_Item/Albums_Item21.jpg",
	ICON_DIR "/Albums_Item/Albums_Item22.jpg",
};

static Evas_Object *_create_imageslider(Evas_Object * parent);
static void _test_cb(void *data, Evas_Object * obj, void *event_info);
static void _photo_clicked_cb(void *data, Evas_Object * obj, void *event_info);

static Evas_Object *_create_imageslider(Evas_Object * parent)
{
	int i;
	Evas_Object *is;

	is = elm_imageslider_add(parent);

	for (i = 0; i < IMAGE_COUNT; i++) {
		elm_imageslider_item_append(is, imageslider_photos_path[i], _test_cb, NULL);
	}
	elm_imageslider_item_append_relative(is, imageslider_photos_path[10], _test_cb, 3, NULL);
	evas_object_smart_callback_add(is, "clicked", _photo_clicked_cb, NULL);
	return is;
}

static void _test_cb(void *data, Evas_Object * obj, void *event_info)
{
	Elm_Imageslider_Item *it = event_info;

	fprintf(stderr, "[[[ DEBUG ]]]::PHOTO FILE: %s\n", elm_imageslider_item_photo_file_get(it));
}

static void _photo_clicked_cb(void *data, Evas_Object * obj, void *event_info)
{
	fprintf(stderr, "[[[ DEBUG ]]]::(TestApp) CLICKED!!!\n");
}

void imageslider_cb(void *data, Evas_Object * obj, void *event_info)
{
	struct appdata *ad;

	Evas_Object *img_slider;
	Elm_Object_Item *it;

	ad = (struct appdata *)data;
	if (ad == NULL) return;
	img_slider = _create_imageslider(ad->nf);
	it = elm_naviframe_item_push(ad->nf, _("Image Slider"), NULL, NULL, img_slider, NULL);
}
