#pragma once

#include "Object.h"
#include "Event.h"

#include <mutex>
#include <memory>

namespace state_machine {

class State;
class ContextHandleBase;
class StateMachine;

class Context : public Object
{
protected:
	Context(bool isAsync);

	// Internal use.
	// Initialize of all members should be preformed by derived class.
	Context(bool isAsync, ContextHandleBase* hContext);

public:
	virtual ~Context();

	INLINE bool isAsync() const { return m_isAsync; }

	/*
		Start event handling in this context.

		Set initialState as current state.
		initialState->entry(Event* e, State* previousState) will be called.
		The method should ignore:
			e parameter if userEvent is not specified.
			previousState parameter which points internal State object.
	*/
	// Supported by Context.
	HRESULT start(State* initialState, Event& userEvent);
	// Supported by AsyncContext.
	HRESULT start(State* initialState, Event* userEvent);
	// Supported by Context and AsyncContext.
	HRESULT start(State* initialState);

	// Stops state machine.
	HRESULT stop();

	// true if event handling has been started(After calling start() before stop()).
	bool isStarted() const;

	// Handles event in this context.
	HRESULT handleEvent(Event& e);

	HRESULT queueEvent(Event* e);

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
	// nullptr might be returned before calling start() or after stop()
	// Do NOT delete returned object.
	template<class T = State>
	T* getCurrentState() const { return dynamic_cast<T*>(getCurrentRawState()); }

	// Do NOT delete returned object.
	StateMachine* getStateMachine();

	// Internal use.
	template<class T = ContextHandleBase>
	T* getHandle() const { return dynamic_cast<T*>(m_hContext.get()); }

private:
	// Do NOT delete returned object.
	State* getCurrentRawState() const;

	bool m_isAsync;
	std::unique_ptr<ContextHandleBase> m_hContext;
};

} // namespace state_machine
