#include <Elementary.h>
#include <Ecore_X.h>
#include <appcore-efl.h>
#include <libgen.h>

#include <dlog.h>
#include <ail.h>

#include <livebox.h>

#include "dlist.h"
#include "CUtil.h"
#include "CResourceMgr.h"
#include "CLiveBoxMgr.h"
#include "CMain.h"

int errno;

CMain::CMain()
: m_fVerbose(0)
, m_pLiveBoxMgr(NULL)
{
}

CMain::~CMain()
{
}

int CMain::OnCreate(void)
{
	Evas_Object *win;
	int w;
	int h;

	/* Getting the entire screen(root window) size */
	ecore_x_window_size_get(0, &w, &h);

	win = elm_win_add(NULL, "Live viewer", ELM_WIN_BASIC);
	if (!win) {
		ErrPrint("Failed to create a new window\n");
		return -EFAULT;
	}
	evas_object_move(win, 0, 0);
	evas_object_resize(win, w, h);
	evas_object_show(win);

	try {
		m_pLiveBoxMgr = new CLiveBoxMgr();
	} catch (...) {
		ErrPrint("Failed to allocate a new livebox manager\n");
		evas_object_del(win);
		return -EFAULT;
	}

	CResourceMgr::GetInstance()->RegisterObject("window", win);
	return 0;
}

int CMain::OnTerminate(void)
{
	Evas_Object *win;

	delete m_pLiveBoxMgr;

	win = (Evas_Object *)CResourceMgr::GetInstance()->UnregisterObject("window");
	if (win)
		evas_object_del(win);

	return 0;
}

int CMain::OnPause(void)
{
	return 0;
}

int CMain::OnResume(void)
{
	return 0;
}

int CMain::OnReset(bundle *b)
{
	const char *tmp;
	const char *pkgname;
	const char *cluster;
	const char *category;
	const char *content;
	double period;

	pkgname = bundle_get_val(b, "pkgname");
	if (!pkgname) {
		DbgPrint("reset ignored\n");
		return 0;
	}

	cluster = bundle_get_val(b, "cluster");
	if (!cluster)
		cluster = "user,created";

	category = bundle_get_val(b, "category");
	if (!category)
		category = "default";

	content = bundle_get_val(b, "content");
	if (!content)
		content = NULL;

	tmp = bundle_get_val(b, "verbose");
	if (!tmp || sscanf(tmp, "%d", &m_fVerbose) != 1)
		m_fVerbose = 0;

	tmp = bundle_get_val(b, "period");
	if (!tmp || sscanf(tmp, "%lf", &period) != 1)
		period = DEFAULT_PERIOD;

	DbgPrint("App reset (dump info)\n");
	DbgPrint("pkgname: %s\n", pkgname);
	DbgPrint("cluster: %s\n", cluster);
	DbgPrint("category: %s\n", category);
	DbgPrint("content: %s\n", content);
	DbgPrint("period: %lf\n", period);
	DbgPrint("verbose: %d\n", m_fVerbose);

	return 0;
}

static int app_create(void *data)
{
	return 0;
}

static int app_terminate(void *data)
{
	return 0;
}

static int app_pause(void *data)
{
	return 0;
}

static int app_resume(void *data)
{
	return 0;
}

static int app_reset(bundle *b, void *data)
{
	return 0;
}

int main(int argc, char *argv[])
{
	struct appcore_ops ops;

	ops.create = app_create;
	ops.terminate = app_terminate;
	ops.pause = app_pause;
	ops.resume = app_resume;
	ops.reset = app_reset;
	ops.data = NULL;

	return appcore_efl_main("test", &argc, &argv, &ops);
}

/* End of a file */
