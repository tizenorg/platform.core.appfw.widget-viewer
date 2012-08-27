struct fb_info;

enum buffer_type { /*!< Must have to be sync with libprovider, liblivebox-viewer */
	BUFFER_TYPE_FILE,
	BUFFER_TYPE_SHM,
	BUFFER_TYPE_PIXMAP,
	BUFFER_TYPE_ERROR,
};

extern int fb_init(void *disp);
extern int fb_fini(void);
extern const char *fb_id(struct fb_info *info);
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
