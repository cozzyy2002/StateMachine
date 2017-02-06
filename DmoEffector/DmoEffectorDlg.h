
// DmoEffectorDlg.h : header file
//

#pragma once
#include "afxwin.h"

#include "MainController.h"
#include "AsioDriver.h"
#include "Device.h"


// CDmoEffectorDlg dialog
class CDmoEffectorDlg : public CDialogEx
{
protected:
	CMainController m_mainController;
	CAsioDriver::list_t m_asioDriverList;
	CDevice::list_t m_inputDeviceList;
	CDevice::list_t m_outputDeviceList;

// Construction
public:
	CDmoEffectorDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DMOEFFECTOR_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CComboBox m_inputDeviceSel;
	afx_msg void OnBnClickedButtonStart();
	CComboBox m_outputDeviceSel;
	afx_msg void OnBnClickedButtonStop();
	CComboBox m_asioDriverSel;
};
