
// StateMachineDoc.h : interface of the CStateMachineDoc class
//


#pragma once

#include "AppObjects.h"

#include <StateMachine/StateMachine.h>
#include <StateMachine/Context.h>
#include <StateMachine/State.h>
#include <StateMachine/Event.h>

using namespace state_machine;

class CStateMachineDoc : public CDocument
{
protected: // create from serialization only
	CStateMachineDoc();
	DECLARE_DYNCREATE(CStateMachineDoc)

// Attributes
public:
	std::unique_ptr<CAppContext> m_context;
	StateMachine* m_stateMachine;
	CStateMachineApp* m_app;

// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// Implementation
public:
	virtual ~CStateMachineDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// Helper function that sets search content for a Search Handler
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
public:
//	afx_msg void OnButtonContextCreate();
//	afx_msg void OnUpdateButtonContextCreate(CCmdUI *pCmdUI);
};
