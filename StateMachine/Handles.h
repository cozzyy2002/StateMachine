#pragma once

#include <StateMachine/Object.h>
#include <deque>
#include <memory>
#include <mutex>
#include <thread>

namespace state_machine {

class Event;
class State;
class Context;
class AsyncContext;
class StateMachine;
class StateMachineImpl;
class StateHandle;

class ContextHandle : public Object
{
public:
	ContextHandle(StateMachine& stateMachine);
	virtual ~ContextHandle();

	// Set initialState as current state and call initialState->entry().
	HRESULT start(Context& context, State* initialState, Event& userEvent);

	// Stops state machine.
	HRESULT stop(Context& context);

	HRESULT handleEvent(Context& context, Event& e);

	// See Context::getStateLock().
	std::lock_guard<std::mutex>* getStateLock(Context& context);

	bool isEventHandling() const { return m_isEventHandling; }

	std::shared_ptr<State> currentState;
	StateMachineImpl* const stateMachine;

	// true if the state machine is handling event in this context.
	// Used to recursive call check by state machine.
	bool m_isEventHandling;

	// Mutex used to lock StateMachine::handleEvent().
	std::mutex stateLock;
};

class AsyncContextHandle : public ContextHandle
{
public:
	AsyncContextHandle(StateMachine& stateMachine);

	HRESULT start(AsyncContext& context, State* initialState, Event* userEvent);
	HRESULT stop(AsyncContext& context);
	HRESULT handleEvent(AsyncContext& context, Event& e);
	HRESULT queueEvent(AsyncContext& context, Event* e);

	bool isStarted(const AsyncContext& context) const;

protected:
	// Event queue.
	// push_back() to queue new event.
	// pop_front() to retrieve the event to be handled.
	// If event is nullptr, worker thread will terminate.
	std::deque<std::unique_ptr<Event>> eventQueue;

	bool isWorkerThreadRunning;

	// Mutex used to lock event queue
	std::mutex eventQueueLock;

	// Win32 event handle to notify that one or more events are in the event queue.
	CHandle hEventAvailable;

	std::thread workerThread;
	void handleEvent();
};

class StateHandle : public Object
{
public:
	StateHandle() {}
	virtual ~StateHandle() {}

	// Always returns no master state.
	virtual State* getMasterState() const { return nullptr; }
};

class SubStateHandle : public StateHandle
{
public:
	SubStateHandle() {}
	virtual ~SubStateHandle() {}

	// See SubState::backToMaster().
	State* backToMaster();

	// Returns master state.
	virtual State* getMasterState() const override { return m_masterState.get(); }

	std::shared_ptr<State> m_masterState;
};

} // namespace state_machine
