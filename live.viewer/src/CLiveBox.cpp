#include <Elementary.h>
#include <libgen.h>

#include <livebox.h>
#include <dlog.h>
#include <bundle.h>
#include <app.h>

#include <X11/Xlib.h>
#include <Ecore_X.h>

#include <livebox-service.h>

#include "dlist.h"
#include "CUtil.h"
#include "CResourceMgr.h"
#include "CLiveBox.h"
#include "CLiveBoxMgr.h"
#include "CMain.h"

#define LOGSIZE 128

static void s_DestroyPD(struct livebox *handle, int ret, void *data)
{
	CLiveBox *box;
	char buffer[LOGSIZE];

	box = (CLiveBox *)data;
	if (!box) {
		ErrPrint("Unknown livebox\n");
		return;
	}

	snprintf(buffer, sizeof(buffer), "PD Close returns %d", ret);
	CMain::GetInstance()->AppendLog(buffer);
	if (ret >= 0) {
		box->OnDestroyPD();
		CMain::GetInstance()->TogglePDCtrl(false);
	}
}

static void s_OnEdjeEvent(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	CLiveBox *box = (CLiveBox *)data;

	if (!strcmp(emission, "closed") && !strcmp(source, "pd")) {
		/*!
		 * \note
		 * PD Close animation is finished
		 */
		livebox_destroy_pd(box->Handler(), s_DestroyPD, box);
		return;
	}
}

static void s_OnLBMouseDown(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Event_Mouse_Down *down = (Evas_Event_Mouse_Down *)event_info;
	CLiveBox *box = (CLiveBox *)data;
	struct livebox *handler;
	enum livebox_lb_type type;

	handler = box->Handler();
	if (!handler)
		return;

	type = livebox_lb_type(handler);
	if (type == LB_TYPE_BUFFER || type == LB_TYPE_PIXMAP) {
		int ix, iy, iw, ih;
		double rx, ry;
		evas_object_geometry_get(obj, &ix, &iy, &iw, &ih);

		rx = (double)(down->canvas.x - ix) / (double)iw;
		ry = (double)(down->canvas.y - iy) / (double)ih;

		livebox_content_event(handler, LB_MOUSE_DOWN, rx, ry);
	}
}

static void s_OnLBMouseMove(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Event_Mouse_Move *move = (Evas_Event_Mouse_Move *)event_info;
	CLiveBox *box = (CLiveBox *)data;
	struct livebox *handler;
	enum livebox_lb_type type;

	handler = box->Handler();
	if (!handler)
		return;

	type = livebox_lb_type(handler);
	if (type == LB_TYPE_BUFFER || type == LB_TYPE_PIXMAP) {
		int ix, iy, iw, ih;
		double rx, ry;
		evas_object_geometry_get(obj, &ix, &iy, &iw, &ih);

		rx = (double)(move->cur.canvas.x - ix) / (double)iw;
		ry = (double)(move->cur.canvas.y - iy) / (double)ih;

		livebox_content_event(handler, LB_MOUSE_MOVE, rx, ry);
	}
}

static void s_OnLBMouseUp(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Event_Mouse_Up *up = (Evas_Event_Mouse_Up *)event_info;
	CLiveBox *box = (CLiveBox *)data;
	struct livebox *handler;
	enum livebox_lb_type type;

	handler = box->Handler();
	if (!handler)
		return;

	int ix, iy, iw, ih;
	double rx, ry;
	evas_object_geometry_get(obj, &ix, &iy, &iw, &ih);

	rx = (double)(up->canvas.x - ix) / (double)iw;
	ry = (double)(up->canvas.y - iy) / (double)ih;

	type = livebox_lb_type(handler);
	if (type == LB_TYPE_BUFFER || type == LB_TYPE_PIXMAP)
		livebox_content_event(handler, LB_MOUSE_UP, rx, ry);
	else
		livebox_click(handler, rx, ry);
}

static void s_OnPDMouseDown(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Event_Mouse_Down *down = (Evas_Event_Mouse_Down *)event_info;
	CLiveBox *box = (CLiveBox *)data;
	struct livebox *handler;
	int ix, iy, iw, ih;
	double rx;
	double ry;

	handler = box->Handler();
	if (!handler)
		return;

	evas_object_geometry_get(obj, &ix, &iy, &iw, &ih);

	rx = (double)(down->canvas.x - ix) / (double)iw;
	ry = (double)(down->canvas.y - iy) / (double)ih;

	livebox_content_event(handler, PD_MOUSE_DOWN, rx, ry);
}

static void s_OnPDMouseMove(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Event_Mouse_Move *move = (Evas_Event_Mouse_Move *)event_info;
	CLiveBox *box = (CLiveBox *)data;
	struct livebox *handler;
	int ix, iy, iw, ih;
	double rx, ry;

	handler = box->Handler();
	if (!handler)
		return;

	evas_object_geometry_get(obj, &ix, &iy, &iw, &ih);

	rx = (double)(move->cur.canvas.x - ix) / (double)iw;
	ry = (double)(move->cur.canvas.y - iy) / (double)ih;

	livebox_content_event(handler, PD_MOUSE_MOVE, rx, ry);
}

static void s_OnPDMouseUp(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Event_Mouse_Up *up = (Evas_Event_Mouse_Up *)event_info;
	CLiveBox *box = (CLiveBox *)data;
	struct livebox *handler;
	int ix, iy, iw, ih;
	double rx, ry;

	handler = box->Handler();
	if (!handler)
		return;

	evas_object_geometry_get(obj, &ix, &iy, &iw, &ih);

	rx = (double)(up->canvas.x - ix) / (double)iw;
	ry = (double)(up->canvas.y - iy) / (double)ih;

	livebox_content_event(handler, PD_MOUSE_UP, rx, ry);
}

static void s_CreatePD(struct livebox *handle, int ret, void *data)
{
	CLiveBox *box;
	char buffer[LOGSIZE];

	box = (CLiveBox *)data;
	if (!box) {
		ErrPrint("Unknown livebox\n");
		return;
	}

	snprintf(buffer, sizeof(buffer), "PD Open returns %d", ret);
	CMain::GetInstance()->AppendLog(buffer);
	if (ret >= 0) {
		box->OnCreatePD();
		CMain::GetInstance()->TogglePDCtrl(true);
	}
}

static void s_OnExit(void *data, Evas_Object *obj, void *event_info)
{
	elm_exit();
}

static void s_OnCreate(struct livebox *handle, int ret, void *data)
{
	CLiveBox *box;

	box = (CLiveBox *)data;
	if (!box) {
		ErrPrint("Unknown livebox\n");
		return;
	}

	DbgPrint("Live box CREATE returns %d\n", ret);
	if (ret < 0) {
		Evas_Object *win;
		Evas_Object *popup;
		Evas_Object *button;
		char buffer[256];

		delete box;

		win = (Evas_Object *)CResourceMgr::GetInstance()->GetObject("window");
		if (!win)
			return;

		popup = elm_popup_add(win);
		if (!popup)
			return;

		elm_popup_orient_set(popup, ELM_POPUP_ORIENT_CENTER);
		elm_object_part_text_set(popup, "title,text", "Unable to load a livebox");

		button = elm_button_add(popup);
		if (!button) {
			evas_object_del(popup);
			return;
		}

		elm_object_text_set(button, "Exit");
		elm_object_part_content_set(popup, "button2", button);
		evas_object_smart_callback_add(button, "clicked", s_OnExit, popup);

		snprintf(buffer, sizeof(buffer), "%s(%s): %d", livebox_pkgname(handle), livebox_content(handle), ret);
		elm_object_part_text_set(popup, "default", buffer);

		evas_object_show(popup);
	} else {
		box->OnCreate(handle);
	}
}

static void s_ResizeCB(struct livebox *handle, int ret, void *data)
{
	CLiveBox *box;
	char buffer[LOGSIZE];

	box = (CLiveBox *)data;
	if (!box) {
		ErrPrint("Unknown livebox\n");
		return;
	}

	snprintf(buffer, sizeof(buffer), "Resize returns %d", ret);
	CMain::GetInstance()->AppendLog(buffer);
	if (ret < 0) {
		ErrPrint("Failed to resize a livebox\n");
	} else {
		box->OnUpdateLB();
	}
}

CLiveBox::CLiveBox(const char *pkgname, const char *content, const char *cluster, const char *category, double period)
: m_pHandler(NULL)
, m_pIconSlot(NULL)
, m_pLBImage(NULL)
, m_pPDImage(NULL)
{
	struct livebox *handler;
	livebox_activate(pkgname, NULL, NULL);
	handler = livebox_add(pkgname, content, cluster, category, period, s_OnCreate, this);
	if (!handler) /* Do not set this as a m_pHandler */
		throw EFAULT;

	memset(m_sSize, 0, sizeof(m_sSize));
}

CLiveBox::~CLiveBox(void)
{
	CLiveBoxMgr *mgr;

	CMain::GetInstance()->DestroyPDCtrl();

	mgr = (CLiveBoxMgr *)CResourceMgr::GetInstance()->GetObject("LiveBoxMgr");
	if (mgr)
		mgr->Remove(this);

	if (m_pIconSlot) {
		Evas_Object *image;

		image = elm_object_part_content_unset(m_pIconSlot, "livebox");
		if (image != m_pLBImage) {
			DbgPrint("Something goes to wrong :(\n");
			if (image)
				evas_object_del(image);
		}

		image = elm_object_part_content_unset(m_pIconSlot, "pd");
		if (image != m_pPDImage) {
			DbgPrint("Something goes to wrong :(\n");
			if (image)
				evas_object_del(image);
		}

		evas_object_del(m_pIconSlot);
	}

	if (m_pLBImage)
		evas_object_del(m_pLBImage);

	if (m_pPDImage)
		evas_object_del(m_pPDImage);

	if (m_pHandler)
		livebox_del(m_pHandler, NULL, NULL);
}

CLiveBox::CLiveBox(struct livebox *handler)
: m_pHandler(handler)
{
	if (m_OnCreate() < 0)
		throw EFAULT;

	livebox_set_data(m_pHandler, this);
}

int CLiveBox::CreatePD(void)
{
	livebox_create_pd(m_pHandler, s_CreatePD, this);
	return 0;
}

int CLiveBox::DestroyPD(void)
{
	elm_object_signal_emit(m_pIconSlot, "close", "pd");
	return 0;
}

void CLiveBox::OnCreate(struct livebox *handler)
{
	int ret;

	m_pHandler = handler;
	livebox_set_data(m_pHandler, this);

	ret = m_OnCreate();
	if (ret < 0) {
		livebox_set_data(m_pHandler, NULL);
		/*!
		 * \note
		 * Keep the m_pHandler to delete it from the destructor
		 */
		delete this;
	} else {
		CMain::GetInstance()->UpdateCtrl(this);
		if (livebox_has_pd(m_pHandler))
			CMain::GetInstance()->CreatePDCtrl(this);
	}

	return;
}

void CLiveBox::OnUpdateLB(void)
{
	int w, h;
	int ow, oh;
	enum livebox_lb_type type;
	int size_type;

	if (!m_pIconSlot || !m_pLBImage)
		return;

	size_type = livebox_size(m_pHandler);
	if (livebox_service_get_size(size_type, &w, &h) < 0)
		return;

	/*
	char buffer[LOGSIZE];
	const char *tmp;
	snprintf(buffer, sizeof(buffer), "LB Updated (%dx%d)", w, h);
	CMain::GetInstance()->AppendLog(buffer);

	tmp = livebox_category_title(m_pHandler);
	snprintf(buffer, sizeof(buffer), "Title: %s", tmp ? tmp : "(nil)");
	CMain::GetInstance()->AppendLog(buffer);

	tmp = livebox_content(m_pHandler);
	snprintf(buffer, sizeof(buffer), "Content: %s", tmp);
	CMain::GetInstance()->AppendLog(buffer);
	*/

	if (w < 0 || h < 0)
		return;

	if ((w / 1000) || (h / 1000))
		return;

	if (w == 0 && h == 0)
		return;

	type = livebox_lb_type(m_pHandler);

	if (!!getenv("UPDATE_USING_PIXMAP") && type == LB_TYPE_PIXMAP) {
		if ((int)evas_object_data_get(m_pLBImage, "pixmap") != livebox_lb_pixmap(m_pHandler)) {
			Evas_Native_Surface surf;

			surf.version = EVAS_NATIVE_SURFACE_VERSION;
			surf.type = EVAS_NATIVE_SURFACE_X11;
			surf.data.x11.visual = ecore_x_default_visual_get(ecore_x_display_get(), ecore_x_default_screen_get());
			surf.data.x11.pixmap = livebox_lb_pixmap(m_pHandler);
			DbgPrint("Surface set to render canvas: 0x%X\n", surf.data.x11.pixmap);

			evas_object_image_native_surface_set(m_pLBImage, &surf);
			evas_object_data_set(m_pLBImage, "pixmap", (void *)livebox_lb_pixmap(m_pHandler));
		}

		DbgPrint("Pixmap: %X, Size: %dx%d\n", livebox_lb_pixmap(m_pHandler), w, h);
		evas_object_image_size_set(m_pLBImage, w, h);
		evas_object_image_pixels_dirty_set(m_pLBImage, 1);
		evas_object_image_fill_set(m_pLBImage, 0, 0, w, h);
		evas_object_image_data_update_add(m_pLBImage, 0, 0, w, h);
		// evas_object_resize(m_pLBImage, w, h);
	} else if (type == LB_TYPE_BUFFER || type == LB_TYPE_PIXMAP) {
		void *data;

		data = livebox_acquire_fb(m_pHandler);
		if (!data)
			return;

		CUtil::UpdateCanvasImage(m_pLBImage, data, w, h);

		livebox_release_fb(data);
	} else {
		const char *filename;

		filename = livebox_filename(m_pHandler);
		if (!filename)
			return;

		CUtil::UpdateCanvasImage(m_pLBImage, filename, w, h);
	}

	if (sscanf(m_sSize, "resize,to,%dx%d", &ow, &oh) == 2) {
		if (ow == w && oh == h)
			return;
	}

	snprintf(m_sSize, sizeof(m_sSize), "resize,to,%dx%d", w, h);
	elm_object_signal_emit(m_pIconSlot, m_sSize, "livebox");
	return;
}

void CLiveBox::OnDestroyPD(void)
{
	Evas_Object *pd;

	if (!m_pPDImage)
		return;

	pd = elm_object_part_content_unset(m_pIconSlot, "pd");
	if (pd != m_pPDImage) {
		DbgPrint("Swallowed object is not matched\n");
		if (m_pPDImage)
			evas_object_del(m_pPDImage);
	}

	if (pd)
		evas_object_del(pd);

	m_pPDImage = NULL;
}

void CLiveBox::OnCreatePD(void)
{
	int w;
	int h;

	if (m_pPDImage) {
		DbgPrint("PD is already created\n");
		return;
	}

	if (livebox_get_pdsize(m_pHandler, &w, &h) < 0)
		return;

	if (w <= 0 || h <= 0)
		return;

	m_pPDImage = (Evas_Object *)CUtil::CreateCanvasImage((Evas_Object *)CResourceMgr::GetInstance()->GetObject("window"), w, h);
	if (!m_pPDImage)
		return;

	evas_object_event_callback_add(m_pPDImage, EVAS_CALLBACK_MOUSE_DOWN, s_OnPDMouseDown, this);
	evas_object_event_callback_add(m_pPDImage, EVAS_CALLBACK_MOUSE_MOVE, s_OnPDMouseMove, this);
	evas_object_event_callback_add(m_pPDImage, EVAS_CALLBACK_MOUSE_UP, s_OnPDMouseUp, this);

	elm_object_part_content_set(m_pIconSlot, "pd", m_pPDImage);

	OnUpdatePD();
	elm_object_signal_emit(m_pIconSlot, "open", "pd");
}

void CLiveBox::OnUpdatePD(void)
{
	int w;
	int h;
	enum livebox_pd_type type;

	if (!m_pPDImage)
		return;

	if (livebox_get_pdsize(m_pHandler, &w, &h) < 0)
		return;

	/*
	char buffer[LOGSIZE];
	snprintf(buffer, sizeof(buffer), "PD Updated (%dx%d)", w, h);
	CMain::GetInstance()->AppendLog(buffer);
	*/

	if (w < 0 || h < 0)
		return;

	if ((w / 1000) || (h / 1000))
		return;

	if (w == 0 && h == 0)
		return;

	type = livebox_pd_type(m_pHandler);
	if (!!getenv("UPDATE_USING_PIXMAP") && type == PD_TYPE_PIXMAP) {
		if ((int)evas_object_data_get(m_pPDImage, "pixmap") != livebox_lb_pixmap(m_pHandler)) {
			Evas_Native_Surface surf;

			surf.version = EVAS_NATIVE_SURFACE_VERSION;
			surf.type = EVAS_NATIVE_SURFACE_X11;
			surf.data.x11.visual = ecore_x_default_visual_get(ecore_x_display_get(), ecore_x_default_screen_get());
			surf.data.x11.pixmap = livebox_lb_pixmap(m_pHandler);
			DbgPrint("Surface set to render canvas: 0x%X\n", surf.data.x11.pixmap);

			evas_object_image_native_surface_set(m_pPDImage, &surf);
			evas_object_data_set(m_pPDImage, "pixmap", (void *)livebox_lb_pixmap(m_pHandler));
		}

		DbgPrint("Pixmap: %X, Size: %dx%d\n", livebox_lb_pixmap(m_pHandler), w, h);
		evas_object_image_size_set(m_pPDImage, w, h);
		evas_object_image_pixels_dirty_set(m_pPDImage, 1);
		evas_object_image_fill_set(m_pPDImage, 0, 0, w, h);
		evas_object_image_data_update_add(m_pPDImage, 0, 0, w, h);
		evas_object_resize(m_pPDImage, w, h);
	} else if (type == PD_TYPE_BUFFER || type == PD_TYPE_PIXMAP) {
		void *data;
		data = livebox_acquire_pdfb(m_pHandler);
		if (!data)
			return;
		CUtil::UpdateCanvasImage(m_pPDImage, data, w, h);
		evas_object_size_hint_min_set(m_pPDImage, w, h);
		evas_object_size_hint_max_set(m_pPDImage, w, h);
		livebox_release_pdfb(data);
	} else {
		ErrPrint("Unsupported media format\n");
	}

	return;
}

void CLiveBox::OnPeriodChanged(void)
{
	char buffer[LOGSIZE];
	snprintf(buffer, sizeof(buffer), "Period: %lf", livebox_period(m_pHandler));
	CMain::GetInstance()->AppendLog(buffer);
}

void CLiveBox::OnGroupChanged(void)
{
}

void CLiveBox::OnPinupChanged(void)
{
	char buffer[LOGSIZE];
	snprintf(buffer, sizeof(buffer), "Pinup: %d", livebox_is_pinned_up(m_pHandler));
	CMain::GetInstance()->AppendLog(buffer);
}

int CLiveBox::m_OnCreate(void)
{
	CLiveBoxMgr *mgr;
	Evas_Object *win;
	Evas_Object *layout;
	int w;
	int h;

	win = (Evas_Object *)CResourceMgr::GetInstance()->GetObject("window");
	if (!win)
		return -EFAULT;

	evas_object_geometry_get(win, NULL, NULL, &w, &h);

	m_pIconSlot = elm_layout_add(win);
	if (!m_pIconSlot) {
		ErrPrint("Failed to append a new layout\n");
		return -EFAULT;
	}

	if (elm_layout_file_set(m_pIconSlot, PKGROOT "/res/edje/live-viewer.edj", "icon,slot") == EINA_FALSE) {
		ErrPrint("Failed to load an icon slot EDJE\n");
		evas_object_del(m_pIconSlot);
		m_pIconSlot = NULL;
		return -EIO;
	}

	elm_object_signal_callback_add(m_pIconSlot, "*", "*", s_OnEdjeEvent, this);

	m_pLBImage = (Evas_Object *)CUtil::CreateCanvasImage(win, w, h >> 1);
	if (!m_pLBImage) {
		evas_object_del(m_pIconSlot);
		m_pIconSlot = NULL;
		return -EFAULT;
	}

	evas_object_event_callback_add(m_pLBImage, EVAS_CALLBACK_MOUSE_DOWN, s_OnLBMouseDown, this);
	evas_object_event_callback_add(m_pLBImage, EVAS_CALLBACK_MOUSE_MOVE, s_OnLBMouseMove, this);
	evas_object_event_callback_add(m_pLBImage, EVAS_CALLBACK_MOUSE_UP, s_OnLBMouseUp, this);

	elm_object_part_content_set(m_pIconSlot, "livebox", m_pLBImage);

	evas_object_move(m_pIconSlot, 0, 0);
	evas_object_resize(m_pIconSlot, w, (h >> 1));
	evas_object_show(m_pIconSlot);

	layout = (Evas_Object *)CResourceMgr::GetInstance()->GetObject("layout");
	if (!layout)
		return -EFAULT;

	elm_object_part_content_set(layout, "viewer", m_pIconSlot);

	mgr = (CLiveBoxMgr *)CResourceMgr::GetInstance()->GetObject("LiveBoxMgr");
	if (mgr)
		mgr->Add(this);
	else
		ErrPrint("Live box manager is not exists\n");

	/* Try to update content */
	OnUpdateLB();
	return 0;
}

int CLiveBox::Resize(int size_type)
{
	if (size_type == LB_SIZE_TYPE_UNKNOWN)
		return -1;

	livebox_resize(m_pHandler, size_type, s_ResizeCB, this);
	return 0;
}

int CLiveBox::GetSizeList(int *cnt, int *list)
{
	if (!m_pHandler || !cnt || !list)
		return -EINVAL;

	return livebox_get_supported_sizes(m_pHandler, cnt, list);
}

/* End of a file */
