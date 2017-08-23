#pragma once

#include "Object.h"
#include "Event.h"

#include <mutex>
#include <memory>

namespace state_machine {

class State;
class StateMachine;
class ContextHandle;

class Context : public Object
{
protected:
	Context(StateMachine& stateMachine);

public:
	virtual ~Context();

	/*
		Start event handling in this context.

		Set initialState as current state.
		initialState->entry(Event* e, State* previousState) will be called.
		The method should ignore:
			e parameter if userEvent is not specified.
			previousState parameter which points internal State object.
	*/
	virtual HRESULT start(State* initialState, Event& userEvent = Event());

	// Stops state machine.
	virtual HRESULT stop();

	// true if event handling has been started(After calling start() before stop()).
	bool isStarted() const { return getCurrentState() ? true : false; }

	// Handles event in this context.
	virtual HRESULT handleEvent(Event& e);

	// Determine whether StateMachine::handleEvent() requires exclusive execution.
	// Returning true means that the method might be called from more than one thread simultaneously.
	// If this feature is required, override this method to return true.
	virtual INLINE bool isStateLockEnabled() const { return false; }

	// Returns lock_guard<mutex> pointer used by the state machine.
	// If you use this method outside of StateMachine,
	// call this method with std::unique_ptr<> like:
	//   std::unique_prt<std::lock_guard<std::mutex>> _lock(context->getStateLock());
	// If isStateLockEnabled() returns false, this method returns nullptr;
	std::lock_guard<std::mutex>* getStateLock();

	bool isEventHandling() const;

	// Template method that returns current State object.
	// Do NOT delete returned object.
	template<class T = State>
	T* getCurrentState() const { return dynamic_cast<T*>(getCurrentRawState()); }

	// Internal use.
	INLINE ContextHandle* getHandle() const { return m_hContext.get(); }

protected:
	std::unique_ptr<ContextHandle> m_hContext;

	// Do NOT delete returned object.
	State* getCurrentRawState() const;
};

} // namespace state_machine
