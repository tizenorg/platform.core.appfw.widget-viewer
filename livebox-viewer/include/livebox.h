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

#ifndef __LIVEBOX_H
#define __LIVEBOX_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file livebox.h
 * @brief This file declares API of liblivebox-viewer library
 */

/**
 * @addtogroup CAPI_LIVEBOX_VIEWER_MODULE
 * @{
 */

/**
 * @brief
 * Structure for a Livebox instance.
 */
struct livebox;

/**
 * @brief Definition for a default update period for Livebox (defined in the package manifest file).
 */
#define DEFAULT_PERIOD -1.0f

/**
 * @brief Enumeration for Mouse & Key event for buffer type Livebox or PD.
 * @details Viewer should send these events to livebox.
 */
enum content_event_type {
	CONTENT_EVENT_MOUSE_DOWN	= 0x00000001, /**< LB mouse down event for livebox */
	CONTENT_EVENT_MOUSE_UP		= 0x00000002, /**< LB mouse up event for livebox */
	CONTENT_EVENT_MOUSE_MOVE	= 0x00000004, /**< LB mouse move event for livebox */
	CONTENT_EVENT_MOUSE_ENTER	= 0x00000008, /**< LB mouse enter event for livebox */
	CONTENT_EVENT_MOUSE_LEAVE	= 0x00000010, /**< LB mouse leave event for livebox */
	CONTENT_EVENT_MOUSE_SET		= 0x00000020, /**< LB mouse set auto event for livebox */
	CONTENT_EVENT_MOUSE_UNSET	= 0x00000040, /**< LB mouse unset auto event for livebox */

	CONTENT_EVENT_KEY_DOWN		= 0x00000001, /**< LB key press */
	CONTENT_EVENT_KEY_UP		= 0x00000002, /**< LB key release */
	CONTENT_EVENT_KEY_FOCUS_IN	= 0x00000008, /**< LB key focused in */
	CONTENT_EVENT_KEY_FOCUS_OUT	= 0x00000010, /**< LB key focused out */
	CONTENT_EVENT_KEY_SET		= 0x00000020, /**< LB Key, start feeding event by master */
	CONTENT_EVENT_KEY_UNSET		= 0x00000040, /**< LB key, stop feeding event by master */

	CONTENT_EVENT_ON_SCROLL		= 0x00000080, /**< LB On scrolling */
	CONTENT_EVENT_ON_HOLD		= 0x00000100, /**< LB On holding */
	CONTENT_EVENT_OFF_SCROLL	= 0x00000200, /**< LB Stop scrolling */
	CONTENT_EVENT_OFF_HOLD		= 0x00000400, /**< LB Stop holding */

	CONTENT_EVENT_KEY_MASK		= 0x80000000, /**< Mask value for key event */
	CONTENT_EVENT_MOUSE_MASK	= 0x20000000, /**< Mask value for mouse event */
	CONTENT_EVENT_PD_MASK		= 0x10000000, /**< Mask value for PD event */
	CONTENT_EVENT_LB_MASK		= 0x40000000, /**< Mask value for LB event */

	LB_MOUSE_ON_SCROLL		= CONTENT_EVENT_LB_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_ON_SCROLL, /**< Mouse event occurs while scrolling */
	LB_MOUSE_ON_HOLD		= CONTENT_EVENT_LB_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_ON_HOLD, /**< Mouse event occurs on holding */
	LB_MOUSE_OFF_SCROLL		= CONTENT_EVENT_LB_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_OFF_SCROLL, /**< Scrolling stopped */
	LB_MOUSE_OFF_HOLD		= CONTENT_EVENT_LB_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_OFF_HOLD, /**< Holding stopped */

	LB_MOUSE_DOWN			= CONTENT_EVENT_LB_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_DOWN, /**< Mouse down on the livebox */
	LB_MOUSE_UP			= CONTENT_EVENT_LB_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_UP, /**< Mouse up on the livebox */
	LB_MOUSE_MOVE			= CONTENT_EVENT_LB_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_MOVE, /**< Move move on the livebox */
	LB_MOUSE_ENTER			= CONTENT_EVENT_LB_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_ENTER, /**< Mouse enter to the livebox */
	LB_MOUSE_LEAVE			= CONTENT_EVENT_LB_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_LEAVE, /**< Mouse leave from the livebox */
	LB_MOUSE_SET			= CONTENT_EVENT_LB_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_SET, /**< Mouse event, start feeding event by master */
	LB_MOUSE_UNSET			= CONTENT_EVENT_LB_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_UNSET, /**< Mouse event, stop feeding event by master */

	PD_MOUSE_ON_SCROLL		= CONTENT_EVENT_PD_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_ON_SCROLL, /**< Mouse event occurs while scrolling */
	PD_MOUSE_ON_HOLD		= CONTENT_EVENT_PD_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_ON_HOLD, /**< Mouse event occurs on holding */
	PD_MOUSE_OFF_SCROLL		= CONTENT_EVENT_PD_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_OFF_SCROLL, /**< Scrolling stopped */
	PD_MOUSE_OFF_HOLD		= CONTENT_EVENT_PD_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_OFF_HOLD, /**< Holding stopped */

	PD_MOUSE_DOWN			= CONTENT_EVENT_PD_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_DOWN, /**< Mouse down on the PD */
	PD_MOUSE_UP			= CONTENT_EVENT_PD_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_UP, /**< Mouse up on the PD */
	PD_MOUSE_MOVE			= CONTENT_EVENT_PD_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_MOVE, /**< Mouse move on the PD */
	PD_MOUSE_ENTER			= CONTENT_EVENT_PD_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_ENTER, /**< Mouse enter to the PD */
	PD_MOUSE_LEAVE			= CONTENT_EVENT_PD_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_LEAVE, /**< Mouse leave from the PD */
	PD_MOUSE_SET			= CONTENT_EVENT_PD_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_SET, /**< Mouse event, start feeding event by master */
	PD_MOUSE_UNSET			= CONTENT_EVENT_PD_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_UNSET, /**< Mouse event, stop feeding event by master */

	LB_KEY_DOWN			= CONTENT_EVENT_LB_MASK | CONTENT_EVENT_KEY_MASK | CONTENT_EVENT_KEY_DOWN, /**< Key down on the livebox */
	LB_KEY_UP			= CONTENT_EVENT_LB_MASK | CONTENT_EVENT_KEY_MASK | CONTENT_EVENT_KEY_UP, /**< Key up on the livebox */
	LB_KEY_SET			= CONTENT_EVENT_LB_MASK | CONTENT_EVENT_KEY_MASK | CONTENT_EVENT_KEY_SET, /**< Key event, start feeding event by master */
	LB_KEY_UNSET			= CONTENT_EVENT_LB_MASK | CONTENT_EVENT_KEY_MASK | CONTENT_EVENT_KEY_UNSET, /**< Key event, stop feeding event by master */
	LB_KEY_FOCUS_IN			= CONTENT_EVENT_LB_MASK | CONTENT_EVENT_KEY_MASK | CONTENT_EVENT_KEY_FOCUS_IN, /**< Key event, focus in */
	LB_KEY_FOCUS_OUT		= CONTENT_EVENT_LB_MASK | CONTENT_EVENT_KEY_MASK | CONTENT_EVENT_KEY_FOCUS_OUT, /**< Key event, foucs out */

	PD_KEY_DOWN			= CONTENT_EVENT_PD_MASK | CONTENT_EVENT_KEY_MASK | CONTENT_EVENT_KEY_DOWN, /**< Key down on the livebox */
	PD_KEY_UP			= CONTENT_EVENT_PD_MASK | CONTENT_EVENT_KEY_MASK | CONTENT_EVENT_KEY_UP, /**< Key up on the livebox */
	PD_KEY_SET			= CONTENT_EVENT_PD_MASK | CONTENT_EVENT_KEY_MASK | CONTENT_EVENT_KEY_SET, /**< Key event, start feeding event by master */
	PD_KEY_UNSET			= CONTENT_EVENT_PD_MASK | CONTENT_EVENT_KEY_MASK | CONTENT_EVENT_KEY_UNSET, /**< Key event, stop feeding event by master */
	PD_KEY_FOCUS_IN			= CONTENT_EVENT_PD_MASK | CONTENT_EVENT_KEY_MASK | CONTENT_EVENT_KEY_FOCUS_IN, /**< Key event, focus in */
	PD_KEY_FOCUS_OUT		= CONTENT_EVENT_PD_MASK | CONTENT_EVENT_KEY_MASK | CONTENT_EVENT_KEY_FOCUS_OUT, /**< Key event, focus out */

	CONTENT_EVENT_MAX		= 0xFFFFFFFF /**< Unknown event */
};

/**
 * @brief Enumeration for Accessibility event for buffer type Livebox or PD.
 * @details These events are sync'd with Tizen accessibility event set.
 */
enum access_event_type {
	ACCESS_EVENT_PD_MASK		= 0x10000000, /**< PD Accessibility event mask */
	ACCESS_EVENT_LB_MASK		= 0x20000000, /**< LB Accessibility event mask */

	ACCESS_EVENT_HIGHLIGHT		= 0x00000100, /**< LB accessibility: Hightlight a object */
	ACCESS_EVENT_HIGHLIGHT_NEXT	= 0x00000200, /**< LB accessibility: Set highlight to next object */
	ACCESS_EVENT_HIGHLIGHT_PREV	= 0x00000400, /**< LB accessibility: Set highlight to prev object */
	ACCESS_EVENT_UNHIGHLIGHT	= 0x00000800, /**< LB accessibility unhighlight */
	ACCESS_EVENT_ACTIVATE		= 0x00001000, /**< LB accessibility activate */
	ACCESS_EVENT_ACTION_DOWN	= 0x00010000, /**< LB accessibility value changed */
	ACCESS_EVENT_ACTION_UP		= 0x00020000, /**< LB accessibility value changed */
	ACCESS_EVENT_SCROLL_DOWN	= 0x00100000, /**< LB accessibility scroll down */
	ACCESS_EVENT_SCROLL_MOVE	= 0x00200000, /**< LB accessibility scroll move */
	ACCESS_EVENT_SCROLL_UP		= 0x00400000, /**< LB accessibility scroll up */

	LB_ACCESS_HIGHLIGHT		= ACCESS_EVENT_LB_MASK | ACCESS_EVENT_HIGHLIGHT,	/**< Access event - Highlight an object in the livebox */
	LB_ACCESS_HIGHLIGHT_NEXT	= ACCESS_EVENT_LB_MASK | ACCESS_EVENT_HIGHLIGHT_NEXT,	/**< Access event - Move highlight to the next object in a livebox */
	LB_ACCESS_HIGHLIGHT_PREV	= ACCESS_EVENT_LB_MASK | ACCESS_EVENT_HIGHLIGHT_PREV,	/**< Access event - Move highlight to the prev object in a livebox */
	LB_ACCESS_UNHIGHLIGHT		= ACCESS_EVENT_LB_MASK | ACCESS_EVENT_UNHIGHLIGHT,	/**< Access event - Delete highlight from the livebox */
	LB_ACCESS_ACTIVATE		= ACCESS_EVENT_LB_MASK | ACCESS_EVENT_ACTIVATE,		/**< Access event - Launch or activate the highlighted object */
	LB_ACCESS_ACTION_DOWN		= ACCESS_EVENT_LB_MASK | ACCESS_EVENT_ACTION_DOWN,	/**< Access event - down */
	LB_ACCESS_ACTION_UP		= ACCESS_EVENT_LB_MASK | ACCESS_EVENT_ACTION_UP,	/**< Access event - up */
	LB_ACCESS_SCROLL_DOWN		= ACCESS_EVENT_LB_MASK | ACCESS_EVENT_SCROLL_DOWN,	/**< Access event - scroll down */
	LB_ACCESS_SCROLL_MOVE		= ACCESS_EVENT_LB_MASK | ACCESS_EVENT_SCROLL_MOVE,	/**< Access event - scroll move */
	LB_ACCESS_SCROLL_UP		= ACCESS_EVENT_LB_MASK | ACCESS_EVENT_SCROLL_UP,	/**< Access event - scroll up */

	PD_ACCESS_HIGHLIGHT		= ACCESS_EVENT_PD_MASK | ACCESS_EVENT_HIGHLIGHT,	/**< Access event - Highlight an object in the PD */
	PD_ACCESS_HIGHLIGHT_NEXT	= ACCESS_EVENT_PD_MASK | ACCESS_EVENT_HIGHLIGHT_NEXT,	/**< Access event - Move highlight to the next object in a PD */
	PD_ACCESS_HIGHLIGHT_PREV	= ACCESS_EVENT_PD_MASK | ACCESS_EVENT_HIGHLIGHT_PREV,	/**< Access event - Move highlight to the prev object in a PD */
	PD_ACCESS_UNHIGHLIGHT		= ACCESS_EVENT_PD_MASK | ACCESS_EVENT_UNHIGHLIGHT,	/**< Access event - Delet highlight from the PD */
	PD_ACCESS_ACTIVATE		= ACCESS_EVENT_PD_MASK | ACCESS_EVENT_ACTIVATE,		/**< Access event - Launch or activate the highlighted object */
	PD_ACCESS_ACTION_DOWN		= ACCESS_EVENT_PD_MASK | ACCESS_EVENT_ACTION_DOWN,	/**< Access event - down */
	PD_ACCESS_ACTION_UP		= ACCESS_EVENT_PD_MASK | ACCESS_EVENT_ACTION_UP,	/**< Access event - up */
	PD_ACCESS_SCROLL_DOWN		= ACCESS_EVENT_PD_MASK | ACCESS_EVENT_SCROLL_DOWN,	/**< Access event - scroll down */
	PD_ACCESS_SCROLL_MOVE		= ACCESS_EVENT_PD_MASK | ACCESS_EVENT_SCROLL_MOVE,	/**< Access event - scroll move */
	PD_ACCESS_SCROLL_UP		= ACCESS_EVENT_PD_MASK | ACCESS_EVENT_SCROLL_UP		/**< Access event - scroll up */
};

/**
 * @brief Enumeration for Livebox LB content type.
 */
enum livebox_lb_type {
	LB_TYPE_IMAGE = 0x01, /**< Contents of a livebox is based on the image file */
	LB_TYPE_BUFFER = 0x02, /**< Contents of a livebox is based on canvas buffer(shared) */
	LB_TYPE_TEXT = 0x04, /**< Contents of a livebox is based on formatted text file */
	LB_TYPE_PIXMAP = 0x08, /**< Contens of a livebox is shared by the pixmap(depends on X) */
	LB_TYPE_ELEMENTARY = 0x10, /**< Using Elementary for sharing content & event */
	LB_TYPE_INVALID = 0xFF /**< Unknown LB type */
};

/**
 * @brief Enumeration for Livebox PD content type.
 */
enum livebox_pd_type {
	PD_TYPE_BUFFER = 0x01, /**< Contents of a PD is based on canvas buffer(shared) */
	PD_TYPE_TEXT = 0x02, /**< Contents of a PD is based on formatted text file */
	PD_TYPE_PIXMAP = 0x04, /**< Contents of a livebox is shared by the pixmap(depends on X) */
	PD_TYPE_ELEMENTARY = 0x08, /**< Using Elementary for sharing content & event */
	PD_TYPE_INVALID = 0xFF /**< Unknown PD type */
};

/**
 * @brief Enumeration for Livebox event type.
 * @details These events will be sent from the provider.
 */
enum livebox_event_type { /**< livebox_event_handler_set Event list */
	LB_EVENT_LB_UPDATED, /**< Contents of the given livebox is updated */
	LB_EVENT_PD_UPDATED, /**< Contents of the given pd is updated */

	LB_EVENT_CREATED, /**< A new livebox is created */
	LB_EVENT_DELETED, /**< A livebox is deleted */

	LB_EVENT_GROUP_CHANGED, /**< Group (Cluster/Sub-cluster) information is changed */
	LB_EVENT_PINUP_CHANGED, /**< PINUP status is changed */
	LB_EVENT_PERIOD_CHANGED, /**< Update period is changed */

	LB_EVENT_LB_SIZE_CHANGED, /**< Livebox size is changed */
	LB_EVENT_PD_SIZE_CHANGED, /**< PD size is changed */

	LB_EVENT_PD_CREATED, /**< If a PD is created even if you didn't call the livebox_create_pd API */
	LB_EVENT_PD_DESTROYED, /**< If a PD is destroyed even if you didn't call the livebox_destroy_pd API */

	LB_EVENT_HOLD_SCROLL, /**< If the screen should be freezed */
	LB_EVENT_RELEASE_SCROLL, /**< If the screen can be scrolled */

	LB_EVENT_LB_UPDATE_BEGIN, /**< Livebox LB content update is started */
	LB_EVENT_LB_UPDATE_END, /**< Livebox LB content update is finished */

	LB_EVENT_PD_UPDATE_BEGIN, /**< Livebox PD content update is started */
	LB_EVENT_PD_UPDATE_END, /**< Livebox PD content update is finished */

	LB_EVENT_UPDATE_MODE_CHANGED, /**< Livebox Update mode is changed */

	LB_EVENT_REQUEST_CLOSE_PD, /**< Livebox requests to close the PD */

	LB_EVENT_IGNORED /**< Request is ignored */
};

/**
 * @brief Enumeration for Livebox option types.
 */
enum livebox_option_type {
	LB_OPTION_MANUAL_SYNC,			/**< Sync manually */
	LB_OPTION_FRAME_DROP_FOR_RESIZE,	/**< Drop frames while resizing */
	LB_OPTION_SHARED_CONTENT,		/**< Use only one real instance for multiple fake instances if user creates it using same content */

	LB_OPTION_ERROR = 0xFFFFFFFF		/**< To specify the size of this enumeration type */
};

enum livebox_fault_type {
	LB_FAULT_DEACTIVATED, /*!< Livebox is deactivated by its fault operation */
	LB_FAULT_PROVIDER_DISCONNECTED /*!< Provider is disconnected */
};

/**
 * @brief Enumeration for Livebox visible states.
 * @details Must be sync'd with a provider.
 */
enum livebox_visible_state {
	LB_SHOW = 0x00, /**< Livebox is shown. Default state */
	LB_HIDE = 0x01, /**< Livebox is hidden, Update timer will not be freezed. but you cannot receive any updates events. */

	LB_HIDE_WITH_PAUSE = 0x02, /**< Livebix is hidden, it will pause the update timer, but if a livebox updates its contents, update event will be triggered */

	LB_VISIBLE_ERROR = 0xFFFFFFFF /**< To specify the size of this enumeration type */
};

/**
 * @brief Structure for TEXT type livebox contents handling opertators.
 */
struct livebox_script_operators {
	int (*update_begin)(struct livebox *handle); /**< Content parser is started */
	int (*update_end)(struct livebox *handle); /**< Content parser is finished */

	/* Listed functions will be called when parser meets each typed content */
	int (*update_text)(struct livebox *handle, const char *id, const char *part, const char *data); /**< Update text content */
	int (*update_image)(struct livebox *handle, const char *id, const char *part, const char *data, const char *option); /**< Update image content */
	int (*update_script)(struct livebox *handle, const char *id, const char *new_id, const char *part, const char *file, const char *group); /**< Update script content */
	int (*update_signal)(struct livebox *handle, const char *id, const char *emission, const char *signal); /**< Update signal */
	int (*update_drag)(struct livebox *handle, const char *id, const char *part, double dx, double dy); /**< Update drag info */
	int (*update_info_size)(struct livebox *handle, const char *id, int w, int h); /**< Update content size */
	int (*update_info_category)(struct livebox *handle, const char *id, const char *category); /**< Update content category info */
	int (*update_access)(struct livebox *handle, const char *id, const char *part, const char *text, const char *option); /**< Update access information */
	int (*operate_access)(struct livebox *handle, const char *id, const char *part, const char *operation, const char *option); /**< Update access operation */
	int (*update_color)(struct livebox *handle, const char *id, const char *part, const char *data); /**< Update color */
};

/**
 * @brief Called for every async function.
 * @details Prototype of the return callback of every async functions.
 * @param[in] handle Handle of the livebox instance
 * @param[in] ret Result status of operation (LB_STATUS_XXX defined from liblivebox-service)
 * @param[in] data Data for result callback
 * @return void
 * @see livebox_add()
 * @see livebox_del()
 * @see livebox_activate()
 * @see livebox_resize()
 * @see livebox_set_group()
 * @see livebox_set_period()
 * @see livebox_access_event()
 * @see livebox_set_pinup()
 * @see livebox_create_pd()
 * @see livebox_create_pd_with_position()
 * @see livebox_destroy_pd()
 * @see livebox_emit_text_signal()
 * @see livebox_acquire_pd_pixmap()
 * @see livebox_acquire_lb_pixmap()
 * @see livebox_set_update_mode()
 */
typedef void (*ret_cb_t)(struct livebox *handle, int ret, void *data);

/**
 * @brief Initializes the livebox system.
 * @remarks This API uses get/setenv APIs.
 *   Those APIs are not thread-safe.
 *   So you have to consider to use the livebox_init_with_options instead of this if you are developing multi-threaded viewer.
 * @param[in] disp X Display connection object (If you have X Display connection object already, you can re-use it. But you should care its life cycle.
 *                 It must be alive before calling livebox_fini())
 * @return int
 * @retval #LB_STATUS_SUCCESS if success
 * @see livebox_fini()
 * @see livebox_init_with_options()
 */
extern int livebox_init(void *disp);

/**
 * @brief Initializes the livebox system with some options.
 * @details livebox_init function uses environment value to initiate some configurable values.
 *          But some applications do not want to use the env value.
 *          For them, this API will give a chance to set default options using given arguments.
 * @param[in] disp Display (if @a disp is @c NULL, the library will try to acquire a new connection with X)
 * @param[in] prevent_overwrite Overwrite flag (when the content of an image type livebox is updated, it will be overwriten (0) or not (1))
 * @param[in] event_filter If the next event comes in this period, ignore it. It is too fast to processing it in time // need to be elaborated
 * @param[in] use_thread Use the receive thread // need to be elaborated
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int Integer, Livebox status code
 * @retval #LB_STATUS_SUCCESS if success
 * @see livebox_init()
 * @see livebox_fini()
 */
extern int livebox_init_with_options(void *disp, int prevent_overwrite, double event_filter, int use_thread);

/**
 * @brief Finalizes the livebox system.
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #LB_STATUS_SUCCES if success
 * @retval #LB_STATUS_ERROR_INVALID if livebox_init is not called
 * @see livebox_init()
 * @see livebox_init_with_options()
 */
extern int livebox_fini(void);

/**
 * @brief Notifies the status of a client ("it is paused") to the provider.
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #LB_STATUS_SUCCESS if success
 * @retval #LB_STATUS_ERROR_FAULT if it failed to send state (paused) info
 * @see livebox_client_resumed()
 */
extern int livebox_client_paused(void);

/**
 * @brief Notifies the status of client ("it is resumed") to the provider.
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #LB_STATUS_SUCCESS if success
 * @retval #LB_STATUS_ERROR_FAULT if it failed to send state (resumed) info
 * @see livebox_client_paused()
 */
extern int livebox_client_resumed(void);

/**
 * @brief Adds a new livebox.
 * @details If the screen size is "1280x720", the below size lists are used for default.
 * Or you can find the default sizes in pixel from /usr/share/data-provider-master/resolution.ini.
 * Size types are defined from the liblivebox-service package (livebox-service.h).
 *
 * Normal mode livebox
 * 1x1=175x175, #LB_SIZE_TYPE_1x1
 * 2x1=354x175, #LB_SIZE_TYPE_2x1
 * 2x2=354x354, #LB_SIZE_TYPE_2x2
 * 4x1=712x175, #LB_SIZE_TYPE_4x1
 * 4x2=712x354, #LB_SIZE_TYPE_4x2
 * 4x4=712x712, #LB_SIZE_TYPE_4x4
 *
 * Extended sizes
 * 4x3=712x533, #LB_SIZE_TYPE_4x3
 * 4x5=712x891, #LB_SIZE_TYPE_4x5
 * 4x6=712x1070, #LB_SIZE_TYPE_4x6
 *
 * Easy mode livebox
 * 21x21=224x215, #LB_SIZE_TYPE_EASY_1x1
 * 23x21=680x215, #LB_SIZE_TYPE_EASY_3x1
 * 23x23=680x653, #LB_SIZE_TYPE_EASY_3x3
 *
 * Special livebox
 * 0x0=720x1280, #LB_SIZE_TYPE_0x0
 *
 * @remarks
 *    Even if you get a handle from the return value of this function, it is not a created instance.
 *    So you have to consider it as a not initialized handle.
 *    It can be initialized only after getting the return callback with "ret == #LB_STATUS_SUCCESS"
 * @param[in] pkgname Livebox Id
 * @param[in] content Contents that will be passed to the livebox instance
 * @param[in] cluster Main group
 * @param[in] category Sub group
 * @param[in] period Update period (@c DEFAULT_PERIOD can be used for this; this argument will be used to specify the period of updating contents of a livebox)
 * @param[in] type Size type (defined from liblivebox-service package)
 * @param[in] cb After the request is sent to the master provider, this callback will be called
 * @param[in] data This data will be passed to the callback
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return handle
 * @retval Handle Livebox handle but not yet initialized
 * @retval @c NULL if it fails to create a handle
 * @see ret_cb_t
 */
extern struct livebox *livebox_add(const char *pkgname, const char *content, const char *cluster, const char *category, double period, int type, ret_cb_t cb, void *data);

/**
 * @brief Deletes a livebox (will replace livebox_del).
 * @remarks If you call this with an uninitialized handle, the return callback will be called synchronously.
 *    So before returning from this function, the return callback will be called first.
 * @param[in] handler Handler of a livebox instance
 * @param[in] type Deletion type (LB_DELETE_PERMANENTLY or LB_DELETE_TEMPORARY)
 * @param[in] cb Return callback
 * @param[in] data User data for return callback
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #LB_STATUS_ERROR_INVALID Invalid argument
 * @retval #LB_STATUS_ERROR_BUSY Already in process
 * @retval #LB_STATUS_ERROR_FAULT Failed to create a request packet
 * @retval #LB_STATUS_SUCCESS Successfully sent, return callack will be called
 * @see ret_cb_t
 */
extern int livebox_del(struct livebox *handler, int type, ret_cb_t cb, void *data);

/**
 * @brief Sets a livebox events callback.
 * @details To get the event push from the provider, register the event callback using this function.
 *    The callback will be called if there is any event from the provider.
 * @param[in] cb Event handler
 * @param[in] data User data for the event handler
 * @privlevel N/P
 * @return int
 * @retval #LB_STATUS_SUCCESS If succeed to set event handler
 * @retval #LB_STATUS_ERROR_INVALID Invalid argument
 * @retval #LB_STATUS_ERROR_MEMORY Not enough memory
 * @see livebox_unset_event_handler()
 */
extern int livebox_set_event_handler(int (*cb)(struct livebox *handler, enum livebox_event_type event, void *data), void *data);

/**
 * @brief Unsets the livebox event handler.
 * @param[in] cb Event handler
 * @privlevel N/P
 * @return void * Event handler data
 * @retval pointer Pointer of 'data' which is used with the livebox_set_event_handler
 * @see livebox_set_event_handler()
 */
extern void *livebox_unset_event_handler(int (*cb)(struct livebox *handler, enum livebox_event_type event, void *data));

/**
 * @brief Registers the livebox fault event handler.
 * @details Argument list: event, pkgname, filename, funcname.
 * @param[in] cb Event handler
 * @param[in] data Event handler data
 * @privlevel N/P
 * @return int
 * @retval #LB_STATUS_SUCCESS If succeed to set fault event handler
 * @retval #LB_STATUS_ERROR_INVALID Invalid argument
 * @retval #LB_STATUS_ERROR_MEMORY Not enough memory
 * @see livebox_unset_fault_handler()
 */
extern int livebox_set_fault_handler(int (*cb)(enum livebox_fault_type, const char *, const char *, const char *, void *), void *data);

/**
 * @brief Unsets the livebox fault event handler.
 * @param[in] cb Event handler
 * @privlevel N/P
 * @return void * Callback data which is set via livebox_set_fault_handler
 * @retval pointer Pointer of 'data' which is used with the livebox_set_fault_handler
 * @see livebox_set_fault_handler()
 */
extern void *livebox_unset_fault_handler(int (*cb)(enum livebox_fault_type, const char *, const char *, const char *, void *));

/**
 * @brief Activates the faulted livebox.
 * @details Request result will be returned via return callback.
 * @remarks Even though this function returns SUCCESS, it means that it just successfully sent a request to the provider.
 *    So you have to check the return callback and its "ret" argument.
 * @param[in] pkgname Package name which should be activated
 * @param[in] cb Result callback
 * @param[in] data Callback data
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int type
 * @retval #LB_STATUS_SUCCESS Successfully sent a request
 * @retval #LB_STATUS_ERROR_INVALID Invalid argument
 * @retval #LB_STATUS_ERROR_FAULT Failed to make a request
 * @see ret_cb_t
 */
extern int livebox_activate(const char *pkgname, ret_cb_t cb, void *data);

/**
 * @brief Resizes the livebox.
 * @details
 * Normal mode livebox size
 * 1x1=175x175, LB_SIZE_TYPE_1x1
 * 2x1=354x175, LB_SIZE_TYPE_2x1
 * 2x2=354x354, LB_SIZE_TYPE_2x2
 * 4x1=712x175, LB_SIZE_TYPE_4x1
 * 4x2=712x354, LB_SIZE_TYPE_4x2
 * 4x4=712x712, LB_SIZE_TYPE_4x4
 *
 * Extended livebox size
 * 4x3=712x533, LB_SIZE_TYPE_4x3
 * 4x5=712x891, LB_SIZE_TYPE_4x5
 * 4x6=712x1070, LB_SIZE_TYPE_4x6
 *
 * Easy mode livebox size
 * 21x21=224x215, LB_SIZE_TYPE_EASY_1x1
 * 23x21=680x215, LB_SIZE_TYPE_EASY_3x1
 * 23x23=680x653, LB_SIZE_TYPE_EASY_3x3
 *
 * Special mode livebox size
 * 0x0=720x1280, LB_SIZE_TYPE_0x0
 * @remarks You have to check the return callback.
 * @param[in] handler Handler of a livebox instance
 * @param[in] type Type of a livebox size (e.g., LB_SIZE_TYPE_1x1, ...)
 * @param[in] cb Result callback of the resize operation
 * @param[in] data User data for return callback
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int type
 * @retval #LB_STATUS_ERROR_INVALID Invalid argument
 * @retval #LB_STATUS_ERROR_BUSY Previous request of resize is in progress
 * @retval #LB_STATUS_ERROR_ALREADY Already resized, there is no differences between current size and requested size
 * @retval #LB_STATUS_ERROR_PERMISSION Permission denied, you only have view the content of this box
 * @retval #LB_STATUS_ERROR_FAULT Failed to make a request
 * @see ret_cb_t
 */
extern int livebox_resize(struct livebox *handler, int type, ret_cb_t cb, void *data);

/**
 * @brief Sends the click event for a livebox.
 * @param[in] handler Handler of a livebox instance
 * @param[in] x Rational X of the content width
 * @param[in] y Rational Y of the content height
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #LB_STATUS_ERROR_INVALID Invalid argument
 * @retval #LB_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #LB_STATUS_SUCCESS Successfully done
 */
extern int livebox_click(struct livebox *handler, double x, double y);

/**
 * @brief Changes the cluster/sub-cluster name of the given livebox handler.
 * @param[in] handler Handler of a livebox instance
 * @param[in] cluster New cluster of a livebox
 * @param[in] category New category of a livebox
 * @param[in] cb Result callback for changing the cluster/category of a livebox
 * @param[in] data User data for the result callback
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #LB_STATUS_SUCCESS Request is successfully sent. the return callback will be called
 * @retval #LB_STATUS_ERROR_BUSY Previous request is not finished yet
 * @retval #LB_STATUS_ERROR_ALREADY Group name is same with current one
 * @retval #LB_STATUS_ERROR_PERMISSION You have no permission to change property of this livebox instance
 * @retval #LB_STATUS_ERROR_FAULT Failed to make a request
 * @see ret_cb_t
 */
extern int livebox_set_group(struct livebox *handler, const char *cluster, const char *category, ret_cb_t cb, void *data);

/**
 * @brief Gets the cluster and category (sub-cluster) name of the given livebox (it is not I18N format, only English).
 * @remarks You have to do not release the cluster & category.
 *    It is allocated inside of a given livebox instance, so you can only read it.
 * @param[in] handler Handler of a livebox instance
 * @param[out] cluster Storage(memory) for containing the cluster name
 * @param[out] category Storage(memory) for containing the category name
 * @privlevel N/P
 * @return int
 * @retval #LB_STATUS_ERROR_INVALID Invalid argument
 * @retval #LB_STATUS_SUCCESS Successfully done
 */
extern int livebox_get_group(struct livebox *handler, const char **cluster, const char **category);

/**
 * @brief Gets the period of the livebox handler.
 * @remarks If this function returns 0.0f, it means the livebox has no update period or the handle is not valid.
 *    This function only works after the return callback of livebox_create fucntion is called.
 * @param[in] handler Handler of a livebox instance
 * @privlevel N/P
 * @return double
 * @retval Current update period of a livebox
 * @retval 0.0f This means the box has no update period or the handles is not valid
 */
extern double livebox_period(struct livebox *handler);

/**
 * @brief Changes the update period.
 * @param[in] handler Handler of a livebox instance
 * @param[in] period New update period of a livebox
 * @param[in] cb Result callback of changing the update period of this livebox
 * @param[in] data User data for the result callback
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #LB_STATUS_SUCCESS Successfully done
 * @retval #LB_STATUS_ERROR_INVALID Invalid argument
 * @retval #LB_STATUS_ERROR_BUSY
 * @retval #LB_STATUS_ERROR_ALREADY
 * @retval #LB_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @see ret_cb_t
 */
extern int livebox_set_period(struct livebox *handler, double period, ret_cb_t cb, void *data);

/**
 * @brief Checks whether the given livebox is a text type or not.
 * @param[in] handler Handler of a livebox instance
 * @privlevel N/P
 * @return livebox_lb_type
 * @retval #LB_TYPE_IMAGE Contents of a livebox is based on the image file
 * @retval #LB_TYPE_BUFFER Contents of a livebox is based on canvas buffer(shared)
 * @retval #LB_TYPE_TEXT Contents of a livebox is based on formatted text file
 * @retval #LB_TYPE_PIXMAP Contens of a livebox is shared by the pixmap(depends on X)
 * @retval #LB_TYPE_INVALID
 * @see livebox_lb_type()
 */
extern enum livebox_lb_type livebox_lb_type(struct livebox *handler);

/**
 * @brief Checks if the given livebox is created by user or not.
 * @details If the livebox instance is created by a system this will return 0.
 * @param[in] handler Handler of a livebox instance
 * @privlevel N/P
 * @return int
 * @retval #LB_STATUS_ERROR_INVALID Invalid argument
 * @retval 0 Automatically created livebox by the provider
 * @retval 1 Created by user via livebox_add()
 * @see livebox_add()
 * @see livebox_set_event_handler()
 */
extern int livebox_is_user(struct livebox *handler);

/**
 * @brief Gets content information string of the given livebox.
 * @param[in] handler Handler of a livebox instance
 * @privlevel N/P
 * @return const char *
 * @retval content_info Livebox content info that can be used again via content_info argument of livebox_add()
 * @see livebox_add()
 */
extern const char *livebox_content(struct livebox *handler);

/**
 * @brief Gets the sub cluster title string of the given livebox.
 * @details This API is now used for accessibility.
 *  Each box should set their content as a string to be read by TTS.
 *  So if the box has focused on the homescreen, the homescreen will read text using this API.
 * @remarks The title returned by this API is read by TTS.
 *  But it is just recomended to a homescreen.
 *  So, to read it or not depends on implementation of the homescreen.
 * @param[in] handler Handler of a livebox instance
 * @privlevel N/P
 * @return const char *
 * @retval sub Cluster name
 * @retval @c NULL
 */
extern const char *livebox_category_title(struct livebox *handler);

/**
 * @brief Gets the filename of the given livebox, if it is an IMAGE type livebox.
 * @details If the box is developed as image format to represent its contents, the homescreen should know its image file name.
 * @param[in] handler Handler of a livebox instance
 * @privlevel N/P
 * @return const char *
 * @retval filename If the livebox type is image this function will give you a abs-path of an image file (content is rendered)
 * @retval @c NULL If this has no image file or type is not image file.
 */
extern const char *livebox_filename(struct livebox *handler);

/**
 * @brief Gets the package name of the given livebox handler.
 * @param[in] handler Handler of a livebox instance
 * @privlevel N/P
 * @return const char *
 * @retval pkgname Package name
 * @retval @c NULL If the handler is not valid
 */
extern const char *livebox_pkgname(struct livebox *handler);

/**
 * @brief Gets the priority of a current content.
 * @param[in] handler Handler of a livebox instance
 * @privlevel N/P
 * @return double
 * @retval 0.0f Handler is @c NULL
 * @retval -1.0f Handler is not valid (not yet initialized)
 * @retval real Number between 0.0 and 1.0
 */
extern double livebox_priority(struct livebox *handler);

/**
 * @brief Acquires the buffer of a given livebox (only for the buffer type).
 * @param[in] handler Handler of a livebox instance
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return void *
 * @retval address Address of a FB
 * @retval @c NULL If it fails to get fb address
 */
extern void *livebox_acquire_fb(struct livebox *handler);

/**
 * @brief Releases the buffer of a livebox (only for the buffer type).
 * @param[in] buffer Buffer
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #LB_STATUS_ERROR_INVALID Invalid argument
 * @retval #LB_STATUS_SUCCESS Successfully done
 * @see livebox_acquire_fb()
 */
extern int livebox_release_fb(void *buffer);

/**
 * @brief Gets the reference count of Livebox buffer (only for the buffer type).
 * @param[in] buffer Buffer
 * @privlevel N/P
 * @return int
 * @retval #LB_STATUS_ERROR_INVALID Invalid argument
 * @retval #LB_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval refcnt Positive integer including ZERO
 * @see livebox_pdfb_refcnt()
 */
extern int livebox_fb_refcnt(void *buffer);

/**
 * @brief Acquires the buffer of a PD frame (only for the buffer type).
 * @param[in] handler Handler of a livebox instance
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval @c NULL
 * @retval adress Address of a buffer of PD
 * @see livebox_release_pdfb()
 */
extern void *livebox_acquire_pdfb(struct livebox *handler);

/**
 * @brief Releases the acquired buffer of the PD Frame (only for the buffer type).
 * @param[in] buffer Buffer
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #LB_STATUS_ERROR_INVALID Invalid argument
 * @retval #LB_STATUS_SUCCESS Successfully done
 * @see livebox_acquire_pdfb()
 */
extern int livebox_release_pdfb(void *buffer);

/**
 * @brief Gets the reference count of a given PD buffer (only for the buffer type).
 * @param[in] buffer Buffer
 * @privlevel N/P
 * @return int
 * @retval #LB_STATUS_ERROR_INVALID Invalid argument
 * @retval #LB_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval reference Reference count
 * @see livebox_fb_refcnt()
 */
extern int livebox_pdfb_refcnt(void *buffer);

/**
 * @brief Gets the size of the Livebox.
 * @param[in] handler Handler of a livebox instance
 * @privlevel N/P
 * @return int
 * @retval #LB_SIZE_TYPE_NxM
 * @retval #LB_SIZE_TYPE_INVALID
 */
extern int livebox_size(struct livebox *handler);

/**
 * @brief Gets the size of the Progressive Disclosure.
 * @param[in] handler Handler of a livebox instance
 * @param[out] w
 * @param[out] h
 * @privlevel N/P
 * @return int type
 * @retval #LB_STATUS_ERROR_INVALID Invalid argument
 * @retval #LB_STATUS_SUCCESS Successfully done
 */
extern int livebox_get_pdsize(struct livebox *handler, int *w, int *h);

/**
 * @brief Gets a list of the supported sizes of a given handler.
 * @param[in] handler Handler of a livebox instance
 * @param[out] cnt
 * @param[out] size_list
 * @privlevel N/P
 * @return int type
 * @retval #LB_STATUS_ERROR_INVALID Invalid argument
 * @retval #LB_STATUS_SUCCESS Successfully done
 */
extern int livebox_get_supported_sizes(struct livebox *handler, int *cnt, int *size_list);

/**
 * @brief Gets BUFFER SIZE of the livebox if it is a buffer type.
 * @param[in] handler Handler of a livebox instance
 * @privlevel N/P
 * @return int
 * @retval #LB_STATUS_ERROR_INVALID Invalid argument
 * @retval size Size of livebox buffer
 */
extern int livebox_lbfb_bufsz(struct livebox *handler);

/**
 * @brief Gets BUFFER SIZE of the progiressive disclosure if it is a buffer type.
 * @param[in] handler Handler of a livebox instance
 * @privlevel N/P
 * @return int
 * @retval #LB_STATUS_ERROR_INVALID Invalid argument
 * @retval size Size of PD buffer
 */
extern int livebox_pdfb_bufsz(struct livebox *handler);

/**
 * @brief Sends a content event (for buffer type) to the provider (livebox).
 * @param[in] handler Handler of a livebox instance
 * @param[in] type Event type
 * @param[in] x Coordinates of X axis
 * @param[in] y Coordinates of Y axis
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #LB_STATUS_ERROR_INVALID Invalid argument
 * @retval #LB_STATUS_ERROR_BUSY Previous operation is not finished yet
 * @retval #LB_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #LB_STATUS_SUCCESS Successfully sent
 * @see livebox_access_event()
 * @see livebox_key_event()
 */
extern int livebox_mouse_event(struct livebox *handler, enum content_event_type type, double x, double y);

/**
 * @brief Sends an access event (for buffer type) to the provider (livebox).
 * @param[in] handler Handler of a livebox instance
 * @param[in] type Event type
 * @param[in] x Coordinates of X axsis
 * @param[in] y Coordinates of Y axsis
 * @param[in] cb Result callback function
 * @param[in] data Callback data
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #LB_STATUS_ERROR_INVALID Invalid argument
 * @retval #LB_STATUS_ERROR_BUSY Previous operation is not finished yet
 * @retval #LB_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #LB_STATUS_SUCCESS Successfully sent
 * @see livebox_mouse_event()
 * @see livebox_key_event()
 */
extern int livebox_access_event(struct livebox *handler, enum access_event_type type, double x, double y, ret_cb_t cb, void *data);

/**
 * @brief Sends a key event (for buffer type) to the provider (livebox).
 * @param[in] handler Handler of a livebox instance
 * @param[in] type Key event type
 * @param[in] keycode Code of key
 * @param[in] cb Result callback
 * @param[in] data Callback data
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #LB_STATUS_ERROR_INVALID Invalid argument
 * @retval #LB_STATUS_ERROR_BUSY Previous operation is not finished yet
 * @retval #LB_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #LB_STATUS_SUCCESS Successfully sent
 * @see livebox_mouse_event()
 * @see livebox_access_event()
 */
extern int livebox_key_event(struct livebox *handler, enum content_event_type type, unsigned int keycode, ret_cb_t cb, void *data);

/**
 * @brief Sets pin-up status of the given handler.
 * @details If the livebox supports the pinup feature,
 *   you can freeze the update of the given livebox.
 *   But it is different from pause.
 *   The box will be updated and it will decide wheter update its content or not when the pinup is on.
 * @param[in] handler Handler of a livebox instance
 * @param[in] flag Pinup value
 * @param[in] cb Result callback
 * @param[in] data Callback data
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #LB_STATUS_ERROR_INVALID Invalid parameters
 * @see ret_cb_t
 * @see livebox_set_visibility()
 * @see livebox_is_pinned_up()
 */
extern int livebox_set_pinup(struct livebox *handler, int flag, ret_cb_t cb, void *data);

/**
 * @brief Checks the PIN-UP status of the given handler.
 * @param[in] handler Handler of a livebox instance
 * @privlevel N/P
 * @return int
 * @retval #LB_STATUS_ERROR_INVALID Invalid parameters
 * @retval 1 Box is pinned up
 * @retval 0 Box is not pinned up
 * @see livebox_set_pinup()
 */
extern int livebox_is_pinned_up(struct livebox *handler);

/**
 * @brief Checks the availability of the PIN-UP feature for the given handler.
 * @param[in] handler Handler of a livebox instance
 * @privlevel N/P
 * @return int
 * @retval #LB_STATUS_ERROR_INVALID Invalid argument
 * @retval 1 If the box support Pinup feature
 * @retval 0 If the box does not support the Pinup feature
 * @see livebox_is_pinned_up()
 * @see livebox_set_pinup()
 */
extern int livebox_has_pinup(struct livebox *handler);

/**
 * @brief Checks the existence of PD for the given handler.
 * @param[in] handler Handler of a livebox instance
 * @privlevel N/P
 * @return int
 * @retval #LB_STATUS_ERROR_INVALID Invalid argument
 * @retval 1 If the box support the PD
 * @retval 0 If the box has no PD
 */
extern int livebox_has_pd(struct livebox *handler);

/**
 * @brief Creates PD of the given handler.
 * @param[in] handler Handler of a livebox instance
 * @param[in] cb Result callback
 * @param[in] data Callback data
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #LB_STATUS_SUCCESS Successfully done
 * @retval #LB_STATUS_ERROR_INVALID Invalid argument
 * @retval #LB_STATUS_ERROR_BUSY Previous operation is not finished yet
 * @retval #LB_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @see ret_cb_t
 * @see livebox_create_pd_with_position()
 * @see livebox_move_pd()
 * @see livebox_destroy_pd()
 */
extern int livebox_create_pd(struct livebox *handler, ret_cb_t cb, void *data);

/**
 * @brief Creates PD of the given handler with the relative position from livebox.
 * @param[in] handler Handler of a livebox instance
 * @param[in] x 0.0 ~ 1.0
 * @param[in] y 0.0 ~ 1.0
 * @param[in] cb Result callback
 * @param[in] data Callback data
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #LB_STATUS_SUCCESS Successfully done
 * @retval #LB_STATUS_ERROR_INVALID Invalid argument
 * @retval #LB_STATUS_ERROR_BUSY Previous operation is not finished yet
 * @retval #LB_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @see livebox_create_pd()
 * @see livebox_destroy_pd()
 * @see livebox_move_pd()
 */
extern int livebox_create_pd_with_position(struct livebox *handler, double x, double y, ret_cb_t cb, void *data);

/**
 * @brief Updates a position of the given PD.
 * @param[in] handler Handler of a livebox instance
 * @param[in] x 0.0 ~ 1.0
 * @param[in] y 0.0 ~ 1.0
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #LB_STATUS_SUCCESS If sending a request for updating position of the PD has been done successfully
 * @retval #LB_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #LB_STATUS_ERROR_INVALID Invalid argument
 */
extern int livebox_move_pd(struct livebox *handler, double x, double y);

/**
 * @brief Destroys the PD of the given handler if it is created.
 * @param[in] handler Handler of a livebox instance
 * @param[in] cb
 * @param[in] data
 * @privlevel platform
 * @privilege %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #LB_STATUS_ERROR_INVALID Invalid argument
 * @retval #LB_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #LB_STATUS_SUCCESS Successfully done
 * @see ret_cb_t
 */
extern int livebox_destroy_pd(struct livebox *handler, ret_cb_t cb, void *data);

/**
 * @brief Checks the create status of the given livebox handler.
 * @param[in] handler Handler of a livebox instance
 * @privlevel N/P
 * @return int
 * @retval #LB_STATUS_ERROR_INVALID Invalid argument
 * @retval 0 PD is not created
 * @retval 1 PD is created
 */
extern int livebox_pd_is_created(struct livebox *handler);

/**
 * @brief Checks the content type of a progressive disclosure of the given handler.
 * @param[in] handler Handler of a livebox instance
 * @privlevel N/P
 * @return int
 * @retval #PD_TYPE_BUFFER Contents of a PD is based on canvas buffer(shared)
 * @retval #PD_TYPE_TEXT Contents of a PD is based on formatted text file
 * @retval #PD_TYPE_PIXMAP Contents of a livebox is shared by the pixmap(depends on X)
 * @retval #PD_TYPE_INVALID
 * @see livebox_pd_type()
 */
extern enum livebox_pd_type livebox_pd_type(struct livebox *handler);

/**
 * @brief Checks the existence of a livebox about the given package name.
 * @param[in] pkgname Package name
 * @privlevel platform
 * @privileve %http://developer.samsung.com/privilege/core/dynamicbox.info
 * @return int
 * @retval 1 If the box exists
 * @retval 0 If the box does not exist
 */
extern int livebox_is_exists(const char *pkgname);

/**
 * @brief Sets a function table for parsing the text content of a livebox.
 * @param[in] handler Handler of a livebox instance
 * @param[in] ops
 * @privlevel N/P
 * @return int
 * @retval #LB_STATUS_SUCCESS Successfully done
 * @retval #LB_STATUS_ERROR_INVALID Invalid argument
 * @see livebox_set_pd_text_handler()
 */
extern int livebox_set_text_handler(struct livebox *handler, struct livebox_script_operators *ops);

/**
 * @brief Sets a function table for parsing the text content of a Progressive Disclosure.
 * @param[in] handler Handler of a livebox instance
 * @param[in] ops
 * @privlevel N/P
 * @return int
 * @retval #LB_STATUS_SUCCESS Successfully done
 * @retval #LB_STATUS_ERROR_INVALID Invalid argument
 * @see livebox_set_text_handler()
 */
extern int livebox_set_pd_text_handler(struct livebox *handler, struct livebox_script_operators *ops);

/**
 * @brief Emits a text signal to the given livebox only if it is a text type.
 * @param[in] handler Handler of a livebox instance
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
 * @retval #LB_STATUS_ERROR_INVALID Invalid parameters
 * @retval #LB_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #LB_STATUS_SUCCESS Successfully emitted
 * @see ret_cb_t
 */
extern int livebox_emit_text_signal(struct livebox *handler, const char *emission, const char *source, double sx, double sy, double ex, double ey, ret_cb_t cb, void *data);

/**
 * @brief Sets a private data pointer to carry it using the given handler.
 * @param[in] handler Handler of a livebox instance
 * @param[in] data Data pointer
 * @privlevel N/P
 * @return int
 * @retval #LB_STATUS_SUCCESS Successfully registered
 * @retval #LB_STATUS_ERROR_INVALID Invalid argument
 * @see livebox_get_data()
 */
extern int livebox_set_data(struct livebox *handler, void *data);

/**
 * @brief Gets a private data pointer which is carried by a given handler.
 * @param[in] handler Handler of a livebox instance
 * @privlevel N/P
 * @return void *
 * @retval data Data pointer
 * @retval @c NULL If there is no data
 * @see livebox_set_data()
 */
extern void *livebox_get_data(struct livebox *handler);

/**
 * @brief Subscribes an event for liveboxes only in a given cluster and sub-cluster.
 * @details If you wrote a view-only client,
 *   you can receive the event of specific liveboxes which belong to a given cluster/category.
 *   But you cannot modify their attributes (such as size, ...).
 * @param[in] cluster   Cluster ("*" can be used for subscribe all cluster's liveboxes event; If you use the "*", value in the category will be ignored)
 * @param[in] category Category ("*" can be used for subscribe liveboxes events of all category(sub-cluster) in a given "cluster")
 * @privlevel platform
 * @privileve %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #LB_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #LB_STATUS_SUCCESS Successfully requested
 * @see livebox_unsubscribe_group()
 */
extern int livebox_subscribe_group(const char *cluster, const char *category);

/**
 * @brief Unsubscribes an event for the liveboxes, but you will receive already added liveboxes events.
 * @param[in] cluster Cluster("*" can be used for subscribe all cluster's liveboxes event; If you use the "*", value in the category will be ignored)
 * @param[in] category Category ("*" can be used for subscribe all sub-cluster's liveboxes event in a given "cluster")
 * @privlevel platform
 * @privileve %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #LB_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #LB_STATUS_SUCCESS Successfully requested
 * @see livebox_subscribe_group()
 */
extern int livebox_unsubscribe_group(const char *cluster, const char *category);

/**
 * @brief Refreshes the group (cluster/sub-cluser (aka. category)).
 * @details This function will trigger the update of all liveboxes in a given cluster/category group.
 * @remarks Basically, a default livebox system doesn't use the cluster/category concept.
 *    But you can use it. So if you decide to use it, then you can trigger the update of all liveboxes in the given group.
 * @param[in] cluster Cluster ID
 * @param[in] category Sub-cluster ID
 * @param[in] force 1 if the boxes should be updated even if they are paused
 * @privlevel platform
 * @privileve %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #LB_STATUS_ERROR_INVALID Invalid argument
 * @retval #LB_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #LB_STATUS_SUCCESS Successfully requested
 * @see livebox_refresh()
 */
extern int livebox_refresh_group(const char *cluster, const char *category, int force);

/**
 * @brief Refreshes a livebox.
 * @param[in] handler Handler of a livebox instance
 * @param[in] force 1 if the box should be updated even if it is paused
 * @privlevel platform
 * @privileve %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #LB_STATUS_ERROR_INVALID Invalid argument
 * @retval #LB_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #LB_STATUS_SUCCESS Successfully requested
 * @see livebox_refresh_group()
 */
extern int livebox_refresh(struct livebox *handler, int force);

/**
 * @brief Gets Pixmap Id of a livebox content.
 * @details This function doesn't guarantee the life-cycle of the pixmap.
 *   If the service provider destroyed the pixmap, you will not know about it.
 *   So you should validate it before accessing it.
 * @param[in] handler Handler of a livebox instance
 * @privlevel N/P
 * @return int
 * @retval 0 If the pixmap is not created
 * @retval pixmap Pixmap Id need to be casted to (unsigned int) type
 * @see livebox_pd_pixmap()
 */
extern int livebox_lb_pixmap(const struct livebox *handler);

/**
 * @brief Gets Pixmap Id of a PD content.
 * @details This function doesn't guarantee the life-cycle of the pixmap.
 *   If the service provider destroyed the pixmap, you will not know about it.
 *   So you should validate it before accessing it.
 * @param[in] handler Handler of a livebox instance
 * @privlevel N/P
 * @return int
 * @retval 0 If the pixmap is not created
 * @retval pixmap Pixmap Id need to be casted to (unsigned int) type
 * @see livebox_lb_pixmap()
 */
extern int livebox_pd_pixmap(const struct livebox *handler);

/**
 * @brief Acquires the pixmap of PD.
 * @details After acquiring the pixmap of PD, it will not be destroyed.
 *   So if the new update is comming with a new pixmap Id, you should release old pixmap manually.
 * @param[in] handler Handler of a livebox instance
 * @param[in] cb Result callback for acquiring request
 * @param[in] data Callback Data
 * @privlevel platform
 * @privileve %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #LB_STATUS_ERROR_INVALID Invalid argument
 * @retval #LB_STATUS_ERROR_FAULT Failed to send a request to the service provider or there is critical error that is unrecoverable
 * @retval #LB_STATUS_SUCCESS Successfully requested to acquire the pixmap of PD
 * @see livebox_release_pd_pixmap()
 * @see livebox_acquire_lb_pixmap()
 * @see ret_cb_t
 */
extern int livebox_acquire_pd_pixmap(struct livebox *handler, ret_cb_t cb, void *data);

/**
 * @brief Releases the acquired pixmap ID.
 * @param[in] handler Handler of a livebox instance
 * @param[in] pixmap Pixmap Id to release it
 * @privlevel platform
 * @privileve %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #LB_STATUS_ERROR_INVALID Invalid argument
 * @retval #LB_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #LB_STATUS_SUCCESS Successfully released (request is sent)
 * @see livebox_acquire_pd_pixmap()
 * @see livebox_release_lb_pixmap()
 */
extern int livebox_release_pd_pixmap(struct livebox *handler, int pixmap);

/**
 * @brief Gets the PIXMAP of a livebox.
 * @details Even if a render process releases the pixmap, the pixmap will be kept before being released by livebox_release_lb_pixmap.
 *   You should release the pixmap manually.
 * @param[in] handler Handler of a livebox instance
 * @param[in] cb Callback function which will be called with result of acquiring lb pixmap
 * @param[in] data Callback data
 * @privlevel platform
 * @privileve %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #LB_STATUS_ERROR_INVALID Invalid argument
 * @retval #LB_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #LB_STATUS_SUCCESS Successfully requested
 * @pre Livebox service system should support the PIXMAP type buffer.
 *   The livebox should be designed to use the buffer (script type).
 * @see livebox_release_lb_pixmap()
 * @see livebox_acquire_pd_pixmap()
 * @see ret_cb_t
 */
extern int livebox_acquire_lb_pixmap(struct livebox *handler, ret_cb_t cb, void *data);

/**
 * @brief Releases the pixmap of a livebox.
 * @details After a client gets a new pixmap or does not need to keep the current pixmap anymore, use this function to release it.
 *   If there is no user for a given pixmap, the pixmap will be destroyed.
 * @param[in] handler Handler of a livebox instance
 * @param[in] pixmap Pixmap Id of given livebox handler
 * @privlevel platform
 * @privileve %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #LB_STATUS_ERROR_INVALID Invalid argument
 * @retval #LB_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #LB_STATUS_SUCCESS Successfully done
 * @pre The pixmap should be acquired by livebox_acquire_lb_pixmap
 * @see livebox_acquire_lb_pixmap()
 * @see livebox_release_pd_pixmap()
 */
extern int livebox_release_lb_pixmap(struct livebox *handler, int pixmap);

/**
 * @brief Updates a visible state of the livebox.
 * @param[in] handler Handler of a livebox instance
 * @param[in] state Configure the current visible state of a livebox
 * @privlevel platform
 * @privileve %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #LB_STATUS_ERROR_INVALID Invalid argument
 * @retval #LB_STATUS_ERROR_BUSY
 * @retval #LB_STATUS_ERROR_PERMISSION
 * @retval #LB_STATUS_ERROR_ALREADY
 * @retval #LB_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #LB_STATUS_SUCCESS Successfully done
 */
extern int livebox_set_visibility(struct livebox *handler, enum livebox_visible_state state);

/**
 * @brief Gets the current visible state of a livebox.
 * @param[in] handler Handler of a livebox instance
 * @privlevel N/P
 * @return livebox_visible_state
 * @retval #LB_SHOW Livebox is shown (Default state)
 * @retval #LB_HIDE Livebox is hidden, Update timer is not frozen (but a user cannot receive any updated events; a user should refresh(reload) the content of a livebox when a user make this show again)
 * @retval #LB_HIDE_WITH_PAUSE Livebox is hidden, it will pause the update timer, but if a livebox updates its contents, update event will occur
 * @retval #LB_VISIBLE_ERROR To enlarge the size of this enumeration type
 */
extern enum livebox_visible_state livebox_visibility(struct livebox *handler);

/**
 * @brief Sets an update mode of the current livebox.
 * @details If you set 1 for active update mode, you should get a buffer without updated event from provider.
 *   But if it is passive mode, you have to update content of a box when you get updated events.
 *   Default is Passive mode.
 * @param[in] handler Handler of a livebox instance
 * @param[in] active_update 1 means active update, 0 means passive update (default)
 * @param[in] cb Result callback function
 * @param[in] data Callback data
 * @privlevel platform
 * @privileve %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #LB_STATUS_ERROR_INVALID Invalid argument
 * @retval #LB_STATUS_ERROR_BUSY
 * @retval #LB_STATUS_ERROR_PERMISSION
 * @retval #LB_STATUS_ERROR_ALREADY
 * @retval #LB_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #LB_STATUS_SUCCESS Successfully done
 * @see ret_cb_t
 */
extern int livebox_set_update_mode(struct livebox *handler, int active_update, ret_cb_t cb, void *data);

/**
 * @brief Checks the active update mode of the given livebox.
 * @param[in] handler Handler of a livebox instance
 * @privlevel N/P
 * @return int
 * @retval 0 If passive mode
 * @retval 1 If active mode or error code
 */
extern int livebox_is_active_update(struct livebox *handler);

/**
 * @brief Syncs manually
 * param[in] handler Handler of a livebox instance
 * @privlevel platform
 * @privileve %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return void
 * @retval #LB_STATUS_SUCCESS If success
 * @retval #LB_STATUS_ERROR_INVALID Invalid handle
 * @see livebox_set_manual_sync()
 * @see livebox_manual_sync()
 * @see livebox_sync_lb_fb()
 */
extern int livebox_sync_pd_fb(struct livebox *handler);

/**
 * @brief Syncs manually
 * @param[in] handler Handler of a livebox instance
 * @privlevel platform
 * @privileve %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return void
 * @retval #LB_STATUS_SUCCESS If success
 * @retval #LB_STATUS_ERROR_INVALID Invalid handle
 * @see livebox_set_manual_sync()
 * @see livebox_manual_sync()
 * @see livebox_sync_pd_fb()
 */
extern int livebox_sync_lb_fb(struct livebox *handler);

/**
 * @brief Gets an alternative icon of the given livebox instance.
 * @details If the box should be represented as a shortcut icon, this function will get the alternative icon.
 * @param[in] handler Handler of a livebox instance
 * @privlevel N/P
 * @return const char *
 * @retval address Absolute path of an alternative icon file
 * @retval @c NULL Livebox has no alternative icon file
 * @see livebox_alt_name()
 */
extern const char *livebox_alt_icon(struct livebox *handler);

/**
 * @brief Gets an alternative name of the given livebox instance.
 * @details If the box should be represented as a shortcut name, this function will get the alternative name.
 * @param[in] handler Handler of a livebox instance
 * @privlevel N/P
 * @return const char *
 * @retval name Alternative name of a livebox
 * @retval @c NULL Livebox has no alternative name
 * @see livebox_alt_icon()
 */
extern const char *livebox_alt_name(struct livebox *handler);

/**
 * @brief Gets a lock for a frame buffer.
 * @details This function should be used to prevent from rendering to the frame buffer while reading it.
 *   And the locking area should be short and must be released ASAP, or the render thread will be hanged.
 * @param[in] handler Handler of a livebox instance
 * @param[in] is_pd 1 for PD or 0
 * @privlevel platform
 * @privileve %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #LB_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #LB_STATUS_ERROR_INVALID Invalid argument
 * @retval #LB_STATUS_SUCCESS Successfully done
 * @see livebox_release_fb_lock()
 */
extern int livebox_acquire_fb_lock(struct livebox *handler, int is_pd);

/**
 * @brief Releases a lock of the frame buffer.
 * @details This function should be called ASAP after acquiring a lock of FB, or the render process will be blocked.
 * @param[in] handler Handler of a livebox instance
 * @param[in] is_pd 1 for PD or 0
 * @privlevel platform
 * @privileve %http://developer.samsung.com/privilege/core/dynamicbox.viewer
 * @return int
 * @retval #LB_STATUS_ERROR_FAULT Unrecoverable error occurred
 * @retval #LB_STATUS_ERROR_INVALID Invalid argument
 * @retval #LB_STATUS_SUCCESS Successfully done
 * @see livebox_acquire_fb_lock()
 */
extern int livebox_release_fb_lock(struct livebox *handler, int is_pd);

/**
 * @brief Sets options for controlling a livebox sub-system.
 * @details
 *   LB_OPTION_FRAME_DROP_FOR_RESIZE
 *     While resizing the box, viewer doesn't want to know the updated frames of an old size content anymore.
 *     In that case, turn this on, the provider will not send the updated event to the viewer about an old content.
 *     So the viewer can reduce its burden to update unnecessary frames.
 *   LB_OPTION_MANUAL_SYNC
 *     If you don't want to update frames automatically, or you want only reload the frames by your hands, (manually)
 *     Turn this on.
 *     After turnning it on, you should sync it using livebox_sync_pd_fb and livebox_sync_lb_pfb.
 *   LB_OPTION_SHARED_CONTENT
 *     If this option is turnned on, even though you create a new livebox,
 *     if there are already added same instances that have same contents, the instance will not be created again.
 *     Instead of creating a new instance, a viewer will provide an old instance with a new handle.
 * @param[in] option Option which will be affected by this call
 * @param[in] state New value for given option
 * @privlevel N/P
 * @return int
 * @retval #LB_STATUS_ERROR_INVALID Unknown option
 * @retval #LB_STATUS_ERROR_FAULT Failed to change the state of option
 * @retval #LB_STATUS_SUCCESS Successfully changed
 * @see livebox_get_option()
 * @see livebox_sync_pd_fb()
 * @see livebox_sync_lb_fb()
 */
extern int livebox_set_option(enum livebox_option_type option, int state);

/**
 * @brief Gets options of a livebox sub-system.
 * @param[in] option Type of option
 * @privlevel N/P
 * @return int
 * @retval #LB_STATUS_ERROR_INVALID Invalid option
 * @retval #LB_STATUS_ERROR_FAULT Failed to get option
 * @retval >=0 Value of given option (must be >=0)
 * @see livebox_set_option()
 */
extern int livebox_option(enum livebox_option_type option);


/**
 * @brief Set a handler for launching an app for auto-launch feature
 * @details If a user clicks a box, and the box uses auto-launch option, the launcher_handler will be called.
 * @param[in] launch_handler Handler for launching an app manually
 * @param[in] data Callback data which will be given a data for launch_handler
 * @privlevel N/P
 * @return int type
 * @retval #LB_STATUS_SUCCESS Succeed to set new handler. there is no other cases
 */
extern int livebox_set_auto_launch_handler(int (*launch_handler)(struct livebox *handler, const char *appid, void *data), void *data);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
