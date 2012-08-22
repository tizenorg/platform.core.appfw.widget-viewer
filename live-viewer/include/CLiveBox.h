class CLiveBox {
public:
	CLiveBox(const char *pkgname, const char *content, const char *cluster, const char *category, double period);
	CLiveBox(struct livebox *handler);
	virtual ~CLiveBox(void);

	void OnCreate(struct livebox *handler);

	void OnUpdateLB(void);
	void OnUpdatePD(void);

	void OnPeriodChanged(void);
	void OnGroupChanged(void);
	void OnPinupChanged(void);

	void SetHandler(struct livebox *handler) { m_pHandler = handler; }
	struct livebox *Handler(void) { return m_pHandler; }

	int CreatePD(void);
	int DestroyPD(void);

private:
	int m_OnCreate(void);

	struct livebox *m_pHandler;

	Evas_Object *m_pIconSlot;
	Evas_Object *m_pLBImage;
	Evas_Object *m_pPDImage;
};

/* End of a file */
