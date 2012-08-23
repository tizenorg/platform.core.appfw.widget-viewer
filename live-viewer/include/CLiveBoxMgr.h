class CLiveBoxMgr {
public:
	CLiveBoxMgr(void);
	virtual ~CLiveBoxMgr(void);

	static int OnEvent(struct livebox *handler, const char *event);
	static int OnFaultEvent(const char *event, const char *pkgname, const char *id, const char *funcname, void *data);

	int Add(CLiveBox *box);
	int Remove(CLiveBox *box);

private:
	static struct dlist *s_pBoxList;
};
