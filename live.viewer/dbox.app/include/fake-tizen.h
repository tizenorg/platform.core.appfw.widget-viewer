#ifndef __APPCORE_EFL_FAKE_H_
#define __APPCORE_EFL_FAKE_H_

#include <Ecore_Input.h>
#include <Ecore.h>

//#define PREFIX "./"
//#define ELM_DEMO_EDJ "./elm_demo_tizen.edj"
#define _(A) A
#define N_(A) A
#define KEY_END "15"
#define VCONFKEY_LANGSET NULL

typedef struct _bundle
{
	void *data;
} bundle;

struct appcore_ops
{
	int (*create)(void *data);
	int (*terminate)(void *data);
	int (*pause)(void *data);
	int (*resume)(void *data);
	int (*reset)(bundle *b, void *data);
	void *data;
};

#define APPCORE_EVENT_LANG_CHANGE 0

enum appcore_rm {
    APPCORE_RM_UNKNOWN,
            /**< Unknown mode */
    APPCORE_RM_PORTRAIT_NORMAL,
                 /**< Portrait mode */
    APPCORE_RM_PORTRAIT_REVERSE,
                  /**< Portrait upside down mode */
    APPCORE_RM_LANDSCAPE_NORMAL,
                  /**< Left handed landscape mode */
    APPCORE_RM_LANDSCAPE_REVERSE,
                /**< Right handed landscape mode */
};

int appcore_measure_time();
void appcore_measure_start();
int  appcore_measure_time_from(const char *str);
void appcore_set_rotation_cb(int (*_rotation_cb)(enum appcore_rm mode, void *data), void *data);
void appcore_set_i18n(const char *pkg, const char* locale);
int appcore_efl_main(const char *pkg, int *argc, char **argv, struct appcore_ops *ops);
typedef int (*FakeCb)(void *);
void appcore_set_event_callback(int event, FakeCb fake_cb, void *data);

// vconf
char *vconf_get_str(const char *in_key);
void vconf_get_int(const char *key, int *value);
#define VCONFKEY_SETAPPL_ACCESSIBILITY_FONT_SIZE 1

#endif
