
// StateMachineApp.h : main header file for the StateMachineApp application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CStateMachineApp:
// See StateMachineApp.cpp for the implementation of this class
//

class CStateMachineApp : public CWinAppEx
{
public:
	CStateMachineApp();


// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
protected:
	HMENU  m_hMDIMenu;
	HACCEL m_hMDIAccel;

public:
	BOOL  m_bHiColorIcons;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	afx_msg void OnFileNew();
	DECLARE_MESSAGE_MAP()
};

extern CStateMachineApp theApp;
