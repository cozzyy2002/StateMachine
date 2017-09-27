#pragma once

#include <StateMachine/Object.h>
#include <StateMachine/State.h>
#include <list>
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
	ContextHandle(Context& context);
	virtual ~ContextHandle();

	// Set initialState as current state and call initialState->entry().
	virtual HRESULT start(State* initialState, Event& userEvent);
	virtual HRESULT start(State* initialState, Event* userEvent);
	virtual HRESULT start(State* initialState);

	// Stops state machine.
	virtual HRESULT stop();

	virtual HRESULT handleEvent(Event& e);
	virtual HRESULT queueEvent(Event* e);

	// See Context::getStateLock().
	std::lock_guard<std::mutex>* getStateLock();

	virtual bool isStarted() const { return currentState ? true : false; }
	virtual bool isEventHandling() const { return m_isEventHandling; }

	StateMachine* getStateMachine();

	Context& context;
	std::unique_ptr<State> currentState;
	std::unique_ptr<StateMachineImpl> stateMachine;

	// true if the state machine is handling event in this context.
	// Used to recursive call check by state machine.
	bool m_isEventHandling;

	// Mutex used to lock StateMachine::handleEvent().
	std::mutex stateLock;
};

class AsyncContextHandle : public ContextHandle
{
public:
	AsyncContextHandle(Context& context);
	virtual ~AsyncContextHandle();

	virtual HRESULT start(State* initialState, Event& userEvent) override;
	virtual HRESULT start(State* initialState, Event* userEvent) override;
	virtual HRESULT start(State* initialState) override;
	virtual HRESULT stop() override;
	virtual HRESULT handleEvent(Event& e) override;
	virtual HRESULT queueEvent(Event* e) override;

	virtual bool isStarted() const override { return isWorkerThreadRunning && ContextHandle::isStarted(); }

	// Win32 event handle to notify that worker thread has started.
	CHandle hWorkerThreadStarted;
	// Win32 event handle to notify that worker thread has terminated.
	CHandle hWorkerThreadTerminated;

protected:
	// Event queue.
	// push_back() to queue new event.
	// pop_front() to retrieve the event to be handled.
	// Queueing event with priority other than Normal causes sorting this queue.
	std::list<std::unique_ptr<Event>> eventQueue;

	bool isWorkerThreadRunning;

	// Mutex used to lock event queue
	std::mutex eventQueueLock;

	// Win32 event handle to notify that one or more events are in the event queue.
	CHandle hEventAvailable;

	void handleEvent();

	static void workerThreadProc(Context& context);
	static void onWorkerThreadTerminated(AsyncContextHandle* h);
};

class StateHandle : public Object
{
public:
	StateHandle() {}
	virtual ~StateHandle() {}

	virtual bool isSubState() const { return false; }

	// Returns nullptr, and log fatal message.
	virtual State* backToMaster() const;

	// Always returns no master state.
	virtual State* getRawMasterState() const { return nullptr; }
};

class SubStateHandle : public StateHandle
{
public:
	SubStateHandle(State* masterState) : masterState(masterState) {}
	virtual ~SubStateHandle() {}

	virtual bool isSubState() const override { return true; }

	// See SubState::backToMaster().
	virtual State* backToMaster() const override;

	// Returns master state.
	virtual State* getRawMasterState() const override { return masterState.get(); }

protected:
	friend class StateMachineImpl;
	std::unique_ptr<State> masterState;
};

} // namespace state_machine
