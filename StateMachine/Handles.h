#pragma once

#include "Object.h"
#include <memory>
#include <mutex>

namespace state_machine {

class Event;
class State;
class Context;
class StateMachine;
class StateMachineImpl;
class StateHandle;
class ContexHandle;

class ContextHandle : public Object
{
public:
	ContextHandle(StateMachine* stateMachine);
	virtual ~ContextHandle();

	// Set initialState as current state and call initialState->entry().
	virtual HRESULT start(Context* context, State* initialState, Event* userEvent);

	// Stops state machine.
	virtual HRESULT stop(Context* context);

	virtual HRESULT handleEvent(Context* context, Event* e);

	// Determine whether StateMachine::handleEvent() requires exclusive execution.
	// Returning true means that the method might be called from more than one thread simultaneously.
	virtual bool isStateLockEnabled() const { return false; }

	// Returns lock_guard<mutex> pointer.
	// If you use this method outside of StateMachine,
	// call this method with std::unique_ptr<> like:
	//   std::unique_prt<std::lock_guard<std::mutex>> _lock(context->getStateLock());
	// If isStateLockEnabled() returns false, this method returns nullptr;
	std::lock_guard<std::mutex>* getStateLock();

	bool isEventHandling() const { return m_isEventHandling; }

	std::shared_ptr<State> currentState;
	StateMachineImpl* const stateMachine;

	// true if the state machine is handling event in this context.
	// Used to recursive call check by state machine.
	bool m_isEventHandling;

	// Mutex used to lock StateMachine::handleEvent().
	std::mutex stateLock;
};

class StateHandle : public Object
{
public:
	StateHandle(bool isSubState);
	virtual ~StateHandle();

	virtual HRESULT handleEvent(Event* e, State* currentState, State** nextState) { return S_OK; }
	virtual HRESULT handleError(Event* e, HRESULT hr) { return hr; }
	virtual HRESULT entry(Event* e, State* previousState) { return S_OK; }
	virtual HRESULT exit(Event* e, State* nextState) { return S_OK; }

	/*
		Returns whether the class is sub state or not.
		State transition to sub class means that previous state becomes master state of the state(sub state).
	*/
	inline bool isSubState() const { return m_isSubState; }

	State* getMasterState() const { return m_masterState.get(); }

	// Next state to tell state machine to go back to the master state.
	// This method can be used in handleEvent() method of sub state.
	// Useage: *nextState = backToMaster();
	State* backToMaster();

	std::shared_ptr<State> m_masterState;

	const bool m_isSubState;
};

} // namespace state_machine
