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
 Customize Genlist Styles (Music Player)
 ********************************************************/

#define	NUM_GENLIST_STYLES 6
typedef enum _Genlist_Style
{
	GENLIST_STYLE_MUSIC_PROGRESS = 0,
	GENLIST_STYLE_GROUP_TITLE,
	GENLIST_STYLE_SONGS,
	GENLIST_STYLE_ALBUM_LIST,
	GENLIST_STYLE_ARTIST_LIST,
	GENLIST_STYLE_TITLE_ARTIST
} Genlist_Style;

typedef struct _Music_Info
{
	char *file;
	char *artist;
	char *title;
	char *time;
	char *track_num;
	char *album_num;
	Genlist_Style genlist_style;
} Music_Info;

typedef struct _Genlist_Data
{
	// For progressbar
	Evas_Object *progressbar[2];
	Ecore_Timer *progressbar_timer[2];
	double progressbar_period[2];

	Elm_Genlist_Item_Class itc[NUM_GENLIST_STYLES];
} Genlist_Data;
static Genlist_Data sd;

static Music_Info info[] = {
	{ "NotNULL", "Progress Types", NULL, NULL, NULL, NULL, GENLIST_STYLE_GROUP_TITLE },
	{ "NotNULL", "Music Play", "Playing Music", NULL, NULL, NULL, GENLIST_STYLE_MUSIC_PROGRESS },
	{ "NotNULL", "Music Play 2", "Playing Music 2", NULL, NULL, NULL, GENLIST_STYLE_MUSIC_PROGRESS },

	{ "NotNULL", "Music 1", NULL, NULL, NULL, NULL, GENLIST_STYLE_GROUP_TITLE },
	{ "AdamGreen_HairyWomenLyrics.jpg", "AdamGreen", "Hairy Women Lyrics", "01:14", "5", "2", GENLIST_STYLE_SONGS },
	{ "BoA_Best&USA.jpg", "BoA", "Best&USA", "03:12", "2", "1", GENLIST_STYLE_ALBUM_LIST },
	{ "CelineDion_TakingChances.jpg", "Celine Dion", "Taking Chances", "05:32", "10", "4", GENLIST_STYLE_ARTIST_LIST },
	{ "JamieCullum_twentysomething.jpg", "Jamie Cullum", "Twenty Something", "03:29", "1", "3", GENLIST_STYLE_TITLE_ARTIST },

	{ "NotNULL", "Music 2", NULL, NULL, NULL, NULL, GENLIST_STYLE_GROUP_TITLE },
	{ "M83_saturdays=youth.jpg", "M83", "Saturdays=Youth", "04:21", "2", "3", GENLIST_STYLE_SONGS },
	{ "Narsha.jpg", "Narsha", "Ppirippappa", "03:10", "7", "5", GENLIST_STYLE_ALBUM_LIST },
	{ "RoadNumberOne.jpg", "IU", "Road Number One", "01:55", "2", "7", GENLIST_STYLE_ARTIST_LIST },
	{ "SS501_rebirth.jpg", "SS501", "Rebirth", "04:12", "4", "1", GENLIST_STYLE_TITLE_ARTIST },

	{ "NotNULL", "Music 3", NULL, NULL, NULL, NULL, GENLIST_STYLE_GROUP_TITLE },
	{ "AdamGreen_HairyWomenLyrics.jpg", "AdamGreen", "Hairy Women Lyrics", "01:14", "5", "3", GENLIST_STYLE_SONGS },
	{ "BoA_Best&USA.jpg", "BoA", "Best&USA", "03:12", "2", "5", GENLIST_STYLE_ALBUM_LIST },
	{ "CelineDion_TakingChances.jpg", "Celine Dion", "Taking Chances", "05:32", "10", "2", GENLIST_STYLE_ARTIST_LIST },
	{ "JamieCullum_twentysomething.jpg", "Jamie Cullum", "Twenty Something", "03:29", "1", "2", GENLIST_STYLE_TITLE_ARTIST },

	{ "NotNULL", "Music 4", NULL, NULL, NULL, NULL, GENLIST_STYLE_GROUP_TITLE },
	{ "M83_saturdays=youth.jpg", "M83", "Saturdays=Youth", "04:21", "2", "1", GENLIST_STYLE_SONGS },
	{ "Narsha.jpg", "Narsha", "Ppirippappa", "03:10", "7", "4", GENLIST_STYLE_ALBUM_LIST },
	{ "RoadNumberOne.jpg", "IU", "Road Number One", "01:55", "2", "5", GENLIST_STYLE_ARTIST_LIST },
	{ "SS501_rebirth.jpg", "SS501", "Rebirth", "04:12", "4", "1", GENLIST_STYLE_TITLE_ARTIST },
	{ NULL, NULL, NULL, NULL, NULL, NULL, 0 }
};

static void _set_genlist_item_class(void);
static char *_gl_text_get(void *data, Evas_Object *obj, const char *part);
static Evas_Object *_gl_content_get(void *data, Evas_Object *obj, const char *part);
static Eina_Bool _gl_state_get(void *data, Evas_Object *obj, const char *part);
static void _gl_del(void *data, Evas_Object *obj);
static void _gl_sel(void *data, Evas_Object *obj, void *event_info);

static void _set_genlist_item_class(void)
{
	int index = 0;

	for (index = 0; index < NUM_GENLIST_STYLES; index++) {
		sd.itc[index].func.text_get = _gl_text_get;
		sd.itc[index].func.content_get = _gl_content_get;
		sd.itc[index].func.state_get = _gl_state_get;
		sd.itc[index].func.del = _gl_del;
	}

	sd.itc[GENLIST_STYLE_MUSIC_PROGRESS].item_style = "music_player/music_progress";
	sd.itc[GENLIST_STYLE_GROUP_TITLE].item_style = "music_player/group_title";
	sd.itc[GENLIST_STYLE_SONGS].item_style = "music_player/songs";
	sd.itc[GENLIST_STYLE_ALBUM_LIST].item_style = "music_player/album_list";
	sd.itc[GENLIST_STYLE_ARTIST_LIST].item_style = "music_player/artist_list";
	sd.itc[GENLIST_STYLE_TITLE_ARTIST].item_style = "music_player/title_artist";
}

static Eina_Bool _progressbar_timer_cb(void *data)
{
	double value = 0.0;
	int prog_index = (int)data;

	value = elm_progressbar_value_get(sd.progressbar[prog_index]);
	if (value == 1.0)
		value = 0.0;
	value = value + 0.015;
	elm_progressbar_value_set(sd.progressbar[prog_index], value);

	return ECORE_CALLBACK_RENEW;
}

static char *_gl_text_get(void *data, Evas_Object *obj, const char *part)
{
	int index = (int)data;
	char buf[PATH_MAX];

	if (!strcmp(part, "elm.text.title")) {
		return strdup(info[index].title);
	} else if (!strcmp(part, "elm.text.artist")) {
		return strdup(info[index].artist);
	} else if (!strcmp(part, "elm.text.time")) {
		return strdup(info[index].time);
	} else if (!strcmp(part, "elm.text.track_num")) {
		sprintf(buf, "%s tracks", info[index].track_num);
		return strdup(buf);
	} else if (!strcmp(part, "elm.text.album_num")) {
		sprintf(buf, "%s albums", info[index].album_num);
		return strdup(buf);
	} else if (!strcmp(part, "elm.text.group_title")) {
		return strdup(info[index].artist);
	}

	return NULL;
}

static Evas_Object *_gl_content_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *icon = elm_image_add(obj);
	char buf[PATH_MAX];
	int index = (int)data;
	int prog_index = index - 1;

	if (!strcmp(part, "elm.icon")) {
		sprintf(buf, "%s/music_player/%s", ICON_DIR, info[index].file);
		elm_image_file_set(icon, buf, NULL);
		evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
		return icon;
	} else if (!strcmp(part, "elm.swallow.progress")) {
		if (prog_index == 0 || prog_index == 1) {
			if (sd.progressbar_timer[prog_index]) {
				ecore_timer_del(sd.progressbar_timer[prog_index]);
				sd.progressbar_timer[prog_index] = NULL;
				sd.progressbar[prog_index] = NULL;
			}
			sd.progressbar[prog_index] = elm_progressbar_add(obj);
			elm_object_style_set(sd.progressbar[prog_index], "music_player/list_progressive");
			elm_progressbar_horizontal_set(sd.progressbar[prog_index], EINA_TRUE);
			elm_progressbar_value_set(sd.progressbar[prog_index], 0.0);
			elm_progressbar_unit_format_set(sd.progressbar[prog_index], NULL);
			sd.progressbar_timer[prog_index] = ecore_timer_add(sd.progressbar_period[prog_index], _progressbar_timer_cb, (void *)prog_index);
		}
		return sd.progressbar[prog_index];
	}

	return NULL;
}

static Eina_Bool _gl_state_get(void *data, Evas_Object *obj, const char *part)
{
	return EINA_FALSE;
}

static void _gl_del(void *data, Evas_Object *obj)
{
	return;
}

static void _gl_sel(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *item = (Elm_Object_Item *)event_info;
	int index = 0;

	if (item != NULL) {
		elm_genlist_item_selected_set(item, EINA_FALSE);
		index = (int)elm_object_item_data_get(item);
		printf("Selected Text : %s\n", info[index].title);
	}
}

static Eina_Bool _pop_cb(void *data, Elm_Object_Item *it)
{
	int i = 0;
	// Deletes a theme extension from list of extensions.
	elm_theme_extension_del(NULL, ELM_DEMO_EDJ);

	// Delete Progressbar
	for (i = 0; i < 2; i++) {
		if (sd.progressbar_timer[i]) {
			ecore_timer_del(sd.progressbar_timer[i]);
			sd.progressbar_timer[i] = NULL;
		}
		if (sd.progressbar[i]) {
			evas_object_del(sd.progressbar[i]);
			sd.progressbar[i] = NULL;
		}
	}
	return EINA_TRUE;
}

static Evas_Object * _create_layout(void *data)
{
	Evas_Object *layout;
	struct appdata *ad;

	ad = (struct appdata *)data;

	// Create a layout
	layout = elm_layout_add(ad->nf);
	// Set genlist sample group to layout
	elm_layout_file_set(layout, ELM_DEMO_EDJ, "elm_demo_tizen/style_genlist");

	return layout;
}

static void _unrealized_cb(void *data, Evas_Object *obj, void *event_info)
{
	int index = (int)elm_object_item_data_get(event_info);
	if ((index == 1) || (index == 2)) {
		if (sd.progressbar_timer[index - 1]) {
			ecore_timer_del(sd.progressbar_timer[index - 1]);
			sd.progressbar_timer[index - 1] = NULL;
		}
	}
}

static Evas_Object * _create_genlist(Evas_Object *parent)
{
	int index = 0;
	Evas_Object *genlist;
	Elm_Object_Item *item = NULL;
	Elm_Object_Item *git = NULL;

	// Create a genlist
	genlist = elm_genlist_add(parent);
	evas_object_smart_callback_add(genlist, "unrealized", _unrealized_cb, NULL);

	// Append item to the end of genlist
	for (index = 0; info[index].file; index++) {
		// Append item to the end of the genlist
		if (info[index].genlist_style == GENLIST_STYLE_GROUP_TITLE) {
			// Add group item
			git = elm_genlist_item_append(
					genlist,
					&(sd.itc[info[index].genlist_style]),
					(void *)index, NULL, ELM_GENLIST_ITEM_GROUP, _gl_sel, NULL
					);
			elm_genlist_item_select_mode_set(git, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		} else {
			// Add normal item
			item = elm_genlist_item_append(
					genlist,
					&(sd.itc[info[index].genlist_style]),
					(void *)index,
					git,
					ELM_GENLIST_ITEM_NONE,
					_gl_sel,
					NULL
					);
		}
	}

	return genlist;
}

void genlist_music_player_style_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *) data;
	Evas_Object *layout, *genlist;
	Elm_Object_Item *navi_it;
	if (ad == NULL) return;

	// Init Private Data
	sd.progressbar[0] = NULL;
	sd.progressbar_timer[0] = NULL;
	sd.progressbar_period[0] = 0.1;
	sd.progressbar[1] = NULL;
	sd.progressbar_timer[1] = NULL;
	sd.progressbar_period[1] = 0.05;

	// Appends a theme extension to list of extensions.
	elm_theme_extension_add(NULL, ELM_DEMO_EDJ);

	// Create a layout
	layout = _create_layout(ad);

	// Set genlist item class
	_set_genlist_item_class();

	// Create a genlist
	genlist = _create_genlist(layout);
	// Swallow genlist into contents area of layout
	elm_layout_content_set(layout, "contents", genlist);

	// Push genlist to the top of naviframe stack
	navi_it = elm_naviframe_item_push(ad->nf, _("Genlist Style") , NULL, NULL, layout, NULL);
	elm_naviframe_item_pop_cb_set(navi_it, _pop_cb, NULL);
}
