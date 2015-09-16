#pragma once
#include "afxwin.h"
#include "CoreAudio.h"

// CAudioConfigDlg dialog

class CAudioConfigDlg : public CDialog
{
	DECLARE_DYNAMIC(CAudioConfigDlg)

public:
	CAudioConfigDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAudioConfigDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_AUDIO_CONFIG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CComboBox m_devices;
	CCoreAudio	*m_pAudio;
	int			m_audio_device;

	virtual BOOL OnInitDialog();
	virtual void OnOK();
};
