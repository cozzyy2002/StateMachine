#pragma once

#include <StateMachine/Context.h>
#include <StateMachine/State.h>
#include <StateMachine/Event.h>
#include <StateMachine/StateMachine.h>

using namespace state_machine;

class CAppObject
{
protected:
	CAppObject(LPCTSTR name) : m_name(name) {}

	LPCTSTR getName() const { return m_name.c_str(); }

	// Implementation of state_machine::Object::modifyString() called by derived classes.
	void modifyString(std::tstring& _string) { _string += _T(":") + m_name; }
	std::tstring m_name;
};

class CAppContext : public Context, public CAppObject
{
public:
	CAppContext(LPCTSTR name, StateMachine* stateMachine);
	~CAppContext();

protected:
	virtual void modifyString(std::tstring& _string) override { CAppObject::modifyString(_string); }
};

class CAppState : public SubState, public CAppObject
{
public:
	virtual ~CAppState() {}

	HRESULT handleEvent(Event* e, State* currentState, State** nextState) { return S_OK; }
	HRESULT handleIgnoredEvent(Event* e) { return S_OK; }
	HRESULT handleError(Event* e, HRESULT hr) { return S_OK; }
	HRESULT entry(Event* e, State* previousState) { return S_OK; }
	HRESULT exit(Event* e, State* nextState) { return S_OK; }

	virtual bool isSubState() const { return m_isSubState; }
	CAppState* getMasterState() const { isSubState() ? SubState::getMasterState<CAppState>() : nullptr; }

protected:
	CAppState(LPCTSTR name, bool isSubstate) : CAppObject(name) {}
	virtual void modifyString(std::tstring& _string) override { CAppObject::modifyString(_string); }

	bool m_isSubState;
};

class CAppEvent : public Event, public CAppObject
{
public:
	~CAppEvent() {}

protected:
	CAppEvent(LPCTSTR name) : CAppObject(name) {}
	virtual void modifyString(std::tstring& _string) override { CAppObject::modifyString(_string); }
};
