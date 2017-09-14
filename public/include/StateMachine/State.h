#pragma once

#include "Object.h"

#include <memory>

namespace state_machine {

class Context;
class Event;
class StateHandle;
class SubStateHandle;

class State : public Object
{
protected:
	State(bool isSubState = false);

public:
	virtual ~State();

	virtual HRESULT handleEvent(Context& context, Event& e, State& currentState, State** nextState) { return S_OK; }
	virtual HRESULT handleIgnoredEvent(Context& context, Event& e) { return S_EVENT_IGNORED; }
	virtual HRESULT handleError(Context& context, Event& e, HRESULT hr) { return hr; }
	virtual HRESULT entry(Context& context, Event& e, State& previousState) { return S_OK; }
	virtual HRESULT exit(Context& context, Event& e, State& nextState) { return S_OK; }

	// Value to tell state machine that event is not handled.
	// Default value is assigned (S_FALSE + 1).
	// If application uses this value for another meaning of HRESULT,
	// assign different *positive number* like:
	//    State::S_EVENT_IGNORED = 100
	static HRESULT S_EVENT_IGNORED;

	/*
		Returns whether the class is sub state or not.
		State transition to sub state means that previous state becomes master state of the state(sub state).
	*/
	virtual bool isSubState() const;

	// Internal use.
	// Do NOT delete returned object.
	template<class T = StateHandle>
	INLINE T* getHandle() const { return dynamic_cast<T*>(m_hState.get()); }

	/*
		Template method that returns master state pointer.

		Do NOT delete returned object.
	*/
	template<class T = State>
	INLINE T* getMasterState() const { return dynamic_cast<T*>(getRawMasterState()); }

protected:
	// Return value to tell state machine that event is not handled.
	// This method can be used in handleEvent() method.
	// Useage in State::handleEvent(): return eventIsIgnored();
	INLINE HRESULT eventIsIgnored() const { return S_EVENT_IGNORED; }

	// Next state to tell state machine to go back to the master state.
	// This method can be used in handleEvent() method of sub state.
	// Useage in State::handleEvent(Event&, State&, State** nextState): *nextState = backToMaster();
	// Do NOT delete returned object.
	State* backToMaster();

	// Do NOT delete returned object.
	State* getRawMasterState() const;

private:
	std::unique_ptr<StateHandle> m_hState;
};

// Alias for application
class SubState : public State
{
public:
	SubState() : State(true) {}
};

} // namespace state_machine
