class CLiveBoxMgr {
public:
	CLiveBoxMgr(void);
	virtual ~CLiveBoxMgr(void);

	static int OnEvent(struct livebox *handler, const char *event, void *data);
	static int OnFaultEvent(const char *event, const char *pkgname, const char *id, const char *funcname, void *data);

private:
	static struct dlist *s_pBoxList;
};
