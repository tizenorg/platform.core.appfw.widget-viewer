class CMain
{
public:
	CMain();
	virtual ~CMain();

	static int OnCreate(void *_this);
	static int OnTerminate(void *_this);
	static int OnPause(void *_this);
	static int OnResume(void *_this);
	static int OnReset(bundle *b, void *_this);

private:
	int m_OnCreate(void);
	int m_OnTerminate(void);
	int m_OnPause(void);
	int m_OnResume(void);
	int m_OnReset(bundle *b);

	int m_fVerbose;
	CLiveBoxMgr *m_pLiveBoxMgr;
};

/* End of a file */
