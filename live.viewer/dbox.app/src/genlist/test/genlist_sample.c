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

#include "genlist.h"

/* Clear up all genlist items explicitly */
static void genlist_clear(Evas_Object *genlist)
{
	Elm_Object_Item *it = NULL;
	if (!genlist) return;

	while ((it = elm_genlist_first_item_get(genlist))) {
		elm_object_item_del(it);
	}
}

/* Update realized items only */
static void genlist_realized_items_update(Evas_Object *genlist)
{
	Elm_Object_Item *it = NULL;
	Eina_List *realized_items = NULL, *l = NULL;
	if (!genlist) return;

	realized_items = elm_genlist_realized_items_get(genlist);
	EINA_LIST_FOREACH(realized_items, l, it) {
		elm_genlist_item_update(it);
	}
}
