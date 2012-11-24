#if defined(LOG_TAG)
#undef LOG_TAG
#define LOG_TAG "live-viewer"
#endif

#if !defined(FLOG)
#define DbgPrint(format, arg...)	LOGD("[[32m%s/%s[0m:%d] " format, basename(__FILE__), __func__, __LINE__, ##arg)
#define ErrPrint(format, arg...)	LOGE("[[32m%s/%s[0m:%d] " format, basename(__FILE__), __func__, __LINE__, ##arg)
#else
extern FILE *__file_log_fp;
#define DbgPrint(format, arg...) do { fprintf(__file_log_fp, "[LOG] [[32m%s/%s[0m:%d] " format, basename(__FILE__), __func__, __LINE__, ##arg); fflush(__file_log_fp); } while (0)

#define ErrPrint(format, arg...) do { fprintf(__file_log_fp, "[ERR] [[32m%s/%s[0m:%d] " format, basename(__FILE__), __func__, __LINE__, ##arg); fflush(__file_log_fp); } while (0)
#endif

class CUtil {
public:
	static char *GetIconFile(const char *pkgname);
	static Evas_Object *CreateCanvasImage(Evas_Object *parent, int w, int h);

	static int UpdateCanvasImage(Evas_Object *image, void *data, int w, int h);
	static int UpdateCanvasImage(Evas_Object *image, const char *filename, int w, int h);

private:
	/* This class is not able to be instantiated */
	CUtil(void);
	virtual ~CUtil(void);
};

