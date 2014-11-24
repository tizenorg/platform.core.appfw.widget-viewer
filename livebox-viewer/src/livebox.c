#include <stdio.h>

#include <livebox-service.h>

#include "livebox.h"

enum livebox_status {
	LB_STATUS_SUCCESS = 0x00000000, /**< Operation is successfully completed */
	LB_STATUS_ERROR = 0x80000000, /**< This will be OR'd with other specific error value */
	LB_STATUS_ERROR_INVALID = LB_STATUS_ERROR | 0x0001, /**< Invalid request */
	LB_STATUS_ERROR_FAULT = LB_STATUS_ERROR | 0x0002, /**< Fault - Unable to recover from the error */
	LB_STATUS_ERROR_MEMORY = LB_STATUS_ERROR | 0x0004, /**< Memory is not enough to do this operation */
	LB_STATUS_ERROR_EXIST = LB_STATUS_ERROR | 0x0008, /**< Already exists */
	LB_STATUS_ERROR_BUSY = LB_STATUS_ERROR | 0x0010, /**< Busy so the operation is not started(accepted), try again */
	LB_STATUS_ERROR_PERMISSION = LB_STATUS_ERROR | 0x0020, /**< Permission error */
	LB_STATUS_ERROR_ALREADY = LB_STATUS_ERROR | 0x0040, /**< Operation is already started */
	LB_STATUS_ERROR_CANCEL = LB_STATUS_ERROR | 0x0080, /**< Operation is canceled */
	LB_STATUS_ERROR_IO = LB_STATUS_ERROR | 0x0100, /**< I/O Error */
	LB_STATUS_ERROR_NOT_EXIST = LB_STATUS_ERROR | 0x0200, /**< Not exists */
	LB_STATUS_ERROR_TIMEOUT = LB_STATUS_ERROR | 0x0400, /**< Timeout */
	LB_STATUS_ERROR_NOT_IMPLEMENTED = LB_STATUS_ERROR | 0x0800, /**< Operation is not implemented */
	LB_STATUS_ERROR_NO_SPACE = LB_STATUS_ERROR | 0x1000, /**< No space to operate */
	LB_STATUS_ERROR_DISABLED = LB_STATUS_ERROR | 0x2000 /**< Disabled */
};

enum lb_type { /*!< Must have to be sync with data-provider-master */
	_LB_TYPE_NONE = 0x0,
	_LB_TYPE_SCRIPT,
	_LB_TYPE_FILE,
	_LB_TYPE_TEXT,
	_LB_TYPE_BUFFER,
	_LB_TYPE_ELEMENTARY
};

enum pd_type { /*!< Must have to be sync with data-provider-master */
	_PD_TYPE_NONE = 0x0,
	_PD_TYPE_SCRIPT,
	_PD_TYPE_TEXT,
	_PD_TYPE_BUFFER,
	_PD_TYPE_ELEMENTARY
};

enum livebox_state {
	CREATE = 0xBEEFbeef,
	DELETE = 0xDEADdead, /* Delete only for this client */
	DESTROYED = 0x00DEAD00
};

struct livebox_common {
	enum livebox_state state;

	struct dlist *livebox_list;
	int refcnt;

	char *cluster;
	char *category;

	char *pkgname;
	char *id;

	char *content;
	char *title;
	char *filename;

	double timestamp;

	struct alt_info {
		char *icon;
		char *name;
	} alt;

	enum livebox_delete_type delete_type;

	int is_user;
	int is_pd_created;
	int is_pinned_up;
	int is_active_update;
	enum livebox_visible_state visible;

	struct {
		enum lb_type type;
		struct fb_info *fb;

		int size_list;

		int width;
		int height;
		double priority;

		char *auto_launch;
		double period;
		int pinup_supported;
		int mouse_event;

		/* For the filtering event */
		double x;
		double y;
		char *lock;
		int lock_fd;
	} lb;

	struct {
		enum pd_type type;
		struct fb_info *fb;

		int width;
		int height;

		int default_width;
		int default_height;

		/* For the filtering event */
		double x;
		double y;
		char *lock;
		int lock_fd;
	} pd;

	int nr_of_sizes;

	struct requested_flag {
		unsigned int created:1;
		unsigned int deleted:1;
		unsigned int pinup:1;
		unsigned int group_changed:1;
		unsigned int period_changed:1;
		unsigned int size_changed:1;
		unsigned int pd_created:1;
		unsigned int pd_destroyed:1;
		unsigned int update_mode:1;
		unsigned int access_event:1;
		unsigned int key_event:1;

		/*!
		 * \note
		 * Reserved
		 */
		unsigned int reserved:21;
	} request;
};

struct job_item {
	struct livebox *handle;
	ret_cb_t cb;
	int ret;
	void *data;
};

struct livebox {
	enum livebox_state state;

	int refcnt;
	int paused_updating;

	enum livebox_visible_state visible;
	struct livebox_common *common;

	void *data;

	struct callback_table {
		struct livebox_script_operators lb_ops;
		struct livebox_script_operators pd_ops;

		struct created {
			ret_cb_t cb;
			void *data;
		} created;

		struct deleted {
			ret_cb_t cb;
			void *data;
		} deleted;

		struct pinup {
			ret_cb_t cb;
			void *data;
		} pinup;

		struct group_changed {
			ret_cb_t cb;
			void *data;
		} group_changed;

		struct period_changed {
			ret_cb_t cb;
			void *data;
		} period_changed;

		struct size_changed {
			ret_cb_t cb;
			void *data;
		} size_changed;

		struct pd_created {
			ret_cb_t cb;
			void *data;
		} pd_created;

		struct pd_destroyed {
			ret_cb_t cb;
			void *data;
		} pd_destroyed;

		struct update_mode {
			ret_cb_t cb;
			void *data;
		} update_mode;

		struct access_event {
			ret_cb_t cb;
			void *data;
		} access_event;

		struct key_event {
			ret_cb_t cb;
			void *data;
		} key_event;
	} cbs;
};

int livebox_init(void *disp)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

int livebox_init_with_options(void *disp, int prevent_overwrite, double event_filter, int use_thread)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

int livebox_fini(void)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

int livebox_client_paused(void)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

int livebox_client_resumed(void)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

struct livebox *livebox_add(const char *pkgname, const char *content, const char *cluster, const char *category, double period, int type, ret_cb_t cb, void *data)
{
    return NULL;
}

int livebox_del(struct livebox *handler, int type, ret_cb_t cb, void *data)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

int livebox_set_event_handler(int (*cb)(struct livebox *handler, enum livebox_event_type event, void *data), void *data)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

void *livebox_unset_event_handler(int (*cb)(struct livebox *handler, enum livebox_event_type event, void *data))
{
    return NULL;
}

int livebox_set_fault_handler(int (*cb)(enum livebox_fault_type, const char *, const char *, const char *, void *), void *data)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

void *livebox_unset_fault_handler(int (*cb)(enum livebox_fault_type, const char *, const char *, const char *, void *))
{
    return NULL;
}

int livebox_activate(const char *pkgname, ret_cb_t cb, void *data)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

int livebox_resize(struct livebox *handler, int type, ret_cb_t cb, void *data)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

int livebox_click(struct livebox *handler, double x, double y)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

int livebox_set_group(struct livebox *handler, const char *cluster, const char *category, ret_cb_t cb, void *data)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

int livebox_get_group(struct livebox *handler, const char **cluster, const char **category)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

double livebox_period(struct livebox *handler)
{
    return 0.0f;
}

int livebox_set_period(struct livebox *handler, double period, ret_cb_t cb, void *data)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

enum livebox_lb_type livebox_lb_type(struct livebox *handler)
{
    return LB_TYPE_INVALID;
}

int livebox_is_user(struct livebox *handler)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

const char *livebox_content(struct livebox *handler)
{
    return NULL;
}

const char *livebox_category_title(struct livebox *handler)
{
    return NULL;
}

const char *livebox_filename(struct livebox *handler)
{
    return NULL;
}

const char *livebox_pkgname(struct livebox *handler)
{
    return NULL;
}

double livebox_priority(struct livebox *handler)
{
    return 0.0f;
}

void *livebox_acquire_fb(struct livebox *handler)
{
    return NULL;
}

int livebox_release_fb(void *buffer)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

int livebox_fb_refcnt(void *buffer)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

void *livebox_acquire_pdfb(struct livebox *handler)
{
    return NULL;
}

int livebox_release_pdfb(void *buffer)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

int livebox_pdfb_refcnt(void *buffer)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

int livebox_size(struct livebox *handler)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

int livebox_get_pdsize(struct livebox *handler, int *w, int *h)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

int livebox_get_supported_sizes(struct livebox *handler, int *cnt, int *size_list)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

int livebox_lbfb_bufsz(struct livebox *handler)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

int livebox_pdfb_bufsz(struct livebox *handler)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

int livebox_mouse_event(struct livebox *handler, enum content_event_type type, double x, double y)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

int livebox_access_event(struct livebox *handler, enum access_event_type type, double x, double y, ret_cb_t cb, void *data)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

int livebox_key_event(struct livebox *handler, enum content_event_type type, unsigned int keycode, ret_cb_t cb, void *data)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

int livebox_set_pinup(struct livebox *handler, int flag, ret_cb_t cb, void *data)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

int livebox_is_pinned_up(struct livebox *handler)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

int livebox_has_pinup(struct livebox *handler)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

int livebox_has_pd(struct livebox *handler)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

int livebox_create_pd(struct livebox *handler, ret_cb_t cb, void *data)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

int livebox_create_pd_with_position(struct livebox *handler, double x, double y, ret_cb_t cb, void *data)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

int livebox_move_pd(struct livebox *handler, double x, double y)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

int livebox_destroy_pd(struct livebox *handler, ret_cb_t cb, void *data)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

int livebox_pd_is_created(struct livebox *handler)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

enum livebox_pd_type livebox_pd_type(struct livebox *handler)
{
    return PD_TYPE_INVALID;
}

int livebox_is_exists(const char *pkgname)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

int livebox_set_text_handler(struct livebox *handler, struct livebox_script_operators *ops)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

int livebox_set_pd_text_handler(struct livebox *handler, struct livebox_script_operators *ops)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

int livebox_emit_text_signal(struct livebox *handler, const char *emission, const char *source, double sx, double sy, double ex, double ey, ret_cb_t cb, void *data)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

int livebox_set_data(struct livebox *handler, void *data)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

void *livebox_get_data(struct livebox *handler)
{
    return NULL;
}

int livebox_subscribe_group(const char *cluster, const char *category)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

int livebox_unsubscribe_group(const char *cluster, const char *category)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

int livebox_refresh_group(const char *cluster, const char *category, int force)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

int livebox_refresh(struct livebox *handler, int force)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

int livebox_lb_pixmap(const struct livebox *handler)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

int livebox_pd_pixmap(const struct livebox *handler)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

int livebox_acquire_pd_pixmap(struct livebox *handler, ret_cb_t cb, void *data)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

int livebox_release_pd_pixmap(struct livebox *handler, int pixmap)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

int livebox_acquire_lb_pixmap(struct livebox *handler, ret_cb_t cb, void *data)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

int livebox_release_lb_pixmap(struct livebox *handler, int pixmap)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

int livebox_set_visibility(struct livebox *handler, enum livebox_visible_state state)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

enum livebox_visible_state livebox_visibility(struct livebox *handler)
{
    return LB_VISIBLE_ERROR;
}

int livebox_set_update_mode(struct livebox *handler, int active_update, ret_cb_t cb, void *data)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

int livebox_is_active_update(struct livebox *handler)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

int livebox_sync_pd_fb(struct livebox *handler)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

int livebox_sync_lb_fb(struct livebox *handler)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

const char *livebox_alt_icon(struct livebox *handler)
{
    return NULL;
}

const char *livebox_alt_name(struct livebox *handler)
{
    return NULL;
}

int livebox_acquire_fb_lock(struct livebox *handler, int is_pd)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

int livebox_release_fb_lock(struct livebox *handler, int is_pd)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

int livebox_set_option(enum livebox_option_type option, int state)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

int livebox_option(enum livebox_option_type option)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

int livebox_set_auto_launch_handler(int (*launch_handler)(struct livebox *handler, const char *appid, void *data), void *data)
{
    return LB_STATUS_ERROR_NOT_IMPLEMENTED;
}

/* End of a file */
