#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "dlist.h"

struct dlist {
	struct dlist *next;
	struct dlist *prev;
	void *data;
};

struct dlist *dlist_append(struct dlist *list, void *data)
{
	struct dlist *item;

	item = malloc(sizeof(*item));
	if (!item)
		return NULL;

	item->next = NULL;
	item->data = data;

	if (!list) {
		item->prev = item;

		list = item;
	} else {
		item->prev = list->prev;
		item->prev->next = item;

		list->prev = item;
	}

	assert(!list->prev->next && "item NEXT");

	return list;
}

struct dlist *dlist_prepend(struct dlist *list, void *data)
{
	struct dlist *item;

	item = malloc(sizeof(*item));
	if (!item)
		return NULL;

	if (!list) {
		item->prev = item;
		item->next = NULL;
	} else {
		item->prev = list->prev;
		list->prev = item;
		item->next = list;
	}

	return item;
}

struct dlist *dlist_remove(struct dlist *list, struct dlist *l)
{
	if (!list || !l)
		return NULL;

	if (l == list) {
		l->prev = list->prev;
		list = l->next;
	} else {
		l->prev->next = l->next;
	}

	if (l->next)
		l->next->prev = l->prev;

	free(l);
	return list;
}

struct dlist *dlist_find_data(struct dlist *list, void *data)
{
	struct dlist *l;
	void *_data;

	dlist_foreach(list, l, _data) {
		if (data == _data)
			return l;
	}

	return NULL;
}

void *dlist_data(struct dlist *l)
{
	return l ? l->data : NULL;
}

struct dlist *dlist_next(struct dlist *l)
{
	return l ? l->next : NULL;
}

struct dlist *dlist_prev(struct dlist *l)
{
	return l ? l->prev : NULL;
}

int dlist_count(struct dlist *l)
{
	register int i;
	struct dlist *n;
	void *data;

	i = 0;
	dlist_foreach(l, n, data) {
		i++;
	}

	return i;
}

struct dlist *dlist_nth(struct dlist *l, int nth)
{
	register int i;
	struct dlist *n;
	void *data;

	i = 0;
	dlist_foreach(l, n, data) {
		if (i == nth)
			return l;

		i++;
	}

	return NULL;
}

/* End of a file */
