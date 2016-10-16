
// DmoEffectorDlg.h : header file
//

#pragma once
#include "afxwin.h"

#include "Device.h"


// CDmoEffectorDlg dialog
class CDmoEffectorDlg : public CDialogEx
{
protected:
	CDevice::device_list_t m_inputDeviceList;

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
};