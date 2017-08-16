#pragma once

#include <StateMachine/Object.h>
#include <memory>
#include <mutex>

namespace state_machine {

class Event;
class State;
class Context;
class StateMachine;
class StateMachineImpl;
class StateHandle;

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

	// See Context::getStateLock().
	std::lock_guard<std::mutex>* getStateLock(Context* context);

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
	StateHandle() {}
	virtual ~StateHandle() {}

	virtual HRESULT handleEvent(Event* e, State* currentState, State** nextState) { return S_OK; }
	virtual HRESULT handleError(Event* e, HRESULT hr) { return hr; }
	virtual HRESULT entry(Event* e, State* previousState) { return S_OK; }
	virtual HRESULT exit(Event* e, State* nextState) { return S_OK; }

	// Always returns no master state.
	virtual State* getMasterState() const { return nullptr; }
};

class SubStateHandle : public StateHandle
{
public:
	SubStateHandle() {}
	virtual ~SubStateHandle() {}

	// Next state to tell state machine to go back to the master state.
	// This method can be used in handleEvent() method of sub state.
	// Useage: *nextState = backToMaster();
	State* backToMaster();

	// Returns master state.
	virtual State* getMasterState() const override { return m_masterState.get(); }

	std::shared_ptr<State> m_masterState;
};

} // namespace state_machine
