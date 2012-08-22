#include <Elementary.h>
#include <libgen.h>

#include <livebox.h>
#include <dlog.h>

#include "dlist.h"
#include "CUtil.h"
#include "CLiveBox.h"
#include "CLiveBoxMgr.h"

struct dlist *CLiveBoxMgr::s_pBoxList = NULL;

CLiveBoxMgr::CLiveBoxMgr(void)
{
	livebox_init();
}

CLiveBoxMgr::~CLiveBoxMgr(void)
{
	struct dlist *l;
	struct dlist *n;
	CLiveBox *box;
	void *data;

	dlist_foreach_safe(s_pBoxList, l, n, data) {
		box = (CLiveBox *)data;
		s_pBoxList = dlist_remove(s_pBoxList, l);
		delete box;
	}

	livebox_fini();
}

int CLiveBoxMgr::OnEvent(struct livebox *handler, const char *event, void *data)
{
	CLiveBox *box;
	int ret = 0;

	if (!strcmp(event, "lb,created")) {
		try {
			box = new CLiveBox(handler);
		} catch (...) {
			return -EFAULT;
		}

		s_pBoxList = dlist_append(s_pBoxList, box);
	} else if (!strcmp(event, "lb,updated")) {
		box = (CLiveBox *)livebox_get_data(handler);
		if (!box)
			return -EINVAL;

		box->OnUpdateLB();
	} else if (!strcmp(event, "lb,deleted")) {
		box = (CLiveBox *)livebox_get_data(handler);
		if (!box)
			return -EINVAL;

		dlist_remove_data(s_pBoxList, box);
		box->SetHandler(NULL); /* To prevent to delete a livebox again */
		delete box;
	} else if (!strcmp(event, "pd,updated")) {
		box = (CLiveBox *)livebox_get_data(handler);
		if (!box)
			return -EINVAL;
		
		box->OnUpdatePD();
	} else if (!strcmp(event, "group,changed")) {
		box = (CLiveBox *)livebox_get_data(handler);
		if (!box)
			return -EINVAL;
		
		box->OnGroupChanged();
	} else if (!strcmp(event, "pinup,changed")) {
		box = (CLiveBox *)livebox_get_data(handler);
		if (!box)
			return -EINVAL;

		box->OnPinupChanged();
	} else if (!strcmp(event, "period,changed")) {
		box = (CLiveBox *)livebox_get_data(handler);
		if (!box)
			return -EINVAL;

		box->OnPeriodChanged();
	} else {
		DbgPrint("Unknown event: %s\n", event);
		ret = -ENOSYS;
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

/* End of a file */
