#pragma once

#include "Object.h"
#include "Event.h"

#include <mutex>
#include <memory>
#include <thread>

namespace state_machine {

class State;
class ContextHandle;
class StateMachine;

class Context : public Object
{
protected:
	Context(bool isAsync);

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

	// Type of worker thread procedure.
	// Worker thread is the thread in which event passed to Context::queueEvent() is handled.
	// onStartThread() mehtod calls this method in worker thread.
	// This method never return until Context::stop() is called.
	using WorkerThreadProc = void(*)(Context& contex);

	// Creates worker thread and calls WorkerThreadProc in the thread.
	// This method is called by Context::start() method.
	// If Context is created by constructor Context(isAsync = false), this method is not called.
	virtual HRESULT onStartThread(WorkerThreadProc proc) {
		workerThread = std::thread([this, proc]() { proc(*this); });
		return S_OK;
	}

	// Waits for worker thread to exit.
	// This method is called by Context::stop() method.
	// In case derived class overrides onStartThread(), the class should override this mehtod also.
	// If Context is created by constructor Context(isAsync = false), this method is not called.
	virtual HRESULT onStopThread() {
		if(workerThread.joinable()) {
			workerThread.join();
		}
		return S_OK;
	}

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
	template<class T = ContextHandle>
	T* getHandle() const { return dynamic_cast<T*>(m_hContext.get()); }

private:
	// Do NOT delete returned object.
	State* getCurrentRawState() const;

	bool m_isAsync;
	std::unique_ptr<ContextHandle> m_hContext;

	std::thread workerThread;
};

} // namespace state_machine
