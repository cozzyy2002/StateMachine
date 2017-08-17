
// StateMachineDoc.h : interface of the CStateMachineDoc class
//


#pragma once

#include "AppObjects.h"

#include <StateMachine/StateMachine.h>
#include <StateMachine/Context.h>
#include <StateMachine/State.h>
#include <StateMachine/Event.h>

#include <map>

using namespace state_machine;

class CStateMachineDoc : public CDocument
{
protected: // create from serialization only
	CStateMachineDoc();
	DECLARE_DYNCREATE(CStateMachineDoc)

// Attributes
public:
	typedef std::map<std::tstring, std::unique_ptr<CAppContext>> context_list_t;
	context_list_t m_context_list;
	StateMachine* m_stateMachine;
	CStateMachineApp* m_app;

// Operations
public:
	CAppContext* createContext(LPCTSTR name);
	CAppState* createState(LPCTSTR name, bool isSubState);
	HRESULT start(CAppContext* context, LPCTSTR stateName);

	void outputMessage(LPCTSTR format, ...);

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
