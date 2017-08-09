#pragma once

#include <memory>
#include <mutex>

namespace state_machine {

class Event;
class State;
class StateMachine;

class Context
{
	friend class StateMachine;

public:
	Context(StateMachine* stateMachine);
	virtual ~Context();

	// Set initialState as current state and call initialState->entry().
	virtual HRESULT start(State* initialState);

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

private:	// Prevent members from being modified by derived class(User context).
	std::shared_ptr<State> currentState;
	StateMachine* stateMachine;

	// Mutex used to lock StateMachine::handleEvent().
	std::mutex eventQueueLock;
};

} // namespace state_machine
