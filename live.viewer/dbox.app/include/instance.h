struct instance;

extern struct instance *instance_create(const char *id, const char *content, int w, int h);
extern int instance_destroy(struct instance *inst);

extern struct instance *instance_find(const char *id);

extern int instance_set_size_by_id(const char *id, int w, int h);
extern int instance_set_size_by_handle(struct instance *handle, int w, int h);

extern int instance_get_size_by_id(const char *id, int *w, int *h);
extern int instance_get_size_by_handle(struct instance *handle, int *w, int *h);

extern int instance_set_data(struct instance *inst, void *data);
extern void *instance_data(struct instance *inst);

extern int instance_crawling(int (*cb)(struct instance *inst, void *data), void *data);
/* End of a file */
