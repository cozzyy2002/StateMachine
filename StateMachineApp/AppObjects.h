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
	LPCTSTR getName() const { return m_name.c_str(); }

protected:
	CAppObject(LPCTSTR name) : m_name(name) {}

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
	CAppState(LPCTSTR name, bool _isSubState, CStateMachineDoc* doc)
		: CAppObject(name), m_isSubState(_isSubState), m_doc(doc) {}
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
	CStateMachineDoc* m_doc;
};

class CAppEvent : public Event, public CAppObject
{
public:
	CAppEvent(LPCTSTR name) : CAppObject(name) {}
	~CAppEvent() {}

protected:
	virtual void modifyString(std::tstring& _string) override { CAppObject::modifyString(_string); }
};
