class CResourceMgr {
public:
	static CResourceMgr *GetInstance(void);

	int RegisterObject(const char *tag, void *data);
	void *UnregisterObject(const char *tag);
	void *GetObject(const char *tag);

private:
	CResourceMgr(void);
	virtual ~CResourceMgr(void);

	static CResourceMgr *m_pInstance;
	struct dlist *m_pList;
};
