#include <Elementary.h>
#include <Ecore_X.h>
#include <libgen.h>

#include <livebox.h>
#include <dlog.h>

#include "dlist.h"
#include "CUtil.h"
#include "CLiveBox.h"
#include "CLiveBoxMgr.h"
#include "CResourceMgr.h"

struct dlist *CLiveBoxMgr::s_pBoxList = NULL;

static int s_EventHandler(struct livebox *handler, enum livebox_event_type event, void *data)
{
	CLiveBoxMgr *mgr = (CLiveBoxMgr *)data;
	return mgr ? mgr->OnEvent(handler, event) : 0;
}

static int s_FaultHandler(enum livebox_fault_type event, const char *pkgname, const char *filename, const char *funcname, void *data)
{
	DbgPrint("Event: 0x%X, package: %s\n", event, pkgname);
	DbgPrint("ID: %s\n", filename);
	DbgPrint("Function: %s\n", funcname);
	return 0;
}

CLiveBoxMgr::CLiveBoxMgr(void)
{
	livebox_init(ecore_x_display_get());
	livebox_event_handler_set(s_EventHandler, this);
	livebox_fault_handler_set(s_FaultHandler, this);
	CResourceMgr::GetInstance()->RegisterObject("LiveBoxMgr", this);
}

CLiveBoxMgr::~CLiveBoxMgr(void)
{
	struct dlist *l;
	struct dlist *n;
	CLiveBox *box;
	void *data;

	if (CResourceMgr::GetInstance()->UnregisterObject("LiveBoxMgr") != this)
		ErrPrint("Live box manager is not matched\n");

	dlist_foreach_safe(s_pBoxList, l, n, data) {
		box = (CLiveBox *)data;
		delete box;
	}

	livebox_fini();
}

int CLiveBoxMgr::OnEvent(struct livebox *handler, enum livebox_event_type event)
{
	CLiveBox *box;
	int ret = 0;

	if (event == LB_EVENT_CREATED) {
		try {
			box = new CLiveBox(handler);
		} catch (...) {
			return -EFAULT;
		}
	} else {
		box = (CLiveBox *)livebox_get_data(handler);
		if (!box) {
			ErrPrint("Failed to find a livebox\n");
			return -EINVAL;
		}

		switch (event) {
		case LB_EVENT_LB_UPDATED:
			box->OnUpdateLB();
			break;
		case LB_EVENT_DELETED:
			box->SetHandler(NULL); /* To prevent to delete a livebox again */
			delete box;
			break;
		case LB_EVENT_PD_UPDATED:
			DbgPrint("PD Updated\n");
			box->OnUpdatePD();
			break;
		case LB_EVENT_GROUP_CHANGED:
			box->OnGroupChanged();
			break;
		case LB_EVENT_PINUP_CHANGED:
			box->OnPinupChanged();
			break;
		case LB_EVENT_PERIOD_CHANGED:
			box->OnPeriodChanged();
			break;
		default:
			DbgPrint("Unknown event: %s\n", event);
			ret = -ENOSYS;
			break;
		}
	}

	return ret;
}

int CLiveBoxMgr::OnFaultEvent(enum livebox_fault_type event, const char *pkgname, const char *id, const char *funcname, void *data)
{
	ErrPrint("Fault event: 0x%X (%s)\n", event, pkgname);

	switch (event) {
	case LB_FAULT_DEACTIVATED:
		DbgPrint("Instance: %s\n", id);
		DbgPrint("Function: %s\n", funcname);
		break;
	case LB_FAULT_PROVIDER_DISCONNECTED:
		DbgPrint("Provider is disconnected\n");
		break;
	default:
		ErrPrint("Unknown event: 0x%X\n", event);
		break;
	}

	return 0;
}

int CLiveBoxMgr::Add(CLiveBox *box)
{
	struct dlist *l;
	CLiveBox *item;
	void *data;

	dlist_foreach(s_pBoxList, l, data) {
		item = (CLiveBox *)data;
		if (item == box) {
			DbgPrint("Already registered\n");
			return 0;
		}
	}

	s_pBoxList = dlist_append(s_pBoxList, box);
	return 0;
}

int CLiveBoxMgr::Remove(CLiveBox *box)
{
	dlist_remove_data(s_pBoxList, box);
	return 0;
}

/* End of a file */
