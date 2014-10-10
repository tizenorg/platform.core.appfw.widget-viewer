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
#include "math.h"

#define PI 3.1415926

struct video_data {
	struct appdata *ad;
	Evas_Object *video;
	Evas_Object *layout;
	Evas_Object *gl;

	Evas_Coord x, y, w, h;
	Evas_Coord base_x, base_y;
	double zoom;
	double base_zoom;
	double rotate;
	double base_rotate;
};

static void
_item_show(void *data, Evas_Object *obj, void *event_info)
{
	struct video_data *vd = (struct video_data *)data;

	evas_object_show(vd->video);
}

static Eina_Bool
_pop_cb(void *data, Elm_Object_Item *it)
{
	struct video_data *vd = (struct video_data *)data;
	struct appdata *ad;

	if (!vd) return EINA_TRUE;

	ad = vd->ad;

	evas_object_smart_callback_del(ad->nf, "transition,finished", _item_show);
	evas_object_del(vd->video);
	evas_object_del(vd->gl);
	free(vd);

	return EINA_TRUE;
}

static void
_gesture_apply(struct video_data *vd)
{
	Evas_Map *map;

	map = evas_map_new(4);
	evas_map_point_coord_set(map, 0, vd->x, vd->y, 0);
	evas_map_point_coord_set(map, 1, vd->x + vd->w, vd->y, 0);
	evas_map_point_coord_set(map, 2, vd->x + vd->w, vd->y + vd->h, 0);
	evas_map_point_coord_set(map, 3, vd->x, vd->y + vd->h, 0);
	evas_map_point_image_uv_set(map, 0, 0, 0);
	evas_map_point_image_uv_set(map, 1, vd->w, 0);
	evas_map_point_image_uv_set(map, 2, vd->w, vd->h);
	evas_map_point_image_uv_set(map, 3, 0, vd->h);

	evas_map_util_rotate(map, vd->rotate, vd->x + vd->w / 2, vd->y + vd->h / 2);
	evas_map_util_zoom(map, vd->zoom, vd->zoom, vd->x + vd->w / 2, vd->y + vd->h /2);

	evas_object_map_enable_set(vd->video, EINA_TRUE);
	evas_object_map_set(vd->video, map);

	evas_map_free(map);
}

static Evas_Event_Flags
_rotate_move(void *data, void *event_info)
{
	struct video_data *vd = (struct video_data *)data;
	Elm_Gesture_Rotate_Info *info = (Elm_Gesture_Rotate_Info *)event_info;

	vd->rotate = vd->base_rotate + info->angle - info->base_angle;
	if (vd->rotate < 0) vd->rotate += 360.0;
	else if (vd->rotate >= 360) vd->rotate -= 360.0;

	_gesture_apply(vd);

	return EVAS_EVENT_FLAG_NONE;
}

static Evas_Event_Flags
_rotate_end(void *data, void *event_info)
{
	struct video_data *vd = (struct video_data *)data;
	vd->base_rotate = vd->rotate;

	return EVAS_EVENT_FLAG_NONE;
}

static Evas_Event_Flags
_zoom_move(void *data, void *event_info)
{
	struct video_data *vd = (struct video_data *)data;
	Elm_Gesture_Zoom_Info *info = (Elm_Gesture_Zoom_Info *)event_info;

	vd->zoom = vd->base_zoom * info->zoom;
	_gesture_apply(vd);

	return EVAS_EVENT_FLAG_NONE;
}

static Evas_Event_Flags
_zoom_end(void *data, void *event_info)
{
	struct video_data *vd = (struct video_data *)data;
	vd->base_zoom = vd->zoom;

	return EVAS_EVENT_FLAG_NONE;
}

static Evas_Event_Flags
_momentum_move(void *data, void *event_info)
{
	struct video_data *vd = (struct video_data *)data;
	Elm_Gesture_Momentum_Info *info = (Elm_Gesture_Momentum_Info *)event_info;

	// if one finger used 
	if (info->n == 1) {
		int dx, dy;
		double radian = PI * vd->rotate / 180;

		dx = cos(radian) * (info->x2 - info->x1) - sin(radian) * (info->y2 - info->y1);
		dy = sin(radian) * (info->x2 - info->x1) + cos(radian) * (info->y2 - info->y1);
		vd->x = vd->base_x + dx;
		vd->y = vd->base_y + dy;

		_gesture_apply(vd);
	}

	return EVAS_EVENT_FLAG_NONE;
}

static Evas_Event_Flags
_momentum_end(void *data, void *event_info)
{
	struct video_data *vd = (struct video_data *)data;
	Elm_Gesture_Momentum_Info *info = (Elm_Gesture_Momentum_Info *)event_info;

	// if one finger used 
	if (info->n == 1) {
		vd->base_x = vd->x;
		vd->base_y = vd->y;
	}

	return EVAS_EVENT_FLAG_NONE;
}

static void
_create_gesture_layer(struct video_data *vd)
{
	struct appdata *ad = vd->ad;

	vd->gl = elm_gesture_layer_add(ad->nf);
	elm_gesture_layer_attach(vd->gl, vd->video);

	//elm_gesture_layer_cb_set(vd->gl, ELM_GESTURE_ROTATE, ELM_GESTURE_STATE_START, _rotate_start, vd);
	elm_gesture_layer_cb_set(vd->gl, ELM_GESTURE_ROTATE, ELM_GESTURE_STATE_MOVE, _rotate_move, vd);
	elm_gesture_layer_cb_set(vd->gl, ELM_GESTURE_ROTATE, ELM_GESTURE_STATE_END, _rotate_end, vd);
	elm_gesture_layer_cb_set(vd->gl, ELM_GESTURE_ROTATE, ELM_GESTURE_STATE_ABORT, _rotate_end, vd);

	//elm_gesture_layer_cb_set(vd->gl, ELM_GESTURE_ZOOM, ELM_GESTURE_STATE_START, _zoom_start, vd);
	elm_gesture_layer_cb_set(vd->gl, ELM_GESTURE_ZOOM, ELM_GESTURE_STATE_MOVE, _zoom_move, vd);
	elm_gesture_layer_cb_set(vd->gl, ELM_GESTURE_ZOOM, ELM_GESTURE_STATE_END, _zoom_end, vd);
	elm_gesture_layer_cb_set(vd->gl, ELM_GESTURE_ZOOM, ELM_GESTURE_STATE_ABORT, _zoom_end, vd);

	//elm_gesture_layer_cb_set(vd->gl, ELM_GESTURE_MOMENTUM, ELM_GESTURE_STATE_START, _momentum_start, vd);
	elm_gesture_layer_cb_set(vd->gl, ELM_GESTURE_MOMENTUM, ELM_GESTURE_STATE_MOVE, _momentum_move, vd);
	elm_gesture_layer_cb_set(vd->gl, ELM_GESTURE_MOMENTUM, ELM_GESTURE_STATE_END, _momentum_end, vd);
	elm_gesture_layer_cb_set(vd->gl, ELM_GESTURE_MOMENTUM, ELM_GESTURE_STATE_ABORT, _momentum_end, vd);
}

static void
_create_video(struct video_data *vd)
{
	struct appdata *ad = vd->ad;
	Evas_Coord nx, ny, nw, nh;

	vd->video = elm_video_add(ad->nf);
	elm_video_file_set(vd->video, "/opt/media/Videos/Helicopter.mp4");
	elm_video_play(vd->video);

	evas_object_geometry_get(ad->nf, &nx, &ny, &nw, &nh);
	vd->x = vd->base_x = nx + nw / 4;
	vd->y = vd->base_y = ny + nh / 4;
	vd->w = nw / 2;
	vd->h = nh / 2;
	vd->zoom = vd->base_zoom = 1.0;
	vd->rotate = vd->base_rotate = 0.0;

	evas_object_resize(vd->video, vd->w, vd->h);
	_gesture_apply(vd);
}

void video_gesture_cb(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *it;
	struct video_data *vd;
	struct appdata *ad = (struct appdata *)data;
	if (ad == NULL) return;

	vd = calloc(1, sizeof(struct video_data));
	vd->ad = ad;

	vd->layout = elm_layout_add(ad->nf);
	elm_layout_file_set(vd->layout, ELM_DEMO_EDJ, "elmdemo-test/player");
	evas_object_size_hint_weight_set(vd->layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_part_text_set(vd->layout, "info", "Use two fingers to zoom and rotate.<br>Use one finger to move.");

	_create_video(vd);
	_create_gesture_layer(vd);

	it = elm_naviframe_item_push(ad->nf, _("Gesture"), NULL, NULL, vd->layout, NULL);
	elm_naviframe_item_pop_cb_set(it, _pop_cb, vd);
	evas_object_smart_callback_add(ad->nf, "transition,finished", _item_show, vd);
}
