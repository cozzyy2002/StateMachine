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

class CAppState : public State, public CAppObject
{
public:
	CAppState(LPCTSTR name, CAppState* masterState = nullptr)
		: CAppObject(name), State(masterState) {}
	virtual ~CAppState() {}

	HRESULT handleEvent(Context& context, Event& e, State& currentState, State** nextState) override;
	HRESULT handleIgnoredEvent(Context& context, Event& e) override;
	HRESULT handleError(Context& context, Event& e, HRESULT hr) override;
	HRESULT entry(Context& context, Event& e, State& previousState) override;
	HRESULT exit(Context& context, Event& e, State& nextState) override;

	CAppState* getMasterState() const { return State::getMasterState<CAppState>(); }

protected:
	virtual void modifyString(std::tstring& _string) override { CAppObject::modifyString(_string); }
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
