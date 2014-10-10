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

#include "elmdemo_util.h"
#include "elmdemo_test.h"
#include "genlist.h"

char *genlist_long_texts[] = {
	"This is very long string text of email style style styleee",
	"This is very long string text of email style style styleeeee",
	"This is very long string text of email style style styleeeeeee",
	"This is very long string text of email style style styleeeeeeeee",
	"This is very long string text of email style style styleeeeeeeeeeee"
	"This is very long string text of email style style styleee",
	"This is very long string text of email style style styleeeee",
	"This is very long string text of email style style styleeeeeee",
	"This is very long string text of email style style styleeeeeeeee",
	"This is very long string text of email style style styleeeeeeeeeeee"
	"This is very long string text of email style style styleee",
	"This is very long string text of email style style styleeeee",
	"This is very long string text of email style style styleeeeeee",
	"This is very long string text of email style style styleeeeeeeee",
	"This is very long string text of email style style styleeeeeeeeeeee"
	"This is very long string text of email style style styleee",
	"This is very long string text of email style style styleeeee",
	"This is very long string text of email style style styleeeeeee",
	"This is very long string text of email style style styleeeeeeeee",
	"This is very long string text of email style style styleeeeeeeeeeee"
	"This is very long string text of email style style styleee",
	"This is very long string text of email style style styleeeee",
	"This is very long string text of email style style styleeeeeee",
	"This is very long string text of email style style styleeeeeeeee",
	"This is very long string text of email style style styleeeeeeeeeeee"
	"This is very long string text of email style style styleee",
	"This is very long string text of email style style styleeeee",
	"This is very long string text of email style style styleeeeeee",
	"This is very long string text of email style style styleeeeeeeee",
	"This is very long string text of email style style styleeeeeeeeeeee"
	"This is very long string text of email style style styleee",
	"This is very long string text of email style style styleeeee",
	"This is very long string text of email style style styleeeeeee",
	"This is very long string text of email style style styleeeeeeeee",
	"This is very long string text of email style style styleeeeeeeeeeee"
	"This is very long string text of email style style styleee",
	"This is very long string text of email style style styleeeee",
	"This is very long string text of email style style styleeeeeee",
	"This is very long string text of email style style styleeeeeeeee",
	"This is very long string text of email style style styleeeeeeeeeeee"
	"This is very long string text of email style style styleee",
	"This is very long string text of email style style styleeeee",
	"This is very long string text of email style style styleeeeeee",
	"This is very long string text of email style style styleeeeeeeee",
	"This is very long string text of email style style styleeeeeeeeeeee"
	"This is very long string text of email style style styleee",
	"This is very long string text of email style style styleeeee",
	"This is very long string text of email style style styleeeeeee",
	"This is very long string text of email style style styleeeeeeeee",
	"This is very long string text of email style style styleeeeeeeeeeee",
	"This is very long string text of email style style styleee",
	"This is very long string text of email style style styleeeee",
	"This is very long string text of email style style styleeeeeee",
	"This is very long string text of email style style styleeeeeeeee",
	"This is very long string text of email style style styleeeeeeeeeeee",
	"This is very long string text of email style style styleee",
	"This is very long string text of email style style styleeeee",
	"This is very long string text of email style style styleeeeeee",
	"This is very long string text of email style style styleeeeeeeee",
	"This is very long string text of email style style styleeeeeeeeeeee",
	NULL
};

char *genlist_demo_names[] = {
	"Aaliyah", "Aamir", "Aaralyn", "Aaron", "Abagail",
	"Babitha", "Bahuratna", "Bandana", "Bulbul", "Cade", "Caldwell",
	"CaptainFantasticFasterThanSupermanSpidermanBatmanWolverineHulkAndTheFlashCombined",
	"Chandan", "Caster", "Dagan ", "Daulat", "Dag", "Earl", "Ebenzer",
	"Ellison", "Elizabeth", "Filbert", "Fitzpatrick", "Florian", "Fulton",
	"Frazer", "Gabriel", "Gage", "Galen", "Garland", "Gauhar", "Hadden",
	"Hafiz", "Hakon", "Haleem", "Hank", "Hanuman", "Jabali ", "Jaimini",
	"Jayadev", "Jake", "Jayatsena", "Jonathan", "Kamaal", "Jeirk",
	"Jasper", "Jack", "Mac", "Macy", "Marlon", "Milson",
	"Aaliyah", "Aamir", "Aaralyn", "Aaron", "Abagail",
	"Babitha", "Bahuratna", "Bandana", "Bulbul", "Cade", "Caldwell",
	"Chandan", "Caster", "Dagan ", "Daulat", "Dag", "Earl", "Ebenzer",
	"Ellison", "Elizabeth", "Filbert", "Fitzpatrick", "Florian", "Fulton",
	"Frazer", "Gabriel", "Gage", "Galen", "Garland", "Gauhar", "Hadden",
	"Hafiz", "Hakon", "Haleem", "Hank", "Hanuman", "Jabali ", "Jaimini",
	"Jayadev", "Jake", "Jayatsena", "Jonathan", "Kamaal", "Jeirk",
	"Jasper", "Jack", "Mac", "Macy", "Marlon", "Milson",
	NULL
};

char *demo_country_names[] = {
	"Greece", "Hong Kong", "Hungary", "India", "Iran",
	"Finland", "Algeria", "Andorra", "Argentina", "Australia",
	"Canada", "Chad", "Chile", "Cuba ", "Denmark",
        "Sweden", "Taiwan", "Tanzania", "Uganda", "Zimbabwe",
	"Ethiopia", "Fiji", "France", "Germany", "Ghana",
	"Iraq", "Hafiz", "Italy", "Jordan", "Kuwait",
	"Bahamas", "Bangladesh", "Belgium", "Benin", "Bosnia",
	"Dominican Republic", "Egypt", "England", "Europa Island", "China",
	"Macau", "Mexico ", "Panama", "Poland", "Peru",
	"Russia", "Scotland", "Slovakia", "South Korea", "Sri Lanka",
	NULL
};

char *genlist_times[] = {"10:00 AM", "10:01 AM", "10:02 AM", "10:03 AM", "10:04 AM", "10:05 AM",
	"3:50 PM", "3:51 PM", "3:52 PM", "3:53 PM"};

char *genlist_messages[] = {"Good morning!",
	"It's you again. How do you do this?",
	"This is the princess who fell from the sky into my arms.",
	"Yes. We keep meeting like this. You just suddenly show up.",
	"We could make plans to meet.",
	"No, it's nicer this way. I hope we meet again suddenly.",
	"Don't you know that it takes so little to make me happy?",
	"Good morning, Princess!",
	"This is incredible. You owe me an explanation.",
	"No, you're the one who owes me an explanation. You've really got a crush on me. Where shall we go, Princess?"
};

static struct appdata *menu_ad;

static struct _group_menu_item menu_its[] = {
	{ EINA_TRUE, "Item Styles", NULL },
	{ EINA_FALSE, "Normal", genlist_normal_cb },
	{ EINA_FALSE, "Dialogue Group", genlist_dialogue_cb },
	{ EINA_FALSE, "Editfield", genlist_editfield_cb },
	{ EINA_FALSE, "Separator", genlist_separator_cb },
	{ EINA_TRUE, "Features", NULL }, // group title
	{ EINA_FALSE, "Edit Mode/Reorder/Sorting", genlist_edit_mode_cb },
	{ EINA_FALSE, "Expandable (Dialogue)", genlist_expandable_cb },
	{ EINA_FALSE, "Expandable (Normal)", genlist_ndepth_expandable_cb },
	{ EINA_FALSE, "Fast Scroll", genlist_fastscroll_cb },
	{ EINA_FALSE, "Swipe", genlist_swipe_cb },
	{ EINA_FALSE, "Collection", genlist_collection_cb },
	{ EINA_FALSE, "Scroll Jump", genlist_scroll_jump_cb },
	{ EINA_FALSE, "Homogenous", genlist_homogenous_cb },
	{ EINA_FALSE, "Animator", genlist_animator_cb },
	{ EINA_TRUE, "Samples", NULL }, // group title
	{ EINA_FALSE, "Genlist with Check", genlist_check_cb },
	{ EINA_FALSE, "Genlist with Radio", genlist_radio_cb },
	{ EINA_FALSE, "Variable Height", genlist_variable_height_cb },
	{ EINA_FALSE, "Dynamic Color Change", genlist_color_cb },
	{ EINA_TRUE, "Themes", NULL }, // group title
	{ EINA_FALSE, "Default", genlist_theme_default_cb },
	{ EINA_FALSE, "No_Effect", genlist_theme_no_effect_cb },
	/* do not delete below */
	{ EINA_FALSE, NULL, NULL }
};

static char *_gl_text_get(void *data, Evas_Object *obj, const char *part)
{
	int index = (int)data;

	if (!strcmp(part, "elm.text")) {
		return strdup(menu_its[index].name);
	}

	return NULL;
}

static void _gl_sel(void *data, Evas_Object *obj, void *event_info)
{
	int index = (int)data;

	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);
	menu_its[index].func(menu_ad, NULL, NULL);
	return;
}

static void _track_resize(void *data, Evas *e, Evas_Object *obj, void *ei)
{
	int idx;
	Evas_Coord w, h;
	evas_object_geometry_get(obj, NULL, NULL, &w, &h);
	idx = elm_genlist_item_index_get(data);
	printf("track resize: %d: w:%d h:%d\n", idx, w, h);
}

static void _track_move(void *data, Evas *e, Evas_Object *obj, void *ei)
{
	int idx;
	Evas_Coord x, y;
	evas_object_geometry_get(obj, &x, &y, NULL, NULL);
	idx = elm_genlist_item_index_get(data);
	printf("track move: %d: x:%d y:%d\n", idx, x, y);
}

static void
_realized(void *data, Evas_Object *obj, void *ei)
{
	int index = (int)elm_object_item_data_get(ei);
	if (((sizeof(menu_its)/sizeof(menu_its[0])) != index) && (menu_its[index + 1].is_group == EINA_TRUE))
		elm_object_item_signal_emit(ei, "elm,state,bottomline,hide", "");

	// track usage
	Evas_Object *track = elm_object_item_track(ei);
	evas_object_event_callback_add(track, EVAS_CALLBACK_RESIZE, _track_resize, ei);
	evas_object_event_callback_add(track, EVAS_CALLBACK_MOVE, _track_move, ei);
}

static Evas_Object* _create_genlist(struct appdata* ad)
{
	Evas_Object *genlist;
	Elm_Object_Item *git = NULL;
	int index;

	if (ad == NULL) return NULL;

	Elm_Genlist_Item_Class *itc =elm_genlist_item_class_new();
	Elm_Genlist_Item_Class *itc_group =elm_genlist_item_class_new();
	itc->item_style = "default";
	itc->func.text_get = _gl_text_get;

	itc_group->item_style = "groupindex";
	itc_group->func.text_get = _gl_text_get;

	genlist = elm_genlist_add(ad->nf);
	elm_genlist_realization_mode_set(genlist, EINA_TRUE);
	evas_object_smart_callback_add(genlist, "realized", _realized, NULL);

	// HOMOGENEOUS MODE
	// If item height is same when each style name is same,
	// Use homogeneous mode.
	printf("Homogeneous mode enabled\n");
	elm_genlist_homogeneous_set(genlist, EINA_TRUE);

	for (index = 0; menu_its[index].name; index++) {
		if (menu_its[index].is_group == EINA_TRUE) {
			git = elm_genlist_item_append(genlist, itc_group, (void *)index, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
			elm_genlist_item_select_mode_set(git, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		} else {
			elm_genlist_item_append(genlist, itc, (void *)index, git,
					ELM_GENLIST_ITEM_NONE, _gl_sel, (void *)index);
		}
	}
	elm_genlist_item_class_free(itc);
	elm_genlist_item_class_free(itc_group);

	return genlist;
}

void genlist_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad;
	Evas_Object *genlist;

	ad = (struct appdata *) data;
	if (ad == NULL) return;

	menu_ad = ad;
	genlist = _create_genlist(ad);
	elm_naviframe_item_push(ad->nf, _("Genlist"), NULL, NULL, genlist, NULL);
}

char** genlist_get_demo_names(void)
{
	return genlist_demo_names;
}

char** genlist_get_demo_country_names(void)
{
	return demo_country_names;
}
