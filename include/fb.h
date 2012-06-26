struct fb_info;

extern int fb_init(void);
extern int fb_fini(void);
extern const char *fb_filename(struct fb_info *info);
extern int fb_get_size(struct fb_info *info, int *w, int *h);
extern int fb_sync(struct fb_info *info);
extern int fb_size(struct fb_info *info);
extern int fb_refcnt(void *data);
extern int fb_is_created(struct fb_info *info);

extern struct fb_info *fb_create(const char *filename, int w, int h);
extern int fb_create_buffer(struct fb_info *info);
extern int fb_destroy_buffer(struct fb_info *info);
extern int fb_destroy(struct fb_info *info);

extern void *fb_acquire_buffer(struct fb_info *info);
extern int fb_release_buffer(void *data);

/* End of a file */
