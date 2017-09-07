#pragma once

#include <StateMachine/Context.h>
#include <StateMachine/State.h>
#include <StateMachine/Event.h>

using namespace state_machine;

class CStateMachineDoc;

class CAppObject
{
public:
	virtual LPCTSTR getName() { return m_name.c_str(); }

protected:
	CAppObject(LPCTSTR name = _T("")) : m_name(name) {}

	// Implementation of state_machine::Object::modifyString() called by derived classes.
	void modifyString(std::tstring& _string) { _string += _T(":") + m_name; }
	std::tstring m_name;
};

class CAppContext : public Context, public CAppObject
{
public:
	CAppContext(bool isAsync, CStateMachineDoc* doc, LPCTSTR name);
	~CAppContext();

	CStateMachineDoc* const doc;

protected:
	virtual void modifyString(std::tstring& _string) override { CAppObject::modifyString(_string); }
};

class CAppState : public SubState, public CAppObject
{
public:
	CAppState(LPCTSTR name, bool _isSubState)
		: CAppObject(name), m_isSubState(_isSubState) {}
	virtual ~CAppState() {}

	HRESULT handleEvent(Event& e, State& currentState, State** nextState) override;
	HRESULT handleIgnoredEvent(Event& e) override;
	HRESULT handleError(Event& e, HRESULT hr) override;
	HRESULT entry(Event& e, State& previousState) override;
	HRESULT exit(Event& e, State& nextState) override;

	virtual bool isSubState() const override { return m_isSubState; }
	CAppState* getMasterState() const { return isSubState() ? SubState::getMasterState<CAppState>() : nullptr; }

protected:
	virtual void modifyString(std::tstring& _string) override { CAppObject::modifyString(_string); }

	bool m_isSubState;
};

struct StateMethod
{
public:
	StateMethod() : Return(S_OK) {}
	HRESULT Return;
};

struct DoStateMethod : public StateMethod
{
public:
	struct _NextState {
		_NextState() : IsSubState(false) {}
		std::tstring Name;
		bool IsSubState;
	} NextState;
};

class CStateConfig
{
public:
	// Default constructor
	// Each member initializes itself.
	CStateConfig() {}
	// Constructor with configure data.
	CStateConfig(const picojson::value& config);

	DoStateMethod Do;
	StateMethod Entry;
	StateMethod Exit;
};

class CAppEvent : public Event, public CAppObject
{
public:
	CAppEvent(LPCTSTR name, const picojson::value& config);
	~CAppEvent() {}

	const picojson::value& config;

protected:
	virtual void modifyString(std::tstring& _string) override { CAppObject::modifyString(_string); }
};
