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
 * \ingroup CAPI_LIVEBOX_FRAMEWORK Tizen livebox framework
 * \{
 * \defgroup LIVEBOX Application Programming Interfaces for the viewer application
 * \{
 * \tableofcontents
 * \section Intro Introduction
 * Tizen(SLP) homescreen S/W framework is supporing the live box. (aka widget which is similiar with the android widget)
 *
 * \image html front.jpg
 *
 * \section WhatIsTheLivebox 1. What is the Livebox
 * The live box is the widget of the TIZEN.
 *
 * It works as a small application displayed on other applications' (such as homescreen, lockscreen, etc ...) view.
 * Each live box can have (not a mandatory option) a PD (progressive disclosure) in which more detailed information can be found.
 * The content of PD can be exposed when a certain gesture (e.g., flick-down) has been applied to the live box.
 * If you are interested in developing a livebox, there are things you should know prior to making any source code for the box.
 * To make your live box added to any live box viewer application (e.g., live panel in our case), then you need to create and prepare    
 * controller(SO file), layout script (EDJE for a PD if necessary), configuration files.
 *
 * A livebox is managed by data provider, since each SO file of a livebox is loaded on and controlled by data provider using predefined ABI.
 * A viewer will receive any livebox's content in forms of "image file", "buffer" or "text" and display the content in various formats on its window.
 * A livebox developer needs to make sure that your live box generates desirable content in-time on a explicit update-request or periodic update.
 *
 * After a data provider loads a livebox's SO file, it then assigns a specific "file name" for the livebox via an argument of a livebox function.
 * Since then the livebox just generates content using then given file name.
 * Passing an image file (whose name is the previously given name) is the basic method for providing contents to the viewer.
 * But if you want play animation or handles user event in real-time, you can use the buffer type.
 *
 * And you should prepare the content of the Progressive Disclosure.
 * The Progressive Dislcosure is only updated by the "buffer" type. so you should prepare the layout script for it.
 * If you didn't install any script file for progressive disclosure, the viewer will ignore the "flick down" event from your livebox.
 *
 * \subsection Livebox 1.1 Livebox
 * Live box is a default content of your widget. It always displays on the screen and updated periodically.
 * It looks like below captured images.
 * \image html weather.png Weather Livebox
 * \image html stock.png Stock Livebox
 * \image html twitter.png Twitter Livebox
 *
 * \subsection ProgressiveDisclosure 1.2 Progressive Disclosure
 * \image html PD.png Progressive Disclosure
 * Progressive disclosure will be displayed when a user flicks down a livebox. (basically it depends on the implementation of the view applications)
 * To supports this, a developer should prepare the layout script (EDJE only for the moment) of the livebox's PD. (or you can use the buffer directly)
 * Data provider supports EDJE script but the developer can use various scripts if (which is BIG IF) their interpreters can be implemented based on evas & ecore.
 *
 * When a layout script has been installed, data provider can load and rendering the given layout on the buffer.
 * The content on the buffer can be shared between applications that need to display the content on their window.
 * Description data file is necessary to place proper content components in rendered layout.
 * Check this page Description Data. 
 *
 * \subsection ClusterCategory 1.3 What is the "cluster" and "category"
 * The cluster and the sub-cluster is just like the grouping concept.
 * It is used for creating/destorying your livebox instance when the data provider receives any context event from the context engine.
 * You will only get "user,created" cluster and "default" category(sub cluster) info.
 *
 * \section LiveboxContent 2. How the livebox can draw contents for viewer?
 * There are several ways to update the content of a livebox.
 *
 * \li Image file based content updating
 * \li Description file based content updating (with the layout script file)
 * \li Buffer based content updating
 *
 * Each method has specific benefit for implementing the livebox.
 *
 * \subsection ImageFormat 2.1 Via image file
 * This is the basic method for providing content of a livebox to the viewer application.
 * But this can be used only for the livebox. (Unavailable for the progressive disclosure).
 * When your livebox is created, the provider will assign an unique ID for your livebox(it would be a filename).
 * You should keep that ID until your livebox is running. The ID will be passed to you via livebox_create function.
 * \image html image_format.png
 *
 * When you need to update the output of your livebox, you should generate the image file using given ID(filename).
 * Then the data provider will recognize the event of updated output of a livebox and it will send that event to the viewer to reload it on the screen.
 *
 * \subsection ScriptFormat 2.2 Via layout script
 * \image html script_format.png
 * This method is supported for static layout & various contents (text & image)
 * When you develop your livebox, first design the layout of box content using script (edje is default)
 * Then the provider will load it to the content buffer and start rendering.
 * After the sciprt is loaded, you can fill it using description data format.
 * liblivebox defines description data handling functions.
 *
 * \subsection TextFormat 2.3 Via text data
 * \image html text_format.png
 * This is the simplified method to update the content of livebox.
 * So your box only need to update the text data using description data format.
 * Then the viewer will parse it to fill its screen.
 * So there is no buffer area, just viewer decide how handles it.
 *
 * \subsection BufferFormat 2.4 Via buffer
 * This method is very complex to implement.
 * The provider will give a content buffer to you, then your box should render its contents on this buffer.
 * This type is only supported for 3rd party livebox such as OSP and WEB.
 * Inhouse(EFL) livebox is not able to use this buffer type for the box content.
 *
 * \section PackageNTools 3. How can I get the development packages or tools?
 *
 * \section DevelopLivebox 4. How can I write a new livebox
 *
 * \section TestLivebox 5. How can I test my livebox
 *
 * \section LiveboxDirectory 6. Livebox directory hierachy
 * \image html preload_folder.png
 * \image html download_folder.png
 *
 */

/*!
 * Structure for a Livebox instance
 */
struct livebox;

/*!
 * Use the default update period which is defined in the livebox package manifest.
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

	CONTENT_EVENT_KEY_DOWN		= 0x00100000, /*!< LB key press */
	CONTENT_EVENT_KEY_UP		= 0x00200000, /*!< LB key release */

	CONTENT_EVENT_KEY_MASK		= 0x80000000,
	CONTENT_EVENT_MOUSE_MASK	= 0x20000000,
	CONTENT_EVENT_PD_MASK		= 0x10000000,
	CONTENT_EVENT_LB_MASK		= 0x40000000,

	LB_MOUSE_DOWN			= CONTENT_EVENT_LB_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_DOWN, /*!< Mouse down on the livebox */
	LB_MOUSE_UP			= CONTENT_EVENT_LB_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_UP, /*!< Mouse up on the livebox */
	LB_MOUSE_MOVE			= CONTENT_EVENT_LB_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_MOVE, /*!< Move move on the livebox */
	LB_MOUSE_ENTER			= CONTENT_EVENT_LB_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_ENTER, /*!< Mouse enter to the livebox */
	LB_MOUSE_LEAVE			= CONTENT_EVENT_LB_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_LEAVE, /*!< Mouse leave from the livebox */
	LB_MOUSE_SET			= CONTENT_EVENT_LB_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_SET,
	LB_MOUSE_UNSET			= CONTENT_EVENT_LB_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_UNSET,

	PD_MOUSE_DOWN			= CONTENT_EVENT_PD_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_DOWN, /*!< Mouse down on the PD */
	PD_MOUSE_UP			= CONTENT_EVENT_PD_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_UP, /*!< Mouse up on the PD */
	PD_MOUSE_MOVE			= CONTENT_EVENT_PD_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_MOVE, /*!< Mouse move on the PD */
	PD_MOUSE_ENTER			= CONTENT_EVENT_PD_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_ENTER, /*!< Mouse enter to the PD */
	PD_MOUSE_LEAVE			= CONTENT_EVENT_PD_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_LEAVE, /*!< Mouse leave from the PD */
	PD_MOUSE_SET			= CONTENT_EVENT_PD_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_SET,
	PD_MOUSE_UNSET			= CONTENT_EVENT_PD_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_UNSET,

	LB_KEY_DOWN			= CONTENT_EVENT_LB_MASK | CONTENT_EVENT_KEY_MASK | CONTENT_EVENT_KEY_DOWN, /*!< Key down on the livebox */
	LB_KEY_UP			= CONTENT_EVENT_LB_MASK | CONTENT_EVENT_KEY_MASK | CONTENT_EVENT_KEY_UP, /*!< Key up on the livebox */

	PD_KEY_DOWN			= CONTENT_EVENT_PD_MASK | CONTENT_EVENT_KEY_MASK | CONTENT_EVENT_KEY_DOWN, /*!< Key down on the livebox */
	PD_KEY_UP			= CONTENT_EVENT_PD_MASK | CONTENT_EVENT_KEY_MASK | CONTENT_EVENT_KEY_UP, /*!< Key up on the livebox */

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

	PD_ACCESS_HIGHLIGHT		= ACCESS_EVENT_PD_MASK | ACCESS_EVENT_HIGHLIGHT,
	PD_ACCESS_HIGHLIGHT_NEXT	= ACCESS_EVENT_PD_MASK | ACCESS_EVENT_HIGHLIGHT_NEXT,
	PD_ACCESS_HIGHLIGHT_PREV	= ACCESS_EVENT_PD_MASK | ACCESS_EVENT_HIGHLIGHT_PREV,
	PD_ACCESS_UNHIGHLIGHT		= ACCESS_EVENT_PD_MASK | ACCESS_EVENT_UNHIGHLIGHT,
	PD_ACCESS_ACTIVATE		= ACCESS_EVENT_PD_MASK | ACCESS_EVENT_ACTIVATE,
	PD_ACCESS_ACTION_DOWN		= ACCESS_EVENT_PD_MASK | ACCESS_EVENT_ACTION_DOWN,
	PD_ACCESS_ACTION_UP		= ACCESS_EVENT_PD_MASK | ACCESS_EVENT_ACTION_UP,
	PD_ACCESS_SCROLL_DOWN		= ACCESS_EVENT_PD_MASK | ACCESS_EVENT_SCROLL_DOWN,
	PD_ACCESS_SCROLL_MOVE		= ACCESS_EVENT_PD_MASK | ACCESS_EVENT_SCROLL_MOVE,
	PD_ACCESS_SCROLL_UP		= ACCESS_EVENT_PD_MASK | ACCESS_EVENT_SCROLL_UP
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

	LB_EVENT_LB_UPDATE_BEGIN,
	LB_EVENT_LB_UPDATE_END,

	LB_EVENT_PD_UPDATE_BEGIN,
	LB_EVENT_PD_UPDATE_END,

	LB_EVENT_UPDATE_MODE_CHANGED,

	LB_EVENT_IGNORED /*!< Request is ignored */
};

enum livebox_fault_type {
	LB_FAULT_DEACTIVATED, /*!< Livebox is deactivated by its fault operation */
	LB_FAULT_PROVIDER_DISCONNECTED /*!< Provider is disconnected */
};

enum livebox_visible_state { /*!< Must be sync'd with the provider */
	LB_SHOW = 0x00, /*!< Livebox is showed. Default state */
	LB_HIDE = 0x01, /*!< Livebox is hide, Update timer is not be freezed. but you cannot receive any updates events. you should refresh(reload) the content of a livebox when you make this show again */

	LB_HIDE_WITH_PAUSE = 0x02, /*!< Livebix is hide, it will paused the update timer, but if a livebox update its contents, update event will come to you */

	LB_VISIBLE_ERROR = 0xFFFFFFFF /* To enlarge the size of this enumeration type */
};

/*!
 * \note
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
};

/*!
 * \brief Prototype of the return callback of every async functions
 * \param[in] handle Handle of the livebox instance
 * \param[in] ret Result status of operation. LB_STATUS_XXX defined from liblivebox-service
 * \param[in] data data for result callback
 * \return void
 * \sa livebox_add
 * \sa livebox_add_with_size
 * \sa livebox_del
 * \sa livebox_activate
 * \sa livebox_resize
 * \sa livebox_set_group
 * \sa livebox_set_period
 * \sa livebox_access_event
 * \sa livebox_set_pinup
 * \sa livebox_create_pd
 * \sa livebox_create_pd_with_position
 * \sa livebox_destroy_pd
 * \sa livebox_emit_text_signal
 * \sa livebox_acquire_pd_pixmap
 * \sa livebox_acquire_lb_pixmap
 * \sa livebox_set_update_mode
 */
typedef void (*ret_cb_t)(struct livebox *handle, int ret, void *data);

/*!
 * \brief Initialize the livebox system
 * \param[in] disp If you have X Display connection object, you can re-use it. but you should care its life cycle.
 *                 It must be alive before call the livebox_fini
 * \return int LB_STATUS_SUCCESS(0) if success or < 0 (livebox-errno)
 * \sa livebox_fini
 */
extern int livebox_init(void *disp);

/*!
 * \brief Finalize the livebox system
 * \return int LB_STATUS_SUCCESS(0) if success,  LB_STATUS_ERROR_INVALID if livebox_init is not called.
 * \sa livebox_init
 */
extern int livebox_fini(void);

/*!
 * \brief Client is paused.
 * \return int LB_STATUS_SUCCESS if success, LB_STATUS_ERROR_FAULT if it failed to send state(paused) info
 * \sa livebox_client_resumed
 */
extern int livebox_client_paused(void);

/*!
 * \brief Client is rfesumed.
 * \return int LB_STATUS_SUCCESS if success, LB_STATUS_ERROR_FAULT if it failed to send state(resumed) info
 * \sa livebox_client_paused
 */
extern int livebox_client_resumed(void);

/*!
 * \brief Add a new livebox
 * \param[in] pkgname Livebox Id
 * \param[in] content Will be passed to the livebox instance.
 * \param[in] cluster Main group
 * \param[in] category Sub group
 * \param[in] period Update period. if you set DEFAULT_PERIOD, the provider will use the default period which is described in the manifest.
 * \param[in] cb After send the request to the provider, its result will be passed
 * \param[in] data
 * \return handle
 * \sa ret_cb_t
 */
extern struct livebox *livebox_add(const char *pkgname, const char *content, const char *cluster, const char *category, double period, ret_cb_t cb, void *data);

/*!
 * \brief Add a new livebox
 * 1x1=175x175
 * 2x1=354x175
 * 2x2=354x354
 * 4x1=712x175
 * 4x2=712x354
 * 4x3=712x533
 * 4x4=712x712
 * 4x5=712x891
 * 4x6=712x1070
 * 21x21=224x215
 * 23x21=680x215
 * 23x23=680x653
 * 0x0=720x1280
 *
 * \param[in] pkgname Livebox Id
 * \param[in] content Will be passed to the livebox instance.
 * \param[in] cluster Main group
 * \param[in] category Sub group
 * \param[in] period DEFAULT_PERIOD can be used for this. this argument will be used to specify the period of update content of livebox.
 * \param[in] type Size type - which are defined from liblivebox-service package.
 * \param[in] cb After the request is sent to the master provider, this callback will be called.
 * \param[in] data This data will be passed to the callback.
 * \return handle
 * \sa ret_cb_t
 */
extern struct livebox *livebox_add_with_size(const char *pkgname, const char *content, const char *cluster, const char *category, double period, int type, ret_cb_t cb, void *data);

/*!
 * \brief Delete a livebox
 * \param[in] handle Handle of a livebox instance
 * \param[in] ret_cb_t return callback
 * \param[in] data user data for return callback
 * \return int
 * \sa ret_cb_t
 */
extern int livebox_del(struct livebox *handler, ret_cb_t cb, void *data);

/*!
 * \brief Set a livebox events callback
 * \param[in] cb Event handler
 * \param[in] data User data for the event handler
 * \return int LB_STATUS_SUCCESS if succeed to set event handler or LB_STATUS_ERROR_XXXX
 * \sa livebox_unset_event_handler
 */
extern int livebox_set_event_handler(int (*cb)(struct livebox *handler, enum livebox_event_type event, void *data), void *data);

/*!
 * \brief Unset the livebox event handler
 * \param[in] cb
 * \return void * pointer of 'data' which is used with the livebox_set_event_handler
 * \sa livebox_set_event_handler
 */
extern void *livebox_unset_event_handler(int (*cb)(struct livebox *handler, enum livebox_event_type event, void *data));

/*!
 * \note
 *   argument list
 * 	event, pkgname, filename, funcname
 *
 * \brief Live box fault event handler registeration function
 * \param[in] cb
 * \param[in] data
 * \return int LB_STATUS_SUCCESS if succeed to set fault event handler or LB_STATUS_ERROR_XXXX
 * \sa livebox_unset_fault_handler
 */
extern int livebox_set_fault_handler(int (*cb)(enum livebox_fault_type, const char *, const char *, const char *, void *), void *data);

/*!
 * \brief Unset the live box fault event handler
 * \param[in] cb
 * \return pointer of 'data' which is used with the livebox_set_fault_handler
 * \sa livebox_set_fault_handler
 */
extern void *livebox_unset_fault_handler(int (*cb)(enum livebox_fault_type, const char *, const char *, const char *, void *));

/*!
 * \brief Activate the faulted livebox.
 * \param[in] pkgname
 * \param[in] cb
 * \param[in] data
 * \return int
 * \sa ret_cb_t
 */
extern int livebox_activate(const char *pkgname, ret_cb_t cb, void *data);

/*!
 * \brief Resize the livebox
 * 1x1=175x175
 * 2x1=354x175
 * 2x2=354x354
 * 4x1=712x175
 * 4x2=712x354
 * 4x3=712x533
 * 4x4=712x712
 * 4x5=712x891
 * 4x6=712x1070
 * 21x21=224x215
 * 23x21=680x215
 * 23x23=680x653
 * 0x0=720x1280
 *
 * \param[in] handler Handler of a livebox
 * \param[in] type Type of a livebox size, LB_SIZE_TYPE_1x1, ...
 * \param[in] cb Result callback of the resize operation.
 * \param[in] data User data for return callback
 * \return int
 * \sa ret_cb_t
 */
extern int livebox_resize(struct livebox *handler, int type, ret_cb_t cb, void *data);

/*!
 * \brief Send the click event for a livebox.
 * \param[in] handler Handler of a livebox
 * \param[in] x Rational X of the content width.
 * \param[in] y Rational Y of the content height.
 * \return int
 */
extern int livebox_click(struct livebox *handler, double x, double y);

/*!
 * \brief Change the cluster/sub-cluster name of given livebox handler
 * \param[in] handler Handler of a livebox
 * \param[in] cluster New cluster of a livebox
 * \param[in] category New category of a livebox
 * \param[in] cb Result callback for changing the cluster/category of a livebox
 * \param[in] data User data for the result callback
 * \return int
 * \sa ret_cb_t
 */
extern int livebox_set_group(struct livebox *handler, const char *cluster, const char *category, ret_cb_t cb, void *data);

/*!
 * \brief Get the cluster and category(sub-cluster) name of given livebox (It is not I18N format, only english)
 * \param[in] handler Handler of a livebox
 * \param[out] cluster Storage(memory) for containing the cluster name
 * \param[out] category Storage(memory) for containing the category name
 * \return int
 */
extern int livebox_get_group(struct livebox *handler, char ** const cluster, char ** const category);

/*!
 * \brief Get the period of this livebox handler
 * \param[in] handler Handler of a livebox
 * \return double Current update period of a livebox
 */
extern double livebox_period(struct livebox *handler);

/*!
 * \brief Change the update period
 * \param[in] handler Handler of a livebox
 * \param[in] period New update period of a livebox
 * \param[in] cb Result callback of changing the update period of this livebox
 * \param[in] data User data for the result callback
 * \return int
 * \sa ret_cb_t
 */
extern int livebox_set_period(struct livebox *handler, double period, ret_cb_t cb, void *data);

/*!
 * \brief Is this an text type livebox?
 * \param[in] handler
 * \return content_type
 * \sa livebox_lb_type
 */
extern enum livebox_lb_type livebox_lb_type(struct livebox *handler);

/*!
 * \brief Is this livebox is created by a user?
 * \param[in] handler
 * \return int
 */
extern int livebox_is_user(struct livebox *handler);

/*!
 * \brief Get the content information string of given livebox
 * \param[in] handler
 * \return content
 */
extern const char *livebox_content(struct livebox *handler);

/*!
 * \brief Get the sub cluster title string of given livebox
 * \param[in] handler
 * \return sub cluster title
 */
extern const char *livebox_category_title(struct livebox *handler);

/*!
 * \brief Get the filename of given livebox, if it is an IMAGE type livebox
 * \param[in] handler
 * \return filename
 */
extern const char *livebox_filename(struct livebox *handler);

/*!
 * \brief Get the package name of given livebox handler
 * \param[in] handler
 * \return pkgname
 */
extern const char *livebox_pkgname(struct livebox *handler);

/*!
 * \brief Get the priority of a current content.
 * \param[in] handler
 * \return priority
 */
extern double livebox_priority(struct livebox *handler);

/*!
 * \brief Acquire the buffer of given livebox (Only for the buffer type)
 * \param[in] handler
 * \return address of a FB
 */
extern void *livebox_acquire_fb(struct livebox *handler);

/*!
 * \brief Release the buffer of a livebox (Only for the buffer type)
 * \param[in] buffer
 * \return int
 */
extern int livebox_release_fb(void *buffer);

/*!
 * \brief Get the reference count of Livebox buffer (Only for the buffer type)
 * \param[in] buffer
 * \return int
 */
extern int livebox_fb_refcnt(void *buffer);

/*!
 * \brief Acquire the buffer of a PD frame (Only for the buffer type)
 * \param[in] handler
 * \return int
 */
extern void *livebox_acquire_pdfb(struct livebox *handler);

/*!
 * \brief Release the acquired buffer of the PD Frame (Only for the buffer type)
 * \param[in] buffer
 * \return int
 */
extern int livebox_release_pdfb(void *buffer);

/*!
 * \brief Reference count of given PD buffer (Only for the buffer type)
 * \param[in] buffer
 * \return int
 */
extern int livebox_pdfb_refcnt(void *buffer);

/*!
 * \brief Get the size of the Livebox
 * \param[in] handler
 * \param[out] w
 * \param[out] h
 * \return int
 */
extern int livebox_size(struct livebox *handler);

/*!
 * \brief Get the size of the Progressive Disclosure
 * \param[in] handler
 * \param[out] w
 * \param[out] h
 * \return int
 */
extern int livebox_get_pdsize(struct livebox *handler, int *w, int *h);

/*!
 * \brief List of supported sizes of given handler
 * \param[in] handler
 * \param[out] cnt
 * \param[out] w
 * \param[out] h
 * \return int
 */
extern int livebox_get_supported_sizes(struct livebox *handler, int *cnt, int *size_list);

/*!
 * \brief BUFFER SIZE of the livebox if it is a buffer type
 * \param[in] handler
 * \return int
 */
extern int livebox_lbfb_bufsz(struct livebox *handler);

/*!
 * \brief BUFFER SIZE of the progiressive disclosure if it is a buffer type
 * \param[in] handler
 * \return int
 */
extern int livebox_pdfb_bufsz(struct livebox *handler);

/*!
 * \brief Send the content event (for buffer type) to provider(livebox)
 * \param[in] handler
 * \param[in] type
 * \param[in] x
 * \param[in] y
 * \return
 */
extern int livebox_content_event(struct livebox *handler, enum content_event_type type, double x, double y);

/*!
 * \brief Send the access event(for buffer type) to provider(livebox).
 * \param[in] handler
 * \param[in] access_event_type
 * \param[in] x
 * \param[in] y
 * \param[in] cb
 * \param[in] data
 * \return
 */
extern int livebox_access_event(struct livebox *handler, enum access_event_type type, double x, double y, ret_cb_t cb, void *data);

/*!
 * \brief Do pin up or not.
 * \param[in] handler
 * \param[in] flag
 * \param[in] cb
 * \param[in] data
 * \return int
 * \sa ret_cb_t
 */
extern int livebox_set_pinup(struct livebox *handler, int flag, ret_cb_t cb, void *data);

/*!
 * \brief Check the PIN-UP status of given handler
 * \param[in] handler
 * \return int
 */
extern int livebox_is_pinned_up(struct livebox *handler);

/*!
 * \brief Check the PINUP feature availability of the given handler
 * \param[in] handler
 * \return int
 */
extern int livebox_has_pinup(struct livebox *handler);

/*!
 * \brief Check the PD existence of given handler
 * \param[in] handler
 * \return int
 */
extern int livebox_has_pd(struct livebox *handler);

/*!
 * \brief Create the PD of given handler
 * \param[in] handler
 * \param[in] cb
 * \param[in] data
 * \return int
 * \sa ret_cb_t
 */
extern int livebox_create_pd(struct livebox *handler, ret_cb_t cb, void *data);

/*!
 * \brief Create the PD of given handler with the relative position from livebox
 * \param[in] handler
 * \param[in] x 0.0 ~ 1.0
 * \param[in] y 0.0 ~ 1.0
 * \param[in] cb
 * \param[in] data
 * \return int
 */
extern int livebox_create_pd_with_position(struct livebox *handler, double x, double y, ret_cb_t cb, void *data);

/*!
 * \brief PD position is updated.
 * \param[in] handler
 * \param[in] x 0.0 ~ 1.0
 * \param[in] y 0.0 ~ 1.0
 * \return int 0 if succeed to send request for updating position of the PD.
 */
extern int livebox_move_pd(struct livebox *handler, double x, double y);

/*!
 * \brief Destroy the PD of given handler if it is created.
 * \param[in] handler
 * \param[in] cb
 * \param[in] data
 * \return int
 * \sa ret_cb_t
 */
extern int livebox_destroy_pd(struct livebox *handler, ret_cb_t cb, void *data);

/*!
 * \brief Check the create status of given livebox handler
 * \param[in] handler
 * \return int
 */
extern int livebox_pd_is_created(struct livebox *handler);

/*!
 * \brief Check the content type of the progressive disclosure of given handler
 * \param[in] handler
 * \return int
 * \sa livebox_pd_type
 */
extern enum livebox_pd_type livebox_pd_type(struct livebox *handler);

/*!
 * \brief Check the existence of a livebox about given package name
 * \param[in] pkgname
 * \return int
 */
extern int livebox_is_exists(const char *pkgname);

/*!
 * \brief Set function table for parsing the text content of a livebox
 * \param[in] handler
 * \param[in] ops
 * \return int
 */
extern int livebox_set_text_handler(struct livebox *handler, struct livebox_script_operators *ops);

/*!
 * \brief Set function table for parsing the text content of a Progressive Disclosure
 * \param[in] handler
 * \param[in] ops
 * \return int
 */
extern int livebox_set_pd_text_handler(struct livebox *handler, struct livebox_script_operators *ops);

/*!
 * \brief Emit a text signal to given livebox only if it is a text type.
 * \param[in] handler
 * \param[in] emission
 * \param[in] source
 * \param[in] sx
 * \param[in] sy
 * \param[in] ex
 * \param[in] ey
 * \param[in] cb
 * \param[in] data
 * \return int
 * \sa ret_cb_t
 */
extern int livebox_emit_text_signal(struct livebox *handler, const char *emission, const char *source, double sx, double sy, double ex, double ey, ret_cb_t cb, void *data);

/*!
 * \brief Set a private data pointer to carry it using given handler
 * \param[in] handler
 * \param[in] data
 * \return int
 */
extern int livebox_set_data(struct livebox *handler, void *data);

/*!
 * \brief Get private data pointer which is carried by given handler
 * \param[in] handler
 * \return int
 */
extern void *livebox_get_data(struct livebox *handler);

/*!
 * \brief Subscribe the event for liveboxes only in given cluster and sub-cluster
 * \param[in] cluster   "*" can be used for subscribe all cluster's liveboxes event.
 *                      If you use the "*", value in the category will be ignored.
 * \param[in] category  "*" can be used for subscribe liveboxes events of all category(sub-cluster) in given "cluster"
 * \return int Success 0, fails error code
 */
extern int livebox_subscribe_group(const char *cluster, const char *category);

/*!
 * \brief Unsubscribe the event for the liveboxes, but you will receive already added liveboxes event.
 * \param[in] cluster   "*" can be used for subscribe all cluster's liveboxes event.
 *                      If you use the "*", value in the category will be ignored.
 * \param[in] category  "*" can be used for subscribe all sub-cluster's liveboxes event in given "cluster"
 * \return int Success 0, fails error code
 */
extern int livebox_unsubscribe_group(const char *cluster, const char *category);

/*!
 * \brief Refresh the group(cluster/sub-cluser(aka. category))
 * \param[in] cluster Cluster ID
 * \param[in] category Sub-cluster ID
 * \return int Success 0 or negative value
 */
extern int livebox_refresh_group(const char *cluster, const char *category);

/*!
 * \brief Refresh a livebox
 * \param[in] handler
 * \return int Success 0 or negative value
 */
extern int livebox_refresh(struct livebox *handler);

/*!
 * \brief Pixmap Id of a livebox content
 * \param[in] handler
 * \return int
 * \sa livebox_pd_pixmap
 */
extern int livebox_lb_pixmap(const struct livebox *handler);

/*!
 * \brief Pixmap Id of a PD content
 * \param[in] handler
 * \return int
 * \sa livebox_lb_pixmap
 */
extern int livebox_pd_pixmap(const struct livebox *handler);

/*!
 * \brief
 * \param[in] handler
 * \param[in] cb
 * \param[in] data
 * \return int
 * \sa livebox_release_pd_pixmap
 * \sa livebox_acquire_lb_pixmap
 * \sa ret_cb_t
 */
extern int livebox_acquire_pd_pixmap(struct livebox *handler, ret_cb_t cb, void *data);

/*!
 * \brief Release the acquired pixmap ID
 * \param[in] handler
 * \param[in] pixmap
 * \return int
 * \sa livebox_acquire_pd_pixmap
 * \sa livebox_release_lb_pixmap
 */
extern int livebox_release_pd_pixmap(struct livebox *handler, int pixmap);

/*!
 * \brief
 * \param[in] handler
 * \param[in] cb
 * \param[in] data
 * \return int
 * \sa livebox_release_lb_pixmap
 * \sa livebox_acquire_pd_pixmap
 * \sa ret_cb_t
 */
extern int livebox_acquire_lb_pixmap(struct livebox *handler, ret_cb_t cb, void *data);

/*!
 * \brief
 * \param[in] handler
 * \param[in] pixmap
 * \return int
 * \sa livebox_acquire_lb_pixmap
 * \sa livebox_release_pd_pixmap
 */
extern int livebox_release_lb_pixmap(struct livebox *handler, int pixmap);

/*!
 * \brief Update the visible state of a livebox
 * \param[in] handler Handler of a livebox
 * \param[in] state Configure the current visible state of a livebox
 * \return int
 */
extern int livebox_set_visibility(struct livebox *handler, enum livebox_visible_state state);

/*!
 * \brief Current visible state of a livebox
 * \param[in] handler Handler of a livebox
 * \return enum visible state
 */
extern enum livebox_visible_state livebox_visibility(struct livebox *handler);

/*!
 * \brief Set the update mode of current livebox
 *        if you set 1 for active update mode, you should get buffer without updated event from provider.
 *	  But is passive mode, you have to update content of a box when you get updated event.
 *	  Default is Passive mode.
 * \param[in] handler Handler of a livebox
 * \param[in] active_update 1 means active update, 0 means passive update (default)
 * \param[in] cb Result callback function
 * \param[in] data Callback data
 * \return int
 * \sa ret_cb_t
 */
extern int livebox_set_update_mode(struct livebox *handler, int active_update, ret_cb_t cb, void *data);

/*!
 * \brief Is this box in the active update mode?
 * \param[in] handler HAndler of a livebox
 * \return int 0 if passive mode or 1 if active mode or error code
 */
extern int livebox_is_active_update(struct livebox *handler);

/*!
 * \}
 */

/*!
 * \}
 */

#ifdef __cplusplus
}
#endif

#endif
