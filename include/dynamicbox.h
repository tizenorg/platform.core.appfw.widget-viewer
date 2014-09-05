/*
 * Copyright 2013  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.1 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://floralicense.org/license/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __DYNAMICBOX_H
#define __DYNAMICBOX_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file dynamicbox.h
 * @brief This file declares API of libdynamicbox-viewer library
 * @since_tizen 2.3
 */

/**
 * @addtogroup CAPI_DYNAMIC_VIEWER_MODULE
 * @{
 */

/**
 * @internal
 * @brief Structure definition for a Dynamic Box instance.
 * @since_tizen 2.3
 */
typedef struct dynamicbox *dynamicbox_h;

/**
 * @internal
 * @brief Definition for a default update period for Dynamicbox (defined in the package manifest file).
 * @since_tizen 2.3
 */
#define DBOX_DEFAULT_PERIOD -1.0f

/**
 * @internal
 * @brief Enumeration for Mouse & Key event for buffer type Dynamic Box or Glance Bar.
 * @details Viewer should send these events to dynamicbox.
 * @since_tizen 2.3
 */
enum dynamicbox_mouse_event_type {
	DBOX_MOUSE_EVENT_MASK	= 0x20000000, /**< Mask value for mouse event */
	DBOX_MOUSE_EVENT_GBAR_MASK		= 0x10000000, /**< Mask value for Glance Bar event */
	DBOX_MOUSE_EVENT_DBOX_MASK		= 0x40000000, /**< Mask value for Dynamic Box event */

	DBOX_MOUSE_EVENT_DOWN	= 0x00000001, /**< Dynamic Box mouse down event for dynamicbox */
	DBOX_MOUSE_EVENT_UP		= 0x00000002, /**< Dynamic Box mouse up event for dynamicbox */
	DBOX_MOUSE_EVENT_MOVE	= 0x00000004, /**< Dynamic Box mouse move event for dynamicbox */
	DBOX_MOUSE_EVENT_ENTER	= 0x00000008, /**< Dynamic Box mouse enter event for dynamicbox */
	DBOX_MOUSE_EVENT_LEAVE	= 0x00000010, /**< Dynamic Box mouse leave event for dynamicbox */
	DBOX_MOUSE_EVENT_SET		= 0x00000020, /**< Dynamic Box mouse set auto event for dynamicbox */
	DBOX_MOUSE_EVENT_UNSET	= 0x00000040, /**< Dynamic Box mouse unset auto event for dynamicbox */

	DBOX_MOUSE_EVENT_ON_SCROLL		= 0x00000080, /**< Dynamic Box On scrolling */
	DBOX_MOUSE_EVENT_ON_HOLD		= 0x00000100, /**< Dynamic Box On holding */
	DBOX_MOUSE_EVENT_OFF_SCROLL	= 0x00000200, /**< Dynamic Box Stop scrolling */
	DBOX_MOUSE_EVENT_OFF_HOLD		= 0x00000400, /**< Dynamic Box Stop holding */

	DBOX_MOUSE_ON_SCROLL	= DBOX_MOUSE_EVENT_DBOX_MASK | DBOX_MOUSE_EVENT_MASK | DBOX_MOUSE_EVENT_ON_SCROLL, /**< Mouse event occurs while scrolling */
	DBOX_MOUSE_ON_HOLD	= DBOX_MOUSE_EVENT_DBOX_MASK | DBOX_MOUSE_EVENT_MASK | DBOX_MOUSE_EVENT_ON_HOLD, /**< Mouse event occurs on holding */
	DBOX_MOUSE_OFF_SCROLL	= DBOX_MOUSE_EVENT_DBOX_MASK | DBOX_MOUSE_EVENT_MASK | DBOX_MOUSE_EVENT_OFF_SCROLL, /**< Scrolling stopped */
	DBOX_MOUSE_OFF_HOLD	= DBOX_MOUSE_EVENT_DBOX_MASK | DBOX_MOUSE_EVENT_MASK | DBOX_MOUSE_EVENT_OFF_HOLD, /**< Holding stopped */

	DBOX_MOUSE_DOWN		= DBOX_MOUSE_EVENT_DBOX_MASK | DBOX_MOUSE_EVENT_MASK | DBOX_MOUSE_EVENT_DOWN, /**< Mouse down on the dynamicbox */
	DBOX_MOUSE_UP		= DBOX_MOUSE_EVENT_DBOX_MASK | DBOX_MOUSE_EVENT_MASK | DBOX_MOUSE_EVENT_UP, /**< Mouse up on the dynamicbox */
	DBOX_MOUSE_MOVE		= DBOX_MOUSE_EVENT_DBOX_MASK | DBOX_MOUSE_EVENT_MASK | DBOX_MOUSE_EVENT_MOVE, /**< Move move on the dynamicbox */
	DBOX_MOUSE_ENTER	= DBOX_MOUSE_EVENT_DBOX_MASK | DBOX_MOUSE_EVENT_MASK | DBOX_MOUSE_EVENT_ENTER, /**< Mouse enter to the dynamicbox */
	DBOX_MOUSE_LEAVE	= DBOX_MOUSE_EVENT_DBOX_MASK | DBOX_MOUSE_EVENT_MASK | DBOX_MOUSE_EVENT_LEAVE, /**< Mouse leave from the dynamicbox */
	DBOX_MOUSE_SET		= DBOX_MOUSE_EVENT_DBOX_MASK | DBOX_MOUSE_EVENT_MASK | DBOX_MOUSE_EVENT_SET, /**< Mouse event, start feeding event by master */
	DBOX_MOUSE_UNSET	= DBOX_MOUSE_EVENT_DBOX_MASK | DBOX_MOUSE_EVENT_MASK | DBOX_MOUSE_EVENT_UNSET, /**< Mouse event, stop feeding event by master */

	DBOX_GBAR_MOUSE_ON_SCROLL	= DBOX_MOUSE_EVENT_GBAR_MASK | DBOX_MOUSE_EVENT_MASK | DBOX_MOUSE_EVENT_ON_SCROLL, /**< Mouse event occurs while scrolling */
	DBOX_GBAR_MOUSE_ON_HOLD		= DBOX_MOUSE_EVENT_GBAR_MASK | DBOX_MOUSE_EVENT_MASK | DBOX_MOUSE_EVENT_ON_HOLD, /**< Mouse event occurs on holding */
	DBOX_GBAR_MOUSE_OFF_SCROLL	= DBOX_MOUSE_EVENT_GBAR_MASK | DBOX_MOUSE_EVENT_MASK | DBOX_MOUSE_EVENT_OFF_SCROLL, /**< Scrolling stopped */
	DBOX_GBAR_MOUSE_OFF_HOLD	= DBOX_MOUSE_EVENT_GBAR_MASK | DBOX_MOUSE_EVENT_MASK | DBOX_MOUSE_EVENT_OFF_HOLD, /**< Holding stopped */

	DBOX_GBAR_MOUSE_DOWN		= DBOX_MOUSE_EVENT_GBAR_MASK | DBOX_MOUSE_EVENT_MASK | DBOX_MOUSE_EVENT_DOWN, /**< Mouse down on the Glance Bar */
	DBOX_GBAR_MOUSE_UP		= DBOX_MOUSE_EVENT_GBAR_MASK | DBOX_MOUSE_EVENT_MASK | DBOX_MOUSE_EVENT_UP, /**< Mouse up on the Glance Bar */
	DBOX_GBAR_MOUSE_MOVE		= DBOX_MOUSE_EVENT_GBAR_MASK | DBOX_MOUSE_EVENT_MASK | DBOX_MOUSE_EVENT_MOVE, /**< Mouse move on the Glance Bar */
	DBOX_GBAR_MOUSE_ENTER		= DBOX_MOUSE_EVENT_GBAR_MASK | DBOX_MOUSE_EVENT_MASK | DBOX_MOUSE_EVENT_ENTER, /**< Mouse enter to the Glance Bar */
	DBOX_GBAR_MOUSE_LEAVE		= DBOX_MOUSE_EVENT_GBAR_MASK | DBOX_MOUSE_EVENT_MASK | DBOX_MOUSE_EVENT_LEAVE, /**< Mouse leave from the Glance Bar */
	DBOX_GBAR_MOUSE_SET		= DBOX_MOUSE_EVENT_GBAR_MASK | DBOX_MOUSE_EVENT_MASK | DBOX_MOUSE_EVENT_SET, /**< Mouse event, start feeding event by master */
	DBOX_GBAR_MOUSE_UNSET		= DBOX_MOUSE_EVENT_GBAR_MASK | DBOX_MOUSE_EVENT_MASK | DBOX_MOUSE_EVENT_UNSET, /**< Mouse event, stop feeding event by master */

	DBOX_MOUSE_EVENT_MAX		= 0xFFFFFFFF /**< Unknown event */
};

enum dynamicbox_key_event_type {
	DBOX_KEY_EVENT_MASK		= 0x80000000, /**< Mask value for key event */
	DBOX_KEY_EVENT_GBAR_MASK	= 0x10000000, /**< Mask value for Glance Bar event */
	DBOX_KEY_EVENT_DBOX_MASK	= 0x40000000, /**< Mask value for Dynamic Box event */

	DBOX_KEY_EVENT_DOWN		= 0x00000001, /**< Dynamic Box key press */
	DBOX_KEY_EVENT_UP		= 0x00000002, /**< Dynamic Box key release */
	DBOX_KEY_EVENT_FOCUS_IN		= 0x00000008, /**< Dynamic Box key focused in */
	DBOX_KEY_EVENT_FOCUS_OUT	= 0x00000010, /**< Dynamic Box key focused out */
	DBOX_KEY_EVENT_SET		= 0x00000020, /**< Dynamic Box Key, start feeding event by master */
	DBOX_KEY_EVENT_UNSET		= 0x00000040, /**< Dynamic Box key, stop feeding event by master */

	DBOX_KEY_DOWN			= DBOX_KEY_EVENT_MASK | DBOX_KEY_EVENT_DBOX_MASK | DBOX_KEY_EVENT_DOWN, /**< Key down on the dynamicbox */
	DBOX_KEY_UP			= DBOX_KEY_EVENT_MASK | DBOX_KEY_EVENT_DBOX_MASK | DBOX_KEY_EVENT_UP, /**< Key up on the dynamicbox */
	DBOX_KEY_SET			= DBOX_KEY_EVENT_MASK | DBOX_KEY_EVENT_DBOX_MASK | DBOX_KEY_EVENT_SET, /**< Key event, start feeding event by master */
	DBOX_KEY_UNSET			= DBOX_KEY_EVENT_MASK | DBOX_KEY_EVENT_DBOX_MASK | DBOX_KEY_EVENT_UNSET, /**< Key event, stop feeding event by master */
	DBOX_KEY_FOCUS_IN		= DBOX_KEY_EVENT_MASK | DBOX_KEY_EVENT_DBOX_MASK | DBOX_KEY_EVENT_FOCUS_IN, /**< Key event, focus in */
	DBOX_KEY_FOCUS_OUT		= DBOX_KEY_EVENT_MASK | DBOX_KEY_EVENT_DBOX_MASK | DBOX_KEY_EVENT_FOCUS_OUT, /**< Key event, foucs out */
                                                               
	DBOX_GBAR_KEY_DOWN		= DBOX_KEY_EVENT_MASK | DBOX_KEY_EVENT_GBAR_MASK | DBOX_KEY_EVENT_DOWN, /**< Key down on the dynamicbox */
	DBOX_GBAR_KEY_UP		= DBOX_KEY_EVENT_MASK | DBOX_KEY_EVENT_GBAR_MASK | DBOX_KEY_EVENT_UP, /**< Key up on the dynamicbox */
	DBOX_GBAR_KEY_SET		= DBOX_KEY_EVENT_MASK | DBOX_KEY_EVENT_GBAR_MASK | DBOX_KEY_EVENT_SET, /**< Key event, start feeding event by master */
	DBOX_GBAR_KEY_UNSET		= DBOX_KEY_EVENT_MASK | DBOX_KEY_EVENT_GBAR_MASK | DBOX_KEY_EVENT_UNSET, /**< Key event, stop feeding event by master */
	DBOX_GBAR_KEY_FOCUS_IN		= DBOX_KEY_EVENT_MASK | DBOX_KEY_EVENT_GBAR_MASK | DBOX_KEY_EVENT_FOCUS_IN, /**< Key event, focus in */
	DBOX_GBAR_KEY_FOCUS_OUT		= DBOX_KEY_EVENT_MASK | DBOX_KEY_EVENT_GBAR_MASK | DBOX_KEY_EVENT_FOCUS_OUT, /**< Key event, focus out */

	DBOX_KEY_EVENT_MAX		= 0xFFFFFFFF /**< Unknown event */
};

/**
 * @internal
 * @brief Enumeration for Accessibility event for buffer type Dynamic Box or Glance Bar.
 * @details These events are sync'd with Tizen accessibility event set.
 * @since_tizen 2.3
 */
enum dynamicbox_access_event_type {
	DBOX_ACCESS_EVENT_GBAR_MASK    = 0x10000000, /**< Glance Bar Accessibilivent mask */
	DBOX_ACCESS_EVENT_DBOX_MASK    = 0x20000000, /**< Dynamic Box Accessibility event mask */

	DBOX_ACCESS_EVENT_HIGHLIGHT    = 0x00000100, /**< Dynamic Box accessibility: Hightlight a object, Next, Prev,Unhighlight */
	DBOX_ACCESS_EVENT_ACTIVATE     = 0x00000200, /**< Dynamic Box accessibility activate */
	DBOX_ACCESS_EVENT_ACTION       = 0x00000400, /**< Dynamic Box accessibility value changed, Up, Down */
	DBOX_ACCESS_EVENT_SCROLL       = 0x00000800, /**< Dynamic Box accessibility scroll down, move, up */
	DBOX_ACCESS_EVENT_VALUE_CHANGE = 0x00001000, /**< LB accessibility value change */
	DBOX_ACCESS_EVENT_MOUSE        = 0x00002000, /**< Give mouse event to highlight object, down, move, up */
	DBOX_ACCESS_EVENT_BACK         = 0x00004000, /**< Go back to a previous view ex: pop naviframe item */
	DBOX_ACCESS_EVENT_OVER         = 0x00008000, /**< Mouse over an object */
	DBOX_ACCESS_EVENT_READ         = 0x00010000, /**< Highlight an object */
	DBOX_ACCESS_EVENT_ENABLE       = 0x00020000, /**< Disable highlight and read ability, disable, enable */

	DBOX_ACCESS_HIGHLIGHT          = DBOX_ACCESS_EVENT_DBOX_MASK | DBOX_ACCESS_EVENT_HIGHLIGHT, /**< Access event - Highlight an object in the dynamicbox */
	DBOX_ACCESS_ACTIVATE           = DBOX_ACCESS_EVENT_DBOX_MASK | DBOX_ACCESS_EVENT_ACTIVATE,  /**< Access event - Launch or activate the highlighted object */
	DBOX_ACCESS_ACTION             = DBOX_ACCESS_EVENT_DBOX_MASK | DBOX_ACCESS_EVENT_ACTION,    /**< Access event - down */
	DBOX_ACCESS_SCROLL             = DBOX_ACCESS_EVENT_DBOX_MASK | DBOX_ACCESS_EVENT_SCROLL,    /**< Access event - scroll down */
	DBOX_ACCESS_VALUE_CHANGE       = DBOX_ACCESS_EVENT_DBOX_MASK | DBOX_ACCESS_EVENT_VALUE_CHANGE, /**< LB accessibility value change */
	DBOX_ACCESS_MOUSE              = DBOX_ACCESS_EVENT_DBOX_MASK | DBOX_ACCESS_EVENT_MOUSE,  /**< Give mouse event to highlight object */
	DBOX_ACCESS_BACK               = DBOX_ACCESS_EVENT_DBOX_MASK | DBOX_ACCESS_EVENT_BACK,   /**< Go back to a previous view ex: pop naviframe item */
	DBOX_ACCESS_OVER               = DBOX_ACCESS_EVENT_DBOX_MASK | DBOX_ACCESS_EVENT_OVER,   /**< Mouse over an object */
	DBOX_ACCESS_READ               = DBOX_ACCESS_EVENT_DBOX_MASK | DBOX_ACCESS_EVENT_READ,   /**< Highlight an object */
	DBOX_ACCESS_ENABLE             = DBOX_ACCESS_EVENT_DBOX_MASK | DBOX_ACCESS_EVENT_ENABLE, /**< Enable highlight and read ability */

	DBOX_GBAR_ACCESS_HIGHLIGHT     = DBOX_ACCESS_EVENT_GBAR_MASK | DBOX_ACCESS_EVENT_HIGHLIGHT, /**< Access event - Highlight an object in the Glance Bar */
	DBOX_GBAR_ACCESS_ACTIVATE      = DBOX_ACCESS_EVENT_GBAR_MASK | DBOX_ACCESS_EVENT_ACTIVATE,  /**< Access event - Launch or activate the highlighted object */
	DBOX_GBAR_ACCESS_ACTION        = DBOX_ACCESS_EVENT_GBAR_MASK | DBOX_ACCESS_EVENT_ACTION,    /**< Access event - down */
	DBOX_GBAR_ACCESS_SCROLL        = DBOX_ACCESS_EVENT_GBAR_MASK | DBOX_ACCESS_EVENT_SCROLL,    /**< Access event - scroll down */
	DBOX_GBAR_ACCESS_VALUE_CHANGE  = DBOX_ACCESS_EVENT_GBAR_MASK | DBOX_ACCESS_EVENT_VALUE_CHANGE, /**< LB accessibility value change */
	DBOX_GBAR_ACCESS_MOUSE         = DBOX_ACCESS_EVENT_GBAR_MASK | DBOX_ACCESS_EVENT_MOUSE, /**< Give mouse event to highlight object */
	DBOX_GBAR_ACCESS_BACK          = DBOX_ACCESS_EVENT_GBAR_MASK | DBOX_ACCESS_EVENT_BACK, /**< Go back to a previous view ex: pop naviframe item */
	DBOX_GBAR_ACCESS_OVER          = DBOX_ACCESS_EVENT_GBAR_MASK | DBOX_ACCESS_EVENT_OVER, /**< Mouse over an object */
	DBOX_GBAR_ACCESS_READ          = DBOX_ACCESS_EVENT_GBAR_MASK | DBOX_ACCESS_EVENT_READ, /**< Highlight an object */
	DBOX_GBAR_ACCESS_ENABLE        = DBOX_ACCESS_EVENT_GBAR_MASK | DBOX_ACCESS_EVENT_ENABLE, /**< Enable highlight and read ability */
	DBOX_GBAR_ACCESS_EVENT_MAX     = 0xFFFFFFFF
};

/**
 * @internal
 * @brief Enumeration for Dynamic Box content type.
 * @since_tizen 2.3
 */
enum dynamicbox_type {
	DBOX_TYPE_IMAGE       = 0x01, /**< Contents of a dynamicbox is based on the image file */
	DBOX_TYPE_BUFFER      = 0x02, /**< Contents of a dynamicbox is based on canvas buffer(shared) */
	DBOX_TYPE_TEXT        = 0x04, /**< Contents of a dynamicbox is based on formatted text file */
	DBOX_TYPE_RESOURCE_ID = 0x08, /**< Contens of a dynamicbox is shared by the resource id(depends on window system) */
	DBOX_TYPE_UIFW        = 0x10, /**< Using UI F/W resource for sharing content & event */
	DBOX_TYPE_INVALID     = 0xFF /**< Unknown Dynamic Box type */
};

/**
 * @internal
 * @brief Enumeration for Dynamicbox event type.
 * @details These events will be sent from the provider.
 * @since_tizen 2.3
 */
enum dynamicbox_event_type { /**< dynamicbox_event_handler_set Event list */
	DBOX_EVENT_DBOX_UPDATED, /**< Contents of the given dynamicbox is updated */
	DBOX_EVENT_GBAR_UPDATED, /**< Contents of the given pd is updated */

	DBOX_EVENT_CREATED, /**< A new dynamicbox is created */
	DBOX_EVENT_DELETED, /**< A dynamicbox is deleted */

	DBOX_EVENT_GROUP_CHANGED, /**< Group (Cluster/Sub-cluster) information is changed */
	DBOX_EVENT_PINUP_CHANGED, /**< PINUP status is changed */
	DBOX_EVENT_PERIOD_CHANGED, /**< Update period is changed */

	DBOX_EVENT_DBOX_SIZE_CHANGED, /**< Dynamicbox size is changed */
	DBOX_EVENT_GBAR_SIZE_CHANGED, /**< Glance Bar size is changed */

	DBOX_EVENT_GBAR_CREATED, /**< If a Glance Bar is created even if you didn't call the dynamicbox_create_glance_bar API */
	DBOX_EVENT_GBAR_DESTROYED, /**< If a Glance Bar is destroyed even if you didn't call the dynamicbox_destroy_glance_bar API */

	DBOX_EVENT_HOLD_SCROLL, /**< If the screen should be freezed */
	DBOX_EVENT_RELEASE_SCROLL, /**< If the screen can be scrolled */

	DBOX_EVENT_DBOX_UPDATE_BEGIN, /**< Dynamic Box content update is started */
	DBOX_EVENT_DBOX_UPDATE_END, /**< Dynamic Box content update is finished */

	DBOX_EVENT_GBAR_UPDATE_BEGIN, /**< Glance Bar content update is started */
	DBOX_EVENT_GBAR_UPDATE_END, /**< Glance Bar content update is finished */

	DBOX_EVENT_UPDATE_MODE_CHANGED, /**< Dynamic Box Update mode is changed */

	DBOX_EVENT_REQUEST_CLOSE_GBAR, /**< Dynamic Box requests to close the Glance Bar */

	DBOX_EVENT_EXTRA_INFO_UPDATED, /**< Extra information is updated */

	DBOX_EVENT_IGNORED = 0xFF, /**< Request is ignored */
};

/**
 * @internal
 * @brief Enumeration for Dynamicbox option types.
 * @since_tizen 2.3
 */
enum dynamicbox_option_type {
	DBOX_OPTION_MANUAL_SYNC,		/**< Sync frame manually */
	DBOX_OPTION_FRAME_DROP_FOR_RESIZE,	/**< Drop frames while resizing */
	DBOX_OPTION_SHARED_CONTENT,		/**< Use only one real instance for multiple fake instances if user creates dbox for same content */
	DBOX_OPTION_DIRECT_UPDATE,		/**< Use the private socket for receiving updated event */

	DBOX_OPTION_ERROR = 0xFFFFFFFF		/**< To specify the size of this enumeration type */
};

/**
 * @internal
 * @brief Reason of faults
 * @since_tizen 2.3
 */
enum dynamicbox_fault_type {
	DBOX_FAULT_DEACTIVATED, /**< Dynamicbox is deactivated by its fault operation */
	DBOX_FAULT_PROVIDER_DISCONNECTED, /**< Provider is disconnected */
	DBOX_FAULT_MAX = 0xFF
};

/**
 * @internal
 * @brief Enumeration for Dynamicbox visible states.
 * @details Must be sync'd with a provider.
 * @since_tizen 2.3
 */
enum dynamicbox_visible_state {
	DBOX_SHOW            = 0x00, /**< Dynamicbox is shown. Default state */
	DBOX_HIDE            = 0x01, /**< Dynamicbox is hidden, Update timer will not be freezed. but you cannot receive any updates events. */

	DBOX_HIDE_WITH_PAUSE = 0x02, /**< Dynamicbox is hidden, it will pause the update timer, but if a dynamicbox updates its contents, update event will be triggered */

	DBOX_VISIBLE_ERROR   = 0xFF/**< To specify the size of this enumeration type */
};

/**
 * @brief Accessibility Event Information
 * @since_tizen 2.3
 */
struct dynamicbox_access_event_info {
	double x; /**< X Coordinates that the event occurred */
	double y; /**< Y Coordinates that the event occurred */
	enum _dynamicbox_access_type {
		DBOX_ACCESS_TYPE_DOWN = 0x00, /**< Mouse down */
		DBOX_ACCESS_TYPE_MOVE = 0x01, /**< Mouse move */
		DBOX_ACCESS_TYPE_UP   = 0x02, /**< Mouse up */

		DBOX_ACCESS_TYPE_HIGHLIGHT      = 0x00, /**< Highlight */
		DBOX_ACCESS_TYPE_HIGHLIGHT_NEXT = 0x01, /**< Highlight next */
		DBOX_ACCESS_TYPE_HIGHLIGHT_PREV = 0x02, /**< Highlight prev */
		DBOX_ACCESS_TYPE_UNHIGHLIGHT    = 0x03, /**< Unhighlight */

		DBOX_ACCESS_TYPE_DISABLE = 0x00, /**< Disable */
		DBOX_ACCESS_TYPE_ENABLE  = 0x01  /**< Enable */
	} type;
	int info;
};

/**
 * @brief Mouse Event Information
 * @since_tizen 2.3
 */
struct dynamicbox_mouse_event_info {
	double x;
	double y;
};

/**
 * @brief Key Event Information
 * @since_tizen 2.3
 */
struct dynamicbox_key_event_info {
	unsigned int keycode;
};

/**
 * @internal
 * @brief Structure for TEXT type dynamicbox contents handling opertators.
 * @since_tizen 2.3
 */
struct dynamicbox_script_operators {
	int (*update_begin)(dynamicbox_h handle); /**< Content parser is started */
	int (*update_end)(dynamicbox_h handle); /**< Content parser is finished */

	/* Listed functions will be called when parser meets each typed content */
	int (*update_text)(dynamicbox_h handle, const char *id, const char *part, const char *data); /**< Update text content */
	int (*update_image)(dynamicbox_h handle, const char *id, const char *part, const char *data, const char *option); /**< Update image content */
	int (*update_script)(dynamicbox_h handle, const char *id, const char *new_id, const char *part, const char *file, const char *group); /**< Update script content */
	int (*update_signal)(dynamicbox_h handle, const char *id, const char *emission, const char *signal); /**< Update signal */
	int (*update_drag)(dynamicbox_h handle, const char *id, const char *part, double dx, double dy); /**< Update drag info */
	int (*update_info_size)(dynamicbox_h handle, const char *id, int w, int h); /**< Update content size */
	int (*update_info_category)(dynamicbox_h handle, const char *id, const char *category); /**< Update content category info */
	int (*update_access)(dynamicbox_h handle, const char *id, const char *part, const char *text, const char *option); /**< Update access information */
	int (*operate_access)(dynamicbox_h handle, const char *id, const char *part, const char *operation, const char *option); /**< Update access operation */
	int (*update_color)(dynamicbox_h handle, const char *id, const char *part, const char *data); /**< Update color */
};

/**
 * @internal
 * @brief Called for every async function.
 * @details Prototype of the return callback of every async functions.
 * @since_tizen 2.3
 * @param[in] handle Handle of the dynamicbox instance
 * @param[in] ret Result status of operation (DBOX_STATUS_XXX defined from libdynamicbox-service)
 * @param[in] data Data for result callback
 * @return void
 * @see dynamicbox_add()
 * @see dynamicbox_del()
 * @see dynamicbox_activate()
 * @see dynamicbox_resize()
 * @see dynamicbox_set_group()
 * @see dynamicbox_set_period()
 * @see dynamicbox_access_event()
 * @see dynamicbox_set_pinup()
 * @see dynamicbox_create_glance_bar()
 * @see dynamicbox_destroy_glance_bar()
 * @see dynamicbox_emit_text_signal()
 * @see dynamicbox_acquire_resource_id()
 * @see dynamicbox_set_update_mode()
 */
typedef void (*dynamicbox_ret_cb)(dynamicbox_h handle, int ret, void *data);

/**
 * @internal
 * @brief Initializes the dynamicbox system with some options.
 * @details dynamicbox_init function uses environment value to initiate some configurable values.
 *          But some applications do not want to use the env value.
 *          For them, this API will give a chance to set default options using given arguments.
 * @since_tizen 2.3
 * @param[in] disp Display (if @a disp is @c NULL, the library will try to acquire a new connection with X)
 * @param[in] prevent_overwrite Overwrite flag (when the content of an image type dynamicbox is updated, it will be overwriten (0) or not (1))
 * @param[in] event_filter If the next event comes in this period, ignore it. It is too fast to processing it in time // need to be elaborated
 * @param[in] use_thread User can choose the communication method, if this value is true, the viewer library will create a thread to communicate with master service
 * @privlevel public
 * @privilege %http://tizen.org/privilege/dynamicbox.viewer
 * @return int Integer, Dynamicbox status code
 * @retval #DBOX_STATUS_ERROR_NONE if success
 * @retval #DBOX_STATUS_ERROR_OUT_OF_MEMORY If there is not enough memory to do this
 * @retval #DBOX_STATUS_ERROR_IO_ERROR If fails to access dynamicbox database
 * @see dynamicbox_fini()
 */
extern int dynamicbox_init(void *disp, int prevent_overwrite, double event_filter, int use_thread);

/**
 * @internal
 * @brief Finalizes the dynamicbox system.
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_SUCCES if success
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER if dynamicbox_init is not called
 * @see dynamicbox_init()
 * @see dynamicbox_init_with_options()
 */
extern int dynamicbox_fini(void);

/**
 * @internal
 * @brief Notifies the status of a client ("it is paused") to the provider.
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_ERROR_NONE if success
 * @retval #DBOX_STATUS_ERROR_FAULT if it failed to send state (paused) info
 * @see dynamicbox_client_resumed()
 */
extern int dynamicbox_viewer_set_paused(void);

/**
 * @internal
 * @brief Notifies the status of client ("it is resumed") to the provider.
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_ERROR_NONE if success
 * @retval #DBOX_STATUS_ERROR_FAULT if it failed to send state (resumed) info
 * @see dynamicbox_client_paused()
 */
extern int dynamicbox_viewer_set_resumed(void);

/**
 * @internal
 * @brief Adds a new dynamicbox.
 * @details If the screen size is "1280x720", the below size lists are used for default.
 * Or you can find the default sizes in pixel from /usr/share/data-provider-master/resolution.ini.
 * Size types are defined from the libdynamicbox-service package (dynamicbox-service.h).
 *
 * Normal mode dynamicbox
 * 1x1=175x175, #DBOX_SIZE_TYPE_1x1
 * 2x1=354x175, #DBOX_SIZE_TYPE_2x1
 * 2x2=354x354, #DBOX_SIZE_TYPE_2x2
 * 4x1=712x175, #DBOX_SIZE_TYPE_4x1
 * 4x2=712x354, #DBOX_SIZE_TYPE_4x2
 * 4x4=712x712, #DBOX_SIZE_TYPE_4x4
 *
 * Extended sizes
 * 4x3=712x533, #DBOX_SIZE_TYPE_4x3
 * 4x5=712x891, #DBOX_SIZE_TYPE_4x5
 * 4x6=712x1070, #DBOX_SIZE_TYPE_4x6
 *
 * Easy mode dynamicbox
 * 21x21=224x215, #DBOX_SIZE_TYPE_EASY_1x1
 * 23x21=680x215, #DBOX_SIZE_TYPE_EASY_3x1
 * 23x23=680x653, #DBOX_SIZE_TYPE_EASY_3x3
 *
 * Special dynamicbox
 * 0x0=720x1280, #DBOX_SIZE_TYPE_0x0
 * @since_tizen 2.3
 * @remarks
 *    Even if you get a handle from the return value of this function, it is not a created instance.
 *    So you have to consider it as a not initialized handle.
 *    It can be initialized only after getting the return callback with "ret == #DBOX_STATUS_ERROR_NONE"
 * @param[in] pkgname Dynamicbox Id
 * @param[in] content Contents that will be passed to the dynamicbox instance
 * @param[in] cluster Main group
 * @param[in] category Sub group
 * @param[in] period Update period (@c DEFAULT_PERIOD can be used for this; this argument will be used to specify the period of updating contents of a dynamicbox)
 * @param[in] type Size type (defined from libdynamicbox-service package)
 * @param[in] cb After the request is sent to the master provider, this callback will be called
 * @param[in] data This data will be passed to the callback
 * @privlevel public
 * @privilege %http://tizen.org/privilege/dynamicbox.viewer
 * @return handle
 * @retval Handle Dynamicbox handle but not yet initialized
 * @retval @c NULL if it fails to create a handle
 * @see dynamicbox_ret_cb
 */
extern dynamicbox_h dynamicbox_add(const char *pkgname, const char *content, const char *cluster, const char *category, double period, int type, dynamicbox_ret_cb cb, void *data);

/**
 * @internal
 * @brief Deletes a dynamicbox (will replace dynamicbox_del).
 * @since_tizen 2.3
 * @remarks If you call this with an uninitialized handle, the return callback will be called synchronously.
 *    So before returning from this function, the return callback will be called first.
 * @param[in] handler Handler of a dynamicbox instance
 * @param[in] type Deletion type (DBOX_DELETE_PERMANENTLY or DBOX_DELETE_TEMPORARY)
 * @param[in] cb Return callback
 * @param[in] data User data for return callback
 * @privlevel public
 * @privilege %http://tizen.org/privilege/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid argument
 * @retval #DBOX_STATUS_ERROR_BUSY Already in process
 * @retval #DBOX_STATUS_ERROR_FAULT Failed to create a request packet
 * @retval #DBOX_STATUS_ERROR_NONE Successfully sent, return callack will be called
 * @see dynamicbox_ret_cb
 */
extern int dynamicbox_del(dynamicbox_h handler, int type, dynamicbox_ret_cb cb, void *data);

/**
 * @internal
 * @brief Sets a dynamicbox events callback.
 * @details To get the event push from the provider, register the event callback using this function.
 *    The callback will be called if there is any event from the provider.
 * @since_tizen 2.3
 * @param[in] cb Event handler
 * @param[in] data User data for the event handler
 * @privlevel N/P
 * @return int
 * @retval #DBOX_STATUS_ERROR_NONE If succeed to set event handler
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid argument
 * @retval #DBOX_STATUS_ERROR_OUT_OF_MEMORY Not enough memory
 * @see dynamicbox_unset_event_handler()
 */
extern int dynamicbox_set_event_handler(int (*cb)(dynamicbox_h handler, enum dynamicbox_event_type event, void *data), void *data);

/**
 * @internal
 * @brief Unsets the dynamicbox event handler.
 * @since_tizen 2.3
 * @param[in] cb Event handler
 * @return void * Event handler data
 * @retval pointer Pointer of 'data' which is used with the dynamicbox_set_event_handler
 * @see dynamicbox_set_event_handler()
 */
extern void *dynamicbox_unset_event_handler(int (*cb)(dynamicbox_h handler, enum dynamicbox_event_type event, void *data));

/**
 * @internal
 * @brief Registers the dynamicbox fault event handler.
 * @details Argument list: event, pkgname, filename, funcname.
 * @since_tizen 2.3
 * @param[in] cb Event handler
 * @param[in] data Event handler data
 * @return int
 * @retval #DBOX_STATUS_ERROR_NONE If succeed to set fault event handler
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid argument
 * @retval #DBOX_STATUS_ERROR_OUT_OF_MEMORY Not enough memory
 * @see dynamicbox_unset_fault_handler()
 */
extern int dynamicbox_set_fault_handler(int (*cb)(enum dynamicbox_fault_type, const char *, const char *, const char *, void *), void *data);

/**
 * @internal
 * @brief Unsets the dynamicbox fault event handler.
 * @since_tizen 2.3
 * @param[in] cb Event handler
 * @return void * Callback data which is set via dynamicbox_set_fault_handler
 * @retval pointer Pointer of 'data' which is used with the dynamicbox_set_fault_handler
 * @see dynamicbox_set_fault_handler()
 */
extern void *dynamicbox_unset_fault_handler(int (*cb)(enum dynamicbox_fault_type, const char *, const char *, const char *, void *));

/**
 * @internal
 * @brief Activates the faulted dynamicbox.
 * @details Request result will be returned via return callback.
 * @since_tizen 2.3
 * @remarks Even though this function returns ERROR_NONE, it means that it just successfully sent a request to the provider.
 *    So you have to check the return callback and its "ret" argument.
 * @param[in] pkgname Package name which should be activated
 * @param[in] cb Result callback
 * @param[in] data Callback data
 * @privlevel public
 * @privilege %http://tizen.org/privilege/dynamicbox.viewer
 * @return int type
 * @retval #DBOX_STATUS_ERROR_NONE Successfully sent a request
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid argument
 * @retval #DBOX_STATUS_ERROR_FAULT Failed to make a request
 * @see dynamicbox_ret_cb
 */
extern int dynamicbox_activate(const char *pkgname, dynamicbox_ret_cb cb, void *data);

/**
 * @internal
 * @brief Resizes the dynamicbox.
 * @details
 * Normal mode dynamicbox size
 * 1x1=175x175, DBOX_SIZE_TYPE_1x1
 * 2x1=354x175, DBOX_SIZE_TYPE_2x1
 * 2x2=354x354, DBOX_SIZE_TYPE_2x2
 * 4x1=712x175, DBOX_SIZE_TYPE_4x1
 * 4x2=712x354, DBOX_SIZE_TYPE_4x2
 * 4x4=712x712, DBOX_SIZE_TYPE_4x4
 *
 * Extended dynamicbox size
 * 4x3=712x533, DBOX_SIZE_TYPE_4x3
 * 4x5=712x891, DBOX_SIZE_TYPE_4x5
 * 4x6=712x1070, DBOX_SIZE_TYPE_4x6
 *
 * Easy mode dynamicbox size
 * 21x21=224x215, DBOX_SIZE_TYPE_EASY_1x1
 * 23x21=680x215, DBOX_SIZE_TYPE_EASY_3x1
 * 23x23=680x653, DBOX_SIZE_TYPE_EASY_3x3
 *
 * Special mode dynamicbox size
 * 0x0=720x1280, DBOX_SIZE_TYPE_0x0
 * @since_tizen 2.3
 * @remarks You have to check the return callback.
 * @param[in] handler Handler of a dynamicbox instance
 * @param[in] type Type of a dynamicbox size (e.g., DBOX_SIZE_TYPE_1x1, ...)
 * @param[in] cb Result callback of the resize operation
 * @param[in] data User data for return callback
 * @privlevel public
 * @privilege %http://tizen.org/privilege/dynamicbox.viewer
 * @return int type
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid argument
 * @retval #DBOX_STATUS_ERROR_BUSY Previous request of resize is in progress
 * @retval #DBOX_STATUS_ERROR_ALREADY Already resized, there is no differences between current size and requested size
 * @retval #DBOX_STATUS_ERROR_PERMISSION_DENIED Permission denied, you only have view the content of this box
 * @retval #DBOX_STATUS_ERROR_FAULT Failed to make a request
 * @see dynamicbox_ret_cb
 */
extern int dynamicbox_resize(dynamicbox_h handler, int type, dynamicbox_ret_cb cb, void *data);

/**
 * @internal
 * @brief Sends the click event for a dynamicbox.
 * @since_tizen 2.3
 * @param[in] handler Handler of a dynamicbox instance
 * @param[in] x Rational X of the content width
 * @param[in] y Rational Y of the content height
 * @privlevel public
 * @privilege %http://tizen.org/privilege/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid argument
 * @retval #DBOX_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #DBOX_STATUS_ERROR_NONE Successfully done
 */
extern int dynamicbox_click(dynamicbox_h handler, double x, double y);

/**
 * @internal
 * @brief Changes the cluster/sub-cluster name of the given dynamicbox handler.
 * @since_tizen 2.3
 * @param[in] handler Handler of a dynamicbox instance
 * @param[in] cluster New cluster of a dynamicbox
 * @param[in] category New category of a dynamicbox
 * @param[in] cb Result callback for changing the cluster/category of a dynamicbox
 * @param[in] data User data for the result callback
 * @privlevel public
 * @privilege %http://tizen.org/privilege/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_ERROR_NONE Request is successfully sent. the return callback will be called
 * @retval #DBOX_STATUS_ERROR_BUSY Previous request is not finished yet
 * @retval #DBOX_STATUS_ERROR_ALREADY Group name is same with current one
 * @retval #DBOX_STATUS_ERROR_PERMISSION_DENIED You have no permission to change property of this dynamicbox instance
 * @retval #DBOX_STATUS_ERROR_FAULT Failed to make a request
 * @see dynamicbox_ret_cb
 */
extern int dynamicbox_set_group(dynamicbox_h handler, const char *cluster, const char *category, dynamicbox_ret_cb cb, void *data);

/**
 * @internal
 * @brief Gets the cluster and category (sub-cluster) name of the given dynamicbox (it is not I18N format, only English).
 * @since_tizen 2.3
 * @remarks You have to do not release the cluster & category.
 *    It is allocated inside of a given dynamicbox instance, so you can only read it.
 * @param[in] handler Handler of a dynamicbox instance
 * @param[out] cluster Storage(memory) for containing the cluster name
 * @param[out] category Storage(memory) for containing the category name
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid argument
 * @retval #DBOX_STATUS_ERROR_NONE Successfully done
 */
extern int dynamicbox_get_group(dynamicbox_h handler, const char **cluster, const char **category);

/**
 * @internal
 * @brief Gets the period of the dynamicbox handler.
 * @since_tizen 2.3
 * @remarks If this function returns 0.0f, it means the dynamicbox has no update period or the handle is not valid.
 *    This function only works after the return callback of dynamicbox_create fucntion is called.
 * @param[in] handler Handler of a dynamicbox instance
 * @return double
 * @retval Current update period of a dynamicbox
 * @retval 0.0f This means the box has no update period or the handles is not valid
 */
extern double dynamicbox_period(dynamicbox_h handler);

/**
 * @internal
 * @brief Changes the update period.
 * @since_tizen 2.3
 * @param[in] handler Handler of a dynamicbox instance
 * @param[in] period New update period of a dynamicbox
 * @param[in] cb Result callback of changing the update period of this dynamicbox
 * @param[in] data User data for the result callback
 * @privlevel public
 * @privilege %http://tizen.org/privilege/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_ERROR_NONE Successfully done
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid argument
 * @retval #DBOX_STATUS_ERROR_BUSY
 * @retval #DBOX_STATUS_ERROR_ALREADY
 * @retval #DBOX_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @see dynamicbox_ret_cb
 */
extern int dynamicbox_set_period(dynamicbox_h handler, double period, dynamicbox_ret_cb cb, void *data);

/**
 * @internal
 * @brief Checks whether the given dynamicbox is a text type or not.
 * @since_tizen 2.3
 * @param[in] handler Handler of a dynamicbox instance
 * @param[in] gbar 1 for Glance Bar or 0
 * @return dynamicbox_type
 * @retval #DBOX_TYPE_IMAGE Contents of a dynamicbox is based on the image file
 * @retval #DBOX_TYPE_BUFFER Contents of a dynamicbox is based on canvas buffer(shared)
 * @retval #DBOX_TYPE_TEXT Contents of a dynamicbox is based on formatted text file
 * @retval #DBOX_TYPE_PIXMAP Contens of a dynamicbox is shared by the resource id(depends on X)
 * @retval #DBOX_TYPE_INVALID Invalid type
 * @see dynamicbox_type()
 */
extern enum dynamicbox_type dynamicbox_type(dynamicbox_h handler, int gbar);

/**
 * @internal
 * @brief Checks if the given dynamicbox is created by user or not.
 * @since_tizen 2.3
 * @details If the dynamicbox instance is created by a system this will return 0.
 * @param[in] handler Handler of a dynamicbox instance
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid argument
 * @retval 0 Automatically created dynamicbox by the provider
 * @retval 1 Created by user via dynamicbox_add()
 * @see dynamicbox_add()
 * @see dynamicbox_set_event_handler()
 */
extern int dynamicbox_is_created_by_user(dynamicbox_h handler);

/**
 * @internal
 * @brief Gets content information string of the given dynamicbox.
 * @since_tizen 2.3
 * @param[in] handler Handler of a dynamicbox instance
 * @return const char *
 * @retval content_info Dynamicbox content info that can be used again via content_info argument of dynamicbox_add()
 * @see dynamicbox_add()
 */
extern const char *dynamicbox_content(dynamicbox_h handler);

/**
 * @internal
 * @brief Gets the sub cluster title string of the given dynamicbox.
 * @details This API is now used for accessibility.
 *  Each box should set their content as a string to be read by TTS.
 *  So if the box has focused on the homescreen, the homescreen will read text using this API.
 * @since_tizen 2.3
 * @remarks The title returned by this API is read by TTS.
 *  But it is just recomended to a homescreen.
 *  So, to read it or not depends on implementation of the homescreen.
 * @param[in] handler Handler of a dynamicbox instance
 * @return const char *
 * @retval sub Cluster name
 * @retval @c NULL
 */
extern const char *dynamicbox_title(dynamicbox_h handler);

/**
 * @internal
 * @brief Gets the filename of the given dynamicbox, if it is an IMAGE type dynamicbox.
 * @details If the box is developed as image format to represent its contents, the homescreen should know its image file name.
 * @since_tizen 2.3
 * @param[in] handler Handler of a dynamicbox instance
 * @return const char *
 * @retval filename If the dynamicbox type is image this function will give you a abs-path of an image file (content is rendered)
 * @retval @c NULL If this has no image file or type is not image file.
 */
extern const char *dynamicbox_filename(dynamicbox_h handler);

/**
 * @internal
 * @brief Gets the package name of the given dynamicbox handler.
 * @since_tizen 2.3
 * @param[in] handler Handler of a dynamicbox instance
 * @return const char *
 * @retval pkgname Package name
 * @retval @c NULL If the handler is not valid
 */
extern const char *dynamicbox_pkgname(dynamicbox_h handler);

/**
 * @internal
 * @brief Gets the priority of a current content.
 * @since_tizen 2.3
 * @param[in] handler Handler of a dynamicbox instance
 * @return double
 * @retval 0.0f Handler is @c NULL
 * @retval -1.0f Handler is not valid (not yet initialized)
 * @retval real Number between 0.0 and 1.0
 */
extern double dynamicbox_priority(dynamicbox_h handler);

/**
 * @internal
 * @brief Acquires the buffer of a given dynamicbox (only for the buffer type).
 * @since_tizen 2.3
 * @param[in] handler Handler of a dynamicbox instance
 * @param[in] gbar 1 for Glance Bar or 0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/dynamicbox.viewer
 * @return void *
 * @retval address Address of a Frame Buffer
 * @retval @c NULL If it fails to get fb address
 */
extern void *dynamicbox_acquire_fb(dynamicbox_h handler, int gbar);

/**
 * @internal
 * @brief Releases the buffer of a dynamicbox (only for the buffer type).
 * @since_tizen 2.3
 * @param[in] buffer Buffer
 * @privlevel public
 * @privilege %http://tizen.org/privilege/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid argument
 * @retval #DBOX_STATUS_ERROR_NONE Successfully done
 * @see dynamicbox_acquire_fb()
 */
extern int dynamicbox_release_fb(void *buffer);

/**
 * @internal
 * @brief Gets the reference count of Dynamicbox buffer (only for the buffer type).
 * @since_tizen 2.3
 * @param[in] buffer Buffer
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid argument
 * @retval #DBOX_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval refcnt Positive integer including ZERO
 */
extern int dynamicbox_fb_refcnt(void *buffer);

/**
 * @internal
 * @brief Gets the size of the Dynamicbox.
 * @since_tizen 2.3
 * @param[in] handler Handler of a dynamicbox instance
 * @return int
 * @retval #DBOX_SIZE_TYPE_NxM
 * @retval #DBOX_SIZE_TYPE_INVALID Invalid parameters are used
 */
extern int dynamicbox_size(dynamicbox_h handler);

/**
 * @internal
 * @brief Gets the size of the Glance Bar.
 * @since_tizen 2.3
 * @param[in] handler Handler of a dynamicbox instance
 * @param[out] w
 * @param[out] h
 * @return int type
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid parameters are used
 * @retval #DBOX_STATUS_ERROR_NONE Successfully done
 */
extern int dynamicbox_get_glance_bar_size(dynamicbox_h handler, int *w, int *h);

/**
 * @internal
 * @brief Gets a list of the supported sizes of a given handler.
 * @since_tizen 2.3
 * @param[in] handler Handler of a dynamicbox instance
 * @param[out] cnt
 * @param[out] size_list
 * @return int type
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid argument
 * @retval #DBOX_STATUS_ERROR_NONE Successfully done
 */
extern int dynamicbox_get_supported_sizes(dynamicbox_h handler, int *cnt, int *size_list);

/**
 * @internal
 * @brief Gets BUFFER SIZE of the dynamicbox if it is a buffer type.
 * @since_tizen 2.3
 * @param[in] handler Handler of a dynamicbox instance
 * @param[in] gbar 1 for Glance Bar or 0
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid argument
 * @retval size Size of dynamicbox buffer
 */
extern int dynamicbox_fb_bufsz(dynamicbox_h handler, int gbar);

/**
 * @internal
 * @brief Sends a content event (for buffer type) to the provider (dynamicbox).
 * @since_tizen 2.3
 * @param[in] handler Handler of a dynamicbox instance
 * @param[in] type Event type
 * @param[in] x Coordinates of X axis
 * @param[in] y Coordinates of Y axis
 * @privlevel public
 * @privilege %http://tizen.org/privilege/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid argument
 * @retval #DBOX_STATUS_ERROR_BUSY Previous operation is not finished yet
 * @retval #DBOX_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #DBOX_STATUS_ERROR_NONE Successfully sent
 * @see dynamicbox_feed_access_event()
 * @see dynamicbox_feed_key_event()
 */
extern int dynamicbox_feed_mouse_event(dynamicbox_h handler, enum dynamicbox_mouse_event_type type, struct dynamicbox_mouse_event_info *info);

/**
 * @internal
 * @brief Sends an access event (for buffer type) to the provider (dynamicbox).
 * @since_tizen 2.3
 * @param[in] handler Handler of a dynamicbox instance
 * @param[in] type Event type
 * @param[in] x Coordinates of X axsis
 * @param[in] y Coordinates of Y axsis
 * @param[in] cb Result callback function
 * @param[in] data Callback data
 * @privlevel public
 * @privilege %http://tizen.org/privilege/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid argument
 * @retval #DBOX_STATUS_ERROR_BUSY Previous operation is not finished yet
 * @retval #DBOX_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #DBOX_STATUS_ERROR_NONE Successfully sent
 * @see dynamicbox_feed_mouse_event()
 * @see dynamicbox_feed_key_event()
 */
extern int dynamicbox_feed_access_event(dynamicbox_h handler, enum dynamicbox_access_event_type type, struct dynamicbox_access_event_info *info, dynamicbox_ret_cb cb, void *data);

/**
 * @internal
 * @brief Sends a key event (for buffer type) to the provider (dynamicbox).
 * @since_tizen 2.3
 * @param[in] handler Handler of a dynamicbox instance
 * @param[in] type Key event type
 * @param[in] keycode Code of key
 * @param[in] cb Result callback
 * @param[in] data Callback data
 * @privlevel public
 * @privilege %http://tizen.org/privilege/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid argument
 * @retval #DBOX_STATUS_ERROR_BUSY Previous operation is not finished yet
 * @retval #DBOX_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #DBOX_STATUS_ERROR_NONE Successfully sent
 * @see dynamicbox_feed_mouse_event()
 * @see dynamicbox_feed_access_event()
 */
extern int dynamicbox_feed_key_event(dynamicbox_h handler, enum dynamicbox_key_event_type type, struct dynamicbox_key_event_info *info, dynamicbox_ret_cb cb, void *data);

/**
 * @internal
 * @brief Sets pin-up status of the given handler.
 * @details If the dynamicbox supports the pinup feature,
 *   you can freeze the update of the given dynamicbox.
 *   But it is different from pause.
 *   The box will be updated and it will decide wheter update its content or not when the pinup is on.
 * @since_tizen 2.3
 * @param[in] handler Handler of a dynamicbox instance
 * @param[in] flag Pinup value
 * @param[in] cb Result callback
 * @param[in] data Callback data
 * @privlevel public
 * @privilege %http://tizen.org/privilege/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid parameters
 * @see dynamicbox_ret_cb
 * @see dynamicbox_set_visibility()
 * @see dynamicbox_is_pinned_up()
 */
extern int dynamicbox_set_pinup(dynamicbox_h handler, int flag, dynamicbox_ret_cb cb, void *data);

/**
 * @internal
 * @brief Checks the PIN-UP status of the given handler.
 * @since_tizen 2.3
 * @param[in] handler Handler of a dynamicbox instance
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid parameters
 * @retval 1 Box is pinned up
 * @retval 0 Box is not pinned up
 * @see dynamicbox_set_pinup()
 */
extern int dynamicbox_is_pinned_up(dynamicbox_h handler);

/**
 * @internal
 * @brief Checks the availability of the PIN-UP feature for the given handler.
 * @since_tizen 2.3
 * @param[in] handler Handler of a dynamicbox instance
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid argument
 * @retval 1 If the box support Pinup feature
 * @retval 0 If the box does not support the Pinup feature
 * @see dynamicbox_is_pinned_up()
 * @see dynamicbox_set_pinup()
 */
extern int dynamicbox_has_pinup(dynamicbox_h handler);

/**
 * @internal
 * @brief Checks the existence of Glance Bar for the given handler.
 * @since_tizen 2.3
 * @param[in] handler Handler of a dynamicbox instance
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid argument
 * @retval 1 If the box support the Glance Bar
 * @retval 0 If the box has no Glance Bar
 */
extern int dynamicbox_has_glance_bar(dynamicbox_h handler);

/**
 * @internal
 * @brief Creates Glance Bar of the given handler with the relative position from dynamicbox.
 * @since_tizen 2.3
 * @param[in] handler Handler of a dynamicbox instance
 * @param[in] x 0.0 ~ 1.0
 * @param[in] y 0.0 ~ 1.0
 * @param[in] cb Result callback
 * @param[in] data Callback data
 * @privlevel public
 * @privilege %http://tizen.org/privilege/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_ERROR_NONE Successfully done
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid argument
 * @retval #DBOX_STATUS_ERROR_BUSY Previous operation is not finished yet
 * @retval #DBOX_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @see dynamicbox_create_glance_bar()
 * @see dynamicbox_destroy_glance_bar()
 * @see dynamicbox_move_glance_bar()
 */
extern int dynamicbox_create_glance_bar(dynamicbox_h handler, double x, double y, dynamicbox_ret_cb cb, void *data);

/**
 * @internal
 * @brief Updates a position of the given Glance Bar.
 * @since_tizen 2.3
 * @param[in] handler Handler of a dynamicbox instance
 * @param[in] x 0.0 ~ 1.0
 * @param[in] y 0.0 ~ 1.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_ERROR_NONE If sending a request for updating position of the Glance Bar has been done successfully
 * @retval #DBOX_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid argument
 */
extern int dynamicbox_move_glance_bar(dynamicbox_h handler, double x, double y);

/**
 * @internal
 * @brief Destroys the Glance Bar of the given handler if it is created.
 * @since_tizen 2.3
 * @param[in] handler Handler of a dynamicbox instance
 * @param[in] cb
 * @param[in] data
 * @privlevel public
 * @privilege %http://tizen.org/privilege/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid argument
 * @retval #DBOX_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #DBOX_STATUS_ERROR_NONE Successfully done
 * @see dynamicbox_ret_cb
 */
extern int dynamicbox_destroy_glance_bar(dynamicbox_h handler, dynamicbox_ret_cb cb, void *data);

/**
 * @internal
 * @brief Checks the create status of the given dynamicbox handler.
 * @since_tizen 2.3
 * @param[in] handler Handler of a dynamicbox instance
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid argument
 * @retval 0 Glance Bar is not created
 * @retval 1 Glance Bar is created
 */
extern int dynamicbox_glance_bar_is_created(dynamicbox_h handler);

/**
 * @internal
 * @brief Sets a function table for parsing the text content of a dynamicbox.
 * @since_tizen 2.3
 * @param[in] handler Handler of a dynamicbox instance
 * @param[in] gbar 1 for Glance Bar or 0
 * @param[in] ops
 * @return int
 * @retval #DBOX_STATUS_ERROR_NONE Successfully done
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid argument
 * @see dynamicbox_set_gbar_text_handler()
 */
extern int dynamicbox_set_text_handler(dynamicbox_h handler, int gbar, struct dynamicbox_script_operators *ops);

/**
 * @internal
 * @brief Emits a text signal to the given dynamicbox only if it is a text type.
 * @since_tizen 2.3
 * @param[in] handler Handler of a dynamicbox instance
 * @param[in] emission Emission string
 * @param[in] source Source string
 * @param[in] sx Start X
 * @param[in] sy Start Y
 * @param[in] ex End X
 * @param[in] ey End Y
 * @param[in] cb Result callback
 * @param[in] data Callback data
 * @privlevel public
 * @privilege %http://tizen.org/privilege/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid parameters
 * @retval #DBOX_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #DBOX_STATUS_ERROR_NONE Successfully emitted
 * @see dynamicbox_ret_cb
 */
extern int dynamicbox_emit_text_signal(dynamicbox_h handler, const char *emission, const char *source, double sx, double sy, double ex, double ey, dynamicbox_ret_cb cb, void *data);

/**
 * @internal
 * @brief Sets a private data pointer to carry it using the given handler.
 * @since_tizen 2.3
 * @param[in] handler Handler of a dynamicbox instance
 * @param[in] data Data pointer
 * @return int
 * @retval #DBOX_STATUS_ERROR_NONE Successfully registered
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid argument
 * @see dynamicbox_get_data()
 */
extern int dynamicbox_set_data(dynamicbox_h handler, void *data);

/**
 * @internal
 * @brief Gets a private data pointer which is carried by a given handler.
 * @since_tizen 2.3
 * @param[in] handler Handler of a dynamicbox instance
 * @return void *
 * @retval data Data pointer
 * @retval @c NULL If there is no data
 * @see dynamicbox_set_data()
 */
extern void *dynamicbox_data(dynamicbox_h handler);

/**
 * @internal
 * @brief Subscribes an event for dynamicboxes only in a given cluster and sub-cluster.
 * @details If you wrote a view-only client,
 *   you can receive the event of specific dynamicboxes which belong to a given cluster/category.
 *   But you cannot modify their attributes (such as size, ...).
 * @since_tizen 2.3
 * @param[in] cluster Cluster ("*" can be used for subscribe all cluster's dynamicboxes event; If you use the "*", value in the category will be ignored)
 * @param[in] category Category ("*" can be used for subscribe dynamicboxes events of all category(sub-cluster) in a given "cluster")
 * @privlevel public
 * @privilege %http://tizen.org/privilege/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #DBOX_STATUS_ERROR_NONE Successfully requested
 * @see dynamicbox_unsubscribe_group()
 */
extern int dynamicbox_subscribe_group(const char *cluster, const char *category);

/**
 * @internal
 * @brief Unsubscribes an event for the dynamicboxes, but you will receive already added dynamicboxes events.
 * @since_tizen 2.3
 * @param[in] cluster Cluster("*" can be used for subscribe all cluster's dynamicboxes event; If you use the "*", value in the category will be ignored)
 * @param[in] category Category ("*" can be used for subscribe all sub-cluster's dynamicboxes event in a given "cluster")
 * @privlevel public
 * @privilege %http://tizen.org/privilege/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #DBOX_STATUS_ERROR_NONE Successfully requested
 * @see dynamicbox_subscribe_group()
 */
extern int dynamicbox_unsubscribe_group(const char *cluster, const char *category);

/**
 * @internal
 * @brief Refreshes the group (cluster/sub-cluser (aka. category)).
 * @details This function will trigger the update of all dynamicboxes in a given cluster/category group.
 * @since_tizen 2.3
 * @remarks Basically, a default dynamicbox system doesn't use the cluster/category concept.
 *    But you can use it. So if you decide to use it, then you can trigger the update of all dynamicboxes in the given group.
 * @param[in] cluster Cluster ID
 * @param[in] category Sub-cluster ID
 * @param[in] force 1 if the boxes should be updated even if they are paused
 * @privlevel public
 * @privilege %http://tizen.org/privilege/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid argument
 * @retval #DBOX_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #DBOX_STATUS_ERROR_NONE Successfully requested
 * @see dynamicbox_refresh()
 */
extern int dynamicbox_refresh_group(const char *cluster, const char *category, int force);

/**
 * @internal
 * @brief Refreshes a dynamicbox.
 * @since_tizen 2.3
 * @param[in] handler Handler of a dynamicbox instance
 * @param[in] force 1 if the box should be updated even if it is paused
 * @privlevel public
 * @privilege %http://tizen.org/privilege/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid argument
 * @retval #DBOX_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #DBOX_STATUS_ERROR_NONE Successfully requested
 * @see dynamicbox_refresh_group()
 */
extern int dynamicbox_refresh(dynamicbox_h handler, int force);

/**
 * @internal
 * @brief Gets Resource Id of a dynamicbox content.
 * @details This function doesn't guarantee the life-cycle of the resource id.
 *   If the service provider destroyed the resource id, you will not know about it.
 *   So you should validate it before accessing it.
 * @since_tizen 2.3
 * @param[in] handler Handler of a dynamicbox instance
 * @param[in] gbar 1 for Glance Bar or 0
 * @return int
 * @retval 0 If the resource id is not created
 * @retval ResourceId Resource Id
 * @see dynamicbox_resource_id()
 */
extern unsigned int dynamicbox_resource_id(const dynamicbox_h handler, int gbar);

/**
 * @internal
 * @brief Gets the Resource Id of a dynamicbox.
 * @details Even if a render process releases the Resource Id, the Resource Id will be kept before being released by dynamicbox_release_resource_id.
 *   You should release the resource id manually.
 * @since_tizen 2.3
 * @param[in] handler Handler of a dynamicbox instance
 * @param[in] gbar 1 for Glance Bar or 0
 * @param[in] cb Callback function which will be called with result of acquiring lb resource id
 * @param[in] data Callback data
 * @privlevel public
 * @privilege %http://tizen.org/privilege/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid argument
 * @retval #DBOX_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #DBOX_STATUS_ERROR_NONE Successfully requested
 * @pre Dynamicbox service system should support the PIXMAP type buffer.
 *   The dynamicbox should be designed to use the buffer (script type).
 * @see dynamicbox_release_resource_id()
 * @see dynamicbox_ret_cb
 */
extern unsigned int dynamicbox_acquire_resource_id(dynamicbox_h handler, int gbar, dynamicbox_ret_cb cb, void *data);

/**
 * @internal
 * @brief Releases the Resource Id of a dynamicbox.
 * @details After a client gets a new Resource Id or does not need to keep the current Resource Id anymore, use this function to release it.
 *   If there is no user for a given Resource Id, the Resource Id will be destroyed.
 * @since_tizen 2.3
 * @param[in] handler Handler of a dynamicbox instance
 * @param[in] gbar 1 for Glance Bar or 0
 * @param[in] resource_id Resource Id of given dynamicbox handler
 * @privlevel public
 * @privilege %http://tizen.org/privilege/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid argument
 * @retval #DBOX_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #DBOX_STATUS_ERROR_NONE Successfully done
 * @pre The Resource Id should be acquired by dynamicbox_acquire_resource_id
 * @see dynamicbox_acquire_resource_id()
 */
extern int dynamicbox_release_resource_id(dynamicbox_h handler, int gbar, unsigned int resource_id);

/**
 * @internal
 * @brief Updates a visible state of the dynamicbox.
 * @since_tizen 2.3
 * @param[in] handler Handler of a dynamicbox instance
 * @param[in] state Configure the current visible state of a dynamicbox
 * @privlevel public
 * @privilege %http://tizen.org/privilege/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid argument
 * @retval #DBOX_STATUS_ERROR_BUSY
 * @retval #DBOX_STATUS_ERROR_PERMISSION_DENIED
 * @retval #DBOX_STATUS_ERROR_ALREADY
 * @retval #DBOX_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #DBOX_STATUS_ERROR_NONE Successfully done
 */
extern int dynamicbox_set_visibility(dynamicbox_h handler, enum dynamicbox_visible_state state);

/**
 * @internal
 * @brief Updates a visible state of the dynamicbox.
 * @brief Gets the current visible state of a dynamicbox.
 * @since_tizen 2.3
 * @param[in] handler Handler of a dynamicbox instance
 * @return dynamicbox_visible_state
 * @retval #DBOX_SHOW Dynamicbox is shown (Default state)
 * @retval #DBOX_HIDE Dynamicbox is hidden, Update timer is not frozen (but a user cannot receive any updated events; a user should refresh(reload) the content of a dynamicbox when a user make this show again)
 * @retval #DBOX_HIDE_WITH_PAUSE Dynamicbox is hidden, it will pause the update timer, but if a dynamicbox updates its contents, update event will occur
 * @retval #DBOX_VISIBLE_ERROR To enlarge the size of this enumeration type
 */
extern enum dynamicbox_visible_state dynamicbox_visibility(dynamicbox_h handler);

/**
 * @internal
 * @brief Sets an update mode of the current dynamicbox.
 * @details If you set 1 for active update mode, you should get a buffer without updated event from provider.
 *   But if it is passive mode, you have to update content of a box when you get updated events.
 *   Default is Passive mode.
 * @since_tizen 2.3
 * @param[in] handler Handler of a dynamicbox instance
 * @param[in] active_update 1 means active update, 0 means passive update (default)
 * @param[in] cb Result callback function
 * @param[in] data Callback data
 * @privlevel public
 * @privilege %http://tizen.org/privilege/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid argument
 * @retval #DBOX_STATUS_ERROR_BUSY
 * @retval #DBOX_STATUS_ERROR_PERMISSION_DENIED
 * @retval #DBOX_STATUS_ERROR_ALREADY
 * @retval #DBOX_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #DBOX_STATUS_ERROR_NONE Successfully done
 * @see dynamicbox_ret_cb
 */
extern int dynamicbox_set_update_mode(dynamicbox_h handler, int active_update, dynamicbox_ret_cb cb, void *data);

/**
 * @internal
 * @brief Checks the active update mode of the given dynamicbox.
 * @since_tizen 2.3
 * @param[in] handler Handler of a dynamicbox instance
 * @return int
 * @retval 0 If passive mode
 * @retval 1 If active mode or error code
 */
extern int dynamicbox_is_active_update(dynamicbox_h handler);

/**
 * @internal
 * @brief Syncs manually
 * @since_tizen 2.3
 * @param[in] handler Handler of a dynamicbox instance
 * @param[in] gbar 1 for Glance Bar or 0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/dynamicbox.viewer
 * @return void
 * @retval #DBOX_STATUS_ERROR_NONE If success
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid handle
 * @see dynamicbox_set_manual_sync()
 * @see dynamicbox_manual_sync()
 */
extern int dynamicbox_sync_fb(dynamicbox_h handler, int gbar);

/**
 * @internal
 * @brief Gets an alternative icon of the given dynamicbox instance.
 * @details If the box should be represented as a shortcut icon, this function will get the alternative icon.
 * @since_tizen 2.3
 * @param[in] handler Handler of a dynamicbox instance
 * @return const char *
 * @retval address Absolute path of an alternative icon file
 * @retval @c NULL Dynamicbox has no alternative icon file
 * @see dynamicbox_alt_name()
 */
extern const char *dynamicbox_alt_icon(dynamicbox_h handler);

/**
 * @internal
 * @brief Gets an alternative name of the given dynamicbox instance.
 * @details If the box should be represented as a shortcut name, this function will get the alternative name.
 * @since_tizen 2.3
 * @param[in] handler Handler of a dynamicbox instance
 * @return const char *
 * @retval name Alternative name of a dynamicbox
 * @retval @c NULL Dynamicbox has no alternative name
 * @see dynamicbox_alt_icon()
 */
extern const char *dynamicbox_alt_name(dynamicbox_h handler);

/**
 * @internal
 * @brief Gets a lock for a frame buffer.
 * @details This function should be used to prevent from rendering to the frame buffer while reading it.
 *   And the locking area should be short and must be released ASAP, or the render thread will be hanged.
 * @since_tizen 2.3
 * @param[in] handler Handler of a dynamicbox instance
 * @param[in] gbar 1 for Glance Bar or 0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid argument
 * @retval #DBOX_STATUS_ERROR_NONE Successfully done
 * @see dynamicbox_release_fb_lock()
 */
extern int dynamicbox_acquire_fb_lock(dynamicbox_h handler, int gbar);

/**
 * @internal
 * @brief Releases a lock of the frame buffer.
 * @details This function should be called ASAP after acquiring a lock of FB, or the render process will be blocked.
 * @since_tizen 2.3
 * @param[in] handler Handler of a dynamicbox instance
 * @param[in] gbar 1 for Glance Bar or 0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid argument
 * @retval #DBOX_STATUS_ERROR_NONE Successfully done
 * @see dynamicbox_acquire_fb_lock()
 */
extern int dynamicbox_release_fb_lock(dynamicbox_h handler, int gbar);

/**
 * @internal
 * @brief Sets options for controlling a dynamicbox sub-system.
 * @details
 *   DBOX_OPTION_FRAME_DROP_FOR_RESIZE
 *     While resizing the box, viewer doesn't want to know the updated frames of an old size content anymore.
 *     In that case, turn this on, the provider will not send the updated event to the viewer about an old content.
 *     So the viewer can reduce its burden to update unnecessary frames.
 *   DBOX_OPTION_MANUAL_SYNC
 *     If you don't want to update frames automatically, or you want only reload the frames by your hands, (manually)
 *     Turn this on.
 *     After turnning it on, you should sync it using dynamicbox_sync_gbar_fb and dynamicbox_sync_dbox_pfb.
 *   DBOX_OPTION_SHARED_CONTENT
 *     If this option is turnned on, even though you create a new dynamicbox,
 *     if there are already added same instances that have same contents, the instance will not be created again.
 *     Instead of creating a new instance, a viewer will provide an old instance with a new handle.
 * @since_tizen 2.3
 * @param[in] option Option which will be affected by this call
 * @param[in] state New value for given option
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Unknown option
 * @retval #DBOX_STATUS_ERROR_FAULT Failed to change the state of option
 * @retval #DBOX_STATUS_ERROR_NONE Successfully changed
 * @see dynamicbox_get_option()
 * @see dynamicbox_sync_fb()
 */
extern int dynamicbox_set_option(enum dynamicbox_option_type option, int state);

/**
 * @internal
 * @brief Gets options of a dynamicbox sub-system.
 * @since_tizen 2.3
 * @param[in] option Type of option
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID_PARAMETER Invalid option
 * @retval #DBOX_STATUS_ERROR_FAULT Failed to get option
 * @retval >=0 Value of given option (must be >=0)
 * @see dynamicbox_set_option()
 */
extern int dynamicbox_option(enum dynamicbox_option_type option);


/**
 * @internal
 * @brief Set a handler for launching an app for auto-launch feature
 * @details If a user clicks a box, and the box uses auto-launch option, the launcher_handler will be called.
 * @since_tizen 2.3
 * @param[in] launch_handler Handler for launching an app manually
 * @param[in] data Callback data which will be given a data for launch_handler
 * @return int type
 * @retval #DBOX_STATUS_ERROR_NONE Succeed to set new handler. there is no other cases
 */
extern int dynamicbox_set_auto_launch_handler(int (*launch_handler)(dynamicbox_h handler, const char *appid, void *data), void *data);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
