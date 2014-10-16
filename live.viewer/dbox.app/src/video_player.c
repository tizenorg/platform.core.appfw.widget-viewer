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
#include "video.h"
#include "Emotion.h"

static void
_info_clicked(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *video;
	Evas_Object *emo;
	char info[512] = {0,};
        char buf[128];
	const char *str;
	double len;

	video = evas_object_data_get(obj, "video");
	emo = elm_video_emotion_get(video);

	str = emotion_object_meta_info_get(emo, EMOTION_META_INFO_TRACK_TITLE);
	if (str) {
		sprintf(buf, "Title: %s, ", str);
		strcat(info, buf);
	}
	str = emotion_object_meta_info_get(emo, EMOTION_META_INFO_TRACK_ARTIST);
	if (str) {
		sprintf(buf, "Artist: %s, ", str);
		strcat(info, buf);
	}
	str = emotion_object_meta_info_get(emo, EMOTION_META_INFO_TRACK_ALBUM);
	if (str) {
		sprintf(buf, "Album: %s, ", str);
		strcat(info, buf);
	}
	str = emotion_object_meta_info_get(emo, EMOTION_META_INFO_TRACK_YEAR);
	if (str) {
		sprintf(buf, "Year: %s, ", str);
		strcat(info, buf);
	}
	str = emotion_object_meta_info_get(emo, EMOTION_META_INFO_TRACK_GENRE);
	if (str) {
		sprintf(buf, "Genre: %s, ", str);
		strcat(info, buf);
	}
	str = emotion_object_meta_info_get(emo, EMOTION_META_INFO_TRACK_COMMENT);
	if (str) {
		sprintf(buf, "Genre: %s, ", str);
		strcat(info, buf);
	}
	str = emotion_object_meta_info_get(emo, EMOTION_META_INFO_TRACK_DISC_ID);
	if (str) {
		sprintf(buf, "DISK ID: %s, ", str);
		strcat(info, buf);
	}
	len = elm_video_play_length_get(video);
	if (len > 0.0) {
		int h, m, s;

		h = len/3600;
		m = len/60 - h*60;
		s = len - h*3600 - m*60;
		sprintf(buf, "Length: %d:%02d:%2d", h, m, s);
		strcat(info, buf);
	}

	elm_object_part_text_set(data, "info", buf);
}

void video_player_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *layout, *video, *player;
	struct appdata *ad = (struct appdata *) data;
	if (ad == NULL) return;

	layout = elm_layout_add(ad->nf);
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "elmdemo-test/player");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	video = elm_video_add(ad->nf);
	elm_video_file_set(video, "/opt/media/Videos/Helicopter.mp4");
	elm_video_play(video);
	elm_object_part_content_set(layout, "video", video);

	player = elm_player_add(ad->nf);
	elm_object_content_set(player, video);
	evas_object_data_set(player, "video", video);
	evas_object_smart_callback_add(player, "info,clicked", _info_clicked, layout);
	elm_object_part_content_set(layout, "player", player);


	elm_naviframe_item_push(ad->nf, _("Player"), NULL, NULL, layout, NULL);
}
