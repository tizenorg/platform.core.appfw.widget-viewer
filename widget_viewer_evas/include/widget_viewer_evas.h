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

#ifndef __WIDGET_EVAS_H
#define __WIDGET_EVAS_H

#ifdef __cplusplus
extern "C" {
#endif

#define WIDGET_EVAS_DEFAULT_PERIOD                -1.0f                   /**< Default Update Period */
#define WIDGET_SMART_SIGNAL_WIDGET_CREATE_ABORTED   "widget,create,aborted"   /**< widget creation is aborted */
#define WIDGET_SMART_SIGNAL_WIDGET_CREATED          "widget,created"          /**< widget is created */
#define WIDGET_SMART_SIGNAL_WIDGET_RESIZE_ABORTED   "widget,resize,aborted"   /**< Resizing widget is aborted */
#define WIDGET_SMART_SIGNAL_WIDGET_RESIZED          "widget,resized"          /**< widget is resized */
#define WIDGET_SMART_SIGNAL_WIDGET_FAULTED          "widget,faulted"          /**< widget has faulted */
#define WIDGET_SMART_SIGNAL_UPDATED               "updated"               /**< widget content is updated */
#define WIDGET_SMART_SIGNAL_EXTRA_INFO_UPDATED    "info,updated"          /**< widget extra info is updated */
#define WIDGET_SMART_SIGNAL_PROVIDER_DISCONNECTED "provider,disconnected" /**< Provider is disconnected */
#define WIDGET_SMART_SIGNAL_GBAR_DESTROYED        "gbar,destroyed"        /**< GBAR is destroyed */
#define WIDGET_SMART_SIGNAL_GBAR_ABORTED          "gbar,aborted"          /**< GBAR creation is aborted */
#define WIDGET_SMART_SIGNAL_GBAR_CREATED          "gbar,created"          /**< GBAR is created */
#define WIDGET_SMART_SIGNAL_FLICKDOWN_CANCELLED   "flickdown,cancelled"   /**< Flick down is canceld */
#define WIDGET_SMART_SIGNAL_CONTROL_SCROLLER      "control,scroller"      /**< Control Scroller */
#define WIDGET_SMART_SIGNAL_WIDGET_DELETED          "widget,deleted"          /**< DynamicBox is deleted */
#define WIDGET_SMART_SIGNAL_PERIOD_CHANGED        "widget,period,changed"   /**< Period is changed */

/**
 * \brief
 * Data structure for Smart Callback Event
 */
struct widget_evas_event_info {
    const char *pkgname;    /**< widget application Id */
    int event;		    /**< Event type - WIDGET_EVENT_XXX, refer the widget_viewer.h */
    int error;		    /**< Error type - WIDGET_STATUS_XXX, refer the widget_viewer.h */
};

enum widget_evas_raw_event_type {
    WIDGET_EVAS_RAW_DELETE = 0x00,
    WIDGET_EVAS_RAW_CREATE = 0x02,
    WIDGET_EVAS_RAW_MAX = 0xff,
};

struct widget_evas_raw_event_info {
    const char *pkgname;
    enum widget_evas_raw_event_type type;
    int error;
    Evas_Object *widget;
};

/**
 * \brief
 * Configuration keys
 */
enum widget_evas_conf {
    WIDGET_EVAS_MANUAL_PAUSE_RESUME = 0x0001,   /**< Visibility will be changed manually */
    WIDGET_EVAS_SHARED_CONTENT = 0x0002,	    /**< Multiple instances will share the content of one real instance */
    WIDGET_EVAS_SUPPORT_GBAR = 0x0004,	    /**< GBAR will be used */
    WIDGET_EVAS_USE_FIXED_SIZE = 0x0008,	    /**< widget will be resized to specific size only */
    WIDGET_EVAS_EASY_MODE = 0x0010,		    /**< Easy mode on/off */
    WIDGET_EVAS_SCROLL_X = 0x0020,		    /**< Box will be scrolled from left to right vice versa */
    WIDGET_EVAS_SCROLL_Y = 0x0040,		    /**< Box will be scrolled from top to bottom vice versa */
    WIDGET_EVAS_EVENT_AUTO_FEED = 0x0080,	    /**< Feeds event automatically from the master provider */
    WIDGET_EVAS_DELAYED_PAUSE_RESUME = 0x0100,  /**< Delaying the pause/resume when it is automatically changed */
    WIDGET_EVAS_SENSITIVE_MOVE = 0x0200,	    /**< Force feeds mouse up event if the box is moved */
    WIDGET_EVAS_AUTO_RENDER_SELECTION = 0x0400, /**< Select render automatically, if a box moved, do not sync using animator, or use the animator */
    WIDGET_EVAS_DIRECT_UPDATE = 0x0800,	    /**< Enable direct update path */
    WIDGET_EVAS_USE_RENDER_ANIMATOR = 0x1000,   /**< Use the render animator or not */
    WIDGET_EVAS_UNKNOWN = 0xFFFF
};

enum widget_access_result {
    WIDGET_ACCESS_RESULT_DONE = 0x00,
    WIDGET_ACCESS_RESULT_FIRST = 0x01,
    WIDGET_ACCESS_RESULT_LAST = 0x02,
    WIDGET_ACCESS_RESULT_READ = 0x04,
    WIDGET_ACCESS_RESULT_ERROR = 0x80,
    WIDGET_ACCESS_RESULT_UNKNOWN = 0xFF
};

/**
 * @brief Initialize the widget system
 * @since_tizen 2.4
 * @param[in] win Window object
 * @param[in] force_to_buffer if you want use the naive buffer directly (instead of resource id), use 1 or 0.
 * @return int
 * @retval
 * @see evas_object_widget_fini()
 */
extern int evas_object_widget_init(Evas_Object *win, int force_to_buffer);

/**
 * @brief Finalize the widget system
 * @since_tizen 2.4
 * @return int
 * @retval
 * @see evas_object_widget_init()
 */
extern int evas_object_widget_fini(void);

/**
 * @brief Create a new widget object
 * @since_tizen 2.4
 * @param[in] parent
 * @param[in] widget_id
 * @param[in] content_info
 * @param[in] cluster
 * @param[in] category
 * @param[in] period update period
 * @return Evas_Object*
 * @retval NULL if it fails to create a new widget object
 */
extern Evas_Object *evas_object_widget_add(Evas_Object *parent, const char *widget_id, const char *content_info, const char *cluster, const char *category, double period);

extern int evas_object_widget_subscribe_group(const char *cluster, const char *sub_cluster);
extern int evas_object_widget_unsubscribe_group(const char *cluster, const char *sub_cluster);

extern int evas_object_widget_subscribe_category(const char *category);
extern int evas_object_widget_unsubscribe_category(const char *category);

/**
 * @brief Close the Glance Bar if it is opened
 * @since_tizen 2.4
 * @param[in] widget widget object
 * @return int
 */
extern int evas_object_widget_destroy_gbar(Evas_Object *widget);

/**
 * @brief if a viewer is resumed, use this function to notify it to the providers.
 * @since_tizen 2.4
 * @details if you call this, all providers will gets resumed event.
 * @return int
 * @see evas_object_widget_paused()
 */
extern int evas_object_widget_resumed(void);

/**
 * @brief If a viewer is paused, use this function to notify it to the providers
 * @since_tizen 2.4
 * @detail if you call this, all providers will gets paused event.
 * @return int
 */
extern int evas_object_widget_paused(void);

/**
 * @brief Change the state of each widget. if you want made a box should be paused, call this.
 * @since_tizen 2.4
 * @param[in] widget widget object
 * @return int
 */
extern int evas_object_widget_pause(Evas_Object *widget);

/**
 * @brief Change the state of each widget. If you want made a box should be resumed, call this.
 * @since_tizen 2.4
 * @param[in] widget widget object
 * @return int
 */
extern int evas_object_widget_resume(Evas_Object *widget);

/**
 * @brief Set the viewe port of given widget
 * @since_tizen 2.4
 * @param[in] widget
 * @param[in] x
 * @param[in] y
 * @param[in] w
 * @param[in] h
 * @return int
 */
extern int evas_object_widget_view_port_set(Evas_Object *widget, int x, int y, int w, int h);

/**
 * @brief Get the current view port of given widget
 * @since_tizen 2.4
 * @param[in] widget
 * @param[out] x
 * @param[out] y
 * @param[out] w
 * @param[out] h
 * @return int
 */
extern int evas_object_widget_view_port_get(Evas_Object *widget, int *x, int *y, int *w, int *h);

/**
 * @brief Change the configurable values of widget system
 * @since_tizen 2.4
 * @param[in] type Configuration item
 * @param[in] value Its value
 * @return int
 */
extern int evas_object_widget_conf_set(enum widget_evas_conf type, int value);

/**
 * @brief Content string of widget
 * @since_tizen 2.4
 * @details This string should be used for creating widget again after reboot device or recovered from crash(abnormal status)
 * @param[in] widget widget Object
 * @return const char * String of content
 * @retval NULL if there is no specific content string.
 */
extern const char *evas_object_widget_content(Evas_Object *widget);

/**
 * @brief Summarized string of widget content.
 * @since_tizen 2.4
 * @details If the accessibility feature is turned on, the homescreen can read this text to describe the widget.
 * @param[in] widget widget Object
 * @return const char * Text should be read
 * @retval NULL if there is no summarized text for content of given widget
 */
extern const char *evas_object_widget_title(Evas_Object *widget);

/**
 * @brief Get the widget Id
 * @since_tizen 2.4
 * @param[in] widget widget Object
 * @return const char * Dynamic Box Id
 * @retval NULL if an error occurred
 */
extern const char *evas_object_widget_widget_id(Evas_Object *widget);

/**
 * @brief Current period of updates
 * @since_tizen 2.4
 * @param[in] widget widget Object
 * @return double
 * @retval Update period
 */
extern double evas_object_widget_period(Evas_Object *widget);

/**
 * @brief Cancelate click event procedure.
 * @since_tizen 2.4
 * @details If you call this after feed the mouse_down(or mouse_set) event, the box will get ON_HOLD events.\n
 *          If a box gets ON_HOLD event, it will not do anything even if you feed mouse_up(or mouse_unset) event.\n
 * @param[in] widget widget Object
 * @return void
 */
extern void evas_object_widget_cancel_click(Evas_Object *widget);

/**
 * @brief This function should be called right after create the widget object. before resizing it.
 * @since_tizen 2.4
 * @param[in] widget
 * @return void
 */
extern void evas_object_widget_disable_preview(Evas_Object *widget);

/**
 * @brief While loading a box, hide the help text
 * @since_tizen 2.4
 * @param[in] widget
 * @return void
 */
extern void evas_object_widget_disable_overlay_text(Evas_Object *widget);

/**
 * @brief Do not display the overlay layer while loading a new box.
 * @since_tizen 2.4
 * @details if you disable it, there is no preview & help text while creating a widget object
 * @return void
 */
extern void evas_object_widget_disable_loading(Evas_Object *widget);

/**
 * @brief Feeds the mouse_up event forcely.
 * @since_tizen 2.4
 * @details This is very similiar with evas_object_widget_cancel_click(), but this will sends mouse_up event explicitly.\n
 *          Also feed the ON_HOLD event before feeds mouse_up event.
 * @param[in] widget Dynamic Box
 * @return int
 */
extern int evas_object_widget_force_mouse_up(Evas_Object *widget);

/**
 * @brief Feeds accessibility events
 * @since_tizen 2.4
 * @param[in] widget
 * @param[in] type
 * @param[in] info
 * @param[in] ret_cb
 * @param[in] dta
 * @return int
 */
extern int evas_object_widget_access_action(Evas_Object *widget, int type, void *info, void (*ret_cb)(Evas_Object *obj, int ret, void *data), void *data);

/**
 * @brief Activate
 * @since_tizen 2.4
 * @param[in] widget
 */
extern void evas_object_widget_activate(Evas_Object *widget);

/**
 * @brief
 * @since_tizen 2.4
 * @param[in] widget
 */
extern int evas_object_widget_is_faulted(Evas_Object *widget);

/**
 * @brief
 * @since_tizen 2.4
 * @param[in] type
 * @param[in] cb
 * @param[in] data
 * @return int
 */
extern int evas_object_widget_unset_raw_event_callback(enum widget_evas_raw_event_type type, void (*cb)(struct widget_evas_raw_event_info *info, void *data), void *data);

/**
 * @brief
 * @since_tizen 2.4
 * @param[in] type
 * @param[in] cb
 * @param[in] data
 */
extern int evas_object_widget_set_raw_event_callback(enum widget_evas_raw_event_type type, void (*cb)(struct widget_evas_raw_event_info *info, void *data), void *data);

/**
 * @brief If you don't want change the visibility automatically, freeze it.\n
 *        The visibility will not be changed even though a box disappeared(hidden)/displayed(shown) from/on the screen.
 * @since_tizen 2.4
 * @param[in] widget
 * @param[in] status
 * @return int
 */
extern int evas_object_widget_freeze_visibility(Evas_Object *widget, int status);

/**
 * @brief
 * @since_tizen 2.4
 * @param[in] widget
 * @return int
 */
extern int evas_object_widget_thaw_visibility(Evas_Object *widget);

/**
 * @brief Get the state of visibility option.
 * @since_tizen 2.4
 * @param[in] widget
 * @return int
 */
extern int evas_object_widget_visibility_is_freezed(Evas_Object *widget);

/**
 * @brief Dump a contents of widget to a given filename.
 * @since_tizen 2.4
 * @param[in] widget widget object
 * @param[in] filename Filename will be used for saving content of a widget
 * @return int
 */
extern int evas_object_widget_dump(Evas_Object *widget, const char *filename);

/**
 * @brief Validate the object, whether it is a widget object or not
 * @since_tizen 2.4
 * @param[in] widget
 * @return int
 */
extern int evas_object_widget_is_widget(Evas_Object *widget);

/**
 * @brief Before delete a box, set the deletion mode using this.
 * @since_tizen 2.4
 * @param[in] widget widget Object which will be deleted soon
 * @param[in] flag 1 if you delete this widget instance permanently, of 0 if you want keep it and it will be re-created soon.
 * @return void
 */
extern void evas_object_widget_set_permanent_delete(Evas_Object *widget, int flag);

#ifdef __cplusplus
}
#endif

#endif

/* End of a file */
