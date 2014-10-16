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
#include "fastscroll.h"
#include <locale.h>

/*********************************************************
  Fastscroll
 ********************************************************/

static Evas_Object* _create_scroller(Evas_Object* parent)
{
	Evas_Object* scroller = elm_scroller_add(parent);
	elm_scroller_bounce_set(scroller, EINA_FALSE, EINA_TRUE);
	elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	evas_object_show(scroller);

	return scroller;
}

static void _index_selected(void *data, Evas_Object *obj, void *event_info)
{
	elm_index_item_selected_set(event_info, EINA_FALSE);
}

static void
_index_changed_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
	// this is called on every change, no matter how often
//	elm_genlist_item_bring_in(elm_object_item_data_get(event_info),
//		ELM_GENLIST_ITEM_SCROLLTO_TOP);
}

static void _index_level_up_cb(void *data , Evas_Object *obj , void *event_info)
{
	const Elm_Object_Item *it, *new_item;
	const Elm_Object_Item *index_obj;
	char buf[27], index1_letter, *index0_letter;
	int i = 0, j = 0, k, index, flag = 0;
	int level = elm_index_item_level_get(obj);

	index_obj = elm_index_selected_item_get(obj, level - 1);
	index0_letter =elm_index_item_letter_get(index_obj);
	for(i = 0; i < 5; i++)
	{
		char buf[32];
		snprintf(buf, sizeof(buf), "%c", index0_letter[0] + i + 1);
		if(buf[0] >90 || buf[0] < 65)
		{
			j = 0;
			k = 65;
			while(i <5)
			{
				snprintf(buf, sizeof(buf), "%c", k + j);
				elm_index_item_append(obj, buf, NULL, NULL);
				i++;
				j++;
			}
			break;
		}
		elm_index_item_append(obj, buf, NULL, NULL);
	}
	elm_index_level_go(obj, 1);
}

static Evas_Object* _create_fastscroll(Evas_Object* parent, Evas_Object* scroller)
{
	Evas_Object *layout, *index;
	int i = 0, j, len;
	char *str;
	char buf[PATH_MAX] = {0, };
	Eina_Unicode uni;
	Elm_Object_Item *it;
	const char *locale;

	layout = elm_layout_add(parent);
	elm_layout_theme_set(layout, "layout", "application", "fastscroll");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_content_set(scroller, layout);

	index = elm_index_add(layout);
	elm_object_part_content_set(layout, "elm.swallow.fastscroll", index);
	elm_index_autohide_disabled_set(index, EINA_TRUE);
	elm_index_omit_enabled_set(index, EINA_TRUE);

	//1. Special character & Numbers
	elm_index_item_append(index, "#", NULL, NULL);

	//2. Local language
	str = dgettext("efl-assist", "IDS_EA_BODY_ABCDEFGHIJKLMNOPQRSTUVWXYZ");
	len = strlen(str);

	while (i < len)
	{
		j = i;
		uni = eina_unicode_utf8_get_next(str, &i);
		snprintf(buf, i - j + 1, "%s", str + j);
		buf[i - j + 1] = 0;

		it = elm_index_item_append(index, buf, NULL, NULL);
		elm_index_item_priority_set(it, 0);
	}

	//3. English - in case of non-latin
	locale = vconf_get_str(VCONFKEY_LANGSET);
	if (!ea_locale_latin_get(locale))
	{
		str = dgettext("efl-assist", "IDS_EA_BODY_ABCDEFGHIJKLMNOPQRSTUVWXYZ_SECOND");
		len = strlen(str);

		i = 0;
		while (i < len)
		{
			j = i;
			uni = eina_unicode_utf8_get_next(str, &i);
			snprintf(buf, i - j + 1, "%s", str + j);
			buf[i - j + 1] = 0;

			it = elm_index_item_append(index, buf, NULL, NULL);
			elm_index_item_priority_set(it, 1);
		}
	}

	elm_index_level_go(index, 0);
	elm_index_item_level_set(index,1);

	evas_object_smart_callback_add(index, "selected", _index_selected, NULL);
	evas_object_smart_callback_add(index, "level,up", _index_level_up_cb, NULL);
	evas_object_smart_callback_add(index, "changed", _index_changed_cb, NULL);

	return index;
}

void _language_changed(void *data, Evas_Object *obj, void *event_info)
{
	int i = 0, j, len;
	char *str;
	char buf[PATH_MAX] = {0, };
	Eina_Unicode uni;
	Elm_Object_Item *it;
	const char *locale;

	elm_index_item_clear(obj);

	//1. Special character & Numbers
	elm_index_item_append(obj, "#", NULL, NULL);

	//2. Local language
	str = dgettext("efl-assist", "IDS_EA_BODY_ABCDEFGHIJKLMNOPQRSTUVWXYZ");
	len = strlen(str);

	while (i < len)
	{
		j = i;
		uni = eina_unicode_utf8_get_next(str, &i);
		snprintf(buf, i - j + 1, "%s", str + j);
		buf[i - j + 1] = 0;

		it = elm_index_item_append(obj, buf, NULL, NULL);
		elm_index_item_priority_set(it, 0);
	}

	//3. English - in case of non-latin
	locale = vconf_get_str(VCONFKEY_LANGSET);
	if (!ea_locale_latin_get(locale))
	{
		str = dgettext("efl-assist", "IDS_EA_BODY_ABCDEFGHIJKLMNOPQRSTUVWXYZ_SECOND");
		len = strlen(str);

		i = 0;
		while (i < len)
		{
			j = i;
			uni = eina_unicode_utf8_get_next(str, &i);
			snprintf(buf, i - j + 1, "%s", str + j);
			buf[i - j + 1] = 0;

			it = elm_index_item_append(obj, buf, NULL, NULL);
			elm_index_item_priority_set(it, 1);
		}
	}

	elm_index_level_go(obj, 0);
}

void
fastscroll_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad;
	Evas_Object *scroller, *index;

	ad = (struct appdata *) data;
	if (ad == NULL) return;

	scroller = _create_scroller(ad->nf);
	elm_naviframe_item_push(ad->nf, _("Fastscroll"), NULL, NULL, scroller, NULL);

	index = _create_fastscroll(ad->nf, scroller);
	evas_object_smart_callback_add(index, "language,changed", _language_changed, index);
}
