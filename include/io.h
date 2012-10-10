extern int io_init(void);
extern int io_fini(void);
extern char *io_lb_pkgname(const char *pkgname);
extern char *io_app_pkgname(const char *lbpkg);
extern int io_enumerate_cluster_list(int (*cb)(const char *cluster, void *data), void *data);
extern int io_enumerate_category_list(const char *cluster, int (*cb)(const char *cluster, const char *category, void *data), void *data);

/* End of a file */
