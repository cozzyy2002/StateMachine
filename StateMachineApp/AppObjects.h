#pragma once

#include <StateMachine/Context.h>
#include <StateMachine/State.h>
#include <StateMachine/Event.h>
#include <StateMachine/StateMachine.h>

using namespace state_machine;

class CStateMachineDoc;

class CAppObject
{
public:
	virtual LPCTSTR getName() { return m_name.c_str(); }

protected:
	CAppObject(CStateMachineDoc* doc, LPCTSTR name = _T("")) : doc(doc), m_name(name) {}

	// Implementation of state_machine::Object::modifyString() called by derived classes.
	void modifyString(std::tstring& _string) { _string += _T(":") + m_name; }
	std::tstring m_name;

	CStateMachineDoc * doc;
};

class CAppContext : public Context, public CAppObject
{
public:
	CAppContext(CStateMachineDoc* doc, LPCTSTR name, StateMachine* stateMachine);
	~CAppContext();

protected:
	virtual void modifyString(std::tstring& _string) override { CAppObject::modifyString(_string); }
};

class CAppState : public SubState, public CAppObject
{
public:
	CAppState(CStateMachineDoc* doc, LPCTSTR name, bool _isSubState)
		: CAppObject(doc, name), m_isSubState(_isSubState) {}
	virtual ~CAppState() {}

	HRESULT handleEvent(Event* e, State* currentState, State** nextState) override;
	HRESULT handleIgnoredEvent(Event* e) override;
	HRESULT handleError(Event* e, HRESULT hr) override;
	HRESULT entry(Event* e, State* previousState) override;
	HRESULT exit(Event* e, State* nextState) override;

	virtual bool isSubState() const override { return m_isSubState; }
	CAppState* getMasterState() const { return isSubState() ? SubState::getMasterState<CAppState>() : nullptr; }

protected:
	virtual void modifyString(std::tstring& _string) override { CAppObject::modifyString(_string); }

	bool m_isSubState;
};

class CAppEvent : public Event, public CAppObject
{
public:
	CAppEvent(CStateMachineDoc* doc, LPCTSTR name, const picojson::value& config);
	~CAppEvent() {}

	const picojson::value& config;

protected:
	virtual void modifyString(std::tstring& _string) override { CAppObject::modifyString(_string); }
};
