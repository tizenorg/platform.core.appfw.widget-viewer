#include <Elementary.h>
#include <libgen.h>

#include <livebox.h>
#include <dlog.h>

#include "dlist.h"
#include "CUtil.h"
#include "CLiveBox.h"
#include "CLiveBoxMgr.h"
#include "CResourceMgr.h"

struct dlist *CLiveBoxMgr::s_pBoxList = NULL;

static int s_EventHandler(struct livebox *handler, const char *event, void *data)
{
	CLiveBoxMgr *mgr = (CLiveBoxMgr *)data;
	return mgr ? mgr->OnEvent(handler, event) : 0;
}

static int s_FaultHandler(const char *event, const char *pkgname, const char *filename, const char *funcname, void *data)
{
	DbgPrint("Event: %s, package: %s\n", event, pkgname);
	DbgPrint("ID: %s\n", filename);
	DbgPrint("Function: %s\n", funcname);
	return 0;
}

CLiveBoxMgr::CLiveBoxMgr(void)
{
	livebox_init();
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

int CLiveBoxMgr::OnEvent(struct livebox *handler, const char *event)
{
	CLiveBox *box;
	int ret = 0;

	DbgPrint("Event: %s\n", event);
	if (!strcmp(event, "lb,created")) {
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

		if (!strcmp(event, "lb,updated")) {
			box->OnUpdateLB();
		} else if (!strcmp(event, "lb,deleted")) {
			box->SetHandler(NULL); /* To prevent to delete a livebox again */
			delete box;
		} else if (!strcmp(event, "pd,updated")) {
			box->OnUpdatePD();
		} else if (!strcmp(event, "group,changed")) {
			box->OnGroupChanged();
		} else if (!strcmp(event, "pinup,changed")) {
			box->OnPinupChanged();
		} else if (!strcmp(event, "period,changed")) {
			box->OnPeriodChanged();
		} else {
			DbgPrint("Unknown event: %s\n", event);
			ret = -ENOSYS;
		}
	}

	return ret;
}

int CLiveBoxMgr::OnFaultEvent(const char *event, const char *pkgname, const char *id, const char *funcname, void *data)
{
	ErrPrint("Fault event: %s (%s)\n", event, pkgname);

	if (!strcmp(event, "deactivated")) {
		DbgPrint("Instance: %s\n", id);
		DbgPrint("Function: %s\n", funcname);
	} else if (!strcmp(event, "provider,disconnected")) {
		DbgPrint("Provider is disconnected\n");
	} else {
		ErrPrint("Unknown event: %s\n", event);
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
