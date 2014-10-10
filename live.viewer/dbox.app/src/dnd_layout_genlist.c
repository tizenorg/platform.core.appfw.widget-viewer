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

static Evas_Object *genlist, *layout, *box;
static Elm_Genlist_Item_Class *itc;

static void _gl_del(void *data, Evas_Object *obj)
{
	Item_Data *id = data;
	if (id) free(id);
}

static char *
_gl_text_get(void *data, Evas_Object *obj, const char *part)
{
	Item_Data *id = data;
	return strdup(id->path);
}

static Evas_Object *
_gl_content_get(void *data, Evas_Object *obj, const char *part)
{
	Item_Data *id = data;
	Evas_Object *icon = elm_image_add(obj);
	elm_image_file_set(icon, id->path, NULL);
	evas_object_size_hint_align_set(icon, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	return icon;
}

static void _gl_sel(void *data, Evas_Object *obj, void *ei)
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

static void
_gl_dragdone(void *data, Evas_Object *obj EINA_UNUSED, Eina_Bool doaccept)
{
	printf("<%s> <%d> data=<%p> doaccept=<%d>\n",
			__func__, __LINE__, data, doaccept);

	if (data)
		eina_list_free(data);
	return;
}

static const char *
_gl_get_drag_data(Evas_Object *obj, Elm_Object_Item *it, Eina_List **items)
{
	/* Construct a string of dragged info, user frees returned string */
	const char *drag_data = NULL;
	printf("<%s> <%d>\n", __func__, __LINE__);

	if (it)
	{
		/* Now we can actually compose string to send and start dragging */
		const char *t;

		t = elm_object_item_part_text_get(it, "elm.text");

		drag_data = malloc(strlen(t) + 8);
		if (!drag_data) return NULL;
		strcpy((char *) drag_data, "file://");
		strcat((char *) drag_data, t);
		strcat((char *) drag_data, "\0");

		printf("<%s> <%d> Sending <%s>\n", __func__, __LINE__, drag_data);
	}
	return drag_data;
}

static Eina_Bool
_gl_dnd_default_anim_data_getcb(Evas_Object *obj, Elm_Object_Item *it, Elm_Drag_User_Info *info)
{
	/* This called before starting to drag, mouse-down was on it */
	info->format = ELM_SEL_FORMAT_TARGETS;
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


////////////////////// FOR LAYOUT DROPPING /////////////////////////
static Eina_Bool
_ly_dropcb(void *data, Evas_Object *obj, Elm_Object_Item *it, Elm_Selection_Data *ev, int xposret, int yposret)
{
	/* This function is called when data is dropped on the genlist */
	printf("<%s> <%d> str=<%s>\n", __func__, __LINE__, (char *) ev->data);

	char *p1;
	p1 = ev->data;
	const char *p2 = strstr(p1, "file://");
	if (p2) p1 = p1 + 7;
	if (!p1) goto end;
	elm_object_part_text_set(obj, "file.text", p1);
	printf("[%s : %d] %s \n", __func__, __LINE__, p1);
	return EINA_TRUE;

end:
	elm_object_part_text_set(obj, "file.text", "Wrong File Format");
	return EINA_TRUE;
}
///////////////////////////////////////////////////////////////////////////////
/////////////////////////// for dnd :: END ////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

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
	elm_drag_item_container_add(genlist, ANIM_TIME, DRAG_TIMEOUT,
			_gl_item_getcb, _gl_dnd_default_anim_data_getcb);
	//

	itc = elm_genlist_item_class_new();
	itc->item_style = "1text.1icon.7";
	itc->func.text_get = _gl_text_get;
	itc->func.content_get = _gl_content_get;
	itc->func.del = _gl_del;

	elm_genlist_block_count_set(genlist, 14);

	for (index = 0; index < 100; index++) {
		Item_Data *id = calloc(sizeof(Item_Data), 1);

		photo_index  = index % (sizeof(photo_path)/sizeof(*photo_path));
		sprintf(buf, ICON_DIR"/genlist/%s", photo_path[photo_index]);
		id->path = eina_stringshare_add(buf);

		item = elm_genlist_item_append(
				genlist,			// genlist object
				itc,				// item class
				id,		            // data
				NULL,
				ELM_GENLIST_ITEM_NONE,
				_gl_sel,
				NULL);
		id->item = item;
	}
}

static Eina_Bool
_pop_cb(void *data, Elm_Object_Item *it)
{
	////// for dnd :: START /////////////
	elm_drag_item_container_del(genlist);
	elm_drop_item_container_del(layout);
	//
	elm_genlist_item_class_free(itc);

	return EINA_TRUE;
}

void
_create_layout(void *data)
{
	struct appdata *ad = data;
	Evas_Object *entry;

	layout = elm_layout_add(ad->nf);
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "dnd_layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	entry = elm_entry_add(ad->nf);
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_entry_single_line_set(entry, EINA_TRUE);
	evas_object_size_hint_align_set(entry, EVAS_HINT_FILL, EVAS_HINT_FILL);

	elm_object_part_content_set(layout, "entry1.swallow", entry);

	entry = elm_entry_add(ad->nf);
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_entry_single_line_set(entry, EINA_TRUE);
	evas_object_size_hint_align_set(entry, EVAS_HINT_FILL, EVAS_HINT_FILL);

	elm_object_part_content_set(layout, "entry2.swallow", entry);

	////// for dnd :: START /////////////
	elm_drop_item_container_add(layout, ELM_SEL_FORMAT_TARGETS, NULL, NULL, NULL,
			NULL, NULL, NULL, NULL, _ly_dropcb, NULL);
	//
}

void dnd_layout_genlist_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad;
	Elm_Object_Item *navi_it;

	ad = data;

	_create_layout(ad);
	_create_genlist(ad);

	box = elm_box_add(ad->nf);
	elm_box_horizontal_set(box, EINA_TRUE);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(box);

	elm_box_pack_end(box, layout);
	evas_object_show(layout);
	elm_box_pack_end(box, genlist);
	evas_object_show(genlist);

	navi_it = elm_naviframe_item_push (ad->nf, _("DND:Layout Genlist") , NULL, NULL, box, NULL);
	elm_naviframe_item_pop_cb_set(navi_it, _pop_cb, NULL);
}
