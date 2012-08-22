#include <Elementary.h>

#include <dlog.h>

#include "dlist.h"

#include "CUtil.h"
#include "CResourceMgr.h"

int errno;

struct item_info {
	item_info(const char *tag, void *data) {
		this->tag = strdup(tag);
		if (!this->tag) {
			ErrPrint("Heap: %s\n", strerror(errno));
			throw -ENOMEM;
		}

		this->data = data;
	}

	virtual ~item_info() { free(this->tag); }

	char *tag;
	void *data;
};

CResourceMgr *CResourceMgr::m_pInstance = NULL;

CResourceMgr::CResourceMgr *GetInstance(void)
{
	if (!CResourceMgr::m_pInstance)
		CResourceMgr::m_pInstance = new CResourceMgr();

	return CResourceMgr::m_pInstance;
}

CResourceMgr::CResourceMgr(void)
: m_pList(NULL)
{
}

CResourceMgr::~CResourceMgr(void)
{
}

int CResourceMgr::RegisterObject(const char *tag, void *data)
{
	struct item *info;

	if (!data || !tag)
		return -EINVAL;

	if (GetObject(tag))
		return -EEXIST;

	try {
		info = new item(tag, data);
	} catch (...) {
		ErrPrint("Failed to allocate a new item\n");
		return -EFAULT;
	}

	m_pList = dlist_append(m_pList, info);
	return 0;
}

void *CResourceMgr::UnregisterObject(const char *tag)
{
	struct dlist *l;
	struct dlist *n;
	struct item *info;
	void *ret;

	dlist_foreach_safe(m_pList, l, n, info) {
		if (!strcmp(info->tag, tag)) {
			m_pList = dlist_remove(m_pList, info);
			ret = info->data;
			delete info;

			return ret;
		}
	}

	return NULL;
}

void *CResourceMgr::GetObject(const char *tag)
{
	struct dlist *l;
	struct item *info;

	dlist_foreach(m_pList, l, info) {
		if (!strcmp(info->tag, tag))
			return info->data;
	}

	return NULL;
}

/* End of a file */
