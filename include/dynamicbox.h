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
 */

/**
 * @addtogroup CAPI_DYNAMIC_VIEWER_MODULE
 * @{
 */

/**
 * @brief
 * Structure for a Dynamicbox instance.
 */
struct dynamicbox;

/**
 * @brief Definition for a default update period for Dynamicbox (defined in the package manifest file).
 */
#define DBOX_DEFAULT_PERIOD -1.0f

/**
 * @brief Enumeration for Mouse & Key event for buffer type Dynamicbox or PD.
 * @details Viewer should send these events to dynamicbox.
 */
enum dynamicbox_content_event_type {
	DBOX_CONTENT_EVENT_MOUSE_DOWN	= 0x00000001, /**< LB mouse down event for dynamicbox */
	DBOX_CONTENT_EVENT_MOUSE_UP		= 0x00000002, /**< LB mouse up event for dynamicbox */
	DBOX_CONTENT_EVENT_MOUSE_MOVE	= 0x00000004, /**< LB mouse move event for dynamicbox */
	DBOX_CONTENT_EVENT_MOUSE_ENTER	= 0x00000008, /**< LB mouse enter event for dynamicbox */
	DBOX_CONTENT_EVENT_MOUSE_LEAVE	= 0x00000010, /**< LB mouse leave event for dynamicbox */
	DBOX_CONTENT_EVENT_MOUSE_SET		= 0x00000020, /**< LB mouse set auto event for dynamicbox */
	DBOX_CONTENT_EVENT_MOUSE_UNSET	= 0x00000040, /**< LB mouse unset auto event for dynamicbox */

	DBOX_CONTENT_EVENT_KEY_DOWN		= 0x00000001, /**< LB key press */
	DBOX_CONTENT_EVENT_KEY_UP		= 0x00000002, /**< LB key release */
	DBOX_CONTENT_EVENT_KEY_FOCUS_IN	= 0x00000008, /**< LB key focused in */
	DBOX_CONTENT_EVENT_KEY_FOCUS_OUT	= 0x00000010, /**< LB key focused out */
	DBOX_CONTENT_EVENT_KEY_SET		= 0x00000020, /**< LB Key, start feeding event by master */
	DBOX_CONTENT_EVENT_KEY_UNSET		= 0x00000040, /**< LB key, stop feeding event by master */

	DBOX_CONTENT_EVENT_ON_SCROLL		= 0x00000080, /**< LB On scrolling */
	DBOX_CONTENT_EVENT_ON_HOLD		= 0x00000100, /**< LB On holding */
	DBOX_CONTENT_EVENT_OFF_SCROLL	= 0x00000200, /**< LB Stop scrolling */
	DBOX_CONTENT_EVENT_OFF_HOLD		= 0x00000400, /**< LB Stop holding */

	DBOX_CONTENT_EVENT_KEY_MASK		= 0x80000000, /**< Mask value for key event */
	DBOX_CONTENT_EVENT_MOUSE_MASK	= 0x20000000, /**< Mask value for mouse event */
	DBOX_CONTENT_EVENT_PD_MASK		= 0x10000000, /**< Mask value for PD event */
	DBOX_CONTENT_EVENT_DBOX_MASK		= 0x40000000, /**< Mask value for LB event */

	DBOX_MOUSE_ON_SCROLL	= DBOX_CONTENT_EVENT_DBOX_MASK | DBOX_CONTENT_EVENT_MOUSE_MASK | DBOX_CONTENT_EVENT_ON_SCROLL, /**< Mouse event occurs while scrolling */
	DBOX_MOUSE_ON_HOLD		= DBOX_CONTENT_EVENT_DBOX_MASK | DBOX_CONTENT_EVENT_MOUSE_MASK | DBOX_CONTENT_EVENT_ON_HOLD, /**< Mouse event occurs on holding */
	DBOX_MOUSE_OFF_SCROLL	= DBOX_CONTENT_EVENT_DBOX_MASK | DBOX_CONTENT_EVENT_MOUSE_MASK | DBOX_CONTENT_EVENT_OFF_SCROLL, /**< Scrolling stopped */
	DBOX_MOUSE_OFF_HOLD		= DBOX_CONTENT_EVENT_DBOX_MASK | DBOX_CONTENT_EVENT_MOUSE_MASK | DBOX_CONTENT_EVENT_OFF_HOLD, /**< Holding stopped */

	DBOX_MOUSE_DOWN			= DBOX_CONTENT_EVENT_DBOX_MASK | DBOX_CONTENT_EVENT_MOUSE_MASK | DBOX_CONTENT_EVENT_MOUSE_DOWN, /**< Mouse down on the dynamicbox */
	DBOX_MOUSE_UP			= DBOX_CONTENT_EVENT_DBOX_MASK | DBOX_CONTENT_EVENT_MOUSE_MASK | DBOX_CONTENT_EVENT_MOUSE_UP, /**< Mouse up on the dynamicbox */
	DBOX_MOUSE_MOVE			= DBOX_CONTENT_EVENT_DBOX_MASK | DBOX_CONTENT_EVENT_MOUSE_MASK | DBOX_CONTENT_EVENT_MOUSE_MOVE, /**< Move move on the dynamicbox */
	DBOX_MOUSE_ENTER		= DBOX_CONTENT_EVENT_DBOX_MASK | DBOX_CONTENT_EVENT_MOUSE_MASK | DBOX_CONTENT_EVENT_MOUSE_ENTER, /**< Mouse enter to the dynamicbox */
	DBOX_MOUSE_LEAVE		= DBOX_CONTENT_EVENT_DBOX_MASK | DBOX_CONTENT_EVENT_MOUSE_MASK | DBOX_CONTENT_EVENT_MOUSE_LEAVE, /**< Mouse leave from the dynamicbox */
	DBOX_MOUSE_SET			= DBOX_CONTENT_EVENT_DBOX_MASK | DBOX_CONTENT_EVENT_MOUSE_MASK | DBOX_CONTENT_EVENT_MOUSE_SET, /**< Mouse event, start feeding event by master */
	DBOX_MOUSE_UNSET		= DBOX_CONTENT_EVENT_DBOX_MASK | DBOX_CONTENT_EVENT_MOUSE_MASK | DBOX_CONTENT_EVENT_MOUSE_UNSET, /**< Mouse event, stop feeding event by master */

	DBOX_PD_MOUSE_ON_SCROLL		= DBOX_CONTENT_EVENT_PD_MASK | DBOX_CONTENT_EVENT_MOUSE_MASK | DBOX_CONTENT_EVENT_ON_SCROLL, /**< Mouse event occurs while scrolling */
	DBOX_PD_MOUSE_ON_HOLD		= DBOX_CONTENT_EVENT_PD_MASK | DBOX_CONTENT_EVENT_MOUSE_MASK | DBOX_CONTENT_EVENT_ON_HOLD, /**< Mouse event occurs on holding */
	DBOX_PD_MOUSE_OFF_SCROLL	= DBOX_CONTENT_EVENT_PD_MASK | DBOX_CONTENT_EVENT_MOUSE_MASK | DBOX_CONTENT_EVENT_OFF_SCROLL, /**< Scrolling stopped */
	DBOX_PD_MOUSE_OFF_HOLD		= DBOX_CONTENT_EVENT_PD_MASK | DBOX_CONTENT_EVENT_MOUSE_MASK | DBOX_CONTENT_EVENT_OFF_HOLD, /**< Holding stopped */

	DBOX_PD_MOUSE_DOWN			= DBOX_CONTENT_EVENT_PD_MASK | DBOX_CONTENT_EVENT_MOUSE_MASK | DBOX_CONTENT_EVENT_MOUSE_DOWN, /**< Mouse down on the PD */
	DBOX_PD_MOUSE_UP			= DBOX_CONTENT_EVENT_PD_MASK | DBOX_CONTENT_EVENT_MOUSE_MASK | DBOX_CONTENT_EVENT_MOUSE_UP, /**< Mouse up on the PD */
	DBOX_PD_MOUSE_MOVE			= DBOX_CONTENT_EVENT_PD_MASK | DBOX_CONTENT_EVENT_MOUSE_MASK | DBOX_CONTENT_EVENT_MOUSE_MOVE, /**< Mouse move on the PD */
	DBOX_PD_MOUSE_ENTER			= DBOX_CONTENT_EVENT_PD_MASK | DBOX_CONTENT_EVENT_MOUSE_MASK | DBOX_CONTENT_EVENT_MOUSE_ENTER, /**< Mouse enter to the PD */
	DBOX_PD_MOUSE_LEAVE			= DBOX_CONTENT_EVENT_PD_MASK | DBOX_CONTENT_EVENT_MOUSE_MASK | DBOX_CONTENT_EVENT_MOUSE_LEAVE, /**< Mouse leave from the PD */
	DBOX_PD_MOUSE_SET			= DBOX_CONTENT_EVENT_PD_MASK | DBOX_CONTENT_EVENT_MOUSE_MASK | DBOX_CONTENT_EVENT_MOUSE_SET, /**< Mouse event, start feeding event by master */
	DBOX_PD_MOUSE_UNSET			= DBOX_CONTENT_EVENT_PD_MASK | DBOX_CONTENT_EVENT_MOUSE_MASK | DBOX_CONTENT_EVENT_MOUSE_UNSET, /**< Mouse event, stop feeding event by master */

	DBOX_KEY_DOWN			= DBOX_CONTENT_EVENT_DBOX_MASK | DBOX_CONTENT_EVENT_KEY_MASK | DBOX_CONTENT_EVENT_KEY_DOWN, /**< Key down on the dynamicbox */
	DBOX_KEY_UP				= DBOX_CONTENT_EVENT_DBOX_MASK | DBOX_CONTENT_EVENT_KEY_MASK | DBOX_CONTENT_EVENT_KEY_UP, /**< Key up on the dynamicbox */
	DBOX_KEY_SET			= DBOX_CONTENT_EVENT_DBOX_MASK | DBOX_CONTENT_EVENT_KEY_MASK | DBOX_CONTENT_EVENT_KEY_SET, /**< Key event, start feeding event by master */
	DBOX_KEY_UNSET			= DBOX_CONTENT_EVENT_DBOX_MASK | DBOX_CONTENT_EVENT_KEY_MASK | DBOX_CONTENT_EVENT_KEY_UNSET, /**< Key event, stop feeding event by master */
	DBOX_KEY_FOCUS_IN		= DBOX_CONTENT_EVENT_DBOX_MASK | DBOX_CONTENT_EVENT_KEY_MASK | DBOX_CONTENT_EVENT_KEY_FOCUS_IN, /**< Key event, focus in */
	DBOX_KEY_FOCUS_OUT		= DBOX_CONTENT_EVENT_DBOX_MASK | DBOX_CONTENT_EVENT_KEY_MASK | DBOX_CONTENT_EVENT_KEY_FOCUS_OUT, /**< Key event, foucs out */

	DBOX_PD_KEY_DOWN			= DBOX_CONTENT_EVENT_PD_MASK | DBOX_CONTENT_EVENT_KEY_MASK | DBOX_CONTENT_EVENT_KEY_DOWN, /**< Key down on the dynamicbox */
	DBOX_PD_KEY_UP				= DBOX_CONTENT_EVENT_PD_MASK | DBOX_CONTENT_EVENT_KEY_MASK | DBOX_CONTENT_EVENT_KEY_UP, /**< Key up on the dynamicbox */
	DBOX_PD_KEY_SET				= DBOX_CONTENT_EVENT_PD_MASK | DBOX_CONTENT_EVENT_KEY_MASK | DBOX_CONTENT_EVENT_KEY_SET, /**< Key event, start feeding event by master */
	DBOX_PD_KEY_UNSET			= DBOX_CONTENT_EVENT_PD_MASK | DBOX_CONTENT_EVENT_KEY_MASK | DBOX_CONTENT_EVENT_KEY_UNSET, /**< Key event, stop feeding event by master */
	DBOX_PD_KEY_FOCUS_IN		= DBOX_CONTENT_EVENT_PD_MASK | DBOX_CONTENT_EVENT_KEY_MASK | DBOX_CONTENT_EVENT_KEY_FOCUS_IN, /**< Key event, focus in */
	DBOX_PD_KEY_FOCUS_OUT		= DBOX_CONTENT_EVENT_PD_MASK | DBOX_CONTENT_EVENT_KEY_MASK | DBOX_CONTENT_EVENT_KEY_FOCUS_OUT, /**< Key event, focus out */

	DBOX_CONTENT_EVENT_MAX		= 0xFFFFFFFF /**< Unknown event */
};

/**
 * @brief Enumeration for Accessibility event for buffer type Dynamicbox or PD.
 * @details These events are sync'd with Tizen accessibility event set.
 */
enum dynamicbox_access_event_type {
	DBOX_ACCESS_EVENT_PD_MASK		= 0x10000000, /**< PD Accessibility event mask */
	DBOX_ACCESS_EVENT_DBOX_MASK		= 0x20000000, /**< LB Accessibility event mask */

	DBOX_ACCESS_EVENT_HIGHLIGHT		= 0x00000100, /**< LB accessibility: Hightlight a object */
	DBOX_ACCESS_EVENT_HIGHLIGHT_NEXT	= 0x00000200, /**< LB accessibility: Set highlight to next object */
	DBOX_ACCESS_EVENT_HIGHLIGHT_PREV	= 0x00000400, /**< LB accessibility: Set highlight to prev object */
	DBOX_ACCESS_EVENT_UNHIGHLIGHT	= 0x00000800, /**< LB accessibility unhighlight */
	DBOX_ACCESS_EVENT_ACTIVATE		= 0x00001000, /**< LB accessibility activate */
	DBOX_ACCESS_EVENT_ACTION_DOWN	= 0x00010000, /**< LB accessibility value changed */
	DBOX_ACCESS_EVENT_ACTION_UP		= 0x00020000, /**< LB accessibility value changed */
	DBOX_ACCESS_EVENT_SCROLL_DOWN	= 0x00100000, /**< LB accessibility scroll down */
	DBOX_ACCESS_EVENT_SCROLL_MOVE	= 0x00200000, /**< LB accessibility scroll move */
	DBOX_ACCESS_EVENT_SCROLL_UP		= 0x00400000, /**< LB accessibility scroll up */

	DBOX_ACCESS_HIGHLIGHT		= DBOX_ACCESS_EVENT_DBOX_MASK | DBOX_ACCESS_EVENT_HIGHLIGHT,	/**< Access event - Highlight an object in the dynamicbox */
	DBOX_ACCESS_HIGHLIGHT_NEXT	= DBOX_ACCESS_EVENT_DBOX_MASK | DBOX_ACCESS_EVENT_HIGHLIGHT_NEXT,	/**< Access event - Move highlight to the next object in a dynamicbox */
	DBOX_ACCESS_HIGHLIGHT_PREV	= DBOX_ACCESS_EVENT_DBOX_MASK | DBOX_ACCESS_EVENT_HIGHLIGHT_PREV,	/**< Access event - Move highlight to the prev object in a dynamicbox */
	DBOX_ACCESS_UNHIGHLIGHT		= DBOX_ACCESS_EVENT_DBOX_MASK | DBOX_ACCESS_EVENT_UNHIGHLIGHT,	/**< Access event - Delete highlight from the dynamicbox */
	DBOX_ACCESS_ACTIVATE		= DBOX_ACCESS_EVENT_DBOX_MASK | DBOX_ACCESS_EVENT_ACTIVATE,		/**< Access event - Launch or activate the highlighted object */
	DBOX_ACCESS_ACTION_DOWN		= DBOX_ACCESS_EVENT_DBOX_MASK | DBOX_ACCESS_EVENT_ACTION_DOWN,	/**< Access event - down */
	DBOX_ACCESS_ACTION_UP		= DBOX_ACCESS_EVENT_DBOX_MASK | DBOX_ACCESS_EVENT_ACTION_UP,	/**< Access event - up */
	DBOX_ACCESS_SCROLL_DOWN		= DBOX_ACCESS_EVENT_DBOX_MASK | DBOX_ACCESS_EVENT_SCROLL_DOWN,	/**< Access event - scroll down */
	DBOX_ACCESS_SCROLL_MOVE		= DBOX_ACCESS_EVENT_DBOX_MASK | DBOX_ACCESS_EVENT_SCROLL_MOVE,	/**< Access event - scroll move */
	DBOX_ACCESS_SCROLL_UP		= DBOX_ACCESS_EVENT_DBOX_MASK | DBOX_ACCESS_EVENT_SCROLL_UP,	/**< Access event - scroll up */

	DBOX_PD_ACCESS_HIGHLIGHT		= DBOX_ACCESS_EVENT_PD_MASK | DBOX_ACCESS_EVENT_HIGHLIGHT,	/**< Access event - Highlight an object in the PD */
	DBOX_PD_ACCESS_HIGHLIGHT_NEXT	= DBOX_ACCESS_EVENT_PD_MASK | DBOX_ACCESS_EVENT_HIGHLIGHT_NEXT,	/**< Access event - Move highlight to the next object in a PD */
	DBOX_PD_ACCESS_HIGHLIGHT_PREV	= DBOX_ACCESS_EVENT_PD_MASK | DBOX_ACCESS_EVENT_HIGHLIGHT_PREV,	/**< Access event - Move highlight to the prev object in a PD */
	DBOX_PD_ACCESS_UNHIGHLIGHT		= DBOX_ACCESS_EVENT_PD_MASK | DBOX_ACCESS_EVENT_UNHIGHLIGHT,	/**< Access event - Delet highlight from the PD */
	DBOX_PD_ACCESS_ACTIVATE			= DBOX_ACCESS_EVENT_PD_MASK | DBOX_ACCESS_EVENT_ACTIVATE,		/**< Access event - Launch or activate the highlighted object */
	DBOX_PD_ACCESS_ACTION_DOWN		= DBOX_ACCESS_EVENT_PD_MASK | DBOX_ACCESS_EVENT_ACTION_DOWN,	/**< Access event - down */
	DBOX_PD_ACCESS_ACTION_UP		= DBOX_ACCESS_EVENT_PD_MASK | DBOX_ACCESS_EVENT_ACTION_UP,	/**< Access event - up */
	DBOX_PD_ACCESS_SCROLL_DOWN		= DBOX_ACCESS_EVENT_PD_MASK | DBOX_ACCESS_EVENT_SCROLL_DOWN,	/**< Access event - scroll down */
	DBOX_PD_ACCESS_SCROLL_MOVE		= DBOX_ACCESS_EVENT_PD_MASK | DBOX_ACCESS_EVENT_SCROLL_MOVE,	/**< Access event - scroll move */
	DBOX_PD_ACCESS_SCROLL_UP		= DBOX_ACCESS_EVENT_PD_MASK | DBOX_ACCESS_EVENT_SCROLL_UP		/**< Access event - scroll up */
};

/**
 * @brief Enumeration for Dynamicbox LB content type.
 */
enum dynamicbox_dbox_type {
	DBOX_TYPE_IMAGE = 0x01, /**< Contents of a dynamicbox is based on the image file */
	DBOX_TYPE_BUFFER = 0x02, /**< Contents of a dynamicbox is based on canvas buffer(shared) */
	DBOX_TYPE_TEXT = 0x04, /**< Contents of a dynamicbox is based on formatted text file */
	DBOX_TYPE_PIXMAP = 0x08, /**< Contens of a dynamicbox is shared by the pixmap(depends on X) */
	DBOX_TYPE_ELEMENTARY = 0x10, /**< Using Elementary for sharing content & event */
	DBOX_TYPE_INVALID = 0xFF /**< Unknown LB type */
};

/**
 * @brief Enumeration for Dynamicbox PD content type.
 */
enum dynamicbox_pd_type {
	DBOX_PD_TYPE_BUFFER = 0x01, /**< Contents of a PD is based on canvas buffer(shared) */
	DBOX_PD_TYPE_TEXT = 0x02, /**< Contents of a PD is based on formatted text file */
	DBOX_PD_TYPE_PIXMAP = 0x04, /**< Contents of a dynamicbox is shared by the pixmap(depends on X) */
	DBOX_PD_TYPE_ELEMENTARY = 0x08, /**< Using Elementary for sharing content & event */
	DBOX_PD_TYPE_INVALID = 0xFF /**< Unknown PD type */
};

/**
 * @brief Enumeration for Dynamicbox event type.
 * @details These events will be sent from the provider.
 */
enum dynamicbox_event_type { /**< dynamicbox_event_handler_set Event list */
	DBOX_EVENT_DBOX_UPDATED, /**< Contents of the given dynamicbox is updated */
	DBOX_EVENT_PD_UPDATED, /**< Contents of the given pd is updated */

	DBOX_EVENT_CREATED, /**< A new dynamicbox is created */
	DBOX_EVENT_DELETED, /**< A dynamicbox is deleted */

	DBOX_EVENT_GROUP_CHANGED, /**< Group (Cluster/Sub-cluster) information is changed */
	DBOX_EVENT_PINUP_CHANGED, /**< PINUP status is changed */
	DBOX_EVENT_PERIOD_CHANGED, /**< Update period is changed */

	DBOX_EVENT_DBOX_SIZE_CHANGED, /**< Dynamicbox size is changed */
	DBOX_EVENT_PD_SIZE_CHANGED, /**< PD size is changed */

	DBOX_EVENT_PD_CREATED, /**< If a PD is created even if you didn't call the dynamicbox_create_pd API */
	DBOX_EVENT_PD_DESTROYED, /**< If a PD is destroyed even if you didn't call the dynamicbox_destroy_pd API */

	DBOX_EVENT_HOLD_SCROLL, /**< If the screen should be freezed */
	DBOX_EVENT_RELEASE_SCROLL, /**< If the screen can be scrolled */

	DBOX_EVENT_DBOX_UPDATE_BEGIN, /**< Dynamicbox LB content update is started */
	DBOX_EVENT_DBOX_UPDATE_END, /**< Dynamicbox LB content update is finished */

	DBOX_EVENT_PD_UPDATE_BEGIN, /**< Dynamicbox PD content update is started */
	DBOX_EVENT_PD_UPDATE_END, /**< Dynamicbox PD content update is finished */

	DBOX_EVENT_UPDATE_MODE_CHANGED, /**< Dynamicbox Update mode is changed */

	DBOX_EVENT_REQUEST_CLOSE_PD, /**< Dynamicbox requests to close the PD */

	DBOX_EVENT_EXTRA_INFO_UPDATED, /**< Extra information is updated */

	DBOX_EVENT_IGNORED /**< Request is ignored */
};

/**
 * @brief Enumeration for Dynamicbox option types.
 */
enum dynamicbox_option_type {
	DBOX_OPTION_MANUAL_SYNC,			/**< Sync manually */
	DBOX_OPTION_FRAME_DROP_FOR_RESIZE,	/**< Drop frames while resizing */
	DBOX_OPTION_SHARED_CONTENT,		/**< Use only one real instance for multiple fake instances if user creates it using same content */

	DBOX_OPTION_ERROR = 0xFFFFFFFF		/**< To specify the size of this enumeration type */
};

enum dynamicbox_fault_type {
	DBOX_FAULT_DEACTIVATED, /*!< Dynamicbox is deactivated by its fault operation */
	DBOX_FAULT_PROVIDER_DISCONNECTED /*!< Provider is disconnected */
};

/**
 * @brief Enumeration for Dynamicbox visible states.
 * @details Must be sync'd with a provider.
 */
enum dynamicbox_visible_state {
	DBOX_SHOW = 0x00, /**< Dynamicbox is shown. Default state */
	DBOX_HIDE = 0x01, /**< Dynamicbox is hidden, Update timer will not be freezed. but you cannot receive any updates events. */

	DBOX_HIDE_WITH_PAUSE = 0x02, /**< Dynamicbox is hidden, it will pause the update timer, but if a dynamicbox updates its contents, update event will be triggered */

	DBOX_VISIBLE_ERROR = 0xFFFFFFFF /**< To specify the size of this enumeration type */
};

/**
 * @brief Structure for TEXT type dynamicbox contents handling opertators.
 */
struct dynamicbox_script_operators {
	int (*update_begin)(struct dynamicbox *handle); /**< Content parser is started */
	int (*update_end)(struct dynamicbox *handle); /**< Content parser is finished */

	/* Listed functions will be called when parser meets each typed content */
	int (*update_text)(struct dynamicbox *handle, const char *id, const char *part, const char *data); /**< Update text content */
	int (*update_image)(struct dynamicbox *handle, const char *id, const char *part, const char *data, const char *option); /**< Update image content */
	int (*update_script)(struct dynamicbox *handle, const char *id, const char *new_id, const char *part, const char *file, const char *group); /**< Update script content */
	int (*update_signal)(struct dynamicbox *handle, const char *id, const char *emission, const char *signal); /**< Update signal */
	int (*update_drag)(struct dynamicbox *handle, const char *id, const char *part, double dx, double dy); /**< Update drag info */
	int (*update_info_size)(struct dynamicbox *handle, const char *id, int w, int h); /**< Update content size */
	int (*update_info_category)(struct dynamicbox *handle, const char *id, const char *category); /**< Update content category info */
	int (*update_access)(struct dynamicbox *handle, const char *id, const char *part, const char *text, const char *option); /**< Update access information */
	int (*operate_access)(struct dynamicbox *handle, const char *id, const char *part, const char *operation, const char *option); /**< Update access operation */
	int (*update_color)(struct dynamicbox *handle, const char *id, const char *part, const char *data); /**< Update color */
};

/**
 * @brief Called for every async function.
 * @details Prototype of the return callback of every async functions.
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
 * @see dynamicbox_create_pd()
 * @see dynamicbox_create_pd_with_position()
 * @see dynamicbox_destroy_pd()
 * @see dynamicbox_emit_text_signal()
 * @see dynamicbox_acquire_pd_pixmap()
 * @see dynamicbox_acquire_dbox_pixmap()
 * @see dynamicbox_set_update_mode()
 */
typedef void (*dynamicbox_ret_cb_t)(struct dynamicbox *handle, int ret, void *data);

/**
 * @brief Initializes the dynamicbox system.
 * @remarks This API uses get/setenv APIs.
 *   Those APIs are not thread-safe.
 *   So you have to consider to use the dynamicbox_init_with_options instead of this if you are developing multi-threaded viewer.
 * @param[in] disp X Display connection object (If you have X Display connection object already, you can re-use it. But you should care its life cycle.
 *                 It must be alive before calling dynamicbox_fini())
 * @return int
 * @retval #DBOX_STATUS_SUCCESS if success
 * @see dynamicbox_fini()
 * @see dynamicbox_init_with_options()
 */
extern int dynamicbox_init(void *disp);

/**
 * @brief Initializes the dynamicbox system with some options.
 * @details dynamicbox_init function uses environment value to initiate some configurable values.
 *          But some applications do not want to use the env value.
 *          For them, this API will give a chance to set default options using given arguments.
 * @param[in] disp Display (if @a disp is @c NULL, the library will try to acquire a new connection with X)
 * @param[in] prevent_overwrite Overwrite flag (when the content of an image type dynamicbox is updated, it will be overwriten (0) or not (1))
 * @param[in] event_filter If the next event comes in this period, ignore it. It is too fast to processing it in time // need to be elaborated
 * @param[in] use_thread Use the receive thread // need to be elaborated
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int Integer, Dynamicbox status code
 * @retval #DBOX_STATUS_SUCCESS if success
 * @see dynamicbox_init()
 * @see dynamicbox_fini()
 */
extern int dynamicbox_init_with_options(void *disp, int prevent_overwrite, double event_filter, int use_thread);

/**
 * @brief Finalizes the dynamicbox system.
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_SUCCES if success
 * @retval #DBOX_STATUS_ERROR_INVALID if dynamicbox_init is not called
 * @see dynamicbox_init()
 * @see dynamicbox_init_with_options()
 */
extern int dynamicbox_fini(void);

/**
 * @brief Notifies the status of a client ("it is paused") to the provider.
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_SUCCESS if success
 * @retval #DBOX_STATUS_ERROR_FAULT if it failed to send state (paused) info
 * @see dynamicbox_client_resumed()
 */
extern int dynamicbox_client_paused(void);

/**
 * @brief Notifies the status of client ("it is resumed") to the provider.
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_SUCCESS if success
 * @retval #DBOX_STATUS_ERROR_FAULT if it failed to send state (resumed) info
 * @see dynamicbox_client_paused()
 */
extern int dynamicbox_client_resumed(void);

/**
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
 *
 * @remarks
 *    Even if you get a handle from the return value of this function, it is not a created instance.
 *    So you have to consider it as a not initialized handle.
 *    It can be initialized only after getting the return callback with "ret == #DBOX_STATUS_SUCCESS"
 * @param[in] pkgname Dynamicbox Id
 * @param[in] content Contents that will be passed to the dynamicbox instance
 * @param[in] cluster Main group
 * @param[in] category Sub group
 * @param[in] period Update period (@c DEFAULT_PERIOD can be used for this; this argument will be used to specify the period of updating contents of a dynamicbox)
 * @param[in] type Size type (defined from libdynamicbox-service package)
 * @param[in] cb After the request is sent to the master provider, this callback will be called
 * @param[in] data This data will be passed to the callback
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return handle
 * @retval Handle Dynamicbox handle but not yet initialized
 * @retval @c NULL if it fails to create a handle
 * @see dynamicbox_ret_cb_t
 */
extern struct dynamicbox *dynamicbox_add(const char *pkgname, const char *content, const char *cluster, const char *category, double period, int type, dynamicbox_ret_cb_t cb, void *data);

/**
 * @brief Deletes a dynamicbox (will replace dynamicbox_del).
 * @remarks If you call this with an uninitialized handle, the return callback will be called synchronously.
 *    So before returning from this function, the return callback will be called first.
 * @param[in] handler Handler of a dynamicbox instance
 * @param[in] type Deletion type (DBOX_DELETE_PERMANENTLY or DBOX_DELETE_TEMPORARY)
 * @param[in] cb Return callback
 * @param[in] data User data for return callback
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID Invalid argument
 * @retval #DBOX_STATUS_ERROR_BUSY Already in process
 * @retval #DBOX_STATUS_ERROR_FAULT Failed to create a request packet
 * @retval #DBOX_STATUS_SUCCESS Successfully sent, return callack will be called
 * @see dynamicbox_ret_cb_t
 */
extern int dynamicbox_del(struct dynamicbox *handler, int type, dynamicbox_ret_cb_t cb, void *data);

/**
 * @brief Sets a dynamicbox events callback.
 * @details To get the event push from the provider, register the event callback using this function.
 *    The callback will be called if there is any event from the provider.
 * @param[in] cb Event handler
 * @param[in] data User data for the event handler
 * @privlevel N/P
 * @return int
 * @retval #DBOX_STATUS_SUCCESS If succeed to set event handler
 * @retval #DBOX_STATUS_ERROR_INVALID Invalid argument
 * @retval #DBOX_STATUS_ERROR_MEMORY Not enough memory
 * @see dynamicbox_unset_event_handler()
 */
extern int dynamicbox_set_event_handler(int (*cb)(struct dynamicbox *handler, enum dynamicbox_event_type event, void *data), void *data);

/**
 * @brief Unsets the dynamicbox event handler.
 * @param[in] cb Event handler
 * @privlevel N/P
 * @return void * Event handler data
 * @retval pointer Pointer of 'data' which is used with the dynamicbox_set_event_handler
 * @see dynamicbox_set_event_handler()
 */
extern void *dynamicbox_unset_event_handler(int (*cb)(struct dynamicbox *handler, enum dynamicbox_event_type event, void *data));

/**
 * @brief Registers the dynamicbox fault event handler.
 * @details Argument list: event, pkgname, filename, funcname.
 * @param[in] cb Event handler
 * @param[in] data Event handler data
 * @privlevel N/P
 * @return int
 * @retval #DBOX_STATUS_SUCCESS If succeed to set fault event handler
 * @retval #DBOX_STATUS_ERROR_INVALID Invalid argument
 * @retval #DBOX_STATUS_ERROR_MEMORY Not enough memory
 * @see dynamicbox_unset_fault_handler()
 */
extern int dynamicbox_set_fault_handler(int (*cb)(enum dynamicbox_fault_type, const char *, const char *, const char *, void *), void *data);

/**
 * @brief Unsets the dynamicbox fault event handler.
 * @param[in] cb Event handler
 * @privlevel N/P
 * @return void * Callback data which is set via dynamicbox_set_fault_handler
 * @retval pointer Pointer of 'data' which is used with the dynamicbox_set_fault_handler
 * @see dynamicbox_set_fault_handler()
 */
extern void *dynamicbox_unset_fault_handler(int (*cb)(enum dynamicbox_fault_type, const char *, const char *, const char *, void *));

/**
 * @brief Activates the faulted dynamicbox.
 * @details Request result will be returned via return callback.
 * @remarks Even though this function returns SUCCESS, it means that it just successfully sent a request to the provider.
 *    So you have to check the return callback and its "ret" argument.
 * @param[in] pkgname Package name which should be activated
 * @param[in] cb Result callback
 * @param[in] data Callback data
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int type
 * @retval #DBOX_STATUS_SUCCESS Successfully sent a request
 * @retval #DBOX_STATUS_ERROR_INVALID Invalid argument
 * @retval #DBOX_STATUS_ERROR_FAULT Failed to make a request
 * @see dynamicbox_ret_cb_t
 */
extern int dynamicbox_activate(const char *pkgname, dynamicbox_ret_cb_t cb, void *data);

/**
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
 * @remarks You have to check the return callback.
 * @param[in] handler Handler of a dynamicbox instance
 * @param[in] type Type of a dynamicbox size (e.g., DBOX_SIZE_TYPE_1x1, ...)
 * @param[in] cb Result callback of the resize operation
 * @param[in] data User data for return callback
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int type
 * @retval #DBOX_STATUS_ERROR_INVALID Invalid argument
 * @retval #DBOX_STATUS_ERROR_BUSY Previous request of resize is in progress
 * @retval #DBOX_STATUS_ERROR_ALREADY Already resized, there is no differences between current size and requested size
 * @retval #DBOX_STATUS_ERROR_PERMISSION Permission denied, you only have view the content of this box
 * @retval #DBOX_STATUS_ERROR_FAULT Failed to make a request
 * @see dynamicbox_ret_cb_t
 */
extern int dynamicbox_resize(struct dynamicbox *handler, int type, dynamicbox_ret_cb_t cb, void *data);

/**
 * @brief Sends the click event for a dynamicbox.
 * @param[in] handler Handler of a dynamicbox instance
 * @param[in] x Rational X of the content width
 * @param[in] y Rational Y of the content height
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID Invalid argument
 * @retval #DBOX_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #DBOX_STATUS_SUCCESS Successfully done
 */
extern int dynamicbox_click(struct dynamicbox *handler, double x, double y);

/**
 * @brief Changes the cluster/sub-cluster name of the given dynamicbox handler.
 * @param[in] handler Handler of a dynamicbox instance
 * @param[in] cluster New cluster of a dynamicbox
 * @param[in] category New category of a dynamicbox
 * @param[in] cb Result callback for changing the cluster/category of a dynamicbox
 * @param[in] data User data for the result callback
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_SUCCESS Request is successfully sent. the return callback will be called
 * @retval #DBOX_STATUS_ERROR_BUSY Previous request is not finished yet
 * @retval #DBOX_STATUS_ERROR_ALREADY Group name is same with current one
 * @retval #DBOX_STATUS_ERROR_PERMISSION You have no permission to change property of this dynamicbox instance
 * @retval #DBOX_STATUS_ERROR_FAULT Failed to make a request
 * @see dynamicbox_ret_cb_t
 */
extern int dynamicbox_set_group(struct dynamicbox *handler, const char *cluster, const char *category, dynamicbox_ret_cb_t cb, void *data);

/**
 * @brief Gets the cluster and category (sub-cluster) name of the given dynamicbox (it is not I18N format, only English).
 * @remarks You have to do not release the cluster & category.
 *    It is allocated inside of a given dynamicbox instance, so you can only read it.
 * @param[in] handler Handler of a dynamicbox instance
 * @param[out] cluster Storage(memory) for containing the cluster name
 * @param[out] category Storage(memory) for containing the category name
 * @privlevel N/P
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID Invalid argument
 * @retval #DBOX_STATUS_SUCCESS Successfully done
 */
extern int dynamicbox_get_group(struct dynamicbox *handler, const char **cluster, const char **category);

/**
 * @brief Gets the period of the dynamicbox handler.
 * @remarks If this function returns 0.0f, it means the dynamicbox has no update period or the handle is not valid.
 *    This function only works after the return callback of dynamicbox_create fucntion is called.
 * @param[in] handler Handler of a dynamicbox instance
 * @privlevel N/P
 * @return double
 * @retval Current update period of a dynamicbox
 * @retval 0.0f This means the box has no update period or the handles is not valid
 */
extern double dynamicbox_period(struct dynamicbox *handler);

/**
 * @brief Changes the update period.
 * @param[in] handler Handler of a dynamicbox instance
 * @param[in] period New update period of a dynamicbox
 * @param[in] cb Result callback of changing the update period of this dynamicbox
 * @param[in] data User data for the result callback
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_SUCCESS Successfully done
 * @retval #DBOX_STATUS_ERROR_INVALID Invalid argument
 * @retval #DBOX_STATUS_ERROR_BUSY
 * @retval #DBOX_STATUS_ERROR_ALREADY
 * @retval #DBOX_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @see dynamicbox_ret_cb_t
 */
extern int dynamicbox_set_period(struct dynamicbox *handler, double period, dynamicbox_ret_cb_t cb, void *data);

/**
 * @brief Checks whether the given dynamicbox is a text type or not.
 * @param[in] handler Handler of a dynamicbox instance
 * @privlevel N/P
 * @return dynamicbox_dbox_type
 * @retval #DBOX_TYPE_IMAGE Contents of a dynamicbox is based on the image file
 * @retval #DBOX_TYPE_BUFFER Contents of a dynamicbox is based on canvas buffer(shared)
 * @retval #DBOX_TYPE_TEXT Contents of a dynamicbox is based on formatted text file
 * @retval #DBOX_TYPE_PIXMAP Contens of a dynamicbox is shared by the pixmap(depends on X)
 * @retval #DBOX_TYPE_INVALID
 * @see dynamicbox_dbox_type()
 */
extern enum dynamicbox_dbox_type dynamicbox_dbox_type(struct dynamicbox *handler);

/**
 * @brief Checks if the given dynamicbox is created by user or not.
 * @details If the dynamicbox instance is created by a system this will return 0.
 * @param[in] handler Handler of a dynamicbox instance
 * @privlevel N/P
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID Invalid argument
 * @retval 0 Automatically created dynamicbox by the provider
 * @retval 1 Created by user via dynamicbox_add()
 * @see dynamicbox_add()
 * @see dynamicbox_set_event_handler()
 */
extern int dynamicbox_is_user(struct dynamicbox *handler);

/**
 * @brief Gets content information string of the given dynamicbox.
 * @param[in] handler Handler of a dynamicbox instance
 * @privlevel N/P
 * @return const char *
 * @retval content_info Dynamicbox content info that can be used again via content_info argument of dynamicbox_add()
 * @see dynamicbox_add()
 */
extern const char *dynamicbox_content(struct dynamicbox *handler);

/**
 * @brief Gets the sub cluster title string of the given dynamicbox.
 * @details This API is now used for accessibility.
 *  Each box should set their content as a string to be read by TTS.
 *  So if the box has focused on the homescreen, the homescreen will read text using this API.
 * @remarks The title returned by this API is read by TTS.
 *  But it is just recomended to a homescreen.
 *  So, to read it or not depends on implementation of the homescreen.
 * @param[in] handler Handler of a dynamicbox instance
 * @privlevel N/P
 * @return const char *
 * @retval sub Cluster name
 * @retval @c NULL
 */
extern const char *dynamicbox_category_title(struct dynamicbox *handler);

/**
 * @brief Gets the filename of the given dynamicbox, if it is an IMAGE type dynamicbox.
 * @details If the box is developed as image format to represent its contents, the homescreen should know its image file name.
 * @param[in] handler Handler of a dynamicbox instance
 * @privlevel N/P
 * @return const char *
 * @retval filename If the dynamicbox type is image this function will give you a abs-path of an image file (content is rendered)
 * @retval @c NULL If this has no image file or type is not image file.
 */
extern const char *dynamicbox_filename(struct dynamicbox *handler);

/**
 * @brief Gets the package name of the given dynamicbox handler.
 * @param[in] handler Handler of a dynamicbox instance
 * @privlevel N/P
 * @return const char *
 * @retval pkgname Package name
 * @retval @c NULL If the handler is not valid
 */
extern const char *dynamicbox_pkgname(struct dynamicbox *handler);

/**
 * @brief Gets the priority of a current content.
 * @param[in] handler Handler of a dynamicbox instance
 * @privlevel N/P
 * @return double
 * @retval 0.0f Handler is @c NULL
 * @retval -1.0f Handler is not valid (not yet initialized)
 * @retval real Number between 0.0 and 1.0
 */
extern double dynamicbox_priority(struct dynamicbox *handler);

/**
 * @brief Acquires the buffer of a given dynamicbox (only for the buffer type).
 * @param[in] handler Handler of a dynamicbox instance
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return void *
 * @retval address Address of a FB
 * @retval @c NULL If it fails to get fb address
 */
extern void *dynamicbox_acquire_fb(struct dynamicbox *handler);

/**
 * @brief Releases the buffer of a dynamicbox (only for the buffer type).
 * @param[in] buffer Buffer
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID Invalid argument
 * @retval #DBOX_STATUS_SUCCESS Successfully done
 * @see dynamicbox_acquire_fb()
 */
extern int dynamicbox_release_fb(void *buffer);

/**
 * @brief Gets the reference count of Dynamicbox buffer (only for the buffer type).
 * @param[in] buffer Buffer
 * @privlevel N/P
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID Invalid argument
 * @retval #DBOX_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval refcnt Positive integer including ZERO
 * @see dynamicbox_pdfb_refcnt()
 */
extern int dynamicbox_fb_refcnt(void *buffer);

/**
 * @brief Acquires the buffer of a PD frame (only for the buffer type).
 * @param[in] handler Handler of a dynamicbox instance
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval @c NULL
 * @retval adress Address of a buffer of PD
 * @see dynamicbox_release_pdfb()
 */
extern void *dynamicbox_acquire_pdfb(struct dynamicbox *handler);

/**
 * @brief Releases the acquired buffer of the PD Frame (only for the buffer type).
 * @param[in] buffer Buffer
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID Invalid argument
 * @retval #DBOX_STATUS_SUCCESS Successfully done
 * @see dynamicbox_acquire_pdfb()
 */
extern int dynamicbox_release_pdfb(void *buffer);

/**
 * @brief Gets the reference count of a given PD buffer (only for the buffer type).
 * @param[in] buffer Buffer
 * @privlevel N/P
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID Invalid argument
 * @retval #DBOX_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval reference Reference count
 * @see dynamicbox_fb_refcnt()
 */
extern int dynamicbox_pdfb_refcnt(void *buffer);

/**
 * @brief Gets the size of the Dynamicbox.
 * @param[in] handler Handler of a dynamicbox instance
 * @privlevel N/P
 * @return int
 * @retval #DBOX_SIZE_TYPE_NxM
 * @retval #DBOX_SIZE_TYPE_INVALID
 */
extern int dynamicbox_size(struct dynamicbox *handler);

/**
 * @brief Gets the size of the Progressive Disclosure.
 * @param[in] handler Handler of a dynamicbox instance
 * @param[out] w
 * @param[out] h
 * @privlevel N/P
 * @return int type
 * @retval #DBOX_STATUS_ERROR_INVALID Invalid argument
 * @retval #DBOX_STATUS_SUCCESS Successfully done
 */
extern int dynamicbox_get_pdsize(struct dynamicbox *handler, int *w, int *h);

/**
 * @brief Gets a list of the supported sizes of a given handler.
 * @param[in] handler Handler of a dynamicbox instance
 * @param[out] cnt
 * @param[out] size_list
 * @privlevel N/P
 * @return int type
 * @retval #DBOX_STATUS_ERROR_INVALID Invalid argument
 * @retval #DBOX_STATUS_SUCCESS Successfully done
 */
extern int dynamicbox_get_supported_sizes(struct dynamicbox *handler, int *cnt, int *size_list);

/**
 * @brief Gets BUFFER SIZE of the dynamicbox if it is a buffer type.
 * @param[in] handler Handler of a dynamicbox instance
 * @privlevel N/P
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID Invalid argument
 * @retval size Size of dynamicbox buffer
 */
extern int dynamicbox_lbfb_bufsz(struct dynamicbox *handler);

/**
 * @brief Gets BUFFER SIZE of the progiressive disclosure if it is a buffer type.
 * @param[in] handler Handler of a dynamicbox instance
 * @privlevel N/P
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID Invalid argument
 * @retval size Size of PD buffer
 */
extern int dynamicbox_pdfb_bufsz(struct dynamicbox *handler);

/**
 * @brief Sends a content event (for buffer type) to the provider (dynamicbox).
 * @param[in] handler Handler of a dynamicbox instance
 * @param[in] type Event type
 * @param[in] x Coordinates of X axis
 * @param[in] y Coordinates of Y axis
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID Invalid argument
 * @retval #DBOX_STATUS_ERROR_BUSY Previous operation is not finished yet
 * @retval #DBOX_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #DBOX_STATUS_SUCCESS Successfully sent
 * @see dynamicbox_access_event()
 * @see dynamicbox_key_event()
 */
extern int dynamicbox_mouse_event(struct dynamicbox *handler, enum dynamicbox_content_event_type type, double x, double y);

/**
 * @brief Sends an access event (for buffer type) to the provider (dynamicbox).
 * @param[in] handler Handler of a dynamicbox instance
 * @param[in] type Event type
 * @param[in] x Coordinates of X axsis
 * @param[in] y Coordinates of Y axsis
 * @param[in] cb Result callback function
 * @param[in] data Callback data
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID Invalid argument
 * @retval #DBOX_STATUS_ERROR_BUSY Previous operation is not finished yet
 * @retval #DBOX_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #DBOX_STATUS_SUCCESS Successfully sent
 * @see dynamicbox_mouse_event()
 * @see dynamicbox_key_event()
 */
extern int dynamicbox_access_event(struct dynamicbox *handler, enum dynamicbox_access_event_type type, double x, double y, dynamicbox_ret_cb_t cb, void *data);

/**
 * @brief Sends a key event (for buffer type) to the provider (dynamicbox).
 * @param[in] handler Handler of a dynamicbox instance
 * @param[in] type Key event type
 * @param[in] keycode Code of key
 * @param[in] cb Result callback
 * @param[in] data Callback data
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID Invalid argument
 * @retval #DBOX_STATUS_ERROR_BUSY Previous operation is not finished yet
 * @retval #DBOX_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #DBOX_STATUS_SUCCESS Successfully sent
 * @see dynamicbox_mouse_event()
 * @see dynamicbox_access_event()
 */
extern int dynamicbox_key_event(struct dynamicbox *handler, enum dynamicbox_content_event_type type, unsigned int keycode, dynamicbox_ret_cb_t cb, void *data);

/**
 * @brief Sets pin-up status of the given handler.
 * @details If the dynamicbox supports the pinup feature,
 *   you can freeze the update of the given dynamicbox.
 *   But it is different from pause.
 *   The box will be updated and it will decide wheter update its content or not when the pinup is on.
 * @param[in] handler Handler of a dynamicbox instance
 * @param[in] flag Pinup value
 * @param[in] cb Result callback
 * @param[in] data Callback data
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID Invalid parameters
 * @see dynamicbox_ret_cb_t
 * @see dynamicbox_set_visibility()
 * @see dynamicbox_is_pinned_up()
 */
extern int dynamicbox_set_pinup(struct dynamicbox *handler, int flag, dynamicbox_ret_cb_t cb, void *data);

/**
 * @brief Checks the PIN-UP status of the given handler.
 * @param[in] handler Handler of a dynamicbox instance
 * @privlevel N/P
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID Invalid parameters
 * @retval 1 Box is pinned up
 * @retval 0 Box is not pinned up
 * @see dynamicbox_set_pinup()
 */
extern int dynamicbox_is_pinned_up(struct dynamicbox *handler);

/**
 * @brief Checks the availability of the PIN-UP feature for the given handler.
 * @param[in] handler Handler of a dynamicbox instance
 * @privlevel N/P
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID Invalid argument
 * @retval 1 If the box support Pinup feature
 * @retval 0 If the box does not support the Pinup feature
 * @see dynamicbox_is_pinned_up()
 * @see dynamicbox_set_pinup()
 */
extern int dynamicbox_has_pinup(struct dynamicbox *handler);

/**
 * @brief Checks the existence of PD for the given handler.
 * @param[in] handler Handler of a dynamicbox instance
 * @privlevel N/P
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID Invalid argument
 * @retval 1 If the box support the PD
 * @retval 0 If the box has no PD
 */
extern int dynamicbox_has_pd(struct dynamicbox *handler);

/**
 * @brief Creates PD of the given handler.
 * @param[in] handler Handler of a dynamicbox instance
 * @param[in] cb Result callback
 * @param[in] data Callback data
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_SUCCESS Successfully done
 * @retval #DBOX_STATUS_ERROR_INVALID Invalid argument
 * @retval #DBOX_STATUS_ERROR_BUSY Previous operation is not finished yet
 * @retval #DBOX_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @see dynamicbox_ret_cb_t
 * @see dynamicbox_create_pd_with_position()
 * @see dynamicbox_move_pd()
 * @see dynamicbox_destroy_pd()
 */
extern int dynamicbox_create_pd(struct dynamicbox *handler, dynamicbox_ret_cb_t cb, void *data);

/**
 * @brief Creates PD of the given handler with the relative position from dynamicbox.
 * @param[in] handler Handler of a dynamicbox instance
 * @param[in] x 0.0 ~ 1.0
 * @param[in] y 0.0 ~ 1.0
 * @param[in] cb Result callback
 * @param[in] data Callback data
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_SUCCESS Successfully done
 * @retval #DBOX_STATUS_ERROR_INVALID Invalid argument
 * @retval #DBOX_STATUS_ERROR_BUSY Previous operation is not finished yet
 * @retval #DBOX_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @see dynamicbox_create_pd()
 * @see dynamicbox_destroy_pd()
 * @see dynamicbox_move_pd()
 */
extern int dynamicbox_create_pd_with_position(struct dynamicbox *handler, double x, double y, dynamicbox_ret_cb_t cb, void *data);

/**
 * @brief Updates a position of the given PD.
 * @param[in] handler Handler of a dynamicbox instance
 * @param[in] x 0.0 ~ 1.0
 * @param[in] y 0.0 ~ 1.0
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_SUCCESS If sending a request for updating position of the PD has been done successfully
 * @retval #DBOX_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #DBOX_STATUS_ERROR_INVALID Invalid argument
 */
extern int dynamicbox_move_pd(struct dynamicbox *handler, double x, double y);

/**
 * @brief Destroys the PD of the given handler if it is created.
 * @param[in] handler Handler of a dynamicbox instance
 * @param[in] cb
 * @param[in] data
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID Invalid argument
 * @retval #DBOX_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #DBOX_STATUS_SUCCESS Successfully done
 * @see dynamicbox_ret_cb_t
 */
extern int dynamicbox_destroy_pd(struct dynamicbox *handler, dynamicbox_ret_cb_t cb, void *data);

/**
 * @brief Checks the create status of the given dynamicbox handler.
 * @param[in] handler Handler of a dynamicbox instance
 * @privlevel N/P
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID Invalid argument
 * @retval 0 PD is not created
 * @retval 1 PD is created
 */
extern int dynamicbox_pd_is_created(struct dynamicbox *handler);

/**
 * @brief Checks the content type of a progressive disclosure of the given handler.
 * @param[in] handler Handler of a dynamicbox instance
 * @privlevel N/P
 * @return int
 * @retval #PD_TYPE_BUFFER Contents of a PD is based on canvas buffer(shared)
 * @retval #PD_TYPE_TEXT Contents of a PD is based on formatted text file
 * @retval #PD_TYPE_PIXMAP Contents of a dynamicbox is shared by the pixmap(depends on X)
 * @retval #PD_TYPE_INVALID
 * @see dynamicbox_pd_type()
 */
extern enum dynamicbox_pd_type dynamicbox_pd_type(struct dynamicbox *handler);

/**
 * @brief Checks the existence of a dynamicbox about the given package name.
 * @param[in] pkgname Package name
 * @privlevel platform
 * @privileve %http://developer.samsung.com/privilege/core/dynamicbox.info
 * @return int
 * @retval 1 If the box exists
 * @retval 0 If the box does not exist
 */
extern int dynamicbox_is_exists(const char *pkgname);

/**
 * @brief Sets a function table for parsing the text content of a dynamicbox.
 * @param[in] handler Handler of a dynamicbox instance
 * @param[in] ops
 * @privlevel N/P
 * @return int
 * @retval #DBOX_STATUS_SUCCESS Successfully done
 * @retval #DBOX_STATUS_ERROR_INVALID Invalid argument
 * @see dynamicbox_set_pd_text_handler()
 */
extern int dynamicbox_set_text_handler(struct dynamicbox *handler, struct dynamicbox_script_operators *ops);

/**
 * @brief Sets a function table for parsing the text content of a Progressive Disclosure.
 * @param[in] handler Handler of a dynamicbox instance
 * @param[in] ops
 * @privlevel N/P
 * @return int
 * @retval #DBOX_STATUS_SUCCESS Successfully done
 * @retval #DBOX_STATUS_ERROR_INVALID Invalid argument
 * @see dynamicbox_set_text_handler()
 */
extern int dynamicbox_set_pd_text_handler(struct dynamicbox *handler, struct dynamicbox_script_operators *ops);

/**
 * @brief Emits a text signal to the given dynamicbox only if it is a text type.
 * @param[in] handler Handler of a dynamicbox instance
 * @param[in] emission Emission string
 * @param[in] source Source string
 * @param[in] sx Start X
 * @param[in] sy Start Y
 * @param[in] ex End X
 * @param[in] ey End Y
 * @param[in] cb Result callback
 * @param[in] data Callback data
 * @privlevel platform
 * @privileve %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID Invalid parameters
 * @retval #DBOX_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #DBOX_STATUS_SUCCESS Successfully emitted
 * @see dynamicbox_ret_cb_t
 */
extern int dynamicbox_emit_text_signal(struct dynamicbox *handler, const char *emission, const char *source, double sx, double sy, double ex, double ey, dynamicbox_ret_cb_t cb, void *data);

/**
 * @brief Sets a private data pointer to carry it using the given handler.
 * @param[in] handler Handler of a dynamicbox instance
 * @param[in] data Data pointer
 * @privlevel N/P
 * @return int
 * @retval #DBOX_STATUS_SUCCESS Successfully registered
 * @retval #DBOX_STATUS_ERROR_INVALID Invalid argument
 * @see dynamicbox_get_data()
 */
extern int dynamicbox_set_data(struct dynamicbox *handler, void *data);

/**
 * @brief Gets a private data pointer which is carried by a given handler.
 * @param[in] handler Handler of a dynamicbox instance
 * @privlevel N/P
 * @return void *
 * @retval data Data pointer
 * @retval @c NULL If there is no data
 * @see dynamicbox_set_data()
 */
extern void *dynamicbox_get_data(struct dynamicbox *handler);

/**
 * @brief Subscribes an event for dynamicboxes only in a given cluster and sub-cluster.
 * @details If you wrote a view-only client,
 *   you can receive the event of specific dynamicboxes which belong to a given cluster/category.
 *   But you cannot modify their attributes (such as size, ...).
 * @param[in] cluster   Cluster ("*" can be used for subscribe all cluster's dynamicboxes event; If you use the "*", value in the category will be ignored)
 * @param[in] category Category ("*" can be used for subscribe dynamicboxes events of all category(sub-cluster) in a given "cluster")
 * @privlevel platform
 * @privileve %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #DBOX_STATUS_SUCCESS Successfully requested
 * @see dynamicbox_unsubscribe_group()
 */
extern int dynamicbox_subscribe_group(const char *cluster, const char *category);

/**
 * @brief Unsubscribes an event for the dynamicboxes, but you will receive already added dynamicboxes events.
 * @param[in] cluster Cluster("*" can be used for subscribe all cluster's dynamicboxes event; If you use the "*", value in the category will be ignored)
 * @param[in] category Category ("*" can be used for subscribe all sub-cluster's dynamicboxes event in a given "cluster")
 * @privlevel platform
 * @privileve %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #DBOX_STATUS_SUCCESS Successfully requested
 * @see dynamicbox_subscribe_group()
 */
extern int dynamicbox_unsubscribe_group(const char *cluster, const char *category);

/**
 * @brief Refreshes the group (cluster/sub-cluser (aka. category)).
 * @details This function will trigger the update of all dynamicboxes in a given cluster/category group.
 * @remarks Basically, a default dynamicbox system doesn't use the cluster/category concept.
 *    But you can use it. So if you decide to use it, then you can trigger the update of all dynamicboxes in the given group.
 * @param[in] cluster Cluster ID
 * @param[in] category Sub-cluster ID
 * @param[in] force 1 if the boxes should be updated even if they are paused
 * @privlevel platform
 * @privileve %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID Invalid argument
 * @retval #DBOX_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #DBOX_STATUS_SUCCESS Successfully requested
 * @see dynamicbox_refresh()
 */
extern int dynamicbox_refresh_group(const char *cluster, const char *category, int force);

/**
 * @brief Refreshes a dynamicbox.
 * @param[in] handler Handler of a dynamicbox instance
 * @param[in] force 1 if the box should be updated even if it is paused
 * @privlevel platform
 * @privileve %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID Invalid argument
 * @retval #DBOX_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #DBOX_STATUS_SUCCESS Successfully requested
 * @see dynamicbox_refresh_group()
 */
extern int dynamicbox_refresh(struct dynamicbox *handler, int force);

/**
 * @brief Gets Pixmap Id of a dynamicbox content.
 * @details This function doesn't guarantee the life-cycle of the pixmap.
 *   If the service provider destroyed the pixmap, you will not know about it.
 *   So you should validate it before accessing it.
 * @param[in] handler Handler of a dynamicbox instance
 * @privlevel N/P
 * @return int
 * @retval 0 If the pixmap is not created
 * @retval pixmap Pixmap Id need to be casted to (unsigned int) type
 * @see dynamicbox_pd_pixmap()
 */
extern int dynamicbox_dbox_pixmap(const struct dynamicbox *handler);

/**
 * @brief Gets Pixmap Id of a PD content.
 * @details This function doesn't guarantee the life-cycle of the pixmap.
 *   If the service provider destroyed the pixmap, you will not know about it.
 *   So you should validate it before accessing it.
 * @param[in] handler Handler of a dynamicbox instance
 * @privlevel N/P
 * @return int
 * @retval 0 If the pixmap is not created
 * @retval pixmap Pixmap Id need to be casted to (unsigned int) type
 * @see dynamicbox_dbox_pixmap()
 */
extern int dynamicbox_pd_pixmap(const struct dynamicbox *handler);

/**
 * @brief Acquires the pixmap of PD.
 * @details After acquiring the pixmap of PD, it will not be destroyed.
 *   So if the new update is comming with a new pixmap Id, you should release old pixmap manually.
 * @param[in] handler Handler of a dynamicbox instance
 * @param[in] cb Result callback for acquiring request
 * @param[in] data Callback Data
 * @privlevel platform
 * @privileve %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID Invalid argument
 * @retval #DBOX_STATUS_ERROR_FAULT Failed to send a request to the service provider or there is critical error that is unrecoverable
 * @retval #DBOX_STATUS_SUCCESS Successfully requested to acquire the pixmap of PD
 * @see dynamicbox_release_pd_pixmap()
 * @see dynamicbox_acquire_dbox_pixmap()
 * @see dynamicbox_ret_cb_t
 */
extern int dynamicbox_acquire_pd_pixmap(struct dynamicbox *handler, dynamicbox_ret_cb_t cb, void *data);

/**
 * @brief Releases the acquired pixmap ID.
 * @param[in] handler Handler of a dynamicbox instance
 * @param[in] pixmap Pixmap Id to release it
 * @privlevel platform
 * @privileve %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID Invalid argument
 * @retval #DBOX_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #DBOX_STATUS_SUCCESS Successfully released (request is sent)
 * @see dynamicbox_acquire_pd_pixmap()
 * @see dynamicbox_release_dbox_pixmap()
 */
extern int dynamicbox_release_pd_pixmap(struct dynamicbox *handler, int pixmap);

/**
 * @brief Gets the PIXMAP of a dynamicbox.
 * @details Even if a render process releases the pixmap, the pixmap will be kept before being released by dynamicbox_release_dbox_pixmap.
 *   You should release the pixmap manually.
 * @param[in] handler Handler of a dynamicbox instance
 * @param[in] cb Callback function which will be called with result of acquiring lb pixmap
 * @param[in] data Callback data
 * @privlevel platform
 * @privileve %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID Invalid argument
 * @retval #DBOX_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #DBOX_STATUS_SUCCESS Successfully requested
 * @pre Dynamicbox service system should support the PIXMAP type buffer.
 *   The dynamicbox should be designed to use the buffer (script type).
 * @see dynamicbox_release_dbox_pixmap()
 * @see dynamicbox_acquire_pd_pixmap()
 * @see dynamicbox_ret_cb_t
 */
extern int dynamicbox_acquire_dbox_pixmap(struct dynamicbox *handler, dynamicbox_ret_cb_t cb, void *data);

/**
 * @brief Releases the pixmap of a dynamicbox.
 * @details After a client gets a new pixmap or does not need to keep the current pixmap anymore, use this function to release it.
 *   If there is no user for a given pixmap, the pixmap will be destroyed.
 * @param[in] handler Handler of a dynamicbox instance
 * @param[in] pixmap Pixmap Id of given dynamicbox handler
 * @privlevel platform
 * @privileve %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID Invalid argument
 * @retval #DBOX_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #DBOX_STATUS_SUCCESS Successfully done
 * @pre The pixmap should be acquired by dynamicbox_acquire_dbox_pixmap
 * @see dynamicbox_acquire_dbox_pixmap()
 * @see dynamicbox_release_pd_pixmap()
 */
extern int dynamicbox_release_dbox_pixmap(struct dynamicbox *handler, int pixmap);

/**
 * @brief Updates a visible state of the dynamicbox.
 * @param[in] handler Handler of a dynamicbox instance
 * @param[in] state Configure the current visible state of a dynamicbox
 * @privlevel platform
 * @privileve %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID Invalid argument
 * @retval #DBOX_STATUS_ERROR_BUSY
 * @retval #DBOX_STATUS_ERROR_PERMISSION
 * @retval #DBOX_STATUS_ERROR_ALREADY
 * @retval #DBOX_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #DBOX_STATUS_SUCCESS Successfully done
 */
extern int dynamicbox_set_visibility(struct dynamicbox *handler, enum dynamicbox_visible_state state);

/**
 * @brief Gets the current visible state of a dynamicbox.
 * @param[in] handler Handler of a dynamicbox instance
 * @privlevel N/P
 * @return dynamicbox_visible_state
 * @retval #DBOX_SHOW Dynamicbox is shown (Default state)
 * @retval #DBOX_HIDE Dynamicbox is hidden, Update timer is not frozen (but a user cannot receive any updated events; a user should refresh(reload) the content of a dynamicbox when a user make this show again)
 * @retval #DBOX_HIDE_WITH_PAUSE Dynamicbox is hidden, it will pause the update timer, but if a dynamicbox updates its contents, update event will occur
 * @retval #DBOX_VISIBLE_ERROR To enlarge the size of this enumeration type
 */
extern enum dynamicbox_visible_state dynamicbox_visibility(struct dynamicbox *handler);

/**
 * @brief Sets an update mode of the current dynamicbox.
 * @details If you set 1 for active update mode, you should get a buffer without updated event from provider.
 *   But if it is passive mode, you have to update content of a box when you get updated events.
 *   Default is Passive mode.
 * @param[in] handler Handler of a dynamicbox instance
 * @param[in] active_update 1 means active update, 0 means passive update (default)
 * @param[in] cb Result callback function
 * @param[in] data Callback data
 * @privlevel platform
 * @privileve %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID Invalid argument
 * @retval #DBOX_STATUS_ERROR_BUSY
 * @retval #DBOX_STATUS_ERROR_PERMISSION
 * @retval #DBOX_STATUS_ERROR_ALREADY
 * @retval #DBOX_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #DBOX_STATUS_SUCCESS Successfully done
 * @see dynamicbox_ret_cb_t
 */
extern int dynamicbox_set_update_mode(struct dynamicbox *handler, int active_update, dynamicbox_ret_cb_t cb, void *data);

/**
 * @brief Checks the active update mode of the given dynamicbox.
 * @param[in] handler Handler of a dynamicbox instance
 * @privlevel N/P
 * @return int
 * @retval 0 If passive mode
 * @retval 1 If active mode or error code
 */
extern int dynamicbox_is_active_update(struct dynamicbox *handler);

/**
 * @brief Syncs manually
 * param[in] handler Handler of a dynamicbox instance
 * @privlevel platform
 * @privileve %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return void
 * @retval #DBOX_STATUS_SUCCESS If success
 * @retval #DBOX_STATUS_ERROR_INVALID Invalid handle
 * @see dynamicbox_set_manual_sync()
 * @see dynamicbox_manual_sync()
 * @see dynamicbox_sync_dbox_fb()
 */
extern int dynamicbox_sync_pd_fb(struct dynamicbox *handler);

/**
 * @brief Syncs manually
 * @param[in] handler Handler of a dynamicbox instance
 * @privlevel platform
 * @privileve %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return void
 * @retval #DBOX_STATUS_SUCCESS If success
 * @retval #DBOX_STATUS_ERROR_INVALID Invalid handle
 * @see dynamicbox_set_manual_sync()
 * @see dynamicbox_manual_sync()
 * @see dynamicbox_sync_pd_fb()
 */
extern int dynamicbox_sync_dbox_fb(struct dynamicbox *handler);

/**
 * @brief Gets an alternative icon of the given dynamicbox instance.
 * @details If the box should be represented as a shortcut icon, this function will get the alternative icon.
 * @param[in] handler Handler of a dynamicbox instance
 * @privlevel N/P
 * @return const char *
 * @retval address Absolute path of an alternative icon file
 * @retval @c NULL Dynamicbox has no alternative icon file
 * @see dynamicbox_alt_name()
 */
extern const char *dynamicbox_alt_icon(struct dynamicbox *handler);

/**
 * @brief Gets an alternative name of the given dynamicbox instance.
 * @details If the box should be represented as a shortcut name, this function will get the alternative name.
 * @param[in] handler Handler of a dynamicbox instance
 * @privlevel N/P
 * @return const char *
 * @retval name Alternative name of a dynamicbox
 * @retval @c NULL Dynamicbox has no alternative name
 * @see dynamicbox_alt_icon()
 */
extern const char *dynamicbox_alt_name(struct dynamicbox *handler);

/**
 * @brief Gets a lock for a frame buffer.
 * @details This function should be used to prevent from rendering to the frame buffer while reading it.
 *   And the locking area should be short and must be released ASAP, or the render thread will be hanged.
 * @param[in] handler Handler of a dynamicbox instance
 * @param[in] is_pd 1 for PD or 0
 * @privlevel platform
 * @privileve %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #DBOX_STATUS_ERROR_INVALID Invalid argument
 * @retval #DBOX_STATUS_SUCCESS Successfully done
 * @see dynamicbox_release_fb_lock()
 */
extern int dynamicbox_acquire_fb_lock(struct dynamicbox *handler, int is_pd);

/**
 * @brief Releases a lock of the frame buffer.
 * @details This function should be called ASAP after acquiring a lock of FB, or the render process will be blocked.
 * @param[in] handler Handler of a dynamicbox instance
 * @param[in] is_pd 1 for PD or 0
 * @privlevel platform
 * @privileve %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #DBOX_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #DBOX_STATUS_ERROR_INVALID Invalid argument
 * @retval #DBOX_STATUS_SUCCESS Successfully done
 * @see dynamicbox_acquire_fb_lock()
 */
extern int dynamicbox_release_fb_lock(struct dynamicbox *handler, int is_pd);

/**
 * @brief Sets options for controlling a dynamicbox sub-system.
 * @details
 *   DBOX_OPTION_FRAME_DROP_FOR_RESIZE
 *     While resizing the box, viewer doesn't want to know the updated frames of an old size content anymore.
 *     In that case, turn this on, the provider will not send the updated event to the viewer about an old content.
 *     So the viewer can reduce its burden to update unnecessary frames.
 *   DBOX_OPTION_MANUAL_SYNC
 *     If you don't want to update frames automatically, or you want only reload the frames by your hands, (manually)
 *     Turn this on.
 *     After turnning it on, you should sync it using dynamicbox_sync_pd_fb and dynamicbox_sync_dbox_pfb.
 *   DBOX_OPTION_SHARED_CONTENT
 *     If this option is turnned on, even though you create a new dynamicbox,
 *     if there are already added same instances that have same contents, the instance will not be created again.
 *     Instead of creating a new instance, a viewer will provide an old instance with a new handle.
 * @param[in] option Option which will be affected by this call
 * @param[in] state New value for given option
 * @privlevel N/P
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID Unknown option
 * @retval #DBOX_STATUS_ERROR_FAULT Failed to change the state of option
 * @retval #DBOX_STATUS_SUCCESS Successfully changed
 * @see dynamicbox_get_option()
 * @see dynamicbox_sync_pd_fb()
 * @see dynamicbox_sync_dbox_fb()
 */
extern int dynamicbox_set_option(enum dynamicbox_option_type option, int state);

/**
 * @brief Gets options of a dynamicbox sub-system.
 * @param[in] option Type of option
 * @privlevel N/P
 * @return int
 * @retval #DBOX_STATUS_ERROR_INVALID Invalid option
 * @retval #DBOX_STATUS_ERROR_FAULT Failed to get option
 * @retval >=0 Value of given option (must be >=0)
 * @see dynamicbox_set_option()
 */
extern int dynamicbox_option(enum dynamicbox_option_type option);


/**
 * @brief Set a handler for launching an app for auto-launch feature
 * @details If a user clicks a box, and the box uses auto-launch option, the launcher_handler will be called.
 * @param[in] launch_handler Handler for launching an app manually
 * @param[in] data Callback data which will be given a data for launch_handler
 * @privlevel N/P
 * @return int type
 * @retval #DBOX_STATUS_SUCCESS Succeed to set new handler. there is no other cases
 */
extern int dynamicbox_set_auto_launch_handler(int (*launch_handler)(struct dynamicbox *handler, const char *appid, void *data), void *data);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
