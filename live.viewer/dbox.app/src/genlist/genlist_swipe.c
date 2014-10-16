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
#include "genlist.h"

/*********************************************************
  Genlist Sweep
 ********************************************************/

#define NUM_OF_ITEMS 2000
#define NUM_OF_NAMES 4
#define SCRL_TIME 0.1

extern char *genlist_demo_names[];
Ecore_Timer *scrl_timer;

typedef struct _Item_Data
{
	Eina_Bool disabled;
	Elm_Object_Item *item;
} Item_Data;

static void _view_free_cb(void *data, Evas *e, Evas_Object *gen, void *ei)
{
	elm_theme_free(data);
}

static void _gl_del(void *data, Evas_Object *obj)
{
	// FIXME: Unrealized callback can be called after this.
	// Accessing Item_Data can be dangerous on unrealized callback.
	Item_Data *id = data;
	if (id) free(id);
}

static char *_gl_text_get(void *data, Evas_Object *obj, const char *part)
{
	if (!strcmp(part, "elm.text")) {
		char buf[1024];
		int index = (int)data;
		snprintf(buf, 1023, "%s:%s", part, genlist_demo_names[index%NUM_OF_GENLIST_DEMO_NAMES]);
		return strdup(buf);
	} else if (!strcmp(part, "elm.text.swipe.left")) {
		return strdup(S_("IDS_COM_BODY_MESSAGE")); // multi-languate support
	} else if (!strcmp(part, "elm.text.swipe.right")) {
		return strdup(S_("IDS_COM_BODY_CALL"));    // multi-languate support
	}
	return NULL;
}

static Evas_Object *_gl_content_get(void *data, Evas_Object *obj, const char *part)
{
	int index = (int)data;
	Evas_Object *check;

	if (!strcmp(part, "elm.icon")) {
		check = elm_check_add(obj);
		//set the State pointer to keep the current UI state of Checkbox.
		// Repeat events to below object (genlist)
		// So that if check is clicked, genlist can be clicked.
		evas_object_repeat_events_set(check, EINA_FALSE);
		evas_object_propagate_events_set(check, EINA_FALSE);
		evas_object_size_hint_weight_set(check, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(check, EVAS_HINT_FILL, EVAS_HINT_FILL);

		// If no divider, unregister access object
		elm_access_object_unregister(check);
		return check;
	}
	return NULL;
}

static void _gl_sel(void *data, Evas_Object *obj, void *event_info)
{
	if (event_info)
		elm_genlist_item_selected_set(event_info, EINA_FALSE);

	// If sweep is started, ignore select callback
	if (elm_object_scroll_freeze_get(obj)) return;
	printf("selected\n");
}

static void _start(void *data, Elm_Object_Item *it, const char *em, const char *src)
{
	if (scrl_timer) {
		printf("Swipe start but reverted\n");
		elm_object_item_signal_emit(it, "elm,swipe,revert", "");
		return;
	}
	Evas_Object *obj = elm_object_item_widget_get(it);
	// freeze scroller when swipe is started
	elm_object_scroll_freeze_push(data);
	printf("Swipe start: %d\n", elm_object_scroll_freeze_get(obj));
}

static void _stop_left(void *data, Elm_Object_Item *it, const char *em, const char *src)
{
	Evas_Object *obj = elm_object_item_widget_get(it);
	// unfreeze scroller when swipe is stopped
	elm_object_scroll_freeze_pop(data);
	printf("Swipe stop left: %d\n", elm_object_scroll_freeze_get(obj));
}

static void _stop_right(void *data, Elm_Object_Item *it, const char *em, const char *src)
{
	Evas_Object *obj = elm_object_item_widget_get(it);
	// unfreeze scroller when swipe is stopped
	elm_object_scroll_freeze_pop(data);
	printf("Swipe stop right: %d\n", elm_object_scroll_freeze_get(obj));
}

static void _stop(void *data, Elm_Object_Item *it, const char *em, const char *src)
{
	Evas_Object *obj = elm_object_item_widget_get(it);
	// unfreeze scroller when swipe is stopped
	elm_object_scroll_freeze_pop(data);
	printf("Swipe stop: %d\n", elm_object_scroll_freeze_get(obj));
}

static void _unrealized(void *data, Evas_Object *obj, void *ei)
{
	// Remove callbacks before unrealization
	elm_object_item_signal_callback_del(ei, "elm,swipe,start", "", _start);
	elm_object_item_signal_callback_del(ei, "elm,swipe,stop,left", "", _stop_left);
	elm_object_item_signal_callback_del(ei, "elm,swipe,stop,right", "", _stop_right);
	elm_object_item_signal_callback_del(ei, "elm,swipe,stop", "", _stop);
	// Do not access Item Data because unrealized callback can be called
	// after _gl_del, Item_Data is freed.
}

static void _realized(void *data, Evas_Object *obj, void *ei)
{
	Item_Data *id = elm_object_item_data_get(ei);
	// Add callbacks for sweep start, stop
	elm_object_item_signal_callback_add(ei, "elm,swipe,start", "", _start, obj);
	elm_object_item_signal_callback_add(ei, "elm,swipe,stop,left", "", _stop_left, obj);
	elm_object_item_signal_callback_add(ei, "elm,swipe,stop,right", "", _stop_right, obj);
	elm_object_item_signal_callback_add(ei, "elm,swipe,stop", "", _stop, obj);

	if (id->disabled) {
		printf("Realized: Disable swipe\n");
		elm_object_item_signal_emit(ei, "elm,swipe,disabled", "");
	}
}

static void _swipe_revert(void *data, Evas_Object *obj, void *ei)
{
	printf("Revert sweep\n");
	elm_object_item_signal_emit(ei, "elm,swipe,revert", "");
}

static void _swipe_disable(void *data, Evas_Object *obj, void *ei)
{
	Item_Data *id = elm_object_item_data_get(ei);
	if (id->disabled) {
		printf("Enable swipe\n");
		elm_object_item_signal_emit(ei, "elm,swipe,enabled", "");
		id->disabled = EINA_FALSE;
	} else {
		printf("Disable swipe\n");
		elm_object_item_signal_emit(ei, "elm,swipe,disabled", "");
		id->disabled = EINA_TRUE;
	}
}

static Eina_Bool
_scrl_timer(void *data)
{
	printf("scrolling end\n");
	scrl_timer = NULL;
	return EINA_FALSE;
}

static void
_scroll(void *data, Evas_Object *obj, void *ei)
{
	printf("scrolling\n");
	if (scrl_timer) ecore_timer_del(scrl_timer);
	scrl_timer = ecore_timer_add(SCRL_TIME, _scrl_timer, NULL);
}

void genlist_swipe_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!data) return;
	struct appdata *ad = (struct appdata *) data;
	Evas_Object *genlist;
	int index;

	// Custom theme load
	Elm_Theme *th = elm_theme_new();
	elm_theme_ref_set(th, NULL);
	elm_theme_extension_add(th, ELM_DEMO_EDJ);

	Elm_Genlist_Item_Class *itc;
	itc = elm_genlist_item_class_new();
	itc->item_style = "1text/swipe";
	itc->func.text_get = _gl_text_get;
	itc->func.content_get = _gl_content_get;
	itc->func.del = _gl_del;

	// Create genlist
	genlist = elm_genlist_add(ad->nf);
	elm_object_theme_set(genlist, th);
	// HOMOGENEOUS MODE
	// If item height is same when each style name is same,
	// Use homogeneous mode.
	printf("Homogeneous mode enabled\n");
	elm_genlist_homogeneous_set(genlist, EINA_TRUE);

	// Append genlist items
	for (index = 0; index < NUM_OF_ITEMS; index++) {
		Item_Data *id = calloc(sizeof(Item_Data), 1);
		Elm_Object_Item *it = elm_genlist_item_append(genlist, itc, id, NULL,
				ELM_GENLIST_ITEM_NONE, _gl_sel, NULL);
		id->item = it;
		if (index == 0)
			elm_object_item_disabled_set(it, EINA_TRUE);
	}
	elm_genlist_item_class_free(itc);

	evas_object_smart_callback_add(genlist, "realized", _realized, NULL);
	evas_object_smart_callback_add(genlist, "unrealized", _unrealized, NULL);
	evas_object_smart_callback_add(genlist, "scroll", _scroll, NULL);

	// If you want to revert swipe for specific item.
	evas_object_smart_callback_add(genlist, "longpressed", _swipe_revert, NULL);
	// If you want to disable swipe for specific item.
	evas_object_smart_callback_add(genlist, "clicked,double", _swipe_disable, NULL);

	evas_object_event_callback_add(genlist, EVAS_CALLBACK_FREE, _view_free_cb, th);

	// Push genlist to naviframe
	elm_naviframe_item_push(ad->nf,  _("Swipe"), NULL, NULL, genlist, NULL);
}
