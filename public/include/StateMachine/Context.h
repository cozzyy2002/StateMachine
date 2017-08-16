#pragma once

#include "Object.h"

#include <mutex>
#include <memory>

namespace state_machine {

class Event;
class State;
class StateMachine;
class ContextHandle;

class Context : public Object
{
protected:
	Context(StateMachine* stateMachine);

public:
	virtual ~Context();

	// Set initialState as current state and call initialState->entry().
	virtual HRESULT start(State* initialState, Event* userEvent = nullptr);

	// Stops state machine.
	virtual HRESULT stop();

	virtual HRESULT handleEvent(Event* e);

	// Determine whether StateMachine::handleEvent() requires exclusive execution.
	// Returning true means that the method might be called from more than one thread simultaneously.
	// If this feature is required, override this method to return true.
	virtual bool isStateLockEnabled() const { return false; }

	// Returns lock_guard<mutex> pointer used by the state machine.
	// If you use this method outside of StateMachine,
	// call this method with std::unique_ptr<> like:
	//   std::unique_prt<std::lock_guard<std::mutex>> _lock(context->getStateLock());
	// If isStateLockEnabled() returns false, this method returns nullptr;
	std::lock_guard<std::mutex>* getStateLock();

	bool isEventHandling() const;

	/*
	Returns current State object.
	*/
	State* getCurrentState() const;

	// Internal use.
	ContextHandle* getHandle() const { return m_hContext.get(); }

protected:
	std::unique_ptr<ContextHandle> m_hContext;
};

} // namespace state_machine
