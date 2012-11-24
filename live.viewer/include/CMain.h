class CMain
{
public:
	static CMain *GetInstance();

	int OnCreate(void);
	int OnTerminate(void);
	int OnPause(void);
	int OnResume(void);
	int OnReset(service_h b);

	int UpdateCtrl(CLiveBox *box);
	int AppendLog(const char *str);

	int CreatePDCtrl(CLiveBox *box);
	int TogglePDCtrl(int is_on);
	int DestroyPDCtrl(void);

private:
	CMain();
	virtual ~CMain();

	int m_fVerbose;
	CLiveBoxMgr *m_pLiveBoxMgr;

	int m_CreateController(void);
	int m_CreateLogger(void);

	static CMain *m_pInstance;
};

/* End of a file */
