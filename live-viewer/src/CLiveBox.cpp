#include <Elementary.h>
#include <libgen.h>

#include <livebox.h>
#include <dlog.h>

#include "dlist.h"
#include "CUtil.h"
#include "CResourceMgr.h"
#include "CLiveBox.h"

static void s_OnEdjeEvent(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	CLiveBox *box = (CLiveBox *)data;
	struct livebox *handler;

	handler = box->Handler();
	DbgPrint("handler[%p] emission[%s] source[%s]\n", handler, emission, source);
}

static void s_OnLBMouseDown(void *data, Evas *e, Evas_Object *obj, void *event_info)
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

	livebox_content_event(handler, LB_MOUSE_DOWN, rx, ry);
}

static void s_OnLBMouseMove(void *data, Evas *e, Evas_Object *obj, void *event_info)
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

	livebox_content_event(handler, LB_MOUSE_MOVE, rx, ry);
}

static void s_OnLBMouseUp(void *data, Evas *e, Evas_Object *obj, void *event_info)
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

	livebox_content_event(handler, LB_MOUSE_UP, rx, ry);
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

static void s_OnCreate(struct livebox *handle, int ret, void *data)
{
	CLiveBox *box;

	box = (CLiveBox *)data;
	if (!box) {
		ErrPrint("Unknown livebox\n");
		return;
	}

	DbgPrint("Live box CREATE returns %d\n", ret);
	if (ret < 0)
		delete box;
	else
		box->OnCreate(handle);
}

CLiveBox::CLiveBox(const char *pkgname, const char *content, const char *cluster, const char *category, double period)
: m_pHandler(NULL)
, m_pIconSlot(NULL)
, m_pLBImage(NULL)
, m_pPDImage(NULL)
{
	struct livebox *handler;
	handler = livebox_add(pkgname, content, cluster, category, period, s_OnCreate, this);
	if (!handler) /* Do not set this as a m_pHandler */
		throw EFAULT;
}

CLiveBox::~CLiveBox(void)
{
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
	int w;
	int h;

	if (m_pPDImage) {
		DbgPrint("PD is already created\n");
		return 0;
	}

	if (livebox_get_pdsize(m_pHandler, &w, &h) < 0)
		return -EINVAL;

	if (w <= 0 || h <= 0)
		return 0;

	m_pPDImage = (Evas_Object *)CUtil::CreateCanvasImage((Evas_Object *)CResourceMgr::GetInstance()->GetObject("window"), w, h);
	if (!m_pPDImage)
		return -EFAULT;

	evas_object_event_callback_add(m_pPDImage, EVAS_CALLBACK_MOUSE_DOWN, s_OnPDMouseDown, this);
	evas_object_event_callback_add(m_pPDImage, EVAS_CALLBACK_MOUSE_MOVE, s_OnPDMouseMove, this);
	evas_object_event_callback_add(m_pPDImage, EVAS_CALLBACK_MOUSE_UP, s_OnPDMouseUp, this);

	elm_object_part_content_set(m_pIconSlot, "pd", m_pPDImage);

	OnUpdatePD();
	return 0;
}

int CLiveBox::DestroyPD(void)
{
	Evas_Object *pd;

	pd = elm_object_part_content_unset(m_pIconSlot, "pd");
	if (pd != m_pPDImage) {
		DbgPrint("Swallowed object is not matched\n");
		if (m_pPDImage)
			evas_object_del(m_pPDImage);
	}

	if (pd)
		evas_object_del(pd);

	m_pPDImage = NULL;
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
	}

	return;
}

void CLiveBox::OnUpdateLB(void)
{
	int w;
	int h;
	char size_str[] = "size_0000x0000";

	if (!m_pIconSlot || !m_pLBImage)
		return;

	if (livebox_get_size(m_pHandler, &w, &h) < 0)
		return;

	if (w < 0 || h < 0)
		return;

	if ((w / 1000) || (h / 1000))
		return;

	if (w == 0 && h == 0)
		return;

	if (livebox_lb_type(m_pHandler) == LB_TYPE_BUFFER) {
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

	snprintf(size_str, sizeof(size_str), "size_%dx%d", w, h);
	DbgPrint("Size string: %s\n", size_str);

	edje_object_signal_emit(elm_layout_edje_get(m_pIconSlot), size_str, "lb,resize,to");
	return;
}

void CLiveBox::OnUpdatePD(void)
{
	int w;
	int h;

	if (!m_pPDImage)
		return;

	if (livebox_get_pdsize(m_pHandler, &w, &h) < 0)
		return;

	if (w < 0 || h < 0)
		return;

	if ((w / 1000) || (h / 1000))
		return;

	if (w == 0 && h == 0)
		return;

	if (livebox_pd_type(m_pHandler) == PD_TYPE_BUFFER) {
		void *data;

		data = livebox_acquire_pdfb(m_pHandler);
		if (!data)
			return;
		CUtil::UpdateCanvasImage(m_pPDImage, data, w, h);
		livebox_release_pdfb(data);
	} else {
		ErrPrint("Unsupported media format\n");
	}

	return;
}

void CLiveBox::OnPeriodChanged(void)
{
}

void CLiveBox::OnGroupChanged(void)
{
}

void CLiveBox::OnPinupChanged(void)
{
}

int CLiveBox::m_OnCreate(void)
{
	Evas_Object *win;
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

	if (!elm_layout_file_set(m_pIconSlot, "/usr/share/live-viewer/icon_slot.edj", "icon,slot")) {
		ErrPrint("Failed to load an icon slot EDJE\n");
		evas_object_del(m_pIconSlot);
		m_pIconSlot = NULL;
		return -EIO;
	}

	edje_object_signal_callback_add(elm_layout_edje_get(m_pIconSlot), "*", "*", s_OnEdjeEvent, this);

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

	/* Try to update content */
	OnUpdateLB();
	return 0;
}

/* End of a file */
