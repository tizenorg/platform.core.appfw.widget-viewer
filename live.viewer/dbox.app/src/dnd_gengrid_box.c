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

#define SWALLOW "box_swallow"

#define ANIM_TIME 0.3
#define DRAG_TIMEOUT 0.5

Evas_Object *gengrid, *layout, *box;
Elm_Gengrid_Item_Class *ggic;
Ecore_Timer *tm;

typedef struct _Item_Data
{
	const char *path;
	int highlighted;
	Elm_Object_Item *item;
	Eina_Bool checked;
	Eina_Bool stared;
} Item_Data;

static void
_del(void *data, Evas_Object *obj)
{
	Item_Data *id = data;
	if (id) free(id);
}

static char *
_text_get(void *data, Evas_Object *obj, const char *part)
{
	Item_Data *id = data;
	return strdup(id->path);
}

static Evas_Object *
_content_get(void *data, Evas_Object *obj, const char *part)
{
	Item_Data *id = data;
	Evas_Object *icon = elm_image_add(obj);
	elm_image_file_set(icon, id->path, NULL);
	evas_object_size_hint_align_set(icon, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	return icon;
}

//////////////////////////////////////////////////////////////////////////////
/////////////////////// for dnd :: START /////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//////////////////// FOR GENGRID //////////////////////////////////
static Eina_Bool
_gg_drop_cb(void *data, Evas_Object *obj, Elm_Object_Item *it, Elm_Selection_Data *sel, int xposret, int yposret)
{
	if (!sel->data) return EINA_FALSE;

	int len = sel->len; /* Should check & use length of returned data */
	char *path = NULL;
	if (len)
		path = malloc(len + 1);
	if (!path) return EINA_FALSE;
	strncpy(path, sel->data, len);
	path[len] = '\0';
	char *f = strstr(path, "file://");
	if (!f)
		f = path;
	else
		f = f + 7;
	if (f)
	{
		char *nl = strstr(f, "\n");
		if (nl)
			*nl = '\0';
		Item_Data *id = calloc(sizeof(Item_Data), 1);
		id->path = eina_stringshare_add(f);
		if (it)
			it = elm_gengrid_item_insert_after(obj, ggic, id, it, NULL, NULL);
		else
			it = elm_gengrid_item_append(obj, ggic, id, NULL, NULL);
		id->item = it;
	}
	free(path);

	return EINA_TRUE;
}

static Elm_Object_Item *
_gg_item_get_cb(Evas_Object *obj, Evas_Coord x, Evas_Coord y, int *xposret, Evas_Coord *yposret)
{
	/* This function returns pointer to item under (x,y) coords */
	Elm_Object_Item *item;
	item = elm_gengrid_at_xy_item_get(obj, x, y, xposret, yposret);
	return item;
}

static Evas_Object *
_gg_create_icon(void *data, Evas_Object *win, int *xoff, int *yoff)
{
	Evas_Object *icon = NULL;
	Evas_Object *o = elm_object_item_part_content_get(data, "elm.swallow.icon");

	if (o)
	{
		int xm, ym, w = 150, h = 150;
		const char *f, *g;
		elm_image_file_get(o, &f, &g);
		evas_pointer_canvas_xy_get(evas_object_evas_get(o), &xm, &ym);

		icon = elm_icon_add(win);
		elm_image_file_set(icon, f, g);
		evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(icon, EVAS_HINT_FILL, EVAS_HINT_FILL);
		if (xoff) *xoff = xm - w/2;
		if (yoff) *yoff = ym - h/2;
		evas_object_resize(icon, w, h);
	}

	return icon;
}

static void
_gg_drag_done(void *data, Evas_Object *obj, Eina_Bool accepted)
{
	printf("%s %d data=<%p> doaccept=<%d>\n",
			__func__, __LINE__, data, accepted);

	if (data)
		eina_list_free(data);
}

static const char *
_gg_get_drag_data(Evas_Object *obj, Elm_Object_Item *it, Eina_List **items)
{
	const char *drag_data = NULL;

	/* Drag only current pressed item */
	if (it)
	{
		const char *t = NULL;

		Item_Data *id = elm_object_item_data_get(it);
		if (id)
		{
			t = id->path;
			/* Compose string to send */
			drag_data = malloc(strlen(t) + 9);
		}
		if (!drag_data) return NULL;
		strcpy((char *)drag_data, "file://");
		strcat((char *)drag_data, t);
		strcat((char *)drag_data, "\n");
		printf("%s %d: sending: <%s>\n", __func__, __LINE__, drag_data);
	}

	return drag_data;
}

static Eina_Bool
_gg_dnd_default_anim_data_get_cb(Evas_Object *obj, Elm_Object_Item *it, Elm_Drag_User_Info *info)
{
	/* This called before starting to drag, mouse-down was on it */
	info->format = ELM_SEL_FORMAT_IMAGE;
	info->icons = NULL;
	info->createicon = _gg_create_icon;
	info->createdata = it;
	info->dragdone = _gg_drag_done;

	/* Now, collect data to send for drop from current pressed item */
	info->data = _gg_get_drag_data(obj, it, (Eina_List **)&info->donecbdata);
	info->acceptdata = info->donecbdata;

	if (info->data)
		return EINA_TRUE;
	else
		return EINA_FALSE;
}

//////////////////// FOR BOX   //////////////////////////////////
static Evas_Object *
_box_item_createicon(void *data, Evas_Object *win, int *xoff, int *yoff)
{
	Evas_Object *icon = NULL;
	Evas_Object *obj = data;

	if (obj)
	{
		const char *f, *g;
		elm_image_file_get(obj, &f, &g);
		icon = elm_image_add(win);
		elm_image_file_set(icon, f, g);
		Evas_Coord x, y, w = 150, h = 150;
		evas_pointer_canvas_xy_get(evas_object_evas_get(obj), &x, &y);
		if (xoff) *xoff = x - w/2;
		if (yoff) *yoff = y - h/2;
		evas_object_resize(icon, w, h);
	}

	return icon;
}

static Eina_Bool
_box_item_long_press_cb(void *data)
{
	Evas_Object *obj = data;

	tm = NULL;
	if (obj)
	{
		const char *f;
		char dd[PATH_MAX];

		elm_image_file_get(obj, &f, NULL);
		snprintf(dd, sizeof(dd), "file://%s\n", f);
		printf("%s %d: sending <%s>\n", __func__, __LINE__, dd);

		/* Start dragging */
		elm_drag_start(obj, ELM_SEL_FORMAT_IMAGE,
				dd, ELM_XDND_ACTION_COPY,
				_box_item_createicon, obj,
				NULL, NULL,
				NULL, NULL,
				NULL, NULL);
	}
	return ECORE_CALLBACK_CANCEL;
}

static void
_box_item_mouse_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	if (tm)
		ecore_timer_del(tm);
	tm = ecore_timer_add(DRAG_TIMEOUT, _box_item_long_press_cb, obj);
}

static void
_box_item_mouse_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	if (tm)
	{
		ecore_timer_del(tm);
		tm = NULL;
	}
}

static Eina_Bool
_box_drop_cb(void *data, Evas_Object *obj, Elm_Selection_Data *sel)
{
	Evas_Object *box = data;
	char *path = NULL;
	if (!sel || !sel->data) return EINA_FALSE;

	/* Should check & use length of returned data */
	int len = sel->len;
	if (len)
		path = malloc(len + 1);
	if (!path) return EINA_FALSE;
	strncpy(path, sel->data, len);
	path[len] = '\0';

	char *f = strstr(path, "file://");
	if (!f)
		f = path;
	else
		f = f + 7;
	if (f)
	{
		char *nl = strstr(f, "\n");
		if (nl)
			*nl = '\0';
		/* remove items */
		Eina_List *children = elm_box_children_get(box);
		if (eina_list_count(children) >= 4)
			elm_box_clear(box);
		eina_list_free(children);

		/* add dropped item */
		Evas_Object *item = elm_image_add(layout);
		elm_image_file_set(item, f, NULL);
		evas_object_size_hint_weight_set(item, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(item, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_min_set(item, 150, 150);
		evas_object_size_hint_max_set(item, 150, 150);
		evas_object_event_callback_add(item, EVAS_CALLBACK_MOUSE_DOWN, _box_item_mouse_down_cb, box);
		evas_object_event_callback_add(item, EVAS_CALLBACK_MOUSE_UP, _box_item_mouse_up_cb, box);
		evas_object_show(item);
		elm_box_pack_end(box, item);
	}
	free(path);

	return EINA_TRUE;
}
//////////////////////////////////////////////////////////////////////////////
/////////////////////// for dnd :: END /////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

static void
_create_gengrid(void *data)
{
	struct appdata *ad = data;
	Elm_Object_Item *item;
	char buf[PATH_MAX];
	int idx;

	gengrid = elm_gengrid_add(ad->nf);
	evas_object_size_hint_weight_set(gengrid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(gengrid, EVAS_HINT_FILL, EVAS_HINT_FILL);

	elm_gengrid_item_size_set(gengrid, elm_config_scale_get() * 150, elm_config_scale_get() * 150);
	elm_gengrid_horizontal_set(gengrid, EINA_FALSE);
	elm_gengrid_reorder_mode_set(gengrid, EINA_FALSE);
	elm_gengrid_multi_select_set(gengrid, EINA_FALSE);

	elm_drop_item_container_add(gengrid, ELM_SEL_FORMAT_IMAGE, _gg_item_get_cb,
			NULL, NULL, NULL, NULL, NULL, NULL, _gg_drop_cb, NULL);
	elm_drag_item_container_add(gengrid, ANIM_TIME, DRAG_TIMEOUT,
			_gg_item_get_cb, _gg_dnd_default_anim_data_get_cb);

	ggic = elm_gengrid_item_class_new();
	ggic->item_style = "default";
	ggic->func.text_get = _text_get;
	ggic->func.content_get = _content_get;
	ggic->func.del = _del;

	for (idx = 0; idx < 20; idx++)
	{
		Item_Data *id = calloc(sizeof(Item_Data), 1);
		snprintf(buf, sizeof(buf), "%s/grid_image/%d_raw.jpg", ICON_DIR, idx + 1);
		id->path = eina_stringshare_add(buf);
		item = elm_gengrid_item_append(
				gengrid,
				ggic,
				id,
				NULL,
				NULL);
		id->item = item;
	}

	evas_object_show(gengrid);
}

static void
_create_box(void *data)
{
	struct appdata *ad = data;
	Evas_Object *item;
	char buf[PATH_MAX];
	int idx;

	tm = NULL;
	layout = elm_layout_add(ad->nf);
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "dnd/box_layout");

	box = elm_box_add(ad->nf);
	elm_box_horizontal_set(box, EINA_TRUE);
	elm_object_part_content_set(layout, SWALLOW, box);

	/* Add items to box */
	for (idx = 0; idx < 1; idx ++)
	{
		item = elm_image_add(layout);
		snprintf(buf, sizeof(buf), "%s/grid_image/5%d_raw.jpg", ICON_DIR, idx + 1);
		elm_image_file_set(item, buf, NULL);
		evas_object_size_hint_weight_set(item, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(item, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_min_set(item, 150, 150);
		evas_object_size_hint_max_set(item, 150, 150);
		evas_object_event_callback_add(item, EVAS_CALLBACK_MOUSE_DOWN, _box_item_mouse_down_cb, box);
		evas_object_event_callback_add(item, EVAS_CALLBACK_MOUSE_UP, _box_item_mouse_up_cb, box);
		evas_object_show(item);
		elm_box_pack_end(box, item);
	}
	evas_object_show(layout);

	/* We should use layout as drop target */
	elm_drop_target_add(layout, ELM_SEL_FORMAT_IMAGE,
			NULL, NULL, NULL, NULL, NULL, NULL, _box_drop_cb, box);
}

static Eina_Bool
_pop_cb(void *data, Elm_Object_Item *it)
{
	////// for dnd :: START /////////////
	elm_drag_item_container_del(gengrid);
	elm_drop_item_container_del(gengrid);
	elm_drop_target_del(layout, ELM_SEL_FORMAT_IMAGE,
			NULL, NULL, NULL, NULL, NULL, NULL, _box_drop_cb, box);
	//

	elm_gengrid_item_class_free(ggic);

	return EINA_TRUE;
}

void
dnd_gengrid_box_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad;
	Elm_Object_Item *navi_it;
	Evas_Object *box_main;

	ad = (struct appdata *)data;

	box_main = elm_box_add(ad->nf);
	evas_object_size_hint_weight_set(box_main, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	_create_gengrid(ad);
	elm_box_pack_end(box_main, gengrid);

	_create_box(ad);
	elm_box_pack_end(box_main, layout);

	evas_object_show(box_main);

	navi_it = elm_naviframe_item_push(ad->nf, _("DND: Gengrid Box"), NULL, NULL, box_main, NULL);
	elm_naviframe_item_pop_cb_set(navi_it, _pop_cb, NULL);
}
