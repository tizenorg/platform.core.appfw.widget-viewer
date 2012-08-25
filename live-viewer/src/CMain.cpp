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
#include "CLiveBox.h"
#include "CLiveBoxMgr.h"
#include "CMain.h"

int errno;

CMain *CMain::m_pInstance = NULL;

CMain::CMain()
: m_fVerbose(0)
, m_pLiveBoxMgr(NULL)
{
}

CMain::~CMain()
{
}

int CMain::m_CreateController(void)
{
	Evas_Object *win;
	Evas_Object *layout;
	Evas_Object *list;

	win = (Evas_Object *)CResourceMgr::GetInstance()->GetObject("window");
	layout = (Evas_Object *)CResourceMgr::GetInstance()->GetObject("layout");

	list = elm_list_add(win);
	if (!list) {
		ErrPrint("Failed to create a size-LIST Ctrl\n");
		return -EFAULT;
	}
	elm_object_part_content_set(layout, "controller", list);
	return 0;
}

static void s_BtnClicked(void *data, Evas_Object *obj, void *event_info)
{
	CLiveBox *box = (CLiveBox *)data;

	if (!strcmp(elm_object_part_text_get(obj, NULL), "Open PD"))
		box->CreatePD();
	else if (!strcmp(elm_object_part_text_get(obj, NULL), "Close PD"))
		box->DestroyPD();
	else
		ErrPrint("Unknown label\n");
}

int CMain::TogglePDCtrl(int is_on)
{
	Evas_Object *layout;
	Evas_Object *btn;

	layout = (Evas_Object *)CResourceMgr::GetInstance()->GetObject("layout");
	if (!layout)
		return -EINVAL;

	btn = elm_object_part_content_get(layout, "pd");
	if (!btn)
		return -EINVAL;

	elm_object_part_text_set(btn, NULL, is_on ? "Close PD" : "Open PD");
	return 0;
}

int CMain::CreatePDCtrl(CLiveBox *box)
{
	Evas_Object *win;
	Evas_Object *layout;
	Evas_Object *btn;

	win = (Evas_Object *)CResourceMgr::GetInstance()->GetObject("window");
	layout = (Evas_Object *)CResourceMgr::GetInstance()->GetObject("layout");

	btn = elm_button_add(win);
	if (btn) {
		elm_object_part_text_set(btn, NULL, "Open PD");
		elm_object_part_content_set(layout, "pd", btn);

		evas_object_smart_callback_add(btn, "clicked", s_BtnClicked, box);
	}

	return 0;
}

int CMain::DestroyPDCtrl(void)
{
	Evas_Object *win;
	Evas_Object *layout;
	Evas_Object *btn;

	win = (Evas_Object *)CResourceMgr::GetInstance()->GetObject("window");
	if (!win)
		return -EINVAL;

	layout = (Evas_Object *)CResourceMgr::GetInstance()->GetObject("layout");
	if (!layout)
		return -EINVAL;

	btn = elm_object_part_content_unset(layout, "pd");
	if (btn)
		evas_object_del(btn);

	return 0;
}

int CMain::m_CreateLogger(void)
{
	Evas_Object *win;
	Evas_Object *layout;
	Evas_Object *list;

	win = (Evas_Object *)CResourceMgr::GetInstance()->GetObject("window");
	layout = (Evas_Object *)CResourceMgr::GetInstance()->GetObject("layout");

	list = elm_list_add(win);
	if (!list) {
		ErrPrint("Failed to create a size-LIST Ctrl\n");
		return -EFAULT;
	}
	elm_list_select_mode_set(list, ELM_OBJECT_SELECT_MODE_NONE);
	elm_object_part_content_set(layout, "logger", list);
	return 0;
}

int CMain::AppendLog(const char *str)
{
	Evas_Object *layout;
	Evas_Object *list;
	const Eina_List *items;
	Elm_Object_Item *item;

	DbgPrint("Log: %s\n", str);
	
	layout = (Evas_Object *)CResourceMgr::GetInstance()->GetObject("layout");
	list = elm_object_part_content_get(layout, "logger");

	items = elm_list_items_get(list);
	if (eina_list_count(items) >= 10) {
		item = elm_list_first_item_get(list);
		elm_object_item_del(item);
	}

	item = elm_list_item_append(list, str, NULL, NULL, NULL, NULL);
	if (item)
		elm_list_item_bring_in(item);

	elm_list_go(list);
	return 0;
}

int CMain::OnCreate(void)
{
	Evas_Object *win;
	Evas_Object *layout;
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

	layout = elm_layout_add(win);
	if (layout) {
		if (elm_layout_file_set(layout, "/usr/share/live-viewer/res/edje/live-viewer.edj", "layout") == EINA_FALSE) {
			evas_object_del(layout);
			ErrPrint("Failed to load a layout edje\n");
		} else {
			evas_object_move(layout, 0, 0);
			evas_object_resize(layout, w, h);
			evas_object_show(layout);
			CResourceMgr::GetInstance()->RegisterObject("layout", layout);

			m_CreateController();
			m_CreateLogger();
		}
	}

	return 0;
}

int CMain::OnTerminate(void)
{
	Evas_Object *win;
	Evas_Object *layout;

	delete m_pLiveBoxMgr;

	layout = (Evas_Object *)CResourceMgr::GetInstance()->UnregisterObject("layout");
	if (layout)
		evas_object_del(layout);

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
	CLiveBox *box;

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

	try {
		box = new CLiveBox(pkgname, content, cluster, category, period);
	} catch (...) {
		ErrPrint("Failed to create a new box\n");
		return -EFAULT;
	}

	return 0;
}

static void s_ResizeBox(void *data, Evas_Object *obj, void *event_info)
{
	CLiveBox *box = (CLiveBox *)data;
	Elm_Object_Item *item;
	const char *label;
	int w;
	int h;

	item = elm_list_selected_item_get(obj);
	if (!item)
		return;

	label = elm_object_item_part_text_get(item, NULL);
	if (!label)
		return;

	sscanf(label, "%dx%d", &w, &h);
	DbgPrint("Size %dx%d\n", w, h);

	box->Resize(w, h);
}

int CMain::UpdateCtrl(CLiveBox *box)
{
	Evas_Object *layout;
	Evas_Object *list;
	int cnt = NR_OF_SIZE_LIST;
	int w[NR_OF_SIZE_LIST];
	int h[NR_OF_SIZE_LIST];
	char size_str[] = "0000x0000";
	register int i;
	Elm_Object_Item *item;

	layout = (Evas_Object *)CResourceMgr::GetInstance()->GetObject("layout");
	if (!layout)
		return -EFAULT;

	list = elm_object_part_content_get(layout, "controller");
	if (!list)
		return -EFAULT;

	if (box->GetSizeList(&cnt, w, h) < 0)
		return 0;

	elm_list_clear(list);
	for (i = 0; i < cnt; i++) {
		snprintf(size_str, sizeof(size_str), "%dx%d", w[i], h[i]);
		DbgPrint("Size: %s\n", size_str);
		item = elm_list_item_append(list, size_str, NULL, NULL, s_ResizeBox, box);
		if (!item) {
			ErrPrint("Failed to append a new size list\n");
			return -EFAULT;
		}
	}
	elm_list_go(list);

	return 0;
}

CMain *CMain::GetInstance(void)
{
	if (!CMain::m_pInstance) {
		try {
			CMain::m_pInstance = new CMain();
		} catch (...) {
			return NULL;
		}
	}

	return CMain::m_pInstance;
}

static int app_create(void *data)
{
	CMain::GetInstance()->OnCreate();
	return 0;
}

static int app_terminate(void *data)
{
	CMain::GetInstance()->OnTerminate();
	return 0;
}

static int app_pause(void *data)
{
	CMain::GetInstance()->OnPause();
	return 0;
}

static int app_resume(void *data)
{
	CMain::GetInstance()->OnResume();
	return 0;
}

static int app_reset(bundle *b, void *data)
{
	CMain::GetInstance()->OnReset(b);
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

	return appcore_efl_main("live-viewer", &argc, &argv, &ops);
}

/* End of a file */
