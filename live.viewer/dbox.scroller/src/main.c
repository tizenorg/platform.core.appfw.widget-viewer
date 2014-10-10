#include <Elementary.h>
#include <errno.h>
#include <string.h>

#include <dlog.h>
#include <dynamicbox.h>
#include <dynamicbox_service.h>
#include <dynamicbox_errno.h>

#include <aul.h>

#include "main.h"
#include "genlist06.h"

#define PUBLIC __attribute__((visibility("default")))

static Eina_List *s_list;
int errno;

struct info {
	char *id;
	char *content;
	int size_type;
	Evas_Object *dbox_win;
	enum edge {
		LEFT,
		RIGHT,
		MIDDLE,
	} edge;
	struct down {
		int x;
		int y;
		int pressed;
	} down;
};

static inline struct info *find_item(const char *id)
{
	Eina_List *l;
	struct info *item;

	EINA_LIST_FOREACH(s_list, l, item) {
		if (!strcmp(item->id, id)) {
			return item;
		}
	}

	return NULL;
}

PUBLIC int dynamicbox_initialize(const char *pkgname)
{
	/**
	 * @TODO
	 * Do one-time initialize.
	 * This will be called only once right before the first box is creating
	 */
	return 0;
}

PUBLIC int dynamicbox_finalize(void)
{
	/**
	 * @TODO
	 * Do one-time finalization. 
	 * This will be called only once right after the last box is destroyed
	 */
	return 0;
}

// NOTE: This function is going to be invoked for initializing all resources
PUBLIC int dynamicbox_create(const char *id, const char *content, const char *cluster, const char *category)
{
	struct info *info;

	info = malloc(sizeof(*info));
	if (!info) {
		return DBOX_STATUS_ERROR_OUT_OF_MEMORY;
	}

	info->id = strdup(id);
	if (!info->id) {
		free(info);
		return DBOX_STATUS_ERROR_OUT_OF_MEMORY;
	}

	info->content = strdup(content);
	if (!info->content) {
		free(info->id);
		free(info);
		return DBOX_STATUS_ERROR_OUT_OF_MEMORY;
	}

	/**
	 * @NOTE
	 * cluster == 'user,created'
	 * category == 'default'
	 *
	 * You don't need to care these two values if you don't know what are them
	 */

	info->size_type = DBOX_SIZE_TYPE_UNKNOWN;
	s_list = eina_list_append(s_list, info);

	/**
	 * @NOTE
	 * You can returns DBOX_OUTPUT_UPDATED or DBOX_NEED_TO_SCHEDULE or DBOX_DONE
	 * You also can use them at same time using '|'
	 *
	 * If your content is updated, from this function, you have to
	 * return DBOX_OUTPUT_UPDATED;
	 *
	 * If you want to the provider call your dynamicbox_update_content function ASAP
	 * return DBOX_NEED_TO_SCHEDULE;
	 *
	 * If your content is updated and need to call the update_content function ASAP,
	 * return DBOX_OUTPUT_UPDATED | DBOX_NEED_TO_SCHEDULE
	 *
	 * Don't have any changes, just
	 * return DBOX_DONE
	 */

	/**
	 * @NOTE
	 * You create the default output image from here now.
	 * So you HAVE TO return DBOX_OUTPUT_UPDATED
	 */
	return DBOX_DONE;
}

// NOTE: This function is going to be invoked for release all resources
PUBLIC int dynamicbox_destroy(const char *id)
{
	struct info *item;
	LOGD("[%s:%d]\n", __func__, __LINE__);

	item = find_item(id);
	if (!item) {
		/*!
		 * \NOTE
		 * EXCEPTIONAL CASES
		 */
		return DBOX_STATUS_ERROR_NOT_EXIST;
	}

	s_list = eina_list_remove(s_list, item);

	/* NOTE: You have to clear all resource which are related with
	 *       current instance. If you didn't clear it correctly,
	 *       the live data provider will get suffer from the memory
	 *       pressure. therefore, keep trace on your resources.
	 */
	if (item->dbox_win) {
		evas_object_del(item->dbox_win);
	}
	free(item->content);
	free(item->id);
	free(item);
	return DBOX_DONE;
}

PUBLIC int dynamicbox_need_to_update(const char *id)
{
	struct info *item;

	LOGD("[%s:%d]\n", __func__, __LINE__);

	item = find_item(id);
	if (!item) {
		/* Hmm, there is no matched instance. */
		return DBOX_STATUS_ERROR_NOT_EXIST;
	}

	/**
	 * @TODO
	 * Check the content,
	 * If you found any changes in your content data,
	 * return 1 or 0
	 *
	 * If you return 1, the provider will schedule to update your box ASAP.
	 * Or the provider will skip to call dynamicbox_update_content function.
	 *
	 * this function can be called several times, before call the dynamicbox_update_content function.
	 *
	 * if your dynamicbox_update_content is not called, and you have content which need to be updated, you have to
	 * keep return 1
	 */
	return 1;
}

PUBLIC int dynamicbox_update_content(const char *id)
{
	struct info *item;

	LOGD("[%s:%d]\n", __func__, __LINE__);

	item = find_item(id);
	if (!item) {
		return DBOX_STATUS_ERROR_NOT_EXIST;
	}

	/**
	 * @NOTE
	 * This function have to generate a new content.
	 * If you don't want to generate new content,
	 * return negative values.
	 * or you have to generate the new content. in timeout msec.
	 */
	return DBOX_STATUS_ERROR_DISABLED;
}

PUBLIC int dynamicbox_clicked(const char *id, const char *event, double timestamp, double x, double y)
{
	struct info *item;
	int ix;
	int iy;
	int w;
	int h;

	/**
	 * @NOTE:
	 *   You will get this if a user press your image.
	 *   You can get the absolute coordinate using the given x,y.
	 *
	 *   event: clicked
	 */

	item = find_item(id);
	if (!item) {
		return DBOX_STATUS_ERROR_NOT_EXIST;
	}

	(void)dynamicbox_service_get_size(item->size_type, &w, &h);
	ix = (int)((double)w * x);
	iy = (int)((double)h * y);

	LOGD("[%s:%d] event[%s] %lf %lf (%lf)\n", __func__, __LINE__, event, x, y, timestamp);
	LOGD("[%s:%d] on your image (%d, %d)\n", __func__, __LINE__, ix, iy);

	/**
	 * @NOTE
	 * If you can generate new content in this function,
	 * Generate a new content.
	 * and return DBOX_OUTPUT_UPDATED
	 *
	 * In case of you cannot create the updated image in this function directly,
	 * return DBOX_NEED_TO_SCHEDULE
	 * The provider will call your dynamicbox_need_to_update & dynamicbox_update_content function.
	 *
	 * I recommend that if you are able to generate new content in this function,
	 * generate it directly. and just returns DBOX_OUTPUT_UPDATED
	 *
	 * Because if you return DBOX_NEED_TO_SCHEDULE, the provider will try to update your livebox
	 * But it can be interrupted by other events.
	 * Then you livebox content updating will be delayed
	 */
	return DBOX_DONE; /* No chages */
}

PUBLIC int dynamicbox_content_event(const char *id, const char *emission, const char *source, dynamicbox_event_info_t event_info)
{
	struct timeval tv;
	char buffer[BUFSIZ];
	struct info *item;

	LOGD("[%s:%d] Emission : %s, source: %s [%lf x %lf - %lf x %lf] (%lf x %lf - %d)\n",
		__func__, __LINE__,
		emission, source,
		event_info->part.sx, event_info->part.sy, event_info->part.ex, event_info->part.ey,
		event_info->pointer.x, event_info->pointer.y, event_info->pointer.down);

	item = find_item(id);
	if (!item) {
		return DBOX_STATUS_ERROR_NOT_EXIST;
	}

	gettimeofday(&tv, NULL);
	snprintf(buffer, sizeof(buffer), "%lu.%lu", tv.tv_sec, tv.tv_usec);

	if (!strcmp(emission, "gbar,show") && !strcmp(source, id)) {
		/**
		 * @NOTE
		 * GBAR is opened
		 */
	} else if (!strcmp(emission, "gbar,hide") && !strcmp(source, id)) {
		/*
		 * @NOTE
		 * GBAR is closed
		 */
	}

	/**
	 * @NOTE
	 * If you can generate new content in this function,
	 * Generate a new content.
	 * and return DBOX_OUTPUT_UPDATED
	 *
	 * In case of you cannot create the updated image in this function directly,
	 * return DBOX_NEED_TO_SCHEDULE
	 * The provider will call your dynamicbox_need_to_update & dynamicbox_update_content function.
	 *
	 * I recommend that if you are able to generate new content in this function,
	 * generate it directly. and just returns DBOX_OUTPUT_UPDATED
	 *
	 * Because if you return DBOX_NEED_TO_SCHEDULE, the provider will try to update your livebox
	 * But it can be interrupted by other events.
	 * Then you livebox content updating will be delayed
	 */
	return DBOX_DONE; /* No changes */
}

PUBLIC int dynamicbox_resize(const char *id, int type)
{
	struct info *item;
	int w;
	int h;
	int ret;

	item = find_item(id);
	if (!item) {
		return DBOX_STATUS_ERROR_NOT_EXIST;
	}

	/*!
	 * \TODO
	 * Check the size type, which is supporting by your box.
	 * If the invalid size is requested, you should returns DBOX_STATUS_ERROR_INVALID_PARAMETER
	 */
	ret = dynamicbox_service_get_size(type, &w, &h);
	if (ret != DBOX_STATUS_ERROR_NONE) {
		LOGE("Unsupported size\n");
		return DBOX_STATUS_ERROR_INVALID_PARAMETER;
	}

	if (item->size_type == DBOX_SIZE_TYPE_UNKNOWN) {
		Evas_Object *dbox_parent;

		dbox_parent = dynamicbox_get_evas_object(id, 0);
		if (!dbox_parent) {
			LOGE("Failed to get an evas object\n");
			return DBOX_STATUS_ERROR_FAULT;
		}

		item->dbox_win = elm_win_add(dbox_parent, "DBox Window", ELM_WIN_DYNAMIC_BOX);
		evas_object_del(dbox_parent);
		if (!item->dbox_win) {
			return DBOX_STATUS_ERROR_FAULT;
		}

		evas_object_resize(item->dbox_win, w, h);
		evas_object_show(item->dbox_win);

		genlist_test(item->dbox_win);

		LOGD("Window & Genlist is created\n");
	} else {
		evas_object_resize(item->dbox_win, w, h);
	}

	LOGD("%s is resized to %dx%d\n", id, w, h);
	item->size_type = type;

	/**
	 * @NOTE
	 * If you can generate new content in this function,
	 * Generate a new content.
	 * and return DBOX_OUTPUT_UPDATED
	 *
	 * In case of you cannot create the updated image in this function directly,
	 * return DBOX_NEED_TO_SCHEDULE
	 * The provider will call your dynamicbox_need_to_update & dynamicbox_update_content function.
	 *
	 * I recommend that if you are able to generate new content in this function,
	 * generate it directly. and just returns DBOX_OUTPUT_UPDATED
	 *
	 * Because if you return DBOX_NEED_TO_SCHEDULE, the provider will try to update your livebox
	 * But it can be interrupted by other events.
	 * Then you livebox content updating will be delayed
	 */
	return DBOX_NEED_TO_SCHEDULE;
}

PUBLIC int dynamicbox_need_to_create(const char *cluster, const char *category)
{
	/**
	 * @NOTE
	 * You don't need implement this, if don't know what this is.
	 * return 0 or 1
	 */
	return 0;
}

PUBLIC int dynamicbox_change_group(const char *id, const char *cluster, const char *category)
{
	struct info *item;

	item = find_item(id);
	if (!item) {
		return DBOX_STATUS_ERROR_NOT_EXIST;
	}

	/**
	 * @NOTE
	 * If you can generate new content in this function,
	 * Generate a new content.
	 * and return DBOX_OUTPUT_UPDATED
	 *
	 * In case of you cannot create the updated image in this function directly,
	 * return DBOX_NEED_TO_SCHEDULE
	 * The provider will call your dynamicbox_need_to_update & dynamicbox_update_content function.
	 *
	 * I recommend that if you are able to generate new content in this function,
	 * generate it directly. and just returns DBOX_OUTPUT_UPDATED
	 *
	 * Because if you return DBOX_NEED_TO_SCHEDULE, the provider will try to update your livebox
	 * But it can be interrupted by other events.
	 * Then you livebox content updating will be delayed
	 */
	return DBOX_DONE;
}

PUBLIC int dynamicbox_need_to_destroy(const char *id)
{
	/**
	 * @NOTE
	 * You don't need implement this, if don't know what this is.
	 * This will be called after call the dynamicbox_need_to_update function.
	 * If the dynamicbox_need_to_update function returns 0,
	 * The provider will call this.
	 *
	 * If you return 1, the provider will delete your box.
	 */
	return 0;
}

PUBLIC char *dynamicbox_pinup(const char *id, int pinup)
{
	struct info *item;

	item = find_item(id);
	if (!item) {
		return NULL;
	}

	return strdup(item->content);
}

PUBLIC int dynamicbox_is_pinned_up(const char *id)
{
	struct info *item;

	item = find_item(id);
	if (!item) {
		return DBOX_STATUS_ERROR_NOT_EXIST;
	}

	/**
	 * @NOTE
	 * If you can generate new content in this function,
	 * Generate a new content.
	 * and return DBOX_OUTPUT_UPDATED
	 *
	 * In case of you cannot create the updated image in this function directly,
	 * return DBOX_NEED_TO_SCHEDULE
	 * The provider will call your dynamicbox_need_to_update & dynamicbox_update_content function.
	 *
	 * I recommend that if you are able to generate new content in this function,
	 * generate it directly. and just returns DBOX_OUTPUT_UPDATED
	 *
	 * Because if you return DBOX_NEED_TO_SCHEDULE, the provider will try to update your livebox
	 * But it can be interrupted by other events.
	 * Then you livebox content updating will be delayed
	 */
	return DBOX_DONE;
}

PUBLIC int dynamicbox_system_event(const char *id, int type)
{
	struct info *item;

	item = find_item(id);
	if (!item) {
		return DBOX_STATUS_ERROR_NOT_EXIST;
	}

	if (type == DBOX_SYS_EVENT_FONT_CHANGED) {
		LOGD("Font is changed\n");
	} else if (type == DBOX_SYS_EVENT_LANG_CHANGED) {
		LOGD("Language is changed\n");
	} else if (type == DBOX_SYS_EVENT_TIME_CHANGED) {
		LOGD("Time is changed\n");
	} else if (type == DBOX_SYS_EVENT_REGION_CHANGED) {
		LOGD("Region is changed\n");
	} else if (type == DBOX_SYS_EVENT_PAUSED) {
		LOGD("Paused\n");
		// dynamicbox_release_scroller("com.samsung.w-add-viewer.dbox", id);
	} else if (type == DBOX_SYS_EVENT_RESUMED) {
		LOGD("Resumed [%s]\n", id);
		// dynamicbox_freeze_scroller("com.samsung.w-add-viewer.dbox", id);
	} else if (type == DBOX_SYS_EVENT_MMC_STATUS_CHANGED) {
		LOGD("MMC");
	} else {
		LOGD("Unknown");
	}

	/**
	 * @NOTE
	 * If you can generate new content in this function,
	 * Generate a new content. and return DBOX_OUTPUT_UPDATED
	 *
	 * In case of you cannot create the updated image in this function directly,
	 * return DBOX_NEED_TO_SCHEDULE
	 * The provider will call your dynamicbox_need_to_update & dynamicbox_update_content function.
	 *
	 * I recommend that if you are able to generate new content in this function,
	 * generate it directly. and just returns DBOX_OUTPUT_UPDATED
	 *
	 * Because if you return DBOX_NEED_TO_SCHEDULE, the provider will try to update your livebox
	 * But it can be interrupted by other events.
	 * Then you livebox content updating will be delayed
	 *
	 * DBOX_FORCE_TO_SCHEDULE - If you returns this, the provider will try to update your box
	 *                     even though the box is paused you will get need_to_update & update_content function call.
	 */
	return DBOX_DONE;
}

/* End of a file */
