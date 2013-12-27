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

/*!
 * \addtogroup CAPI_LIVEBOX_VIEWER_MODULE
 * \{
 */

/*!
 * \brief
 * Structure for a Livebox instance
 */
struct livebox;

/*!
 * \brief
 * Use the default update period which is defined in the package manifest file of a livebox.
 */
#define DEFAULT_PERIOD -1.0f

/*!
 * \brief
 * Mouse & Key event for buffer type Livebox or PD
 * Viewer should send these events to livebox.
 */
enum content_event_type {
	CONTENT_EVENT_MOUSE_DOWN	= 0x00000001, /*!< LB mouse down event for livebox */
	CONTENT_EVENT_MOUSE_UP		= 0x00000002, /*!< LB mouse up event for livebox */
	CONTENT_EVENT_MOUSE_MOVE	= 0x00000004, /*!< LB mouse move event for livebox */
	CONTENT_EVENT_MOUSE_ENTER	= 0x00000008, /*!< LB mouse enter event for livebox */
	CONTENT_EVENT_MOUSE_LEAVE	= 0x00000010, /*!< LB mouse leave event for livebox */
	CONTENT_EVENT_MOUSE_SET		= 0x00000020, /*!< LB mouse set auto event for livebox */
	CONTENT_EVENT_MOUSE_UNSET	= 0x00000040, /*!< LB mouse unset auto event for livebox */

	CONTENT_EVENT_KEY_DOWN		= 0x00000001, /*!< LB key press */
	CONTENT_EVENT_KEY_UP		= 0x00000002, /*!< LB key release */
	CONTENT_EVENT_KEY_FOCUS_IN	= 0x00000008, /*!< LB key focused in */
	CONTENT_EVENT_KEY_FOCUS_OUT	= 0x00000010, /*!< LB key focused out */
	CONTENT_EVENT_KEY_SET		= 0x00000020, /*!< LB Key, start feeding event by master */
	CONTENT_EVENT_KEY_UNSET		= 0x00000040, /*!< LB key, stop feeding event by master */

	CONTENT_EVENT_KEY_MASK		= 0x80000000,
	CONTENT_EVENT_MOUSE_MASK	= 0x20000000,
	CONTENT_EVENT_PD_MASK		= 0x10000000,
	CONTENT_EVENT_LB_MASK		= 0x40000000,

	LB_MOUSE_DOWN			= CONTENT_EVENT_LB_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_DOWN, /*!< Mouse down on the livebox */
	LB_MOUSE_UP			= CONTENT_EVENT_LB_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_UP, /*!< Mouse up on the livebox */
	LB_MOUSE_MOVE			= CONTENT_EVENT_LB_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_MOVE, /*!< Move move on the livebox */
	LB_MOUSE_ENTER			= CONTENT_EVENT_LB_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_ENTER, /*!< Mouse enter to the livebox */
	LB_MOUSE_LEAVE			= CONTENT_EVENT_LB_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_LEAVE, /*!< Mouse leave from the livebox */
	LB_MOUSE_SET			= CONTENT_EVENT_LB_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_SET, /*!< Mouse event, start feeding event by master */
	LB_MOUSE_UNSET			= CONTENT_EVENT_LB_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_UNSET, /*!< Mouse event, stop feeding event by master */

	PD_MOUSE_DOWN			= CONTENT_EVENT_PD_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_DOWN, /*!< Mouse down on the PD */
	PD_MOUSE_UP			= CONTENT_EVENT_PD_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_UP, /*!< Mouse up on the PD */
	PD_MOUSE_MOVE			= CONTENT_EVENT_PD_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_MOVE, /*!< Mouse move on the PD */
	PD_MOUSE_ENTER			= CONTENT_EVENT_PD_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_ENTER, /*!< Mouse enter to the PD */
	PD_MOUSE_LEAVE			= CONTENT_EVENT_PD_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_LEAVE, /*!< Mouse leave from the PD */
	PD_MOUSE_SET			= CONTENT_EVENT_PD_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_SET, /*!< Mouse event, start feeding event by master */
	PD_MOUSE_UNSET			= CONTENT_EVENT_PD_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_UNSET, /*!< Mouse event, stop feeding event by master */

	LB_KEY_DOWN			= CONTENT_EVENT_LB_MASK | CONTENT_EVENT_KEY_MASK | CONTENT_EVENT_KEY_DOWN, /*!< Key down on the livebox */
	LB_KEY_UP			= CONTENT_EVENT_LB_MASK | CONTENT_EVENT_KEY_MASK | CONTENT_EVENT_KEY_UP, /*!< Key up on the livebox */
	LB_KEY_SET			= CONTENT_EVENT_LB_MASK | CONTENT_EVENT_KEY_MASK | CONTENT_EVENT_KEY_SET, /*!< Key event, start feeding event by master */
	LB_KEY_UNSET			= CONTENT_EVENT_LB_MASK | CONTENT_EVENT_KEY_MASK | CONTENT_EVENT_KEY_UNSET, /*!< Key event, stop feeding event by master */
	LB_KEY_FOCUS_IN			= CONTENT_EVENT_LB_MASK | CONTENT_EVENT_KEY_MASK | CONTENT_EVENT_KEY_FOCUS_IN, /*!< Key event, focus in */
	LB_KEY_FOCUS_OUT		= CONTENT_EVENT_LB_MASK | CONTENT_EVENT_KEY_MASK | CONTENT_EVENT_KEY_FOCUS_OUT, /*!< Key event, foucs out */

	PD_KEY_DOWN			= CONTENT_EVENT_PD_MASK | CONTENT_EVENT_KEY_MASK | CONTENT_EVENT_KEY_DOWN, /*!< Key down on the livebox */
	PD_KEY_UP			= CONTENT_EVENT_PD_MASK | CONTENT_EVENT_KEY_MASK | CONTENT_EVENT_KEY_UP, /*!< Key up on the livebox */
	PD_KEY_SET			= CONTENT_EVENT_PD_MASK | CONTENT_EVENT_KEY_MASK | CONTENT_EVENT_KEY_SET, /*!< Key event, start feeding event by master */
	PD_KEY_UNSET			= CONTENT_EVENT_PD_MASK | CONTENT_EVENT_KEY_MASK | CONTENT_EVENT_KEY_UNSET, /*!< Key event, stop feeding event by master */
	PD_KEY_FOCUS_IN			= CONTENT_EVENT_PD_MASK | CONTENT_EVENT_KEY_MASK | CONTENT_EVENT_KEY_FOCUS_IN, /*!< Key event, focus in */
	PD_KEY_FOCUS_OUT		= CONTENT_EVENT_PD_MASK | CONTENT_EVENT_KEY_MASK | CONTENT_EVENT_KEY_FOCUS_OUT, /*!< Key event, focus out */

	CONTENT_EVENT_MAX		= 0xFFFFFFFF
};

/*!
 * \brief
 * Accessibility event for buffer type Livebox or PD.
 * These event set are sync'd with Tizen accessibility event set.
 */
enum access_event_type {
	ACCESS_EVENT_PD_MASK		= 0x10000000,
	ACCESS_EVENT_LB_MASK		= 0x20000000,

	ACCESS_EVENT_HIGHLIGHT		= 0x00000100, /*!< LB accessibility: Hightlight a object */
	ACCESS_EVENT_HIGHLIGHT_NEXT	= 0x00000200, /*!< LB accessibility: Set highlight to next object */
	ACCESS_EVENT_HIGHLIGHT_PREV	= 0x00000400, /*!< LB accessibility: Set highlight to prev object */
	ACCESS_EVENT_UNHIGHLIGHT	= 0x00000800, /*!< LB accessibility unhighlight */
	ACCESS_EVENT_ACTIVATE		= 0x00001000, /*!< LB accessibility activate */
	ACCESS_EVENT_ACTION_DOWN	= 0x00010000, /*!< LB accessibility value changed */
	ACCESS_EVENT_ACTION_UP		= 0x00020000, /*!< LB accessibility value changed */
	ACCESS_EVENT_SCROLL_DOWN	= 0x00100000, /*!< LB accessibility scroll down */
	ACCESS_EVENT_SCROLL_MOVE	= 0x00200000, /*!< LB accessibility scroll move */
	ACCESS_EVENT_SCROLL_UP		= 0x00400000, /*!< LB accessibility scroll up */

	LB_ACCESS_HIGHLIGHT		= ACCESS_EVENT_LB_MASK | ACCESS_EVENT_HIGHLIGHT,	/*!< Access event - Highlight an object in the livebox */
	LB_ACCESS_HIGHLIGHT_NEXT	= ACCESS_EVENT_LB_MASK | ACCESS_EVENT_HIGHLIGHT_NEXT,	/*!< Access event - Move highlight to the next object in a livebox */
	LB_ACCESS_HIGHLIGHT_PREV	= ACCESS_EVENT_LB_MASK | ACCESS_EVENT_HIGHLIGHT_PREV,	/*!< Access event - Move highlight to the prev object in a livebox */
	LB_ACCESS_UNHIGHLIGHT		= ACCESS_EVENT_LB_MASK | ACCESS_EVENT_UNHIGHLIGHT,	/*!< Access event - Delete highlight from the livebox */
	LB_ACCESS_ACTIVATE		= ACCESS_EVENT_LB_MASK | ACCESS_EVENT_ACTIVATE,		/*!< Access event - Launch or activate the highlighted object */
	LB_ACCESS_ACTION_DOWN		= ACCESS_EVENT_LB_MASK | ACCESS_EVENT_ACTION_DOWN,	/*!< Access event - down */
	LB_ACCESS_ACTION_UP		= ACCESS_EVENT_LB_MASK | ACCESS_EVENT_ACTION_UP,	/*!< Access event - up */
	LB_ACCESS_SCROLL_DOWN		= ACCESS_EVENT_LB_MASK | ACCESS_EVENT_SCROLL_DOWN,	/*!< Access event - scroll down */
	LB_ACCESS_SCROLL_MOVE		= ACCESS_EVENT_LB_MASK | ACCESS_EVENT_SCROLL_MOVE,	/*!< Access event - scroll move */
	LB_ACCESS_SCROLL_UP		= ACCESS_EVENT_LB_MASK | ACCESS_EVENT_SCROLL_UP,	/*!< Access event - scroll up */

	PD_ACCESS_HIGHLIGHT		= ACCESS_EVENT_PD_MASK | ACCESS_EVENT_HIGHLIGHT,	/*!< Access event - Highlight an object in the PD */
	PD_ACCESS_HIGHLIGHT_NEXT	= ACCESS_EVENT_PD_MASK | ACCESS_EVENT_HIGHLIGHT_NEXT,	/*!< Access event - Move highlight to the next object in a PD */
	PD_ACCESS_HIGHLIGHT_PREV	= ACCESS_EVENT_PD_MASK | ACCESS_EVENT_HIGHLIGHT_PREV,	/*!< Access event - Move highlight to the prev object in a PD */
	PD_ACCESS_UNHIGHLIGHT		= ACCESS_EVENT_PD_MASK | ACCESS_EVENT_UNHIGHLIGHT,	/*!< Access event - Delet highlight from the PD */
	PD_ACCESS_ACTIVATE		= ACCESS_EVENT_PD_MASK | ACCESS_EVENT_ACTIVATE,		/*!< Access event - Launch or activate the highlighted object */
	PD_ACCESS_ACTION_DOWN		= ACCESS_EVENT_PD_MASK | ACCESS_EVENT_ACTION_DOWN,	/*!< Access event - down */
	PD_ACCESS_ACTION_UP		= ACCESS_EVENT_PD_MASK | ACCESS_EVENT_ACTION_UP,	/*!< Access event - up */
	PD_ACCESS_SCROLL_DOWN		= ACCESS_EVENT_PD_MASK | ACCESS_EVENT_SCROLL_DOWN,	/*!< Access event - scroll down */
	PD_ACCESS_SCROLL_MOVE		= ACCESS_EVENT_PD_MASK | ACCESS_EVENT_SCROLL_MOVE,	/*!< Access event - scroll move */
	PD_ACCESS_SCROLL_UP		= ACCESS_EVENT_PD_MASK | ACCESS_EVENT_SCROLL_UP		/*!< Access event - scroll up */
};

/*!
 * \brief
 * Livebox LB content type
 */
enum livebox_lb_type {
	LB_TYPE_IMAGE = 0x01, /*!< Contents of a livebox is based on the image file */
	LB_TYPE_BUFFER = 0x02, /*!< Contents of a livebox is based on canvas buffer(shared) */
	LB_TYPE_TEXT = 0x04, /*!< Contents of a livebox is based on formatted text file */
	LB_TYPE_PIXMAP = 0x08, /*!< Contens of a livebox is shared by the pixmap(depends on X) */

	LB_TYPE_INVALID = 0xFF
};

/*!
 * \brief
 * Livebox PD content type
 */
enum livebox_pd_type {
	PD_TYPE_BUFFER = 0x01, /*!< Contents of a PD is based on canvas buffer(shared) */
	PD_TYPE_TEXT = 0x02, /*!< Contents of a PD is based on formatted text file */
	PD_TYPE_PIXMAP = 0x04, /*!< Contents of a livebox is shared by the pixmap(depends on X) */

	PD_TYPE_INVALID = 0xFF
};

/*!
 * \brief
 * Livebox event type.
 * These event will be sent from the provider.
 */
enum livebox_event_type { /*!< livebox_event_handler_set Event list */
	LB_EVENT_LB_UPDATED, /*!< Contents of the given livebox is updated */
	LB_EVENT_PD_UPDATED, /*!< Contents of the given pd is updated */

	LB_EVENT_CREATED, /*!< A new livebox is created */
	LB_EVENT_DELETED, /*!< A livebox is deleted */

	LB_EVENT_GROUP_CHANGED, /*!< Group (Cluster/Sub-cluster) information is changed */
	LB_EVENT_PINUP_CHANGED, /*!< PINUP status is changed */
	LB_EVENT_PERIOD_CHANGED, /*!< Update period is changed */

	LB_EVENT_LB_SIZE_CHANGED, /*!< Livebox size is changed */
	LB_EVENT_PD_SIZE_CHANGED, /*!< PD size is changed */

	LB_EVENT_PD_CREATED, /*!< If a PD is created even if you didn't call the livebox_create_pd API */
	LB_EVENT_PD_DESTROYED, /*!< If a PD is destroyed even if you didn't call the livebox_destroy_pd API */

	LB_EVENT_HOLD_SCROLL, /*!< If the screen should be freezed */
	LB_EVENT_RELEASE_SCROLL, /*!< If the screen can be scrolled */

	LB_EVENT_LB_UPDATE_BEGIN, /*!< Livebox LB content update is started */
	LB_EVENT_LB_UPDATE_END, /*!< Livebox LB content update is finished */

	LB_EVENT_PD_UPDATE_BEGIN, /*!< Livebox PD content update is started */
	LB_EVENT_PD_UPDATE_END, /*!< Livebox PD content update is finished */

	LB_EVENT_UPDATE_MODE_CHANGED, /*!< Livebox Update mode is changed */

	LB_EVENT_REQUEST_CLOSE_PD, /*!< Livebox requests to close the PD */

	LB_EVENT_IGNORED /*!< Request is ignored */
};

enum livebox_fault_type {
	LB_FAULT_DEACTIVATED, /*!< Livebox is deactivated by its fault operation */
	LB_FAULT_PROVIDER_DISCONNECTED /*!< Provider is disconnected */
};

/*!
 * \brief
 * Must be sync'd with the provider
 */
enum livebox_visible_state {
	LB_SHOW = 0x00, /*!< Livebox is showed. Default state */
	LB_HIDE = 0x01, /*!< Livebox is hide, Update timer is not be freezed. but you cannot receive any updates events. you should refresh(reload) the content of a livebox when you make this show again */

	LB_HIDE_WITH_PAUSE = 0x02, /*!< Livebix is hide, it will paused the update timer, but if a livebox update its contents, update event will come to you */

	LB_VISIBLE_ERROR = 0xFFFFFFFF /* To enlarge the size of this enumeration type */
};

/*!
 * \brief
 * TEXT type livebox contents handling opertators.
 */
struct livebox_script_operators {
	int (*update_begin)(struct livebox *handle); /*!< Content parser is started */
	int (*update_end)(struct livebox *handle); /*!< Content parser is stopped */

	/*!
	 * \brief
	 * Listed functions will be called when parser meets each typed component
	 */
	int (*update_text)(struct livebox *handle, const char *id, const char *part, const char *data); /*!< Update text content */
	int (*update_image)(struct livebox *handle, const char *id, const char *part, const char *data, const char *option); /*!< Update image content */
	int (*update_script)(struct livebox *handle, const char *id, const char *part, const char *file, const char *group); /*!< Update script content */
	int (*update_signal)(struct livebox *handle, const char *id, const char *emission, const char *signal); /*!< Update signal */
	int (*update_drag)(struct livebox *handle, const char *id, const char *part, double dx, double dy); /*!< Update drag info */
	int (*update_info_size)(struct livebox *handle, const char *id, int w, int h); /*!< Update content size */
	int (*update_info_category)(struct livebox *handle, const char *id, const char *category); /*!< Update content category info */
	int (*update_access)(struct livebox *handle, const char *id, const char *part, const char *text, const char *option); /*!< Update access information */
	int (*operate_access)(struct livebox *handle, const char *id, const char *part, const char *operation, const char *option); /*!< Update access operation */
};

/*!
 * \brief Prototype of the return callback of every async functions
 * \details N/A
 * \remarks N/A
 * \param[in] handle Handle of the livebox instance
 * \param[in] ret Result status of operation. LB_STATUS_XXX defined from liblivebox-service
 * \param[in] data data for result callback
 * \return void
 * \pre N/A
 * \post N/A
 * \see livebox_add
 * \see livebox_add_with_size
 * \see livebox_del
 * \see livebox_activate
 * \see livebox_resize
 * \see livebox_set_group
 * \see livebox_set_period
 * \see livebox_access_event
 * \see livebox_set_pinup
 * \see livebox_create_pd
 * \see livebox_create_pd_with_position
 * \see livebox_destroy_pd
 * \see livebox_emit_text_signal
 * \see livebox_acquire_pd_pixmap
 * \see livebox_acquire_lb_pixmap
 * \see livebox_set_update_mode
 */
typedef void (*ret_cb_t)(struct livebox *handle, int ret, void *data);

/*!
 * \brief Initialize the livebox system
 * \details N/A
 * \remarks
 *   This API uses get/setenv APIs.
 *   Those APIs are not thread-safe.
 *   So you have to consider to use the livebox_init_with_options instead of this if you are developing multi-threaded viewer.
 * \param[in] disp If you have X Display connection object already, you can re-use it. but you should care its life cycle.
 *                 It must be alive before call the livebox_fini
 * \return int
 * \retval LB_STATUS_SUCCESS if success
 * \pre N/A
 * \post N/A
 * \see livebox_fini
 * \see livebox_init_with_options
 */
extern int livebox_init(void *disp);

/*!
 * \brief Initialize the livebox system with some options
 * \details livebox_init function uses environment value to initiate some configurable values
 *          But some application doesn't want to use the env value.
 *          For them, this API will give a chance to set default options using given arguments
 * \remarks N/A
 * \param[in] disp if this display is NULL, the library will try to acquire a new connection with X
 * \param[in] prevent_overwrite If the content of image type livebox is updated, don't overwrite it(1) or overwrite old file(0)
 * \param[in] event_filter If the next event comes in this period, ignore it. It is too fast to processing it in time.
 * \param[in] use_thread Use the receive thread.
 * \return int Integer, Livebox status code
 * \retval LB_STATUS_SUCCESS if success
 * \pre N/A
 * \post N/A
 * \see livebox_init
 * \see livebox_fini
 */
extern int livebox_init_with_options(void *disp, int prevent_overwrite, double event_filter, int use_thread);

/*!
 * \brief Finalize the livebox system
 * \details N/A
 * \remarks N/A
 * \return int
 * \retval LB_STATUS_SUCCES if success
 * \retval LB_STATUS_ERROR_INVALID if livebox_init is not called.
 * \pre N/A
 * \post N/A
 * \see livebox_init
 * \see livebox_init_with_options
 */
extern int livebox_fini(void);

/*!
 * \brief Notify the status of client to the provider. "it is paused".
 * \details N/A
 * \remarks N/A
 * \return int
 * \retval LB_STATUS_SUCCESS if success
 * \retval LB_STATUS_ERROR_FAULT if it failed to send state(paused) info
 * \pre N/A
 * \post N/A
 * \see livebox_client_resumed
 */
extern int livebox_client_paused(void);

/*!
 * \brief Notify the status of client to the provider. "it is resumed".
 * \details N/A
 * \remarks N/A
 * \return int
 * \retval LB_STATUS_SUCCESS if success
 * \retval LB_STATUS_ERROR_FAULT if it failed to send state(resumed) info
 * \pre N/A
 * \post N/A
 * \see livebox_client_paused
 */
extern int livebox_client_resumed(void);

/*!
 * \brief Add a new livebox
 * \details N/A
 * \remarks
 *    Even though you get the livebox handle from return value of this function,
 *    it is not matured before return callback called.
 *    You have to use the handle after get the return callback with "ret == LB_STATUS_SUCCESS"
 * \param[in] pkgname Livebox Id
 * \param[in] content Will be passed to the livebox instance.
 * \param[in] cluster Main group
 * \param[in] category Sub group
 * \param[in] period Update period. if you set DEFAULT_PERIOD, the provider will use the default period which is described in the manifest.
 * \param[in] cb After send the request to the provider, its result will be passed
 * \param[in] data
 * \return handle
 * \retval NULL if it fails to add a new instance
 * \retval handle livebox handle
 * \pre N/A
 * \post
 * \see ret_cb_t
 * \see livebox_add_with_size
 */
extern struct livebox *livebox_add(const char *pkgname, const char *content, const char *cluster, const char *category, double period, ret_cb_t cb, void *data);

/*!
 * \brief Add a new livebox
 * \details
 * If the screen size if "1280x720"
 * Below size lists are used for default.
 * Or you can find the default sizes in pixel from /usr/share/data-provider-master/resolution.ini
 * Size types are defined from the liblivebox-service package. (livebox-service.h)
 *
 * Normal mode livebox
 * 1x1=175x175, LB_SIZE_TYPE_1x1
 * 2x1=354x175, LB_SIZE_TYPE_2x1
 * 2x2=354x354, LB_SIZE_TYPE_2x2
 * 4x1=712x175, LB_SIZE_TYPE_4x1
 * 4x2=712x354, LB_SIZE_TYPE_4x2
 * 4x4=712x712, LB_SIZE_TYPE_4x4
 *
 * Extended sizes
 * 4x3=712x533, LB_SIZE_TYPE_4x3
 * 4x5=712x891, LB_SIZE_TYPE_4x5
 * 4x6=712x1070, LB_SIZE_TYPE_4x6
 *
 * Easy mode livebox
 * 21x21=224x215, LB_SIZE_TYPE_EASY_1x1
 * 23x21=680x215, LB_SIZE_TYPE_EASY_3x1
 * 23x23=680x653, LB_SIZE_TYPE_EASY_3x3
 *
 * Special livebox
 * 0x0=720x1280, LB_SIZE_TYPE_0x0
 *
 * \remarks
 *    Even if you get the handle by return value of this function, it is not created instance.
 *    So you have to deal it as not initialized handle.
 *    Only it can be initialized after get the return callback with "ret == LB_STATUS_SUCCESS".
 * \param[in] pkgname Livebox Id
 * \param[in] content Will be passed to the livebox instance.
 * \param[in] cluster Main group
 * \param[in] category Sub group
 * \param[in] period DEFAULT_PERIOD can be used for this. this argument will be used to specify the period of update content of livebox.
 * \param[in] type Size type - which are defined from liblivebox-service package.
 * \param[in] cb After the request is sent to the master provider, this callback will be called.
 * \param[in] data This data will be passed to the callback.
 * \return handle
 * \retval handle Livebox handle but not yet initialized
 * \retval NULL if it fails to create a handle
 * \see ret_cb_t
 * \see livebox_add
 */
extern struct livebox *livebox_add_with_size(const char *pkgname, const char *content, const char *cluster, const char *category, double period, int type, ret_cb_t cb, void *data);

/*!
 * \brief Delete a livebox (deprecated)
 * \details N/A
 * \remarks
 *    If you call this with uninitialized handle, the return callback will be called synchronously.
 *    So before return from this function, the return callback will be called first.
 * \param[in] handler Handler of a livebox instance
 * \param[in] cb return callback
 * \param[in] data user data for return callback
 * \return int
 * \retval LB_STATUS_ERROR_INVALID Invalid argument
 * \retval LB_STATUS_ERROR_BUSY already in process
 * \retval LB_STATUS_ERROR_FAULT failed to create a request packet
 * \retval LB_STATUS_SUCCESS successfully sent, return callack will be called
 * \pre N/A
 * \post N/A
 * \see ret_cb_t
 */
extern int livebox_del(struct livebox *handler, ret_cb_t cb, void *data);

/*!
 * \brief Delete a livebox (will be replaced with livebox_del)
 * \details N/A
 * \remarks
 *    If you call this with uninitialized handle, the return callback will be called synchronously.
 *    So before return from this function, the return callback will be called first.
 * \param[in] handler Handler of a livebox instance
 * \param[in] type Deletion type, LB_DELETE_PERMANENTLY or LB_DELETE_TEMPORARY
 * \param[in] cb return callback
 * \param[in] data user data for return callback
 * \return int
 * \retval LB_STATUS_ERROR_INVALID Invalid argument
 * \retval LB_STATUS_ERROR_BUSY already in process
 * \retval LB_STATUS_ERROR_FAULT failed to create a request packet
 * \retval LB_STATUS_SUCCESS successfully sent, return callack will be called
 * \pre N/A
 * \post N/A
 * \see ret_cb_t
 */
extern int livebox_del_NEW(struct livebox *handler, int type, ret_cb_t cb, void *data);

/*!
 * \brief Set a livebox events callback
 * \details
 *    To get the events push from the provider, register the event callback using this function
 *    The callback will be called if there is any events from the provider.
 * \remarks N/A
 * \param[in] cb Event handler
 * \param[in] data User data for the event handler
 * \return int
 * \retval LB_STATUS_SUCCESS if succeed to set event handler
 * \retval LB_STATUS_ERROR_INVALID Invalid argument
 * \retval LB_STATUS_ERROR_MEMORY Not enough memory
 * \pre NULL
 * \post NULL
 * \see livebox_unset_event_handler
 */
extern int livebox_set_event_handler(int (*cb)(struct livebox *handler, enum livebox_event_type event, void *data), void *data);

/*!
 * \brief Unset the livebox event handler
 * \details N/A
 * \remarks N/A
 * \param[in] cb Event handler
 * \return void * Event handler data
 * \retval pointer of 'data' which is used with the livebox_set_event_handler
 * \pre N/A
 * \post N/A
 * \see livebox_set_event_handler
 */
extern void *livebox_unset_event_handler(int (*cb)(struct livebox *handler, enum livebox_event_type event, void *data));

/*!
 * \brief Live box fault event handler registeration function
 *   argument list
 * 	event, pkgname, filename, funcname
 * \details N/A
 * \remarks N/A
 * \param[in] cb Event handler
 * \param[in] data Event handler data
 * \return int
 * \retval LB_STATUS_SUCCESS if succeed to set fault event handler
 * \retval LB_STATUS_ERROR_INVALID Invalid argument
 * \retval LB_STATUS_ERROR_MEMORY Not enough memory
 * \pre N/A
 * \post N/A
 * \see livebox_unset_fault_handler
 */
extern int livebox_set_fault_handler(int (*cb)(enum livebox_fault_type, const char *, const char *, const char *, void *), void *data);

/*!
 * \brief Unset the live box fault event handler
 * \details N/A
 * \remarks N/A
 * \param[in] cb Event handler
 * \return void * Callback data which is set via livebox_set_fault_handler
 * \retval pointer of 'data' which is used with the livebox_set_fault_handler
 * \pre N/A
 * \post N/A
 * \see livebox_set_fault_handler
 */
extern void *livebox_unset_fault_handler(int (*cb)(enum livebox_fault_type, const char *, const char *, const char *, void *));

/*!
 * \brief Activate the faulted livebox.
 * \details
 *    Request result will be back via return callback.
 * \remarks
 *    Even though this function returns SUCCESS, it means just successfully sent a request to provider.
 *    So you have to check the return callback. and its "ret" argument.
 * \param[in] pkgname Package name which should be activated
 * \param[in] cb Result callback
 * \param[in] data Callback data
 * \return int
 * \retval LB_STATUS_SUCCESS Successfully sent a request
 * \retval LB_STATUS_ERROR_INVALID Invalid argument
 * \retval LB_STATUS_ERROR_FAULT Failed to make a request
 * \pre N/A
 * \post N/A
 * \see ret_cb_t
 */
extern int livebox_activate(const char *pkgname, ret_cb_t cb, void *data);

/*!
 * \brief Resize the livebox
 * \details
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
 * \remarks
 *    You have to check the return callback.
 * \param[in] handler Handler of a livebox instance
 * \param[in] type Type of a livebox size, LB_SIZE_TYPE_1x1, ...
 * \param[in] cb Result callback of the resize operation.
 * \param[in] data User data for return callback
 * \return int
 * \retval LB_STATUS_ERROR_INVALID Invalid argument
 * \retval LB_STATUS_ERROR_BUSY Previous request of resize is in progress.
 * \retval LB_STATUS_ERROR_ALREADY Already resized, there is no differences between current size and requested size.
 * \retval LB_STATUS_ERROR_PERMISSION Permission denied, you only have view the content of this box.
 * \retval LB_STATUS_ERROR_FAULT Failed to make a request
 * \pre N/A
 * \post N/A
 * \see ret_cb_t
 */
extern int livebox_resize(struct livebox *handler, int type, ret_cb_t cb, void *data);

/*!
 * \brief Send the click event for a livebox.
 * \details N/A
 * \remarks N/A
 * \param[in] handler Handler of a livebox instance
 * \param[in] x Rational X of the content width.
 * \param[in] y Rational Y of the content height.
 * \return int
 * \retval LB_STATUS_ERROR_INVALID Invalid argument
 * \retval LB_STATUS_ERROR_FAULT Unrecoverable error occurred
 * \retval LB_STATUS_SUCCESS Successfully done
 * \pre N/A
 * \post N/A
 * \see N/A
 */
extern int livebox_click(struct livebox *handler, double x, double y);

/*!
 * \brief Change the cluster/sub-cluster name of given livebox handler
 * \details N/A
 * \remarks N/A
 * \param[in] handler Handler of a livebox instance
 * \param[in] cluster New cluster of a livebox
 * \param[in] category New category of a livebox
 * \param[in] cb Result callback for changing the cluster/category of a livebox
 * \param[in] data User data for the result callback
 * \return int
 * \retval LB_STATUS_SUCCESS Request is successfully sent. the return callback will be called.
 * \retval LB_STATUS_ERROR_BUSY previous request is not finished yet.
 * \retval LB_STATUS_ERROR_ALREADY group name is same with current one.
 * \retval LB_STATUS_ERROR_PERMISSION you have no permission to change property of this livebox instance.
 * \retval LB_STATUS_ERROR_FAULT Failed to make a request.
 * \pre N/A
 * \post N/A
 * \see ret_cb_t
 */
extern int livebox_set_group(struct livebox *handler, const char *cluster, const char *category, ret_cb_t cb, void *data);

/*!
 * \brief Get the cluster and category(sub-cluster) name of given livebox (It is not I18N format, only english)
 * \details N/A
 * \remarks
 *    You have to do not release the cluster & category.
 *    It is allocated inside of given livebox instance, so you can only read it.
 * \param[in] handler Handler of a livebox instance
 * \param[out] cluster Storage(memory) for containing the cluster name
 * \param[out] category Storage(memory) for containing the category name
 * \return int
 * \retval LB_STATUS_ERROR_INVALID Invalid argument
 * \retval LB_STATUS_SUCCESS Successfully done
 * \pre N/A
 * \post N/A
 * \see N/A
 */
extern int livebox_get_group(struct livebox *handler, char ** const cluster, char ** const category);

/*!
 * \brief Get the period of this livebox handler
 * \details N/A
 * \remarks
 *    if this function returns 0.0f, it means the livebox has no update period.
 *    or the handle is not valid.
 *    This function only can be works after the return callback of livebox_create fucntion is called.
 * \param[in] handler Handler of a livebox instance
 * \return double
 * \retval Current update period of a livebox
 * \retval 0.0f it means the box has no update period, or it can returns 0.0 if the handles is not valid.
 * \pre N/A
 * \post N/A
 * \see N/A
 */
extern double livebox_period(struct livebox *handler);

/*!
 * \brief Change the update period
 * \details N/A
 * \remarks N/A
 * \param[in] handler Handler of a livebox instance
 * \param[in] period New update period of a livebox
 * \param[in] cb Result callback of changing the update period of this livebox
 * \param[in] data User data for the result callback
 * \return int
 * \retval LB_STATUS_SUCCESS Successfully done
 * \retval LB_STATUS_ERROR_INVALID Invalid argument
 * \retval LB_STATUS_ERROR_BUSY
 * \retval LB_STATUS_ERROR_ALREADY
 * \retval LB_STATUS_ERROR_FAULT Unrecoverable error occurred
 * \pre N/A
 * \post N/A
 * \see ret_cb_t
 */
extern int livebox_set_period(struct livebox *handler, double period, ret_cb_t cb, void *data);

/*!
 * \brief Is this an text type livebox?
 * \details N/A
 * \remarks N/A
 * \param[in] handler Handler of a livebox instance
 * \return livebox_lb_type
 * \retval LB_TYPE_IMAGE Contents of a livebox is based on the image file
 * \retval LB_TYPE_BUFFER Contents of a livebox is based on canvas buffer(shared)
 * \retval LB_TYPE_TEXT Contents of a livebox is based on formatted text file
 * \retval LB_TYPE_PIXMAP Contens of a livebox is shared by the pixmap(depends on X)
 * \retval LB_TYPE_INVALID
 * \pre N/A
 * \post N/A
 * \see livebox_lb_type
 */
extern enum livebox_lb_type livebox_lb_type(struct livebox *handler);

/*!
 * \brief Is this livebox is created by a user?
 * \details
 *    If the livebox instance is created by system this will returns 0.
 * \remarks N/A
 * \param[in] handler Handler of a livebox instance
 * \return int
 * \retval LB_STATUS_ERROR_INVALID Invalid argument
 * \retval 0 automatically created livebox by the provider
 * \retval 1 created by user via livebox_add or livebox_add_with_size
 * \pre N/A
 * \post N/A
 * \see livebox_add
 * \see livebox_add_with_size
 * \see livebox_set_event_handler
 */
extern int livebox_is_user(struct livebox *handler);

/*!
 * \brief Get the content information string of given livebox
 * \details N/A
 * \remarks N/A
 * \param[in] handler Handler of a livebox instance
 * \return const char *
 * \retval content_info Livebox content info that can be used again via content_info argument of livebox_add or livebox_add_with_size.
 * \pre N/A
 * \post N/A
 * \see livebox_add
 * \see livebox_add_with_size
 */
extern const char *livebox_content(struct livebox *handler);

/*!
 * \brief Get the sub cluster title string of given livebox
 * \details
 *  This API is now used for accessibility.
 *  Each box should set their content as a string to read by TTS.
 *  So if the box has focus on the homescreen, the homescreen will read a text using this API.
 * \remarks
 *  The title which is returned by this, the TTS should read it.
 *  But it is just recomended to a homescreen.
 *  So read it or not is depends on implementation of the homescreen.
 * \param[in] handler Handler of a livebox instance
 * \return const char *
 * \retval sub cluster name
 * \retval NULL
 * \pre N/A
 * \post N/A
 * \see N/A
 */
extern const char *livebox_category_title(struct livebox *handler);

/*!
 * \brief Get the filename of given livebox, if it is an IMAGE type livebox
 * \details
 *   If the box is developed as image format to represent its contents,
 *   The homescreen should know its image file name.
 * \remarks N/A
 * \param[in] handler Handler of a livebox instance
 * \return const char *
 * \retval filename if the livebox type is image this function will give you a abspath of an image file (content is rendered)
 * \retval NULL if this has no image file or type is not image file.
 * \pre N/A
 * \post N/A
 * \see N/A
 */
extern const char *livebox_filename(struct livebox *handler);

/*!
 * \brief Get the package name of given livebox handler
 * \details N/A
 * \remarks N/A
 * \param[in] handler Handler of a livebox instance
 * \return const char *
 * \retval pkgname package name
 * \retval NULL if the handler is not valid
 * \pre N/A
 * \post N/A
 * \see N/A
 */
extern const char *livebox_pkgname(struct livebox *handler);

/*!
 * \brief Get the priority of a current content.
 * \details N/A
 * \remarks N/A
 * \param[in] handler Handler of a livebox instance
 * \return double
 * \retval 0.0f handler is NULL
 * \retval -1.0f Handler is not valid (not yet initialized)
 * \retval real number between 0.0 and 1.0
 * \pre N/A
 * \post N/A
 * \see N/A
 */
extern double livebox_priority(struct livebox *handler);

/*!
 * \brief Acquire the buffer of given livebox (Only for the buffer type)
 * \details N/A
 * \remarks N/A
 * \param[in] handler Handler of a livebox instance
 * \return void *
 * \retval address of a FB
 * \retval NULL if it fails to get fb address
 * \pre N/A
 * \post N/A
 * \see N/A
 */
extern void *livebox_acquire_fb(struct livebox *handler);

/*!
 * \brief Release the buffer of a livebox (Only for the buffer type)
 * \details N/A
 * \remarks N/A
 * \param[in] buffer
 * \return int
 * \retval LB_STATUS_ERROR_INVALID Invalid argument
 * \retval LB_STATUS_SUCCESS Successfully done
 * \pre N/A
 * \post N/A
 * \see livebox_acquire_fb
 */
extern int livebox_release_fb(void *buffer);

/*!
 * \brief Get the reference count of Livebox buffer (Only for the buffer type)
 * \details N/A
 * \remarks N/A
 * \param[in] buffer
 * \return int
 * \retval LB_STATUS_ERROR_INVALID Invalid argument
 * \retval LB_STATUS_ERROR_FAULT Unrecoverable error occurred
 * \retval refcnt positive integer including ZERO
 * \pre N/A
 * \post N/A
 * \see livebox_pdfb_refcnt
 */
extern int livebox_fb_refcnt(void *buffer);

/*!
 * \brief Acquire the buffer of a PD frame (Only for the buffer type)
 * \details N/A
 * \remarks N/A
 * \param[in] handler Handler of a livebox instance
 * \return int
 * \retval NULL
 * \retval adress of buffer of PD
 * \pre N/A
 * \post N/A
 * \see livebox_release_pdfb
 */
extern void *livebox_acquire_pdfb(struct livebox *handler);

/*!
 * \brief Release the acquired buffer of the PD Frame (Only for the buffer type)
 * \details N/A
 * \remarks N/A
 * \param[in] buffer
 * \return int
 * \retval LB_STATUS_ERROR_INVALID Invalid argument
 * \retval LB_STATUS_SUCCESS Successfully done
 * \pre N/A
 * \post N/A
 * \see livebox_acquire_pdfb
 */
extern int livebox_release_pdfb(void *buffer);

/*!
 * \brief Reference count of given PD buffer (Only for the buffer type)
 * \details N/A
 * \remarks N/A
 * \param[in] buffer
 * \return int
 * \retval LB_STATUS_ERROR_INVALID Invalid argument
 * \retval LB_STATUS_ERROR_FAULT Unrecoverable error occurred
 * \retval reference count
 * \pre N/A
 * \post N/A
 * \see livebox_fb_refcnt
 */
extern int livebox_pdfb_refcnt(void *buffer);

/*!
 * \brief Get the size of the Livebox
 * \details N/A
 * \remarks N/A
 * \param[in] handler Handler of a livebox instance
 * \return int
 * \retval LB_SIZE_TYPE_NxM
 * \retval LB_SIZE_TYPE_INVALID
 * \pre N/A
 * \post N/A
 * \see N/A
 */
extern int livebox_size(struct livebox *handler);

/*!
 * \brief Get the size of the Progressive Disclosure
 * \details N/A
 * \remarks N/A
 * \param[in] handler Handler of a livebox instance
 * \param[out] w
 * \param[out] h
 * \return int
 * \retval LB_STATUS_ERROR_INVALID Invalid argument
 * \retval LB_STATUS_SUCCESS Successfully done
 * \pre N/A
 * \post N/A
 * \see N/A
 */
extern int livebox_get_pdsize(struct livebox *handler, int *w, int *h);

/*!
 * \brief List of supported sizes of given handler
 * \details N/A
 * \remarks N/A
 * \param[in] handler Handler of a livebox instance
 * \param[out] cnt
 * \param[out] size_list
 * \return int
 * \retval LB_STATUS_ERROR_INVALID Invalid argument
 * \retval LB_STATUS_SUCCESS Successfully done
 * \pre N/A
 * \post N/A
 * \see N/A
 */
extern int livebox_get_supported_sizes(struct livebox *handler, int *cnt, int *size_list);

/*!
 * \brief BUFFER SIZE of the livebox if it is a buffer type
 * \details N/A
 * \remarks N/A
 * \param[in] handler Handler of a livebox instance
 * \return int
 * \retval LB_STATUS_ERROR_INVALID Invalid argument
 * \retval size of livebox buffer
 * \pre N/A
 * \post N/A
 * \see N/A
 */
extern int livebox_lbfb_bufsz(struct livebox *handler);

/*!
 * \brief BUFFER SIZE of the progiressive disclosure if it is a buffer type
 * \details N/A
 * \remarks N/A
 * \param[in] handler Handler of a livebox instance
 * \return int
 * \retval LB_STATUS_ERROR_INVALID Invalid argument
 * \retval size of PD buffer
 * \pre N/A
 * \post N/A
 * \see N/A
 */
extern int livebox_pdfb_bufsz(struct livebox *handler);

/*!
 * \brief Send the content event (for buffer type) to provider(livebox)
 * \details N/A
 * \remarks DEPRECATED
 *  Use the livebox_mouse_event function instead of this.
 * \param[in] handler Handler of a livebox instance
 * \param[in] type Event type
 * \param[in] x coordinates of X axis
 * \param[in] y coordinates of Y axis
 * \return int
 * \retval LB_STATUS_ERROR_INVALID Invalid argument
 * \retval LB_STATUS_ERROR_BUSY Previous operaion is not finished yet
 * \retval LB_STATUS_ERROR_FAULT Unrecoverable error occurred
 * \retval LB_STATUS_SUCCESS Successfully sent
 * \pre N/A
 * \post N/A
 * \see livebox_mouse_event
 * \see livebox_access_event
 * \see livebox_key_event
 */
extern int livebox_content_event(struct livebox *handler, enum content_event_type type, double x, double y);

/*!
 * \brief Send the content event (for buffer type) to provider(livebox)
 * \details N/A
 * \remarks N/A
 * \param[in] handler Handler of a livebox instance
 * \param[in] type Event type
 * \param[in] x coordinates of X axis
 * \param[in] y coordinates of Y axis
 * \return int
 * \retval LB_STATUS_ERROR_INVALID Invalid argument
 * \retval LB_STATUS_ERROR_BUSY Previous operation is not finished yet
 * \retval LB_STATUS_ERROR_FAULT Unrecoverable error occurred
 * \retval LB_STATUS_SUCCESS Successfully sent
 * \pre N/A
 * \post N/A
 * \see livebox_content_event
 * \see livebox_access_event
 * \see livebox_key_event
 */
extern int livebox_mouse_event(struct livebox *handler, enum content_event_type type, double x, double y);

/*!
 * \brief Send the access event(for buffer type) to provider(livebox).
 * \details N/A
 * \remarks N/A
 * \param[in] handler Handler of a livebox instance
 * \param[in] type Event type
 * \param[in] x coordinates of X axsis
 * \param[in] y coordinates of Y axsis
 * \param[in] cb Result callback function
 * \param[in] data Callback data
 * \return int
 * \retval LB_STATUS_ERROR_INVALID Invalid argument
 * \retval LB_STATUS_ERROR_BUSY Previous operation is not finished yet
 * \retval LB_STATUS_ERROR_FAULT Unrecoverable error occurred
 * \retval LB_STATUS_SUCCESS Successfully sent
 * \pre N/A
 * \post N/A
 * \see livebox_mouse_event
 * \see livebox_key_event
 */
extern int livebox_access_event(struct livebox *handler, enum access_event_type type, double x, double y, ret_cb_t cb, void *data);

/*!
 * \brief Send the key event(for buffer type) to provider(livebox).
 * \details N/A
 * \remarks N/A
 * \param[in] handler Handler of a livebox instance
 * \param[in] type Key event type
 * \param[in] keycode Code of key
 * \param[in] cb Result callback
 * \param[in] data Callback data
 * \return int
 * \retval LB_STATUS_ERROR_INVALID Invalid argument
 * \retval LB_STATUS_ERROR_BUSY Previous operation is not finished yet
 * \retval LB_STATUS_ERROR_FAULT Unrecoverable error occurred
 * \retval LB_STATUS_SUCCESS Successfully sent
 * \pre N/A
 * \post N/A
 * \see livebox_mouse_event
 * \see livebox_access_event
 */
extern int livebox_key_event(struct livebox *handler, enum content_event_type type, unsigned int keycode, ret_cb_t cb, void *data);

/*!
 * \brief Do pin up or not.
 * \details
 *   If the livebox supports the pinup feature,
 *   you can freeze the update of given livebox.
 *   But it is different with pause.
 *   The box will be updated and it will decide wheter update its content or not when the pinup is on.
 * \remarks N/A
 * \param[in] handler Handler of a livebox instance
 * \param[in] flag Pinup value
 * \param[in] cb Result callback
 * \param[in] data Callback data
 * \return int
 * \retval LB_STATUS_ERROR_INVALID Invalid parameters
 * \see ret_cb_t
 * \see livebox_set_visibility
 * \see livebox_is_pinned_up
 */
extern int livebox_set_pinup(struct livebox *handler, int flag, ret_cb_t cb, void *data);

/*!
 * \brief Check the PIN-UP status of given handler
 * \details N/A
 * \remarks N/A
 * \param[in] handler Handler of a livebox instance
 * \return int
 * \retval LB_STATUS_ERROR_INVALID Invalid parameters
 * \retval 1 box is pinned up
 * \retval 0 box is not pinned up
 * \see livebox_set_pinup
 */
extern int livebox_is_pinned_up(struct livebox *handler);

/*!
 * \brief Check the PINUP feature availability of the given handler
 * \details N/A
 * \remarks N/A
 * \param[in] handler Handler of a livebox instance
 * \return int
 * \retval LB_STATUS_ERROR_INVALID Invalid argument
 * \retval 1 if the box support Pinup feature
 * \retval 0 if the box does not support the Pinup feature
 * \pre N/A
 * \post N/A
 * \see livebox_is_pinned_up
 * \see livebox_set_pinup
 */
extern int livebox_has_pinup(struct livebox *handler);

/*!
 * \brief Check the PD existence of given handler
 * \details N/A
 * \remarks N/A
 * \param[in] handler Handler of a livebox instance
 * \return int
 * \retval LB_STATUS_ERROR_INVALID Invalid argument
 * \retval 1 if the box support the PD
 * \retval 0 if the box has no PD
 * \pre N/A
 * \post N/A
 * \see N/A
 */
extern int livebox_has_pd(struct livebox *handler);

/*!
 * \brief Create the PD of given handler
 * \details N/A
 * \remarks N/A
 * \param[in] handler Handler of a livebox instance
 * \param[in] cb Result callback
 * \param[in] data Callback data
 * \return int
 * \retval LB_STATUS_SUCCESS Successfully done
 * \retval LB_STATUS_ERROR_INVALID Invalid argument
 * \retval LB_STATUS_ERROR_BUSY Previous operation is not finished yet
 * \retval LB_STATUS_ERROR_FAULT Unrecoverable error occurred
 * \pre N/A
 * \post N/A
 * \see ret_cb_t
 * \see livebox_create_pd_with_position
 * \see livebox_move_pd
 * \see livebox_destroy_pd
 */
extern int livebox_create_pd(struct livebox *handler, ret_cb_t cb, void *data);

/*!
 * \brief Create the PD of given handler with the relative position from livebox
 * \details N/A
 * \remarks N/A
 * \param[in] handler Handler of a livebox instance
 * \param[in] x 0.0 ~ 1.0
 * \param[in] y 0.0 ~ 1.0
 * \param[in] cb Result callback
 * \param[in] data Callback data
 * \return int
 * \retval LB_STATUS_SUCCESS Successfully done
 * \retval LB_STATUS_ERROR_INVALID Invalid argument
 * \retval LB_STATUS_ERROR_BUSY Previous operation is not finished yet
 * \retval LB_STATUS_ERROR_FAULT Unrecoverable error occurred
 * \pre N/A
 * \post N/A
 * \see livebox_create_pd
 * \see livebox_destroy_pd
 * \see livebox_move_pd
 */
extern int livebox_create_pd_with_position(struct livebox *handler, double x, double y, ret_cb_t cb, void *data);

/*!
 * \brief PD position is updated.
 * \details N/A
 * \remarks N/A
 * \param[in] handler Handler of a livebox instance
 * \param[in] x 0.0 ~ 1.0
 * \param[in] y 0.0 ~ 1.0
 * \return int
 * \retval LB_STATUS_SUCCESS if succeed to send request for updating position of the PD.
 * \retval LB_STATUS_ERROR_FAULT Unrecoverable error occurred
 * \retval LB_STATUS_ERROR_INVALID Invalid argument
 * \pre N/A
 * \post N/A
 * \see N/A
 */
extern int livebox_move_pd(struct livebox *handler, double x, double y);

/*!
 * \brief Destroy the PD of given handler if it is created.
 * \details N/A
 * \remarks N/A
 * \param[in] handler Handler of a livebox instance
 * \param[in] cb
 * \param[in] data
 * \return int
 * \retval LB_STATUS_ERROR_INVALID Invalid argument
 * \retval LB_STATUS_ERROR_FAULT Unrecoverable error occurred
 * \retval LB_STATUS_SUCCESS Successfully done
 * \pre N/A
 * \post N/A
 * \see ret_cb_t
 */
extern int livebox_destroy_pd(struct livebox *handler, ret_cb_t cb, void *data);

/*!
 * \brief Check the create status of given livebox handler
 * \details N/A
 * \remarks N/A
 * \param[in] handler Handler of a livebox instance
 * \return int
 * \retval LB_STATUS_ERROR_INVALID Invalid argument
 * \retval 0 PD is not created
 * \retval 1 PD is created
 */
extern int livebox_pd_is_created(struct livebox *handler);

/*!
 * \brief Check the content type of the progressive disclosure of given handler
 * \details N/A
 * \remarks N/A
 * \param[in] handler Handler of a livebox instance
 * \return int
 * \retval PD_TYPE_BUFFER Contents of a PD is based on canvas buffer(shared)
 * \retval PD_TYPE_TEXT Contents of a PD is based on formatted text file
 * \retval PD_TYPE_PIXMAP Contents of a livebox is shared by the pixmap(depends on X)
 * \retval PD_TYPE_INVALID
 * \pre N/A
 * \post N/A
 * \see livebox_pd_type
 */
extern enum livebox_pd_type livebox_pd_type(struct livebox *handler);

/*!
 * \brief Check the existence of a livebox about given package name
 * \details N/A
 * \remarks N/A
 * \param[in] pkgname
 * \return int
 * \retval 1 if the box is exists
 * \retval 0 if the box is not exists
 * \pre N/A
 * \post N/A
 * \see N/A
 */
extern int livebox_is_exists(const char *pkgname);

/*!
 * \brief Set function table for parsing the text content of a livebox
 * \details N/A
 * \remarks N/A
 * \param[in] handler Handler of a livebox instance
 * \param[in] ops
 * \return int
 * \retval LB_STATUS_SUCCESS Successfully done
 * \retval LB_STATUS_ERROR_INVALID Invalid argument
 * \see livebox_set_pd_text_handler
 */
extern int livebox_set_text_handler(struct livebox *handler, struct livebox_script_operators *ops);

/*!
 * \brief Set function table for parsing the text content of a Progressive Disclosure
 * \details N/A
 * \remarks N/A
 * \param[in] handler Handler of a livebox instance
 * \param[in] ops
 * \return int
 * \retval LB_STATUS_SUCCESS Successfully done
 * \retval LB_STATUS_ERROR_INVALID Invalid argument
 * \see livebox_set_text_handler
 */
extern int livebox_set_pd_text_handler(struct livebox *handler, struct livebox_script_operators *ops);

/*!
 * \brief Emit a text signal to given livebox only if it is a text type.
 * \details N/A
 * \remarks N/A
 * \param[in] handler Handler of a livebox instance
 * \param[in] emission Emission string
 * \param[in] source Source string
 * \param[in] sx start X
 * \param[in] sy start Y
 * \param[in] ex end X
 * \param[in] ey end Y
 * \param[in] cb Result callback
 * \param[in] data Callback data
 * \return int
 * \retval LB_STATUS_ERROR_INVALID Invalid parameters
 * \retval LB_STATUS_ERROR_FAULT Unrecoverable error occurred
 * \retval LB_STATUS_SUCCESS Successfully emitted
 * \see ret_cb_t
 */
extern int livebox_emit_text_signal(struct livebox *handler, const char *emission, const char *source, double sx, double sy, double ex, double ey, ret_cb_t cb, void *data);

/*!
 * \brief Set a private data pointer to carry it using given handler
 * \details N/A
 * \remarks N/A
 * \param[in] handler Handler of a livebox instance
 * \param[in] data data pointer
 * \return int
 * \retval LB_STATUS_SUCCESS Successfully registered
 * \retval LB_STATUS_ERROR_INVALID Invalid argument
 * \pre N/A
 * \post N/A
 * \see livebox_get_data
 */
extern int livebox_set_data(struct livebox *handler, void *data);

/*!
 * \brief Get private data pointer which is carried by given handler
 * \details N/A
 * \remarks N/A
 * \param[in] handler Handler of a livebox instance
 * \return void *
 * \retval data pointer
 * \retval NULL if there is not data
 * \pre N/A
 * \post N/A
 * \see livebox_set_data
 */
extern void *livebox_get_data(struct livebox *handler);

/*!
 * \brief Subscribe the event for liveboxes only in given cluster and sub-cluster
 * \details
 *   If you wrote a view-only client.
 *   you can receive the event of specific liveboxes which are grouped in given cluster/category
 *   But you cannot modify their attributes (such as size, ...).
 * \remarks N/A
 * \param[in] cluster   "*" can be used for subscribe all cluster's liveboxes event.
 *                      If you use the "*", value in the category will be ignored.
 * \param[in] category  "*" can be used for subscribe liveboxes events of all category(sub-cluster) in given "cluster"
 * \return int
 * \retval LB_STATUS_ERROR_FAULT Unrecoverable error occurred
 * \retval LB_STATUS_SUCCESS Successfully requested
 * \pre N/A
 * \post N/A
 * \see livebox_unsubscribe_group
 */
extern int livebox_subscribe_group(const char *cluster, const char *category);

/*!
 * \brief Unsubscribe the event for the liveboxes, but you will receive already added liveboxes event.
 * \details N/A
 * \remarks N/A
 * \param[in] cluster   "*" can be used for subscribe all cluster's liveboxes event.
 *                      If you use the "*", value in the category will be ignored.
 * \param[in] category  "*" can be used for subscribe all sub-cluster's liveboxes event in given "cluster"
 * \return int
 * \retval LB_STATUS_ERROR_FAULT Unrecoverable error occurred
 * \retval LB_STATUS_SUCCESS Successfully requested
 * \pre N/A
 * \post N/A
 * \see livebox_subscribe_group
 */
extern int livebox_unsubscribe_group(const char *cluster, const char *category);

/*!
 * \brief Refresh the group(cluster/sub-cluser(aka. category))
 * \details
 *    This function will trigger the update of all liveboxes in given cluster/category group
 * \remarks
 *    Basically default livebox system doesn't use the cluster/category concept.
 *    But you can use it. so if you decide to use it then you can trigger the update of all liveboxes in given group.
 * \param[in] cluster Cluster ID
 * \param[in] category Sub-cluster ID
 * \param[in] force 1 if the boxes should be updated even if they are paused
 * \return int
 * \retval LB_STATUS_ERROR_INVALID Invalid argument
 * \retval LB_STATUS_ERROR_FAULT Unrecoverable error occurred
 * \retval LB_STATUS_SUCCESS Successfully requested
 * \pre N/A
 * \post N/A
 * \see livebox_refresh
 */
extern int livebox_refresh_group(const char *cluster, const char *category, int force);

/*!
 * \brief Refresh a livebox
 * \details N/A
 * \remarks N/A
 * \param[in] handler Handler of a livebox instance
 * \param[in] force 1 if the box should be updated even if it is paused
 * \return int
 * \retval LB_STATUS_ERROR_INVALID Invalid argument
 * \retval LB_STATUS_ERROR_FAULT Unrecoverable error occurred
 * \retval LB_STATUS_SUCCESS Successfully requested
 * \pre N/A
 * \post N/A
 * \see livebox_refresh_group
 */
extern int livebox_refresh(struct livebox *handler, int force);

/*!
 * \brief Pixmap Id of a livebox content
 * \details
 *   This function doesn't guarantees the life-cycle of the pixmap.
 *   If the service provider destroy the pixmap, you will not know about it.
 *   So you should validate it before access it.
 * \remarks N/A
 * \param[in] handler Handler of a livebox instance
 * \return int
 * \retval 0 if the pixmap is not created
 * \retval pixmap Pixmap Id need to be casted to (unsigned int) type
 * \pre N/A
 * \post N/A
 * \see livebox_pd_pixmap
 */
extern int livebox_lb_pixmap(const struct livebox *handler);

/*!
 * \brief Pixmap Id of a PD content
 * \details
 *   This function doesn't guarantees the life-cycle of the pixmap.
 *   If the service provider destroy the pixmap, you will not know about it.
 *   So you should validate it before access it.
 * \remarks N/A
 * \param[in] handler Handler of a livebox instance
 * \return int
 * \retval 0 if the pixmap is not created
 * \retval pixmap Pixmap Id need to be casted to (unsigned int) type
 * \pre N/A
 * \post N/A
 * \see livebox_lb_pixmap
 */
extern int livebox_pd_pixmap(const struct livebox *handler);

/*!
 * \brief Acquire the pixmap of PD
 * \details
 *   After acquire the pixmap of PD, it will not be destroyed
 *   So if the new update is comming with new pixmap Id, you should release old pixmap manually
 * \remarks N/A
 * \param[in] handler Handler of a livebox instance
 * \param[in] cb Result callback for acquiring request
 * \param[in] data Callback Data
 * \return int
 * \retval LB_STATUS_ERROR_INVALID Invalid argument
 * \retval LB_STATUS_ERROR_FAULT Failed to send a request to the service provider or there is critical error that is unrecoverable
 * \retval LB_STATUS_SUCCESS Successfully requested to acquire the pixmap of PD
 * \pre N/A
 * \post N/A
 * \see livebox_release_pd_pixmap
 * \see livebox_acquire_lb_pixmap
 * \see ret_cb_t
 */
extern int livebox_acquire_pd_pixmap(struct livebox *handler, ret_cb_t cb, void *data);

/*!
 * \brief Release the acquired pixmap ID
 * \details N/A
 * \remarks N/A
 * \param[in] handler Handler of a livebox instance
 * \param[in] pixmap Pixmap Id to release it
 * \return int
 * \retval LB_STATUS_ERROR_INVALID Invalid argument
 * \retval LB_STATUS_ERROR_FAULT Unrecoverable error occurred
 * \retval LB_STATUS_SUCCESS Successfully released (request is sent)
 * \pre N/A
 * \post N/A
 * \see livebox_acquire_pd_pixmap
 * \see livebox_release_lb_pixmap
 */
extern int livebox_release_pd_pixmap(struct livebox *handler, int pixmap);

/*!
 * \brief Getting the PIXMAP of a livebox
 * \details
 *   Even if the render process release the pixmap, the pixmap will be kept before released by livebox_release_lb_pixmap
 *   You should release the pixmap manually.
 * \remarks N/A
 * \param[in] handler Handler of a livebox instance
 * \param[in] cb Callback function which will be called with result of acquiring lb pixmap
 * \param[in] data Callback data
 * \return int
 * \retval LB_STATUS_ERROR_INVALID Invalid argument
 * \retval LB_STATUS_ERROR_FAULT Unrecoverable error occurred
 * \retval LB_STATUS_SUCCESS Successfully requested
 * \pre
 *   Livebox service system should support the PIXMAP type buffer.
 *   The livebox should be designed to use the buffer (script type)
 * \post N/A
 * \see livebox_release_lb_pixmap
 * \see livebox_acquire_pd_pixmap
 * \see ret_cb_t
 */
extern int livebox_acquire_lb_pixmap(struct livebox *handler, ret_cb_t cb, void *data);

/*!
 * \brief Release the pixmap of a livebox
 * \details
 *   After the client gets new pixmap or no more need to keep current pixmap, use this to release it.
 *   If there is no user for given pixmap, the pixmap will be destroyed.
 * \remarks N/A
 * \param[in] handler Handler of a livebox instance
 * \param[in] pixmap Pixmap Id of given livebox handler
 * \return int
 * \retval LB_STATUS_ERROR_INVALID Invalid argument
 * \retval LB_STATUS_ERROR_FAULT Unrecoverable error occurred
 * \retval LB_STATUS_SUCCESS Successfully done
 * \pre
 *   The pixmap should be acquired by livebox_acquire_lb_pixmap
 * \post N/A
 * \see livebox_acquire_lb_pixmap
 * \see livebox_release_pd_pixmap
 */
extern int livebox_release_lb_pixmap(struct livebox *handler, int pixmap);

/*!
 * \brief Update the visible state of a livebox
 * \details N/A
 * \remarks N/A
 * \param[in] handler Handler of a livebox instance
 * \param[in] state Configure the current visible state of a livebox
 * \return int
 * \retval LB_STATUS_ERROR_INVALID Invalid argument
 * \retval LB_STATUS_ERROR_BUSY
 * \retval LB_STATUS_ERROR_PERMISSION
 * \retval LB_STATUS_ERROR_ALREADY
 * \retval LB_STATUS_ERROR_FAULT Unrecoverable error occurred
 * \retval LB_STATUS_SUCCESS Successfully done
 * \pre N/A
 * \post N/A
 * \see N/A
 */
extern int livebox_set_visibility(struct livebox *handler, enum livebox_visible_state state);

/*!
 * \brief Current visible state of a livebox
 * \details N/A
 * \remarks N/A
 * \param[in] handler Handler of a livebox instance
 * \return livebox_visible_state
 * \retval LB_SHOW Livebox is showed. Default state
 * \retval LB_HIDE Livebox is hide, Update timer is not be freezed. but you cannot receive any updates events. you should refresh(reload) the content of a livebox when you make this show again
 * \retval LB_HIDE_WITH_PAUSE Livebix is hide, it will paused the update timer, but if a livebox update its contents, update event will come to you
 * \retval LB_VISIBLE_ERROR To enlarge the size of this enumeration type
 * \pre N/A
 * \post N/A
 * \see N/A
 */
extern enum livebox_visible_state livebox_visibility(struct livebox *handler);

/*!
 * \brief Set the update mode of current livebox
 * \details N/A
 * \remarks N/A
 *        if you set 1 for active update mode, you should get buffer without updated event from provider.
 *	  But is passive mode, you have to update content of a box when you get updated event.
 *	  Default is Passive mode.
 * \param[in] handler Handler of a livebox instance
 * \param[in] active_update 1 means active update, 0 means passive update (default)
 * \param[in] cb Result callback function
 * \param[in] data Callback data
 * \return int
 * \retval LB_STATUS_ERROR_INVALID Invalid argument
 * \retval LB_STATUS_ERROR_BUSY
 * \retval LB_STATUS_ERROR_PERMISSION
 * \retval LB_STATUS_ERROR_ALREADY
 * \retval LB_STATUS_ERROR_FAULT Unrecoverable error occurred
 * \retval LB_STATUS_SUCCESS Successfully done
 * \pre N/A
 * \post N/A
 * \see ret_cb_t
 */
extern int livebox_set_update_mode(struct livebox *handler, int active_update, ret_cb_t cb, void *data);

/*!
 * \brief Is this box in the active update mode?
 * \details N/A
 * \remarks N/A
 * \param[in] handler Handler of a livebox instance
 * \return int
 * \retval 0 if passive mode
 * \retval 1 if active mode or error code
 * \pre N/A
 * \post N/A
 * \see N/A
 */
extern int livebox_is_active_update(struct livebox *handler);

/*!
 * \brief Use the manual sync for S/W buffer
 * \details N/A
 * \remarks N/A
 * param[in] flag
 * \return void
 * \pre N/A
 * \post N/A
 * \see livebox_manual_sync
 * \see livebox_sync_pd_fb
 * \see livebox_sync_lb_fb
 */
extern void livebox_set_manual_sync(int flag);

/*!
 * \brief Get current mode
 * \details N/A
 * \remarks N/A
 * \return int
 * \retval 0 if auto sync
 * \retval 1 if manual sync
 * \pre N/A
 * \post N/A
 * \see livebox_set_manual_sync
 * \see livebox_sync_pd_fb
 * \see livebox_sync_lb_fb
 */
extern int livebox_manual_sync(void);

/*!
 * \brief Use the frame drop while resizing contents
 * \details N/A
 * \remarks N/A
 * \param[in] flag 1 for dropping frames of old size or 0.
 * \return void
 * \pre N/A
 * \post N/A
 * \see livebox_frame_drop_for_resizing
 */
extern void livebox_set_frame_drop_for_resizing(int flag);

/*!
 * \brief Get current mode
 * \details
 *   While resizing the box, viewer doesn't want to know the updaed frames of old size anymore,
 *   In that case, if this mode is turnned on, the provider will not send the updated event to the viewer about old size.
 *   So the viewer can reduce its burden to update (or ignore) unnecessary frames
 * \remarks N/A
 * \return int
 * \retval 0 if it is disabled
 * \retval 1 if it is enabled
 * \pre N/A
 * \post N/A
 * \see livebox_set_frame_drop_for_resizing
 */
extern int livebox_frame_drop_for_resizing(void);

/*!
 * \brief Sync manually
 * \details N/A
 * \remarks N/A
 * param[in] handler Handler of a livebox instance
 * \return void
 * \retval LB_STATUS_SUCCESS If success
 * \retval LB_STATUS_ERROR_INVALID Invalid handle
 * \pre N/A
 * \post N/A
 * \see livebox_set_manual_sync
 * \see livebox_manual_sync
 * \see livebox_sync_lb_fb
 */
extern int livebox_sync_pd_fb(struct livebox *handler);

/*!
 * \brief Sync manually
 * \details N/A
 * \remarks N/A
 * \param[in] handler Handler of a livebox instance
 * \return void
 * \retval LB_STATUS_SUCCESS If success
 * \retval LB_STATUS_ERROR_INVALID Invalid handle
 * \pre N/A
 * \post N/A
 * \see livebox_set_manual_sync
 * \see livebox_manual_sync
 * \see livebox_sync_pd_fb
 */
extern int livebox_sync_lb_fb(struct livebox *handler);

/*!
 * \brief Getting the alternative icon of given livebox instance.
 * \details If the box should be represented as a shortcut icon, this function will get the alternative icon.
 * \remarks N/A
 * \param[in] handler Handler of a livebox instance
 * \return const char *
 * \retval address Absolute path of an alternative icon file
 * \retval NULL Livebox has no alternative icon file
 * \pre N/A
 * \post N/A
 * \see livebox_alt_name
 */
extern const char *livebox_alt_icon(struct livebox *handler);

/*!
 * \brief Getting the alternative name of given livebox instance.
 * \details If the box should be represented as a shortcut name, this function will get the alternative name.
 * \remarks N/A
 * \param[in] handler Handler of a livebox instance
 * \return const char *
 * \retval name Alternative name of a livebox
 * \retval NULL Livebox has no alternative name
 * \pre N/A
 * \post N/A
 * \see livebox_alt_icon
 */
extern const char *livebox_alt_name(struct livebox *handler);

/*!
 * \brief Get the lock for frame buffer.
 * \details
 *   This function should be used to prevent from rendering to the frame buffer while reading it.
 *   And the locking area should be short and must be released ASAP
 *   Or the render thread will be hang'd.
 * \remarks
 * \param[in] handler Handler of a livebox instance
 * \param[in] is_pd 1 for PD or 0
 * \return int
 * \retval LB_STATUS_ERROR_FAULT Unrecoverable error occurred
 * \retval LB_STATUS_ERROR_INVALID Invalid argument
 * \retval LB_STATUS_SUCCESS Successfully done
 * \pre N/A
 * \post N/A
 * \see livebox_release_fb_lock
 */
extern int livebox_acquire_fb_lock(struct livebox *handler, int is_pd);

/*!
 * \brief Release the lock of frame buffer
 * \details
 *   This function should be called ASAP after acquire a lock of FB.
 *   Or the render process will be blocked.
 * \remarks N/A
 * \param[in] handler Handler of a livebox instance
 * \param[in] is_pd 1 for PD or 0
 * \return int
 * \retval LB_STATUS_ERROR_FAULT Unrecoverable error occurred
 * \retval LB_STATUS_ERROR_INVALID Invalid argument
 * \retval LB_STATUS_SUCCESS Successfully done
 * \pre N/A
 * \post N/A
 * \see livebox_acquire_fb_lock
 */
extern int livebox_release_fb_lock(struct livebox *handler, int is_pd);

/*!
 * \}
 */

#ifdef __cplusplus
}
#endif

#endif
