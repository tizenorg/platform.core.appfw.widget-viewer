/*
 * Copyright 2013  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.0 (the "License");
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

struct livebox;

/*!
 * \note size list
 * 172x172
 * 348x172
 * 348x348
 * 700x348
 * 700x172
 * 700x700
 */
#define DEFAULT_PERIOD -1.0f

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
	CONTENT_EVENT_LB_MASK		= 0x00000000,

	LB_MOUSE_DOWN			= CONTENT_EVENT_LB_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_DOWN,
	LB_MOUSE_UP			= CONTENT_EVENT_LB_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_UP,
	LB_MOUSE_MOVE			= CONTENT_EVENT_LB_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_MOVE,
	LB_MOUSE_ENTER			= CONTENT_EVENT_LB_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_ENTER,
	LB_MOUSE_LEAVE			= CONTENT_EVENT_LB_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_LEAVE,
	LB_MOUSE_SET			= CONTENT_EVENT_LB_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_SET,
	LB_MOUSE_UNSET			= CONTENT_EVENT_LB_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_UNSET,

	PD_MOUSE_DOWN			= CONTENT_EVENT_PD_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_DOWN,
	PD_MOUSE_UP			= CONTENT_EVENT_PD_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_UP,
	PD_MOUSE_MOVE			= CONTENT_EVENT_PD_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_MOVE,
	PD_MOUSE_ENTER			= CONTENT_EVENT_PD_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_ENTER,
	PD_MOUSE_LEAVE			= CONTENT_EVENT_PD_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_LEAVE,
	PD_MOUSE_SET			= CONTENT_EVENT_PD_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_SET,
	PD_MOUSE_UNSET			= CONTENT_EVENT_PD_MASK | CONTENT_EVENT_MOUSE_MASK | CONTENT_EVENT_MOUSE_UNSET,

	LB_KEY_DOWN			= CONTENT_EVENT_LB_MASK | CONTENT_EVENT_KEY_MASK | CONTENT_EVENT_KEY_DOWN,
	LB_KEY_UP			= CONTENT_EVENT_LB_MASK | CONTENT_EVENT_KEY_MASK | CONTENT_EVENT_KEY_UP,

	PD_KEY_DOWN			= CONTENT_EVENT_PD_MASK | CONTENT_EVENT_KEY_MASK | CONTENT_EVENT_KEY_DOWN,
	PD_KEY_UP			= CONTENT_EVENT_PD_MASK | CONTENT_EVENT_KEY_MASK | CONTENT_EVENT_KEY_UP,

	CONTENT_EVENT_MAX	= 0xFFFFFFFF,
};

enum access_event_type {
	ACCESS_EVENT_PD_MASK		= 0x10000000,
	ACCESS_EVENT_LB_MASK		= 0x00000000,

	ACCESS_EVENT_HIGHLIGHT		= 0x00000100, /*!< LB accessibility: Hightlight a object */
	ACCESS_EVENT_HIGHLIGHT_NEXT	= 0x00000200, /*!< LB accessibility: Set highlight to next object */
	ACCESS_EVENT_HIGHLIGHT_PREV	= 0x00000400, /*!< LB accessibility: Set highlight to prev object */
	ACCESS_EVENT_ACTIVATE		= 0x00000800, /*!< LB accessibility activate */
	ACCESS_EVENT_VALUE_CHANGE	= 0x00001000, /*!< LB accessibility value changed */
	ACCESS_EVENT_UNHIGHLIGHT	= 0x00002000, /*!< LB accessibility unhighlight */
	ACCESS_EVENT_SCROLL_DOWN	= 0x00004000, /*!< LB accessibility scroll down */
	ACCESS_EVENT_SCROLL_MOVE	= 0x00008000, /*!< LB accessibility scroll move */
	ACCESS_EVENT_SCROLL_UP		= 0x00010000, /*!< LB accessibility scroll up */

	LB_ACCESS_HIGHLIGHT		= ACCESS_EVENT_LB_MASK | ACCESS_EVENT_HIGHLIGHT,
	LB_ACCESS_HIGHLIGHT_NEXT	= ACCESS_EVENT_LB_MASK | ACCESS_EVENT_HIGHLIGHT_NEXT,
	LB_ACCESS_HIGHLIGHT_PREV	= ACCESS_EVENT_LB_MASK | ACCESS_EVENT_HIGHLIGHT_PREV,
	LB_ACCESS_ACTIVATE		= ACCESS_EVENT_LB_MASK | ACCESS_EVENT_ACTIVATE,
	LB_ACCESS_VALUE_CHANGE		= ACCESS_EVENT_LB_MASK | ACCESS_EVENT_VALUE_CHANGE,
	LB_ACCESS_UNHIGHLIGHT		= ACCESS_EVENT_LB_MASK | ACCESS_EVENT_UNHIGHLIGHT,
	LB_ACCESS_SCROLL_DOWN		= ACCESS_EVENT_LB_MASK | ACCESS_EVENT_SCROLL_DOWN,
	LB_ACCESS_SCROLL_MOVE		= ACCESS_EVENT_LB_MASK | ACCESS_EVENT_SCROLL_MOVE,
	LB_ACCESS_SCROLL_UP		= ACCESS_EVENT_LB_MASK | ACCESS_EVENT_SCROLL_UP,

	PD_ACCESS_HIGHLIGHT		= ACCESS_EVENT_PD_MASK | ACCESS_EVENT_HIGHLIGHT,
	PD_ACCESS_HIGHLIGHT_NEXT	= ACCESS_EVENT_PD_MASK | ACCESS_EVENT_HIGHLIGHT_NEXT,
	PD_ACCESS_HIGHLIGHT_PREV	= ACCESS_EVENT_PD_MASK | ACCESS_EVENT_HIGHLIGHT_PREV,
	PD_ACCESS_ACTIVATE		= ACCESS_EVENT_PD_MASK | ACCESS_EVENT_ACTIVATE,
	PD_ACCESS_VALUE_CHANGE		= ACCESS_EVENT_PD_MASK | ACCESS_EVENT_VALUE_CHANGE,
	PD_ACCESS_UNHIGHLIGHT		= ACCESS_EVENT_PD_MASK | ACCESS_EVENT_UNHIGHLIGHT,
	PD_ACCESS_SCROLL_DOWN		= ACCESS_EVENT_PD_MASK | ACCESS_EVENT_SCROLL_DOWN,
	PD_ACCESS_SCROLL_MOVE		= ACCESS_EVENT_PD_MASK | ACCESS_EVENT_SCROLL_MOVE,
	PD_ACCESS_SCROLL_UP		= ACCESS_EVENT_PD_MASK | ACCESS_EVENT_SCROLL_UP,
};

/* Exported to user app */
enum livebox_lb_type {
	LB_TYPE_IMAGE = 0x01, /*!< Contents of a livebox is based on the image file */
	LB_TYPE_BUFFER = 0x02, /*!< Contents of a livebox is based on canvas buffer(shared) */
	LB_TYPE_TEXT = 0x04, /*!< Contents of a livebox is based on formatted text file */
	LB_TYPE_PIXMAP = 0x08, /*!< Contens of a livebox is shared by the pixmap(depends on X) */

	LB_TYPE_INVALID = 0xFF,
};

enum livebox_pd_type {
	PD_TYPE_BUFFER = 0x01, /*!< Contents of a PD is based on canvas buffer(shared) */
	PD_TYPE_TEXT = 0x02, /*!< Contents of a PD is based on formatted text file */
	PD_TYPE_PIXMAP = 0x04, /*!< Contents of a livebox is shared by the pixmap(depends on X) */

	PD_TYPE_INVALID = 0xFF,
};

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

	LB_EVENT_IGNORED, /*!< Request is ignored */
};

enum livebox_fault_type {
	LB_FAULT_DEACTIVATED, /*!< Livebox is deactivated by its fault operation */
	LB_FAULT_PROVIDER_DISCONNECTED, /*!< Provider is disconnected */
};

enum livebox_visible_state { /*!< Must be sync'd with the provider */
	LB_SHOW = 0x00, /*!< Livebox is showed. Default state */
	LB_HIDE = 0x01, /*!< Livebox is hide, Update timer is not be freezed. but you cannot receive any updates events. you should refresh(reload) the content of a livebox when you make this show again */

	LB_HIDE_WITH_PAUSE = 0x02, /*!< Livebix is hide, it will paused the update timer, but if a livebox update its contents, update event will come to you */

	LB_VISIBLE_ERROR = 0xFFFFFFFF, /* To enlarge the size of this enumeration type */
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
 * \brief Prototype of the return callback of every async functions.
 * \param[in] handle
 * \param[in] ret
 * \param[in] data
 * \return void
 */
typedef void (*ret_cb_t)(struct livebox *handle, int ret, void *data);

/*!
 * \brief Initialize the livebox system
 * \param[in] disp If you have X Display connection object, you can re-use it. but you should care its life cycle.
 *                 It must be alive before call the livebox_fini
 * \return int 0 if success or < 0 (errno)
 * \sa livebox_fini
 */
extern int livebox_init(void *disp);

/*!
 * \brief Finalize the livebox system
 * \return int 0 if success, -EINVAL if livebox_init is not called.
 * \sa livebox_init
 */
extern int livebox_fini(void);

/*!
 * \brief Client is paused.
 * \return int 0 if success, -EFAULT if it failed to send state(paused) info
 * \sa livebox_client_resumed
 */
extern int livebox_client_paused(void);

/*!
 * \brief Client is rfesumed.
 * \return int 0 if success, -EFAULT if it failed to send state(resumed) info
 * \sa livebox_client_paused
 */
extern int livebox_client_resumed(void);

/*!
 * \brief Add a new livebox
 * \param[in] pkgname Livebox Id
 * \param[in] content Will be passed to the livebox.
 * \param[in] cluster
 * \param[in] category
 * \param[in] period Update period. if you set DEFAULT_PERIOD, the provider will use the default period which is described in the manifest.
 * \param[in] cb After send the request to the provider, its result will be passed
 * \param[in] data
 * \return handle
 */
extern struct livebox *livebox_add(const char *pkgname, const char *content, const char *cluster, const char *category, double period, ret_cb_t cb, void *data);
extern struct livebox *livebox_add_with_size(const char *pkgname, const char *content, const char *cluster, const char *category, double period, int type, ret_cb_t cb, void *data);

/*!
 * \brief Delete a livebox
 * \param[in] handle
 * \param[in] ret_cb_t return callback
 * \param[in] data user data for return callback
 * \return int
 */
extern int livebox_del(struct livebox *handler, ret_cb_t cb, void *data);

/*!
 * \brief Set a livebox events callback
 * \param[in] cb Event handler
 * \param[in] data User data for the event handler
 * \return int
 * \sa livebox_unset_event_handler
 */

extern int livebox_set_event_handler(int (*cb)(struct livebox *handler, enum livebox_event_type event, void *data), void *data);

/*!
 * \brief Unset the livebox event handler
 * \param[in] cb
 * \return void * pointer of 'data' which is registered from the livebox_event_handler_set
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
 * \return int
 * \sa livebox_unset_fault_handler
 */
extern int livebox_set_fault_handler(int (*cb)(enum livebox_fault_type, const char *, const char *, const char *, void *), void *data);

/*!
 * \brief Unset the live box fault event handler
 * \param[in] cb
 * \return pointer of 'data'
 * \sa livebox_set_fault_handler
 */
extern void *livebox_unset_fault_handler(int (*cb)(enum livebox_fault_type, const char *, const char *, const char *, void *));

/*!
 * \brief Activate the faulted livebox.
 * \param[in] pkgname
 * \param[in] cb
 * \param[in] data
 * \return int
 */
extern int livebox_activate(const char *pkgname, ret_cb_t cb, void *data);

/*!
 * \brief Resize the livebox
 * \param[in] handler Handler of a livebox
 * \param[in] type Type of a livebox size, LB_SIZE_TYPE_1x1, ...
 * \param[in] cb Result callback of the resize operation.
 * \param[in] data User data for return callback
 * \return int
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
 */
extern int livebox_set_period(struct livebox *handler, double period, ret_cb_t cb, void *data);

/*!
 * \brief Is this an text type livebox?
 * \param[in] handler
 * \return content_type
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
 */
extern int livebox_set_update_mode(struct livebox *handler, int active_update, ret_cb_t cb, void *data);

/*!
 * \brief Is this box in the active update mode?
 * \param[in] handler HAndler of a livebox
 * \return int 0 if passive mode or 1 if active mode or error code
 */
extern int livebox_is_active_update(struct livebox *handler);

#ifdef __cplusplus
}
#endif

#endif
/* End of a file */
