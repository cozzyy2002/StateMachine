
// StateMachineView.h : interface of the CStateMachineView class
//

#pragma once


class CStateMachineView : public CFormView
{
protected: // create from serialization only
	CStateMachineView();
	DECLARE_DYNCREATE(CStateMachineView)

public:
#ifdef AFX_DESIGN_TIME
	enum{ IDD = IDD_STATEMACHINEAPP_FORM };
#endif

// Attributes
public:
	CStateMachineDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnInitialUpdate(); // called first time after construct

// Implementation
public:
	virtual ~CStateMachineView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	CString m_contextName;
	CString m_StateName;
	BOOL m_isSubState;
	afx_msg void OnClickedButtonContextCreate();
	afx_msg void OnClickedButtonContextStart();
};

#ifndef _DEBUG  // debug version in StateMachineView.cpp
inline CStateMachineDoc* CStateMachineView::GetDocument() const
   { return reinterpret_cast<CStateMachineDoc*>(m_pDocument); }
#endif
