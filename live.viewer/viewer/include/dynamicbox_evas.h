/*
 * Samsung API
 * Copyright (c) 2013 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Flora License, Version 1.1 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://floralicense.org/license/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __DYNAMICBOX_EVAS_H
#define __DYNAMICBOX_EVAS_H

#ifdef __cplusplus
extern "C" {
#endif

#define DYNAMICBOX_EVAS_DEFAULT_PERIOD                -1.0f                   /**< Default Update Period */
#define DYNAMICBOX_SMART_SIGNAL_DBOX_CREATE_ABORTED   "dbox,create,aborted"   /**< Livebox creation is aborted */
#define DYNAMICBOX_SMART_SIGNAL_DBOX_CREATED          "dbox,created"          /**< Livebox is created */
#define DYNAMICBOX_SMART_SIGNAL_DBOX_RESIZE_ABORTED   "dbox,resize,aborted"   /**< Resizing dynamicbox is aborted */
#define DYNAMICBOX_SMART_SIGNAL_DBOX_RESIZED          "dbox,resized"          /**< Livebox is resized */
#define DYNAMICBOX_SMART_SIGNAL_DBOX_FAULTED          "dbox,faulted"          /**< Livebox has faulted */
#define DYNAMICBOX_SMART_SIGNAL_UPDATED               "updated"               /**< Livebox content is updated */
#define DYNAMICBOX_SMART_SIGNAL_EXTRA_INFO_UPDATED    "info,updated"          /**< Livebox extra info is updated */
#define DYNAMICBOX_SMART_SIGNAL_PROVIDER_DISCONNECTED "provider,disconnected" /**< Provider is disconnected */
#define DYNAMICBOX_SMART_SIGNAL_GBAR_DESTROYED        "gbar,destroyed"        /**< GBAR is destroyed */
#define DYNAMICBOX_SMART_SIGNAL_GBAR_ABORTED          "gbar,aborted"          /**< GBAR creation is aborted */
#define DYNAMICBOX_SMART_SIGNAL_GBAR_CREATED          "gbar,created"          /**< GBAR is created */
#define DYNAMICBOX_SMART_SIGNAL_FLICKDOWN_CANCELLED   "flickdown,cancelled"   /**< Flick down is canceld */
#define DYNAMICBOX_SMART_SIGNAL_CONTROL_SCROLLER      "control,scroller"      /**< Control Scroller */
#define DYNAMICBOX_SMART_SIGNAL_DBOX_DELETED          "dbox,deleted"          /**< DynamicBox is deleted */
#define DYNAMICBOX_SMART_SIGNAL_PERIOD_CHANGED        "dbox,period,changed"   /**< Period is changed */

/**
 * \brief
 * Data structure for Smart Callback Event
 */
struct dynamicbox_evas_event_info {
	const char *pkgname;	/**< Livebox Application Id */
	int event;		/**< Event type - DBOX_EVENT_XXX, refer the dynamicbox.h */
	int error;		/**< Error type - DBOX_STATUS_XXX, refer the dynamicbox.h */
};

enum dynamicbox_evas_raw_event_type {
	DYNAMICBOX_EVAS_RAW_DELETE = 0x00,
	DYNAMICBOX_EVAS_RAW_CREATE = 0x02,
	DYNAMICBOX_EVAS_RAW_MAX = 0xff,
};

struct dynamicbox_evas_raw_event_info {
	const char *pkgname;
	enum dynamicbox_evas_raw_event_type type;
	int error;
	Evas_Object *dynamicbox;
};

/**
 * \brief
 * Configuration keys
 */
enum dynamicbox_evas_conf {
	DYNAMICBOX_EVAS_MANUAL_PAUSE_RESUME = 0x0001,	/**< Visibility will be changed manually */
	DYNAMICBOX_EVAS_SHARED_CONTENT = 0x0002,		/**< Multiple instances will share the content of one real instance */
	DYNAMICBOX_EVAS_SUPPORT_GBAR = 0x0004,		/**< GBAR will be used */
	DYNAMICBOX_EVAS_USE_FIXED_SIZE = 0x0008,		/**< Livebox will be resized to specific size only */
	DYNAMICBOX_EVAS_EASY_MODE = 0x0010,		/**< Easy mode on/off */
	DYNAMICBOX_EVAS_SCROLL_X = 0x0020,			/**< Box will be scrolled from left to right vice versa */
	DYNAMICBOX_EVAS_SCROLL_Y = 0x0040,			/**< Box will be scrolled from top to bottom vice versa */
	DYNAMICBOX_EVAS_EVENT_AUTO_FEED = 0x0080,		/**< Feeds event automatically from the master provider */
	DYNAMICBOX_EVAS_DELAYED_PAUSE_RESUME = 0x0100,	/**< Delaying the pause/resume when it is automatically changed */
	DYNAMICBOX_EVAS_SENSITIVE_MOVE = 0x0200,		/**< Force feeds mouse up event if the box is moved */
	DYNAMICBOX_EVAS_AUTO_RENDER_SELECTION = 0x0400,	/**< Select render automatically, if a box moved, do not sync using animator, or use the animator */
	DYNAMICBOX_EVAS_UNKNOWN = 0xFFFF
};

enum dynamicbox_access_result {
	DYNAMICBOX_ACCESS_DONE = 0x00,
	DYNAMICBOX_ACCESS_FIRST = 0x01,
	DYNAMICBOX_ACCESS_LAST = 0x02,
	DYNAMICBOX_ACCESS_READ = 0x04,
	DYNAMICBOX_ACCESS_ERROR = 0x80,
	DYNAMICBOX_ACCESS_UNKNOWN = 0xFF
};

extern int evas_object_dynamicbox_init(Evas_Object *win, int force_to_buffer);
extern int evas_object_dynamicbox_fini(void);

extern Evas_Object *evas_object_dynamicbox_add(Evas_Object *parent, const char *dbox_id, const char *content_info, const char *cluster, const char *category, double period);
extern int evas_object_dynamicbox_destroy_pd(Evas_Object *dynamicbox);

extern int evas_object_dynamicbox_resumed(void);
extern int evas_object_dynamicbox_paused(void);

extern int evas_object_dynamicbox_pause(Evas_Object *dynamicbox);
extern int evas_object_dynamicbox_resume(Evas_Object *dynamicbox);

extern int evas_object_dynamicbox_view_port_set(Evas_Object *dynamicbox, int x, int y, int w, int h);
extern int evas_object_dynamicbox_view_port_get(Evas_Object *dynamicbox, int *x, int *y, int *w, int *h);
extern int evas_object_dynamicbox_conf_set(enum dynamicbox_evas_conf type, int value);

extern const char *evas_object_dynamicbox_content(Evas_Object *dynamicbox);
extern const char *evas_object_dynamicbox_title(Evas_Object *dynamicbox);
extern const char *evas_object_dynamicbox_dbox_id(Evas_Object *dynamicbox);

extern double evas_object_dynamicbox_period(Evas_Object *dynamicbox);
extern void evas_object_dynamicbox_cancel_click(Evas_Object *dynamicbox);

/**
 * @brief This function should be called right after create the dynamicbox object. before resizing it.
 */
extern void evas_object_dynamicbox_disable_preview(Evas_Object *dynamicbox);
extern void evas_object_dynamicbox_disable_overlay_text(Evas_Object *dynamicbox);
extern void evas_object_dynamicbox_disable_loading(Evas_Object *dynamicbox);

extern int evas_object_dynamicbox_force_mouse_up(Evas_Object *dynamicbox);
extern int evas_object_dynamicbox_access_action(Evas_Object *dynamicbox, int type, void *_info, void (*ret_cb)(Evas_Object *obj, int ret, void *data), void *data);
extern void evas_object_dynamicbox_activate(Evas_Object *dynamicbox);
extern int evas_object_dynamicbox_is_faulted(Evas_Object *dynamicbox);

extern int evas_object_dynamicbox_unset_raw_event_callback(enum dynamicbox_evas_raw_event_type type, void (*cb)(struct dynamicbox_evas_raw_event_info *info, void *data), void *data);
extern int evas_object_dynamicbox_set_raw_event_callback(enum dynamicbox_evas_raw_event_type type, void (*cb)(struct dynamicbox_evas_raw_event_info *info, void *data), void *data);

extern int evas_object_dynamicbox_freeze_visibility(Evas_Object *dynamicbox, int status);
extern int evas_object_dynamicbox_thaw_visibility(Evas_Object *dynamicbox);
extern int evas_object_dynamicbox_visibility_is_freezed(Evas_Object *dynamicbox);
extern int evas_object_dynamicbox_dump(Evas_Object *dynamicbox, const char *filename);
extern int evas_object_dynamicbox_is_dynamicbox(Evas_Object *dynamicbox);
extern void evas_object_dynamicbox_use_render_animator(int flag);
extern void evas_object_dynamicbox_set_permanent_delete(Evas_Object *dynamicbox, int flag);

#ifdef __cplusplus
}
#endif

#endif

/* End of a file */
