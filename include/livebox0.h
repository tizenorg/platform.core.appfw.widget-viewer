extern int lb_set_group(struct livebox *handler, const char *cluster, const char *category);
extern int lb_set_size(struct livebox *handler, int w, int h);
extern int lb_set_pdsize(struct livebox *handler, int w, int h);
extern int lb_invoke_event_handler(struct livebox *handler, const char *event);
extern int lb_invoke_fault_handler(const char *event, const char *pkgname, const char *filename, const char *function);
extern int lb_set_content(struct livebox *handler, const char *content);
extern int lb_set_auto_launch(struct livebox *handler, int auto_launch);
extern struct livebox *lb_find_livebox(const char *pkgname, const char *filename);
extern struct livebox *lb_new_livebox(const char *pkgname, const char *filename);
extern struct livebox *lb_find_livebox_by_timestamp(double timestamp);
extern void lb_set_filename(struct livebox *handler, const char *filename);
extern void lb_set_size_list(struct livebox *handler, int size_list);
extern void lb_set_priority(struct livebox *handler, double priority);
extern void lb_set_lb_fb(struct livebox *handler, const char *filename);
extern void lb_set_pd_fb(struct livebox *handler, const char *filename);
extern struct fb_info *lb_get_pd_fb(struct livebox *handler);
extern struct fb_info *lb_get_lb_fb(struct livebox *handler);
extern void lb_update_pd_fb(struct livebox *handler, int w, int h);
extern void lb_update_lb_fb(struct livebox *handler, int w, int h);
extern void lb_set_user(struct livebox *handler, int user);
extern void lb_set_pinup(struct livebox *handler, int pinup);
extern void lb_set_text_lb(struct livebox *handler, int flag);
extern void lb_set_text_pd(struct livebox *handler, int flag);
extern int lb_text_lb(struct livebox *handler);
extern int lb_text_pd(struct livebox *handler);
extern void lb_set_period(struct livebox *handler, double period);

struct livebox {
	char *cluster;
	char *category;

	char *pkgname;
	char *filename;

	int size_list;

	double timestamp;

	enum {
		FILEDATA,
		FBDATA,
	} data_type;

	char *content;
	int lb_w;
	int lb_h;
	int pd_w;
	int pd_h;
	int auto_launch;
	double priority;
	double period;

	int nr_of_sizes;

	struct fb_info *lb_fb;
	struct fb_info *pd_fb;

	int is_user;
	int is_pinned_up;
	int pinup_supported;
	int text_lb;
	int text_pd;

	void *data;

	struct livebox_script_operators ops;
	struct livebox_script_operators pd_ops;
};

/* End of a file */
