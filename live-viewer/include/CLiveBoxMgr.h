class CLiveBoxMgr {
public:
	CLiveBoxMgr(void);
	virtual ~CLiveBoxMgr(void);

	static int OnEvent(struct livebox *handler, enum livebox_event_type type);
	static int OnFaultEvent(enum livebox_fault_type type, const char *pkgname, const char *id, const char *funcname, void *data);

	int Add(CLiveBox *box);
	int Remove(CLiveBox *box);

private:
	static struct dlist *s_pBoxList;
};
