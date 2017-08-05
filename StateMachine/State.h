#pragma once

#include <memory>

// Value returned by event handlers when the event has not been handled.
#define S_EVENT_IGNORED ((HRESULT)10)

namespace state_machine {

class Event;

class State
{
public:
	State(State* previousState = nullptr, bool isSubState = false);
	virtual ~State();

	virtual HRESULT handleEvent(const Event* e, const State* currentState, State** nextState) { return S_OK; }
	virtual HRESULT handleIgnoredEvent(const Event* e) { return S_EVENT_IGNORED; }
	virtual HRESULT handleError(const Event* e, HRESULT hr) { return hr; }
	virtual HRESULT entry(const Event* e, const State* previousState) { return S_OK; }
	virtual HRESULT exit(const Event* e, const State* nextState) { return S_OK; }
	virtual LPCTSTR toString() const { return _T("State"); };

	/*
		Returns whether the class is sub state or not.
		State transition to sub class means that previous state becomes master state of the state(sub state).
	*/
	bool isSubState() const { return m_masterState ? true : false; }

	bool isEntryCalled() const { return m_entryCalled; }
	void setEntryCalled(bool entryCalled) { m_entryCalled = entryCalled; }

	State* getMasterState() const { return m_masterState.get(); }
	void setMasterState(std::shared_ptr<State>& master) { m_masterState = master; }

	// Constant to set as next state to tell state machine to go back to the master state.
	static State* RETURN_TO_MASTER;

protected:
	std::shared_ptr<State> m_masterState;

	// Flag to restrict entry() to be called only once.
	bool m_entryCalled;
};

} // namespace state_machine
