class CMain
{
public:
	CMain();
	virtual ~CMain();

	int OnCreate(void);
	int OnTerminate(void);
	int OnPause(void);
	int OnResume(void);
	int OnReset(bundle *b);

private:
	int m_fVerbose;
	CLiveBoxMgr *m_pLiveBoxMgr;
};

/* End of a file */
