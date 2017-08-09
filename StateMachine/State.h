#pragma once

#include "Object.h"
#include <memory>

// Value returned by event handlers when the event has not been handled.
#define S_EVENT_IGNORED ((HRESULT)10)

namespace state_machine {

class Event;

class State : public Object
{
public:
	State(State* previousState = nullptr, bool isSubState = false);
	virtual ~State();

	virtual HRESULT handleEvent(const Event* e, const State* currentState, State** nextState) { return S_OK; }
	virtual HRESULT handleIgnoredEvent(const Event* e) { return S_EVENT_IGNORED; }
	virtual HRESULT handleError(const Event* e, HRESULT hr) { return hr; }
	virtual HRESULT entry(const Event* e, const State* previousState) { return S_OK; }
	virtual HRESULT exit(const Event* e, const State* nextState) { return S_OK; }

	/*
		Returns whether the class is sub state or not.
		State transition to sub class means that previous state becomes master state of the state(sub state).
	*/
	bool isSubState() const { return m_masterState ? true : false; }

	std::shared_ptr<State>& masterState() { return m_masterState; }

protected:
	// Return value to tell state machine to go back to the master state.
	// This method can be used in handleEvent() method of sub state.
	// Useage: return backToMaster();
	State* backToMaster();

	std::shared_ptr<State> m_masterState;

	// Implementation of Object::getObject()
	virtual const Object* getObject() const { return this; }
};

} // namespace state_machine
