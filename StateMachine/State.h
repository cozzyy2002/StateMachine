#pragma once

#include "Object.h"
#include <memory>

// Value returned by event handlers when the event has not been handled.
#define S_EVENT_IGNORED ((HRESULT)10)

namespace state_machine {

class Event;
class StateMachine;

class State : public Object
{
public:
	State(State* previousState = nullptr, bool isSubState = false);
	virtual ~State();

	virtual HRESULT handleEvent(Event* e, State* currentState, State** nextState) { return S_OK; }
	virtual HRESULT handleIgnoredEvent(Event* e) { return eventIsIgnored(); }
	virtual HRESULT handleError(Event* e, HRESULT hr) { return hr; }
	virtual HRESULT entry(Event* e, State* previousState) { return S_OK; }
	virtual HRESULT exit(Event* e, State* nextState) { return S_OK; }

	/*
		Returns whether the class is sub state or not.
		State transition to sub class means that previous state becomes master state of the state(sub state).
	*/
	bool isSubState() const { return m_masterState ? true : false; }

	std::shared_ptr<State>& masterState() { return m_masterState; }

protected:
	// Next state to tell state machine to go back to the master state.
	// This method can be used in handleEvent() method of sub state.
	// Useage: *nextState = backToMaster();
	State* backToMaster();

	// Return value to tell state machine that event is not handled.
	// This method can be used in handleEvent() method.
	// Useage: return eventIsIgnored();
	HRESULT eventIsIgnored() const { return S_EVENT_IGNORED; }

private:
	// Derived class(User state) can not modify members of this class.
	friend class StateMachine;

	std::shared_ptr<State> m_masterState;
};

} // namespace state_machine
