#ifndef __LIVEBOX_H
#define __LIVEBOX_H

#ifdef __cplusplus
extern "C" {
#endif

struct livebox;

/*!
 * \note size list
 * 172x172
 * 372x172
 * 372x372
 * 720x372
 */
#define NR_OF_SIZE_LIST 4
#define DEFAULT_PERIOD -1.0f

static const struct supported_size_list {
	int w;
	int h;
} SIZE_LIST[NR_OF_SIZE_LIST] = {
	{ 172, 172 },
	{ 348, 172 },
	{ 348, 348 },
	{ 700, 348 },
};

struct livebox_script_operators {
	int (*update_begin)(struct livebox *handle);
	int (*update_end)(struct livebox *handle);

	int (*update_text)(struct livebox *handle, const char *id, const char *part, const char *data);
	int (*update_image)(struct livebox *handle, const char *id, const char *part, const char *data);
	int (*update_edje)(struct livebox *handle, const char *id, const char *part, const char *file, const char *group);
	int (*update_signal)(struct livebox *handle, const char *id, const char *emission, const char *signal);
	int (*update_drag)(struct livebox *handle, const char *id, const char *part, double dx, double dy);
	int (*update_info_size)(struct livebox *handle, const char *id, int w, int h);
	int (*update_info_category)(struct livebox *handle, const char *id, const char *category);
};

extern int livebox_init(void);
extern int livebox_fini(void);

extern struct livebox *livebox_add(const char *pkgname, const char *content, const char *cluster, const char *category, double period);
extern int livebox_del(struct livebox *handler, int unused);

/*!
 * \note event list
 * "lb,updated" - Contents of the given livebox is updated.
 * "pd,updated" - Contents of the given pd is updated.
 * "lb,created" - A new livebox is created
 * "lb,deleted" - Given livebox is deleted
 * "pd,created" - frame buffer of PD is created
 * "pd,create,failed" - frame buffer of PD is not created
 * "pd,deleted" - frame buffer of PD is deleted
 * "pinup,changed" - Pinup status is changed
 * "pinup,failed" - Pinup status is not changed
 * "event,ignored" - Event is not delivered to the livebox
 */
extern int livebox_event_handler_set(int (*cb)(struct livebox *handler, const char *event, void *data), void *data);
/*!
 * return pointer of 'data'
 */
extern void *livebox_event_handler_unset(int (*cb)(struct livebox *handler, const char *event, void *data));

/*!
 * \note argument list
 * 	event, pkgname, filename, funcname
 *
 * event list
 * "activated" - Package is successfully activated
 * "invalid,request" - Package is not valid
 * "deactivated" - Package is deactivated
 * "activation,failed" - Failed to activate a package
 * "provider,disconnected" - Provder is disconnected
 */
extern int livebox_fault_handler_set(int (*cb)(const char *, const char *, const char *, const char *, void *), void *data);
/*!
 * return pointer of 'data'
 */
extern void *livebox_fault_handler_unset(int (*cb)(const char *, const char *, const char *, const char *, void *));

extern int livebox_activate(const char *pkgname);

extern int livebox_resize(struct livebox *handler, int w, int h);
extern int livebox_click(struct livebox *handler, double x, double y);

extern int livebox_set_group(struct livebox *handler, const char *cluster, const char *category);
extern int livebox_get_group(struct livebox *handler, char ** const cluster, char ** const category);

extern double livebox_period(struct livebox *handler);
extern int livebox_set_period(struct livebox *handler, double period);

extern int livebox_delete_cluster(const char *cluster);
extern int livebox_delete_category(const char *cluster, const char *category);

extern int livebox_is_text(struct livebox *handler);
extern int livebox_is_file(struct livebox *handler);
extern int livebox_is_user(struct livebox *handler);

extern const char *livebox_content(struct livebox *handler);
extern const char *livebox_filename(struct livebox *handler);
extern void *livebox_fb(struct livebox *handler);
extern void *livebox_pdfb(struct livebox *handler);

extern int livebox_get_size(struct livebox *handler, int *w, int *h);
extern int livebox_get_pdsize(struct livebox *handler, int *w, int *h);

extern int livebox_get_supported_sizes(struct livebox *handler, int *cnt, int *w, int *h);
extern const char *livebox_pkgname(struct livebox *handler);
extern double livebox_priority(struct livebox *handler);

extern int livebox_lbfb_bufsz(struct livebox *handler);
extern int livebox_pdfb_bufsz(struct livebox *handler);

extern int livebox_pd_mouse_down(struct livebox *handler, double x, double y);
extern int livebox_pd_mouse_up(struct livebox *handler, double x, double y);
extern int livebox_pd_mouse_move(struct livebox *handler, double x, double y);

extern int livebox_livebox_mouse_down(struct livebox *handler, double x, double y);
extern int livebox_livebox_mouse_up(struct livebox *handler, double x, double y);
extern int livebox_livebox_mouse_move(struct livebox *handler, double x, double y);

extern int livebox_set_pinup(struct livebox *handler, int flag);
extern int livebox_pinup(struct livebox *handler);
extern int livebox_has_pinup(struct livebox *handler);

extern int livebox_has_pd(struct livebox *handler);
extern int livebox_create_pd(struct livebox *handler);
extern int livebox_destroy_pd(struct livebox *handler);
extern int livebox_pd_is_created(struct livebox *handler);
extern int livebox_pd_is_text(struct livebox *handler);

extern int livebox_set_data(struct livebox *handler, void *data);
extern void *livebox_get_data(struct livebox *handler);

extern int livebox_is_exists(const char *pkgname);

extern int livebox_set_text_handler(struct livebox *handler, struct livebox_script_operators *ops);
extern int livebox_pd_set_text_handler(struct livebox *handler, struct livebox_script_operators *ops);

extern int livebox_text_emit_signal(struct livebox *handler, const char *emission, const char *source, double sx, double sy, double ex, double ey);
#ifdef __cplusplus
}
#endif

#endif
/* End of a file */
