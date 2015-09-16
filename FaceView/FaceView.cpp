
// FaceView.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "FaceView.h"
#include "MainFrm.h"

#include "FaceViewDoc.h"
#include "FaceViewView.h"

#include "liblipsync_license.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CFaceViewApp

BEGIN_MESSAGE_MAP(CFaceViewApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, &CFaceViewApp::OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, &CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinApp::OnFileOpen)
	ON_COMMAND(ID_RECOGNISER_ANNOSOFT, &CFaceViewApp::OnRecogniserAnnosoft)
	ON_COMMAND(ID_GENDER_MALE, &CFaceViewApp::OnGenderMale)
	ON_COMMAND(ID_GENDER_FEMALE, &CFaceViewApp::OnGenderFemale)
	ON_COMMAND(ID_GENDER_GENERIC, &CFaceViewApp::OnGenderGeneric)
END_MESSAGE_MAP()


// CFaceViewApp construction

CFaceViewApp::CFaceViewApp()
{
	// TODO: replace application ID string below with unique ID string; recommended
	// format for string is CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("FaceView.AppID.NoVersion"));

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance

	m_recogniser=0;
	m_gender=0;
}

// The one and only CFaceViewApp object

CFaceViewApp theApp;


// CFaceViewApp initialization

BOOL CFaceViewApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

    // Initialize the COM library
	int hr;
	if ((hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE))!=0) {
		AfxMessageBox("CoInitialize fails",MB_ICONEXCLAMATION);
		exit(1);
	}

	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	EnableTaskbarInteraction(FALSE);

	// AfxInitRichEdit2() is required to use RichEdit control	
	// AfxInitRichEdit2();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need

	// Licensing code: the keys
	const char* cszLicenseKey = "6F58-4050-415C-7C57-4B4C-5859-4059-584C-4B40-4A54-5E5E-5C79-573A-3D27-4226";
	const char* cszUserName = "GEOFF_WILLIAMS";
	const char* cszCompanyName = "UCL";

	ILibLipsyncRegistrar* pRegistrar = NULL;
	// pull the registrar interface from the lipsync tool 
	// registrar defined in liblipsync_license.h
	LipsyncGetInfoMgr(&pRegistrar);
	if (pRegistrar)
	{
		pRegistrar->SetUserName(cszUserName);
		pRegistrar->SetCompanyName(cszCompanyName);
		pRegistrar->SetLicenseKey(cszLicenseKey);
		pRegistrar->Release();
	}
	// end of licensing code snippet

	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("UCL Avatar Therapy"));
	LoadStdProfileSettings(4);  // Load standard INI file options (including MRU)

	// set the data folder path
	char *tmp = _tcsdup(m_pszHelpFilePath);
	char *p=strrchr(tmp,'\\');
	if (!p) p=strrchr(tmp,'/');
	if (p) {
		*p='\0';
		p=strrchr(tmp,'\\');
		if (!p) p=strrchr(tmp,'/');
		if (p) {
			*p='\0';
			m_datadir.Format("%s\\annorec",tmp);
		}
		else {
			m_datadir="../annorec";
		}
	}
	else {
		m_datadir="../annorec";
	}
	free(tmp);
	TRACE("data dir='%s'\n",m_datadir);

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CFaceViewDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CFaceViewView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);


	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Enable DDE Execute open
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);

	// init high-resolution timer
	QueryPerformanceFrequency(&m_hrtfreq);
	QueryPerformanceCounter(&m_hrtinit);

	// load last used audio configuration
	m_microphone = GetProfileString("Audio","Microphone","");
	m_recogniser = GetProfileInt("Recogniser","Type",0);
	m_gender = GetProfileInt("Recogniser","Gender",2);

	// create the Annosoft recogniser
	m_annorec.Create(m_datadir,m_gender);

	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();
	// call DragAcceptFiles only if there's a suffix
	//  In an SDI app, this should occur after ProcessShellCommand
	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();
	return TRUE;
}

int CFaceViewApp::ExitInstance()
{
	//TODO: handle additional resources you may have added
	AfxOleTerm(FALSE);

	CoUninitialize();

	// save last used audio configuration
	WriteProfileString("Audio","Microphone",(LPCTSTR)m_microphone);
	WriteProfileInt("Recogniser","Type",m_recogniser);
	WriteProfileInt("Recogniser","Gender",m_gender);

	return CWinApp::ExitInstance();
}

// CFaceViewApp message handlers


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// App command to run the dialog
void CFaceViewApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CFaceViewApp message handlers

unsigned long CFaceViewApp::GetTime(void)
{
	LARGE_INTEGER	hrtnow;
	QueryPerformanceCounter(&hrtnow);
	return (unsigned long)((1000*(hrtnow.QuadPart-m_hrtinit.QuadPart))/m_hrtfreq.QuadPart);
}


int CFaceViewApp::GetRecognitionResults(long mstime,CArray<CVisemeMixture,CVisemeMixture &> &list)
{
	m_annorec.Process(mstime,list);
	return 0;
}

int CFaceViewApp::PutVoiceData(float *buf,int cnt)
{
	short	sbuf[SAMPLE_RATE];
	for (int i=0;i<cnt;i++) sbuf[i] = (short)(32000.0*buf[i]);

	m_annorec.OnRecord((byte *)sbuf,2*cnt);
	return 0;
}



void CFaceViewApp::OnRecogniserAnnosoft()
{
	m_recogniser=0;
}


void CFaceViewApp::OnGenderMale()
{
	if (m_gender!=0) {
		m_gender=0;
		m_annorec.Create(m_datadir,m_gender);
	}
}


void CFaceViewApp::OnGenderFemale()
{
	if (m_gender!=1) {
		m_gender=1;
		m_annorec.Create(m_datadir,m_gender);
	}
}


void CFaceViewApp::OnGenderGeneric()
{
	if (m_gender!=2) {
		m_gender=2;
		m_annorec.Create(m_datadir,m_gender);
	}
}
