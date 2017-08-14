#pragma once

#include "Object.h"

namespace state_machine {

class Event;
class StateHandle;

class State : public Object
{
protected:
	State(State* previousState = nullptr, bool isSubState = false);

public:
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
	bool isSubState() const;

	/*
		Template method that returns master state pointer.
	*/
	template<class T = State>
	T* getMasterState() const { return (T*)getRawMasterState(); }

	StateHandle* getHadle() const { return m_hState; }

protected:
	// Next state to tell state machine to go back to the master state.
	// This method can be used in handleEvent() method of sub state.
	// Useage: *nextState = backToMaster();
	State* backToMaster();

	// Return value to tell state machine that event is not handled.
	// This method can be used in handleEvent() method.
	// Useage: return eventIsIgnored();
	HRESULT eventIsIgnored() const;

	State* getRawMasterState() const;

	StateHandle* m_hState;
};

} // namespace state_machine
