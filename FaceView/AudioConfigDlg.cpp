// AudioConfigDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FaceView.h"
#include "AudioConfigDlg.h"
#include "afxdialogex.h"


// CAudioConfigDlg dialog

IMPLEMENT_DYNAMIC(CAudioConfigDlg, CDialog)

CAudioConfigDlg::CAudioConfigDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAudioConfigDlg::IDD, pParent)
{

}

CAudioConfigDlg::~CAudioConfigDlg()
{
}

void CAudioConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_AUDIO, m_devices);
}


BEGIN_MESSAGE_MAP(CAudioConfigDlg, CDialog)
END_MESSAGE_MAP()


// CAudioConfigDlg message handlers


BOOL CAudioConfigDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_devices.ResetContent();
	m_devices.AddString("[Default audio recording device]");

	TRACE("Found %d audio devices\n",m_pAudio->getNumCaptureDevices());

	// add devices to input list
	char	name[256];
	int cnt=0;
	for (int i=0;i<m_pAudio->getNumCaptureDevices();i++) {
		m_pAudio->getCaptureDeviceName(i,name,256);
		m_devices.AddString(name);
		TRACE("Add '%s'\n",name);
		cnt++;
	}
	if (cnt==0) {
		AfxMessageBox("No audio capture devices found!",MB_ICONEXCLAMATION);
		return FALSE;
	}
	m_devices.SetCurSel(m_audio_device+1);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CAudioConfigDlg::OnOK()
{
	m_audio_device=m_devices.GetCurSel()-1;

	CDialog::OnOK();
}
