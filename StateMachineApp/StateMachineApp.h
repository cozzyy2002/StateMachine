
// StateMachineApp.h : main header file for the StateMachineApp application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols

#include <StateMachine/StateMachine.h>
#include <memory>

using namespace state_machine;

// CStateMachineApp:
// See StateMachineApp.cpp for the implementation of this class
//

class CStateMachineApp : public CWinAppEx
{
public:
	CStateMachineApp();

	StateMachine* getStateMachine();

protected:
	std::unique_ptr<StateMachine> m_stateMachine;

// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
	UINT  m_nAppLook;
	BOOL  m_bHiColorIcons;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CStateMachineApp theApp;
