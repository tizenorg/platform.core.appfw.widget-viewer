#include <Elementary.h>
#include <errno.h>
#include <string.h>

#include <X11/Xlib.h>
#include <Ecore.h>
#include <Ecore_X.h>

#include <dynamicbox_provider.h>
#include <dynamicbox_provider_buffer.h>

#include <dlog.h>
#include <dynamicbox.h>
#include <dynamicbox_service.h>
#include <dynamicbox_errno.h>

#include <aul.h>
#include <mm_player.h>
#include <mm_message.h>
#include <mm_error.h>
#include <mm_types.h>

#include "main.h"
#include "player.h"

int errno;

int dynamicbox_player_updated(struct info *info)
{
	int ret;
	char *uri;
	int len;
	dynamicbox_damage_region_t region = {
		.x = 0,
		.y = 0,
	};

	if (dynamicbox_service_get_size(info->size_type, &region.w, &region.h) < 0) {
		LOGE("Failed to get size\n");
		return DBOX_STATUS_ERROR_INVALID_PARAMETER;
	}

	len = strlen(info->id) + strlen("file://") + 2;

	uri = malloc(len + 1);
	if (!uri) {
		LOGE("Heap: %s\n", strerror(errno));
		return ECORE_CALLBACK_PASS_ON;
	}
	snprintf(uri, len, "file://%s", info->id);

	ret = dynamicbox_provider_send_updated("com.samsung.w-add-viewer.video", uri, &region);
	free(uri);

	LOGD("============== %s\n", info->id);
	return ret;
}

static int mm_message_handler_cb(int message, void *param, void *user_param)
{
	struct info *info = user_param;
	switch (message) {
	case MM_MESSAGE_END_OF_STREAM:
		dynamicbox_player_stop(info);
		break;
	default:
		return FALSE;
	}

	return TRUE;
}

void clear(unsigned int pixmap, int w, int h)
{
	static unsigned int s_color = 0x00FF0000;
	Display *disp;
	GC gc;

	disp = (Display *)ecore_x_display_get();
	gc = XCreateGC(disp, pixmap, 0, 0);
	XSetForeground(disp, gc, 0xFF000000 | s_color);
	XFillRectangle(disp, pixmap, gc, 0, 0, w, h);
	XSync(disp, FALSE);
	XFreeGC(disp, gc);

	s_color >>= 2;
	if (s_color == 0) {
		s_color = 0xFF000000;
	}
}

int dynamicbox_player_refresh(struct info *info)
{
	unsigned int pixmap;
	char *err = NULL;
	int w;
	int h;

	if (dynamicbox_service_get_size(info->size_type, &w, &h) < 0) {
		LOGE("Failed to get size\n");
		return DBOX_STATUS_ERROR_INVALID_PARAMETER;
	}

	pixmap = dynamicbox_provider_buffer_resource_id(info->dbox_buffer);
	if (pixmap == 0) {
		LOGE("Failed to get correct pixmap\n");
		return DBOX_STATUS_ERROR_INVALID_PARAMETER;
	}

	if (info->damage) {
		ecore_x_damage_free(info->damage);
	}

	info->damage = ecore_x_damage_new(pixmap, ECORE_X_DAMAGE_REPORT_RAW_RECTANGLES);
	if (!info->damage) {
		LOGE("Failed to create a damage handler\n");
		return DBOX_STATUS_ERROR_FAULT;
	}

	clear(pixmap, w, h);

	if (access(info->filename, F_OK) != 0) {
		LOGE("Failed to access a file: %s\n", strerror(errno));
		return DBOX_STATUS_ERROR_INVALID_PARAMETER;
	}

	LOGD("Pixmap: %x (%dx%d)\n", pixmap, w, h);

	mm_player_set_attribute(info->player, &err,
			"display_surface_type", MM_DISPLAY_SURFACE_X,
			"display_width", w,
			"display_height", h,
			"display_overlay", &pixmap, sizeof(pixmap),
			"profile_uri", info->filename, strlen(info->filename),
			"display_rotation", MM_DISPLAY_ROTATION_NONE,
			"profile_play_count", 1,
			NULL); 
	if (err) {
		// Error
		LOGE("ERROR: %s\n", err);
	}

	return DBOX_STATUS_ERROR_NONE;
}

static inline Eina_Bool damage_cb(void *data, int type, void *event)
{
	struct info *info = data;
	Ecore_X_Event_Damage *e = (Ecore_X_Event_Damage *)event;

	if (e->drawable == dynamicbox_provider_buffer_resource_id(info->dbox_buffer)) {
		// For the ugly mm-player
		static struct timeval stv;
		static int first = 0;
		struct timeval etv;
		struct timeval rtv;

		if (first == 0) {
			gettimeofday(&stv, NULL);
			first = 1;
		} else {
			gettimeofday(&etv, NULL);
			timersub(&etv, &stv, &rtv);
			gettimeofday(&stv, NULL);

			if (rtv.tv_usec > 20000) {
				dynamicbox_player_updated(info);
			}
		}
	}

	ecore_x_damage_subtract(e->damage, None, None);
	return ECORE_CALLBACK_PASS_ON;
}

int dynamicbox_player_create(struct info *info)
{
	if (info->player_state != PLAYER_DESTROYED) {
		return DBOX_STATUS_ERROR_NONE;
	}

	if (mm_player_create(&info->player) != MM_ERROR_NONE) {
		LOGE("Failed to create mm player\n");
		return DBOX_STATUS_ERROR_FAULT;
	}

	mm_player_set_message_callback(info->player, mm_message_handler_cb, info);
	info->player_state = PLAYER_CREATED;

	info->damage_handler = ecore_event_handler_add(ECORE_X_EVENT_DAMAGE_NOTIFY, damage_cb, info);
	if (!info->damage_handler) {
		LOGE("Failed to add damage notifier\n");
		mm_player_destroy(info->player);
		return DBOX_STATUS_ERROR_INVALID_PARAMETER;
	}

	return DBOX_STATUS_ERROR_NONE;
}

int dynamicbox_player_play(struct info *info)
{
	if (info->player_state == PLAYER_PLAY) {
		return DBOX_STATUS_ERROR_NONE;
	}

	if (mm_player_realize(info->player) != MM_ERROR_NONE) {
		LOGE("Failed to realize a player\n");
		return DBOX_STATUS_ERROR_FAULT;
	}

	mm_player_start(info->player);
	info->player_state = PLAYER_PLAY;
	return DBOX_STATUS_ERROR_NONE;
}

int dynamicbox_player_stop(struct info *info)
{
	if (!info->player || info->player_state != PLAYER_PLAY) {
		return DBOX_STATUS_ERROR_NONE;
	}

	mm_player_stop(info->player);
	mm_player_unrealize(info->player);
	info->player_state = PLAYER_STOP;

	return DBOX_STATUS_ERROR_NONE;
}

int dynamicbox_player_destroy(struct info *info)
{
	dynamicbox_player_stop(info);

	if (info->damage_handler) {
		ecore_event_handler_del(info->damage_handler);
		info->damage_handler = NULL;
	}

	if (info->player) {
		mm_player_destroy(info->player);
		info->player = 0;
	}

	info->player_state = PLAYER_DESTROYED;
	return DBOX_STATUS_ERROR_NONE;
}

int dynamicbox_player_state(struct info *info)
{
	return info->player_state;
}

/* End of a file */
