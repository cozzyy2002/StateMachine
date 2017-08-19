
// StateMachineDoc.h : interface of the CStateMachineDoc class
//


#pragma once

#include "AppObjects.h"

#include <StateMachine/StateMachine.h>
#include <StateMachine/Context.h>
#include <StateMachine/State.h>
#include <StateMachine/Event.h>

#include <vector>

using namespace state_machine;

class CStateMachineApp;

class CStateMachineDoc : public CDocument
{
protected: // create from serialization only
	CStateMachineDoc();
	DECLARE_DYNCREATE(CStateMachineDoc)

// Attributes
public:
	std::vector<CAppState*> m_stateStack;
	std::unique_ptr<CAppContext> m_context;
	StateMachine* m_stateMachine;
	CStateMachineApp* m_app;

// Operations
public:
	CAppContext* createContext(LPCTSTR name);
	CAppState* createState(LPCTSTR name, BOOL isSubState);
	HRESULT start(LPCTSTR stateName, CAppEvent* e = nullptr);

	bool parse(LPCTSTR source);
	template<typename T>
	T getConfigValue(const picojson::value& obj, LPCSTR key, const T defaultValue) const;
	template<typename T>
	const T& getConfigObject(const picojson::value& obj, LPCSTR key) const;
	std::tstring getConfigString(const picojson::value& obj, LPCSTR key, LPCTSTR defaultValue = _T("")) const;
	const picojson::value& getConfig() const { return m_config; }

	//void onContextDeleted(CAppContext* context);
	void onStateEntryCalled(CAppState* state);
	void onStateExitCalled(CAppState* state);

	void outputMessage(LPCTSTR format, ...);

protected:
	picojson::value m_config;

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

template<typename T>
inline T CStateMachineDoc::getConfigValue(const picojson::value& obj, LPCSTR key, const T defaultValue) const
{
	T ret(defaultValue);
	if(obj.is<picojson::object>()) {
		if(obj.contains(key)) {
			ret = obj.get(key).get<T>();
		}
	}
	return ret;
}

template<typename T>
const T& CStateMachineDoc::getConfigObject(const picojson::value& obj, LPCSTR key) const
{
	// Empty object returned if required object doesn't exist.
	static T dummy;
	if(obj.is<picojson::object>()) {
		if(obj.contains(key)) {
			const picojson::value& value = obj.get(key);
			if(value.is<T>()) {
				return value.get<T>();
			}
		}
	}
	return dummy;
}
