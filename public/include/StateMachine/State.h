#pragma once

#include "Object.h"

#include <memory>

namespace state_machine {

class Event;
class StateHandle;
class SubStateHandle;

class State : public Object
{
protected:
	State();

public:
	virtual ~State();

	virtual HRESULT handleEvent(Event* e, State* currentState, State** nextState) { return S_OK; }
	virtual HRESULT handleIgnoredEvent(Event* e) { return S_EVENT_IGNORED; }
	virtual HRESULT handleError(Event* e, HRESULT hr) { return hr; }
	virtual HRESULT entry(Event* e, State* previousState) { return S_OK; }
	virtual HRESULT exit(Event* e, State* nextState) { return S_OK; }

	// Value to tell state machine that event is not handled.
	// Default value is assigned (S_FALSE + 1).
	// If application uses this value for another meaning of HRESULT,
	// assign different *positive number* like:
	//    State::S_EVENT_IGNORED = 100
	static HRESULT S_EVENT_IGNORED;

	/*
		Returns whether the class is sub state or not.
		State transition to sub class means that previous state becomes master state of the state(sub state).
	*/
	virtual INLINE bool isSubState() const { return false; }

	// Internal use.
	template<class T = StateHandle>
	INLINE T* getHandle() const { return dynamic_cast<T*>(m_hState.get()); }

protected:
	// Return value to tell state machine that event is not handled.
	// This method can be used in handleEvent() method.
	// Useage: return eventIsIgnored();
	INLINE HRESULT eventIsIgnored() const { return S_EVENT_IGNORED; }

	std::unique_ptr<StateHandle> m_hState;
};

class SubState : public State
{
protected:
	SubState();

public:
	virtual ~SubState();

	// See State::isSubState().
	virtual INLINE bool isSubState() const override { return true; }

	/*
		Template method that returns master state pointer.
	*/
	template<class T = State>
	INLINE T* getMasterState() const { return dynamic_cast<T*>(getRawMasterState()); }

protected:
	// Next state to tell state machine to go back to the master state.
	// This method can be used in handleEvent() method of sub state.
	// Useage in State::handleEvent(): *nextState = backToMaster();
	State* backToMaster();

	State* getRawMasterState() const;
};

} // namespace state_machine
