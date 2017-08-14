#pragma once

#include "Object.h"

#include <mutex>

namespace state_machine {

class Event;
class State;
class StateMachine;
class ContextHandle;

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
	std::lock_guard<std::mutex>* getStateLock();

	bool isEventHandling() const;

	ContextHandle* getHadle() const { return m_hContext; }

protected:
	ContextHandle* m_hContext;
};

} // namespace state_machine
