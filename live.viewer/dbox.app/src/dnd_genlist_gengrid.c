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

//////// for dnd :: START ///////
#define DRAG_TIMEOUT 0.3
#define ANIM_TIME 0.5
//
#define TIMER_VAL 0.1

static Evas_Object *genlist, *gengrid, *box;
static Elm_Genlist_Item_Class *glic;
static Elm_Gengrid_Item_Class *ggic;

typedef struct _Item_Data
{
	const char *path;
	int highlighted;
	Elm_Object_Item *item;
	Eina_Bool checked;
	Eina_Bool stared;
} Item_Data;

static char *photo_path[] = {
	"00_list_photo_default.png", "iu.jpg", "iu2.jpg", "koo.jpg", "top.jpg", "boa.jpg",
	"kimtaehee.jpg", "moon.jpg", "taeyon.jpg"
};

static int
_item_ptr_cmp(const void *d1, const void *d2)
{
	return (d1 - d2);
}

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

static void
_sel(void *data, Evas_Object *obj, void *ei)
{
	// Do something when an item is selected.
}

//////////////////////////////////////////////////////////////////////////////
/////////////////////// for dnd :: START /////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//////////////////// FOR GENLIST DRAGGING   //////////////////////////////////
static Elm_Object_Item *
_gl_item_getcb(Evas_Object *obj, Evas_Coord x, Evas_Coord y, int *xposret, int *yposret)
{
	/* This function returns pointer to item under (x,y) coords */
	printf("<%s> <%d> obj=<%p>\n", __func__, __LINE__, obj);
	Elm_Object_Item *gli;
	gli = elm_genlist_at_xy_item_get(obj, x, y, yposret);
	if (gli)
		printf("over <%s>, gli=<%p> yposret %i\n",
				elm_object_item_part_text_get(gli, "elm.text"), gli, *yposret);
	else
		printf("over none, yposret %i\n", *yposret);
	return gli;
}

static Evas_Object *
_gl_createicon(void *data, Evas_Object *win, Evas_Coord *xoff, Evas_Coord *yoff)
{
	printf("<%s> <%d>\n", __func__, __LINE__);
	Evas_Object *icon = NULL;
	Evas_Object *o = elm_object_item_part_content_get(data, "elm.icon");

	if (o)
	{
		int xm, ym, w = 150, h = 150;
		const char *f;
		const char *g;
		elm_image_file_get(o, &f, &g);
		evas_pointer_canvas_xy_get(evas_object_evas_get(o), &xm, &ym);
		if (xoff) *xoff = xm - (w/2);
		if (yoff) *yoff = ym - (h/2);
		icon = elm_icon_add(win);
		elm_image_file_set(icon, f, g);
		evas_object_size_hint_align_set(icon, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		if (xoff && yoff) evas_object_move(icon, *xoff, *yoff);
		evas_object_resize(icon, w, h);
	}

	return icon;
}
static Eina_Bool
_gl_timercb(void *data)
{
	if (!data) return ECORE_CALLBACK_CANCEL;

	Eina_List  *items = elm_genlist_selected_items_get(data);
	Eina_List *l;
	Elm_Object_Item *it;
	EINA_LIST_FOREACH(items, l, it)
	{
		elm_genlist_item_selected_set(it, EINA_FALSE);
	}
	return ECORE_CALLBACK_CANCEL;
}

static void
_gl_dragdone(void *data, Evas_Object *obj, Eina_Bool doaccept)
{
	printf("<%s> <%d> data=<%p> doaccept=<%d>\n",
			__func__, __LINE__, data, doaccept);

	if (data)
		eina_list_free(data);

	/* Unselect selected items */
	ecore_timer_add(TIMER_VAL, _gl_timercb, obj);
}

static const char *
_gl_get_drag_data(Evas_Object *obj, Elm_Object_Item *it, Eina_List **items)
{
	/* Construct a string of dragged info, user frees returned string */
	const char *drag_data = NULL;
	printf("<%s> <%d>\n", __func__, __LINE__);

	*items = eina_list_clone(elm_genlist_selected_items_get(obj));
	if (it)
	{ /* Add the item mouse is over to the list if NOT selected */
		void *p = eina_list_search_unsorted(*items, _item_ptr_cmp, it);
		if (!p)
			*items = eina_list_append(*items, it);
	}

	if (*items)
	{ /* Now we can actually compose string to send and start dragging */
		Eina_List *l;
		unsigned  int len = 0;

		EINA_LIST_FOREACH(*items, l, it)
		{
			Item_Data *id = elm_object_item_data_get(it);
			if (id)
			{
				const char *path = id->path;
				printf("<%s> <%d> item: <%s>\n", __func__, __LINE__, path);
				if (path)
					len += strlen(path);
			}
		}

		drag_data = malloc(len + eina_list_count(*items) + 8);
		strcpy((char *)drag_data, "file://");

		EINA_LIST_FOREACH(*items, l, it)
		{
			Item_Data *id = elm_object_item_data_get(it);
			if (id)
			{
				const char *path = id->path;
				if (path)
				{
					strcat((char *)drag_data, path);
					strcat((char *)drag_data, "\n");
					printf("%s %d: %s\n",__func__, __LINE__, path);
				}
			}
		}
		printf("<%s> <%d> Sending <%s> len: %d\n", __func__, __LINE__, drag_data, strlen(drag_data));
	}

	return drag_data;
}

static Eina_Bool
_gl_dnd_default_anim_data_getcb(Evas_Object *obj, Elm_Object_Item *it, Elm_Drag_User_Info *info)
{
	/* This called before starting to drag, mouse-down was on it */
	info->format = ELM_SEL_FORMAT_IMAGE;
	info->createicon = _gl_createicon;
	info->createdata = it;
	info->icons = NULL;
	info->dragdone = _gl_dragdone;

	/* Now, collect data to send for drop from ALL selected items */
	/* Save list pointer to remove items after drop and free list on done */
	info->data = _gl_get_drag_data(obj, it, (Eina_List **) &info->donecbdata);
	printf("%s - data = %s\n", __FUNCTION__, info->data);
	info->acceptdata = info->donecbdata;

	if (info->data)
		return EINA_TRUE;
	else
		return EINA_FALSE;
}

//////////////////// FOR GENGLIST DROPPING  //////////////////////////////////
static Eina_Bool
_gl_dropcb(void *data, Evas_Object *obj, Elm_Object_Item *it, Elm_Selection_Data *ev, int xposret, int yposret)
{
	if (!ev->data)
		return EINA_FALSE;

	char *p = ev->data;
	printf("<%s> <%d>: items: <%s>\n", __func__, __LINE__, p);
	const char *s = strstr(p, "file://");
	if (s) p = p + 7;
	if (!p) return EINA_TRUE;

	char *p2 = strchr(p, '\n');
	while(p2)
	{
		*p2 = '\0';
		Item_Data *id = calloc(sizeof(Item_Data), 1);
		id->path = strdup(p);
		switch(yposret)
		{
			case -1: /* Dropped on top-part of the it item */
				elm_genlist_item_insert_before(obj,
						glic, id, NULL, it,
						ELM_GENLIST_ITEM_NONE,
						NULL, NULL);
				break;
			case 0: /* Dropped on center of the it item */
			case 1: /* Dropped on botton-part of the it item */
				if (!it) it = elm_genlist_last_item_get(obj);
				if (it)
				{
					it = elm_genlist_item_insert_after(obj,
							glic, id, NULL, it,
							ELM_GENLIST_ITEM_NONE,
							NULL, NULL);
				}
				else
				{
					it = elm_genlist_item_append(obj,
							glic, id, NULL,
							ELM_GENLIST_ITEM_NONE,
							NULL, NULL);
				}
				break;
			default:
				free(id);
				return EINA_FALSE;
		}
		p = p2 + 1;
		p2 = strchr(p, '\n');
	}

	return EINA_TRUE;
}

//////////////////// FOR GENGRID DRAGGING  //////////////////////////////////
static Elm_Object_Item *
_gg_item_getcb(Evas_Object *obj, Evas_Coord x, Evas_Coord y, int *xposret, int *yposret)
{
	/* This function returns pointer to item under (x,y) coords */
	printf("<%s> <%d> obj=<%p>\n", __func__, __LINE__, obj);
	Elm_Object_Item *item;
	item = elm_gengrid_at_xy_item_get(obj, x, y, xposret, yposret);
	if (item)
	{
		Item_Data *id = elm_object_item_data_get(item);
		printf("over <%s>, item=<%p> xposret %i yposret %i\n",
				id->path, item, *xposret, *yposret);
	}
	else
		printf("over none, xposret %i yposret %i\n", *xposret, *yposret);
	return item;
}

static Evas_Object *
_gg_createicon(void *data, Evas_Object *win, Evas_Coord *xoff, Evas_Coord *yoff)
{
	printf("<%s> <%d>\n", __func__, __LINE__);
	Evas_Object *icon = NULL;
	Evas_Object *o = elm_object_item_part_content_get(data, "elm.swallow.icon");

	if (o)
	{
		int xm, ym, w = 150, h = 150;
		const char *f;
		const char *g;
		elm_image_file_get(o, &f, &g);
		evas_pointer_canvas_xy_get(evas_object_evas_get(o), &xm, &ym);
		if (xoff) *xoff = xm - (w/2);
		if (yoff) *yoff = ym - (h/2);
		icon = elm_icon_add(win);
		elm_image_file_set(icon, f, g);
		evas_object_size_hint_align_set(icon, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		if (xoff && yoff) evas_object_move(icon, *xoff, *yoff);
		evas_object_resize(icon, w, h);
	}

	return icon;
}

static Eina_Bool
_gg_timercb(void *data)
{
	if (!data) return ECORE_CALLBACK_CANCEL;

	Eina_List  *items = (Eina_List *)elm_gengrid_selected_items_get(data);
	Eina_List *l;
	Elm_Object_Item *it;
	EINA_LIST_FOREACH(items, l, it)
	{
		elm_gengrid_item_selected_set(it, EINA_FALSE);
	}
	return ECORE_CALLBACK_CANCEL;
}

static void
_gg_dragdone(void *data, Evas_Object *obj, Eina_Bool doaccept)
{
	printf("<%s> <%d> data=<%p> doaccept=<%d>\n",
			__func__, __LINE__, data, doaccept);

	if (data)
		eina_list_free(data);

	/* Unselect selected items */
	ecore_timer_add(TIMER_VAL, _gg_timercb, obj);
}

static const char*
_gg_get_drag_data(Evas_Object *obj, Elm_Object_Item *it, Eina_List **items)
{
	printf("<%s> <%d>\n", __func__, __LINE__);
	/* Construct a string of dragged info, users frees returned string */
	const char *drag_data = NULL;

	*items = eina_list_clone(elm_gengrid_selected_items_get(obj));
	if (it)
	{ /* Add the item mouse is over to the list if NOT selected */
		void *p = eina_list_search_unsorted(*items, _item_ptr_cmp, it);
		if (!p)
			*items = eina_list_append(*items, it);
	}

	if (*items)
	{ /* Now we can actually compose string to send and start dragging */
		Eina_List *l;
		unsigned  int len = 0;

		EINA_LIST_FOREACH(*items, l, it)
		{
			Item_Data *id = elm_object_item_data_get(it);
			if (id)
			{
				const char *path = id->path;
				if (path)
					len += strlen(path);
			}
		}

		drag_data = malloc(len + eina_list_count(*items) + 8);
		strcpy((char *)drag_data, "file://");

		EINA_LIST_FOREACH(*items, l, it)
		{
			Item_Data *id = elm_object_item_data_get(it);
			if (id)
			{
				const char *path = id->path;
				if (path)
				{
					strcat((char *)drag_data, path);
					strcat((char *)drag_data, "\n");
				}
			}
		}
		printf("<%s> <%d> Sending <%s> len: %d\n", __func__, __LINE__, drag_data, strlen(drag_data));
	}

	return drag_data;
}

static Eina_Bool
_gg_dnd_default_anim_data_getcb(Evas_Object *obj,  /* The gengrid object */
      Elm_Object_Item *it,
      Elm_Drag_User_Info *info)
{
	/* This called before starting to drag, mouse-down was on it */
	printf("<%s> <%d>\n", __func__, __LINE__);
	info->format = ELM_SEL_FORMAT_IMAGE;
	info->createicon = _gg_createicon;
	info->createdata = it;
	info->icons = NULL;
	info->dragdone = _gg_dragdone;

	/* Now, collect data to send for drop from ALL selected items */
	/* Save list pointer to remove items after drop and free list on done */
	info->data = _gg_get_drag_data(obj, it, (Eina_List **) &info->donecbdata);
	printf("<%s> <%d> - data = <%s>\n", __func__, __LINE__, info->data);
	info->acceptdata = info->donecbdata;

	if (info->data)
		return EINA_TRUE;
	else
		return EINA_FALSE;
}

//////////////////// FOR GENGRID DROPPING  //////////////////////////////////
static Eina_Bool
_gg_dropcb(void *data, Evas_Object *obj, Elm_Object_Item *it, Elm_Selection_Data *ev, int xposret, int yposret)
{
	/* This function is called when data is dropped on gengrid */
	printf("<%s> <%d>\n", __func__, __LINE__);
	if (!ev->data)
		return EINA_FALSE;

	const char *p = ev->data;
	const char *s = strstr(p, "file://");
	if (s) p = p + 7;
	if (p)
	{
		char *p2 = strchr(p, '\n');
		while(p2)
		{
			*p2 = '\0';

			Item_Data *id = calloc(sizeof(Item_Data), 1);
			id->path = strdup(p);
			if (!it) it = elm_gengrid_last_item_get(obj);
			if (it)
				it = elm_gengrid_item_insert_after(obj, ggic, id, it, NULL, NULL);
			else
				it = elm_gengrid_item_append(obj, ggic, id, NULL, NULL);
			p = p2 + 1;
			p2 = strchr(p, '\n');
		}
	}

	return EINA_TRUE;
}

static void
_create_gengrid(void *data)
{
	struct appdata *ad = data;
	Elm_Object_Item *item;
	char buf[PATH_MAX];
	int index;

	gengrid = elm_gengrid_add(ad->nf);
	evas_object_size_hint_weight_set(gengrid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(gengrid, EVAS_HINT_FILL, EVAS_HINT_FILL);

	elm_gengrid_item_size_set(gengrid, elm_config_scale_get() * 150, elm_config_scale_get() * 150);
	elm_gengrid_horizontal_set(gengrid, EINA_FALSE);
	elm_gengrid_reorder_mode_set(gengrid, EINA_FALSE);
	elm_gengrid_multi_select_set(gengrid, EINA_TRUE);
	///////////// for dnd :: START //////////////////////////
	elm_drop_item_container_add(gengrid, ELM_SEL_FORMAT_IMAGE, _gg_item_getcb,
			NULL, NULL, NULL, NULL, NULL, NULL, _gg_dropcb, NULL);
	elm_drag_item_container_add(gengrid, ANIM_TIME, DRAG_TIMEOUT,
			_gg_item_getcb, _gg_dnd_default_anim_data_getcb);
	//

	ggic = elm_gengrid_item_class_new();
	ggic->item_style = "default";
	ggic->func.text_get = _text_get;
	ggic->func.content_get = _content_get;
	ggic->func.del = _del;

	for (index = 0; index < 20; index++)
	{
		Item_Data *id = calloc(sizeof(Item_Data), 1);

		snprintf(buf, sizeof(buf), "%s/grid_image/%d_raw.jpg", ICON_DIR, index+1);
		id->path = eina_stringshare_add(buf);

		item = elm_gengrid_item_append(
				gengrid,		// gengrid object
				ggic,			// gengrid item class
				id,				// data
				NULL,
				NULL);
		id->item = item;
	}
}

static void
_create_genlist (void *data)
{
	struct appdata *ad = data;
	Elm_Object_Item *item;
	char buf[PATH_MAX];
	int index, photo_index;

	genlist = elm_genlist_add(ad->nf);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
	///////////// for dnd :: START //////////////////////////
	elm_drop_item_container_add(genlist, ELM_SEL_FORMAT_IMAGE, _gl_item_getcb,
			NULL, NULL, NULL, NULL, NULL, NULL, _gl_dropcb, NULL);
	elm_drag_item_container_add(genlist, ANIM_TIME, DRAG_TIMEOUT,
			_gl_item_getcb, _gl_dnd_default_anim_data_getcb);
	//

	glic = elm_genlist_item_class_new();
	glic->item_style = "1text.1icon.7";
	glic->func.text_get = _text_get;
	glic->func.content_get = _content_get;
	glic->func.del = _del;

	elm_genlist_block_count_set(genlist, 14);
	elm_genlist_multi_select_set(genlist, EINA_TRUE); //allow multi-select

	for (index = 0; index < 100; index++)
	{
		Item_Data *id = calloc(sizeof(Item_Data), 1);

		photo_index  = index % (sizeof(photo_path)/sizeof(*photo_path));
		sprintf(buf, ICON_DIR"/genlist/%s", photo_path[photo_index]);
		id->path = eina_stringshare_add(buf);

		item = elm_genlist_item_append(
				genlist,			// genlist object
				glic,				// item class
				id,					// data
				NULL,
				ELM_GENLIST_ITEM_NONE,
				_sel,
				NULL);
		id->item = item;
	}
}

static Eina_Bool
_pop_cb(void *data, Elm_Object_Item *it)
{
	////// for dnd :: START /////////////
	elm_drag_item_container_del(genlist);
	elm_drop_item_container_del(gengrid);
	//
	elm_genlist_item_class_free(glic);
	elm_gengrid_item_class_free(ggic);

	return EINA_TRUE;
}

void dnd_genlist_gengrid_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad;
	Elm_Object_Item *navi_it;

	ad = data;

	_create_genlist(ad);
	_create_gengrid(ad);

	box = elm_box_add(ad->nf);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(box);

	elm_box_pack_end(box, genlist);
	evas_object_show(genlist);
	elm_box_pack_end(box, gengrid);
	evas_object_show(gengrid);

	navi_it = elm_naviframe_item_push(ad->nf, _("DND:Genlist Gengrid (Multi-Select)") , NULL, NULL, box, NULL);
	elm_naviframe_item_pop_cb_set(navi_it, _pop_cb, NULL);
}
