#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <Eina.h>

#include <dynamicbox_service.h>
#include <dynamicbox_errno.h>

#include "instance.h"

struct instance {
	char *id;
	char *content;
	int w;
	int h;
	void *data;
};

static struct info {
	Eina_List *inst_list;
} s_info = {
	.inst_list = NULL,
};

struct instance *instance_create(const char *id, const char *content, int w, int h)
{
	struct instance *inst;

	inst = calloc(1, sizeof(*inst));
	if (!inst) {
		return NULL;
	}

	inst->id = strdup(id);
	if (!inst->id) {
		free(inst);
		return NULL;
	}

	inst->w = w;
	inst->h = h;

	s_info.inst_list = eina_list_append(s_info.inst_list, inst);

	return inst;
}

int instance_destroy(struct instance *inst)
{
	s_info.inst_list = eina_list_remove(s_info.inst_list, inst);
	free(inst->id);
	free(inst);
	return DBOX_STATUS_ERROR_NONE;
}

struct instance *instance_find(const char *id)
{
	Eina_List *l;
	struct instance *inst;

	if (!id) {
		return NULL;
	}

	EINA_LIST_FOREACH(s_info.inst_list, l, inst) {
		if (!strcmp(inst->id, id)) {
			return inst;
		}
	}

	return NULL;
}

int instance_set_size_by_id(const char *id, int w, int h)
{
	struct instance *inst;

	inst = instance_find(id);
	if (!inst) {
		return DBOX_STATUS_ERROR_INVALID_PARAMETER;
	}

	return instance_set_size_by_handle(inst, w, h);
}

int instance_set_size_by_handle(struct instance *inst, int w, int h)
{
	inst->w = w;
	inst->h = h;

	return DBOX_STATUS_ERROR_NONE;
}

int instance_get_size_by_id(const char *id, int *w, int *h)
{
	struct instance *inst;

	inst = instance_find(id);
	if (!inst) {
		return DBOX_STATUS_ERROR_INVALID_PARAMETER;
	}

	return instance_get_size_by_handle(inst, w, h);
}

int instance_get_size_by_handle(struct instance *inst, int *w, int *h)
{
	*w = inst->w;
	*h = inst->h;

	return DBOX_STATUS_ERROR_NONE;
}

int instance_set_data(struct instance *inst, void *data)
{
	inst->data = data;
	return DBOX_STATUS_ERROR_NONE;
}

void *instance_data(struct instance *inst)
{
	return inst->data;
}

int instance_crawling(int (*cb)(struct instance *inst, void *data), void *data)
{
	Eina_List *l;
	Eina_List *n;
	struct instance *inst;
	int cnt = 0;

	EINA_LIST_FOREACH_SAFE(s_info.inst_list, l, n, inst) {
		if (cb(inst, data) < 0) {
			break;
		}
		cnt++;
	}

	return cnt;
}

/* End of a file */
