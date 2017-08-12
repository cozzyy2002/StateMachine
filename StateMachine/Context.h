#pragma once

#include "Object.h"
#include <memory>
#include <mutex>

namespace state_machine {

class Event;
class State;
class StateMachine;
class StateMachineImpl;

class Context : public Object
{
public:
	Context(StateMachine* stateMachine);
	virtual ~Context();

	// Set initialState as current state and call initialState->entry().
	virtual HRESULT start(State* initialState, Event* userEvent = nullptr);

	// Stops state machine.
	virtual HRESULT stop();

	virtual HRESULT handleEvent(Event* e);

	// Determine whether StateMachine::handleEvent() requires exclusive execution.
	// Returning true means that the method might be called from more than one thread simultaneously.
	virtual bool isStateLockEnabled() const { return false; }

	// Returns lock_guard<mutex> pointer.
	// If you use this method outside of StateMachine,
	// call this method with std::unique_ptr<> like:
	//   std::unique_prt<std::lock_guard<std::mutex>> _lock(context->getStateLock());
	// If isStateLockEnabled() returns false, this method returns nullptr;
	std::lock_guard<std::mutex>* geStatetLock();

	bool isEventHandling() const { return m_isEventHandling; }

private:
	// Derived class(User context) can not modify members of this class.
	friend class StateMachineImpl;

	std::shared_ptr<State> currentState;
	StateMachine* const stateMachine;

	// true if the state machine is handling event in this context.
	// Used to recursive call check by state machine.
	bool m_isEventHandling;

	// Mutex used to lock StateMachine::handleEvent().
	std::mutex stateLock;
};

} // namespace state_machine
