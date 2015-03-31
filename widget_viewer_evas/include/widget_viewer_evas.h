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

#ifndef __WIDGET_VIEWER_EVAS_H
#define __WIDGET_VIEWER_EVAS_H

#include "widget_service.h"

#ifdef __cplusplus
extern "C" {
#endif

#define WIDGET_VIEWER_EVAS_DEFAULT_PERIOD                -1.0f                   /**< Default Update Period */

/**
 * @sine_tizen 2.4
 * @brief Event names for smart callback of widget events. You can listen some events from widget by calling evas_object_smart_callback_add.
 * @see #widget_evas_event_info_s
 * @see evas_object_smart_callback_add
 */
#define WIDGET_SMART_SIGNAL_WIDGET_CREATE_ABORTED   "widget,create,aborted"   /**< widget creation is aborted */
#define WIDGET_SMART_SIGNAL_WIDGET_CREATED          "widget,created"          /**< widget is created */
#define WIDGET_SMART_SIGNAL_WIDGET_RESIZE_ABORTED   "widget,resize,aborted"   /**< Resizing widget is aborted */
#define WIDGET_SMART_SIGNAL_WIDGET_RESIZED          "widget,resized"          /**< widget is resized */
#define WIDGET_SMART_SIGNAL_WIDGET_FAULTED          "widget,faulted"          /**< widget has faulted */
#define WIDGET_SMART_SIGNAL_UPDATED                 "updated"                 /**< widget content is updated */
#define WIDGET_SMART_SIGNAL_EXTRA_INFO_UPDATED      "info,updated"            /**< widget extra info is updated */
#define WIDGET_SMART_SIGNAL_PROVIDER_DISCONNECTED   "provider,disconnected"   /**< Provider is disconnected */
#define WIDGET_SMART_SIGNAL_CONTROL_SCROLLER        "control,scroller"        /**< Control Scroller */
#define WIDGET_SMART_SIGNAL_WIDGET_DELETED          "widget,deleted"          /**< widget is deleted */
#define WIDGET_SMART_SIGNAL_PERIOD_CHANGED          "widget,period,changed"   /**< Period is changed */

/**
 * @sine_tizen 2.4
 * @brief Data structure which will be sent as a parameter of smart callback for signals WIDGET_SMART_SIGNAL_XXX
 * @see #WIDGET_SMART_SIGNAL_WIDGET_CREATE_ABORTED
 * @see #WIDGET_SMART_SIGNAL_WIDGET_CREATED
 * @see #WIDGET_SMART_SIGNAL_WIDGET_RESIZE_ABORTED
 * @see #WIDGET_SMART_SIGNAL_WIDGET_RESIZED
 * @see #WIDGET_SMART_SIGNAL_WIDGET_FAULTED
 * @see #WIDGET_SMART_SIGNAL_UPDATED
 * @see #WIDGET_SMART_SIGNAL_EXTRA_INFO_UPDATED
 * @see #WIDGET_SMART_SIGNAL_PROVIDER_DISCONNECTED
 * @see #WIDGET_SMART_SIGNAL_CONTROL_SCROLLER
 * @see #WIDGET_SMART_SIGNAL_WIDGET_DELETED
 * @see #WIDGET_SMART_SIGNAL_PERIOD_CHANGED
 */
typedef struct widget_evas_event_info {
    const char *pkgname;       /**< widget application id */
    widget_event_type_e event; /**< event type for detail event information - WIDGET_EVENT_XXX, refer the widget_serivce.h */
    int error;		           /**< Error type - WIDGET_ERROR_XXX, refer the widget_errno.h */
} widget_evas_event_info_s;



/**
 * \brief
 * Configuration keys
 */
typedef enum widget_evas_conf {
    WIDGET_VIEWER_EVAS_MANUAL_PAUSE_RESUME = 0x0001,   /**< Visibility will be changed manually */
    WIDGET_VIEWER_EVAS_USE_FIXED_SIZE = 0x0008,	    /**< widget will be resized to specific size only */
    WIDGET_VIEWER_EVAS_EASY_MODE = 0x0010,		    /**< Easy mode on/off */
    WIDGET_VIEWER_EVAS_SCROLL_X = 0x0020,		    /**< Box will be scrolled from left to right vice versa */
    WIDGET_VIEWER_EVAS_SCROLL_Y = 0x0040,		    /**< Box will be scrolled from top to bottom vice versa */
    WIDGET_VIEWER_EVAS_EVENT_AUTO_FEED = 0x0080,	    /**< Feeds event automatically from the master provider */
    WIDGET_VIEWER_EVAS_DELAYED_PAUSE_RESUME = 0x0100,  /**< Delaying the pause/resume when it is automatically changed */
    WIDGET_VIEWER_EVAS_SENSITIVE_MOVE = 0x0200,	    /**< Force feeds mouse up event if the box is moved */
    WIDGET_VIEWER_EVAS_AUTO_RENDER_SELECTION = 0x0400, /**< Select render automatically, if a box moved, do not sync using animator, or use the animator */
    WIDGET_VIEWER_EVAS_DIRECT_UPDATE = 0x0800,	    /**< Enable direct update path */
    WIDGET_VIEWER_EVAS_USE_RENDER_ANIMATOR = 0x1000,   /**< Use the render animator or not */
	WIDGET_VIEWER_EVAS_SKIP_ACQUIRE = 0x2000,		/**< Even if the viewer cannot get acquired resource id, try to update using default one */
    WIDGET_VIEWER_EVAS_UNKNOWN = 0xFFFF
} widget_evas_conf_e;

/**
 * @brief Initializes the widget system
 * @since_tizen 2.4
 * @param[in] win Window object
 * @param[in] force_to_buffer if you want use the naive buffer directly (instead of resource id), use 1 or 0.
 * @return int
 * @retval
 * @see #widget_viewer_evas_fini
 */
extern int widget_viewer_evas_init(Evas_Object *win, int force_to_buffer);

/**
 * @brief Finalizes the widget system
 * @since_tizen 2.4
 * @return int
 * @retval
 * @see #widget_viewer_evas_init
 */
extern int widget_viewer_evas_fini(void);

/**
 * @brief Creates a new widget object
 * @since_tizen 2.4
 * @param[in] parent Evas Object of parent
 * @param[in] widget_id widget id
 * @param[in] content_info Contents that will be given to the widget instance
 * @param[in] period Update period (@c WIDGET_DEFAULT_PERIOD can be used for this; this argument will be used to specify the period of updating contents of a widget)
 * @return Evas_Object*
 * @retval NULL if it fails to create a new widget object and you can get the reason of failure using get_last_result()
 * @see #widget_service_get_widget_id
 * @see #widget_service_get_content_string
 * @see #widget_service_get_category
 */
extern Evas_Object *widget_viewer_evas_add_widget(Evas_Object *parent, const char *widget_id, const char *content_info, double period);

/**
 * @brief Notifies the status of a client ("it is paused") to the provider.
 * @details if you call this, all providers will gets resumed event.
 * @since_tizen 2.4
 * @privlevel public
 * @privilege %http://tizen.org/privilege/widget.viewer
 * @return #WIDGET_ERROR_NONE on success,
 *          otherwise an error code (see #WIDGET_ERROR_XXX) on failure
 * @retval #WIDGET_ERROR_FAULT if it failed to send state (paused) info
 * @see widget_viewer_evas_notify_paused_status_of_viewer()
 */
extern int widget_viewer_evas_notify_resumed_status_of_viewer(void);

/**
 * @brief Notifies the status of client ("it is resumed") to the provider.
 * @detail if you call this, all providers will gets paused event.
 * @since_tizen 2.4
 * @privlevel public
 * @privilege %http://tizen.org/privilege/widget.viewer
 * @return #WIDGET_ERROR_NONE on success,
 *          otherwise an error code (see #WIDGET_ERROR_XXX) on failure
 * @retval #WIDGET_ERROR_FAULT if it failed to send state (resumed) info
 * @see widget_viewer_evas_notify_resumed_status_of_viewer()
 */
extern int widget_viewer_evas_notify_paused_status_of_viewer(void);

/**
 * @brief Changes the state of given widget. If you want to make a widget paused, call this function.
 * @since_tizen 2.4
 * @privlevel public
 * @privilege %http://tizen.org/privilege/widget.viewer
 * @param[in] widget widget Evas object
 * @return #WIDGET_ERROR_NONE on success,
 *          otherwise an error code (see #WIDGET_ERROR_XXX) on failure
 * @retval #WIDGET_ERROR_FAULT if it failed to send state (resumed) info
 */
extern int widget_viewer_evas_pause_widget(Evas_Object *widget);

/**
 * @brief Changes the state of given widget. If you want to make a widget resumed, call this function.
 * @since_tizen 2.4
 * @privlevel public
 * @privilege %http://tizen.org/privilege/widget.viewer
 * @param[in] widget widget Evas object
 * @return #WIDGET_ERROR_NONE on success,
 *          otherwise an error code (see #WIDGET_ERROR_XXX) on failure
 * @retval #WIDGET_ERROR_FAULT if it failed to send state (resumed) info
 */
extern int widget_viewer_evas_resume_widget(Evas_Object *widget);

/**
 * @brief Changes the configurable values of widget system
 * @since_tizen 2.4
 * @param[in] type Configuration item
 * @param[in] value Its value
 * @return #WIDGET_ERROR_NONE on success,
 *          otherwise an error code (see #WIDGET_ERROR_XXX) on failure
 * @retval #WIDGET_ERROR_INVALID_PARAMETER Invalid option
 * @see #widget_evas_conf
 */
extern int widget_viewer_evas_set_option(enum widget_evas_conf type, int value);

/**
 * @brief Gets content string of widget
 * @details This string can be used for creating contents of widget again after reboot a device or recovered from crash(abnormal status)
 * @since_tizen 2.4
 * @param[in] widget widget object
 * @return content string to be recognize content of the widget
 * @retval NULL if there is no specific content string.
 */
extern const char *widget_viewer_evas_get_content_info(Evas_Object *widget);

/**
 * @brief Gets summarized string of the widget content for accessibility.
 * @details If the accessibility feature is turned on, a viewer can use this text to describe the widget.
 * @since_tizen 2.4
 * @param[in] widget widget object
 * @return title string to be used for summarizing the widget
 * @retval NULL if there is no summarized text for content of given widget.
 */
extern const char *widget_viewer_evas_get_title_string(Evas_Object *widget);

/**
 * @brief Gets the id of the widget
 * @since_tizen 2.4
 * @param[in] widget widget object
 * @return const char * widget Id
 * @retval NULL if an error occurred
 */
extern const char *widget_viewer_evas_get_widget_id(Evas_Object *widget);

/**
 * @brief Gets the update period of the widget.
 * @since_tizen 2.4
 * @param[in] widget widget object
 * @return period the update period of the widget.
 * @retval Update period
 */
extern double widget_viewer_evas_get_period(Evas_Object *widget);

/**
 * @brief Cancels click event procedure.
 * @details If you call this function after feed the mouse_down(or mouse_set) event, the widget will get ON_HOLD events.\n
 *          If a widget gets ON_HOLD event, it will not do anything even if you feed mouse_up(or mouse_unset) event.\n
 * @since_tizen 2.4
 * @param[in] widget widget object
 * @return void
 */
extern void widget_viewer_evas_cancel_click_event(Evas_Object *widget);

/**
 * @brief This function should be called right after create the widget object. before resizing it.
 * @since_tizen 2.4
 * @param[in] widget widget object
 * @return void
 */
extern void widget_viewer_evas_disable_preview(Evas_Object *widget);

/**
 * @brief While loading a box, hide the help text
 * @since_tizen 2.4
 * @param[in] widget widget object
 * @return void
 */
extern void widget_viewer_evas_disable_overlay_text(Evas_Object *widget);

/**
 * @brief Do not display the overlay layer while loading a new box.
 * @details if you disable it, there is no preview & help text while creating a widget object
 * @since_tizen 2.4
 * @param[in] widget widget object
 * @return void
 */
extern void widget_viewer_evas_disable_loading(Evas_Object *widget);

/**
 * @brief Feeds the mouse_up event to the provider.
 * @details This is very similar with widget_viewer_evas_cancel_click(), but this will sends mouse_up event explicitly.\n
 *          Also feed the ON_HOLD event before feeds mouse_up event.
 * @since_tizen 2.4
 * @param[in] widget widget object
 * @return int
 */
extern int widget_viewer_evas_feed_mouse_up_event(Evas_Object *widget);

/**
 * @brief Activate
 * @since_tizen 2.4
 * @param[in] widget widget object
 * @return void
 */
extern void widget_viewer_evas_activate_faulted_widget(Evas_Object *widget);

/**
 * @brief Check whether the widget is faulted.
 * @since_tizen 2.4
 * @param[in] widget
 */
extern int widget_viewer_evas_is_faulted(Evas_Object *widget);


/**
 * @brief If you don't want change the visibility automatically, freeze it.\n
 *        The visibility will not be changed even though a box disappeared(hidden)/displayed(shown) from/on the screen.
 * @since_tizen 2.4
 * @param[in] widget
 * @param[in] status
 * @return int
 */
extern int widget_viewer_evas_freeze_visibility(Evas_Object *widget, int status);

/**
 * @brief If you want to let the visibility change automatically again, call this function.
 * @since_tizen 2.4
 * @param[in] widget
 * @return int
 */
extern int widget_viewer_evas_thaw_visibility(Evas_Object *widget);

/**
 * @brief Get the state of visibility option.
 * @since_tizen 2.4
 * @param[in] widget
 * @return int
 */
extern int widget_viewer_evas_get_freeze_visibility(Evas_Object *widget);

/**
 * @brief Validate the object, whether it is a widget object or not
 * @since_tizen 2.4
 * @param[in] widget
 * @return int
 */
extern int widget_viewer_evas_is_widget(Evas_Object *widget);

/**
 * @brief Before delete a box, set the deletion mode using this.
 * @since_tizen 2.4
 * @param[in] widget widget object which will be deleted soon
 * @param[in] flag 1 if you delete this widget instance permanently, of 0 if you want keep it and it will be re-created soon.
 * @return void
 */
extern void widget_viewer_evas_set_permanent_delete(Evas_Object *widget, int flag);

/**
 * @brief Emits a text signal to the given widget only if it is a text type.
 * @since_tizen 2.3.1
 * @remarks
 *    This is an ASYNCHRONOUS API.
 *    This function is Asynchronous, so you will get the result from @a smart callback, if you failed to send a text signal,
 *    this function will returns proper error code.
 * @param[in] widget widget object
 * @param[in] event_info.signal_name Emission string
 * @param[in] event_info.source Source string
 * @param[in] event_info.sx Start X
 * @param[in] event_info.sy Start Y
 * @param[in] event_info.ex End X
 * @param[in] event_info.ey End Y
 * @privlevel public
 * @privilege %http://tizen.org/privilege/widget.viewer
 * @return int
 * @retval #WIDGET_STATUS_ERROR_INVALID_PARAMETER Invalid parameters
 * @retval #WIDGET_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #WIDGET_STATUS_ERROR_NONE Successfully emitted
 * @see widget_ret_cb
 */
extern int widget_viewer_evas_emit_text_signal(Evas_Object *widget, widget_text_signal_s event_info, void *data);

#ifdef __cplusplus
}
#endif

#endif

/* End of a file */
