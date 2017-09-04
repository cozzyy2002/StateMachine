
// StateMachineDoc.h : interface of the CStateMachineDoc class
//


#pragma once

#include "AppObjects.h"
#include "JsonUtil.h"

#include <StateMachine/Context.h>
#include <StateMachine/State.h>
#include <StateMachine/Event.h>

#include <vector>

using namespace state_machine;

class CStateMachineApp;

enum class UpdateViewHint {
	ConfigLoaded,
	ConfigParsed,
	StateChanged,
};

class CStateMachineDoc : public CDocument
{
protected: // create from serialization only
	CStateMachineDoc();
	DECLARE_DYNCREATE(CStateMachineDoc)

// Attributes
public:
	std::vector<CAppState*> m_stateStack;
	std::unique_ptr<CAppContext> m_context;
	CStateMachineApp* m_app;

// Operations
public:
	CAppContext* createContext(LPCTSTR name);
	CAppState* createState(LPCTSTR name, BOOL isSubState);
	HRESULT start(LPCTSTR stateName, CAppEvent* e = nullptr);

	bool parse(LPCTSTR source);
	const CString& getConfigSource() const { return m_configSource; }
	const CJsonObject& getConfig() const { return *m_config; }

	//void onContextDeleted(CAppContext* context);
	void onStateEntryCalled(CAppState* state);
	void onStateExitCalled(CAppState* state);

	void outputMessage(LPCTSTR format, ...);

protected:
	// JSON string
	// parse() updates and getConfigSource() returns this value.
	CString m_configSource;
	// parse() sets this value using m_configSource string.
	picojson::value m_configJson;
	// Root config.
	// Constructor sets this value to empty object.
	// And parse() updates using m_configJson.
	std::unique_ptr<CJsonObject> m_config;

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
	virtual void SetTitle(LPCTSTR lpszTitle);
};
