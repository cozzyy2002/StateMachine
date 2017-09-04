#include "stdafx_local.h"
#include <StateMachine/Event.h>
#include <StateMachine/State.h>
#include <StateMachine/Context.h>

#include "StateMachineImpl.h"
#include "Handles.h"

using namespace state_machine;

static auto logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("state_machine.Handle"));

/*
Inner class to let state machine call userState->entry().
Used to initialize user context.
*/
class RootState : public State
{
public:
	RootState(State* userState) : userState(userState) {}

	// Returns user state as next state regardless of the event.
	// Then state machine calls entry() of user state and sets user state as current state.
	virtual HRESULT handleEvent(Event&, State&, State** nextState) override {
		*nextState = userState.release();
		return S_OK;
	}

protected:
	std::unique_ptr<State> userState;
};

ContextHandleBase::ContextHandleBase()
	: stateMachine(new StateMachineImpl())
	, m_isEventHandling(false)
{
}

ContextHandleBase::~ContextHandleBase()
{
}

std::lock_guard<std::mutex>* ContextHandleBase::getStateLock(Context& context)
{
	return context.isStateLockEnabled() ? new std::lock_guard<std::mutex>(stateLock) : nullptr;
}

ContextHandle::ContextHandle()
{
}

ContextHandle::~ContextHandle()
{
}

HRESULT ContextHandle::start(Context& context, State* initialState, Event& userEvent)
{
	// Avoid memory leak even if exit before handle event.
	std::unique_ptr<State> _state(initialState);

	HR_ASSERT(initialState, E_POINTER);
	HR_ASSERT(!currentState, E_ILLEGAL_METHOD_CALL);

	currentState.reset(new RootState(_state.release()));
	return handleEvent(context, userEvent);
}

HRESULT ContextHandle::stop(Context& /*context*/)
{
	currentState.reset();
	return S_OK;
}

HRESULT ContextHandle::handleEvent(Context& context, Event& e)
{
	e.m_context = &context;
	return stateMachine->handleEvent(e);
}

AsyncContextHandle::AsyncContextHandle()
	: ContextHandleBase()
	, isWorkerThreadRunning(false)
{
}

AsyncContextHandle::~AsyncContextHandle()
{
}

HRESULT AsyncContextHandle::start(AsyncContext& context, State* initialState, Event* userEvent)
{
	// Avoid memory leak even if exit before queue event.
	std::unique_ptr<State> _state(initialState);
	std::unique_ptr<Event> _e(userEvent);

	HR_ASSERT(initialState, E_POINTER);
	HR_ASSERT(!currentState, E_ILLEGAL_METHOD_CALL);

	// Start event handling in worker thread.
	hEventAvailable.Attach(CreateEvent(nullptr, TRUE, FALSE, nullptr));
	workerThread = std::thread([this]() { handleEvent(); });

	// Set initial state and queue event to be handled.
	currentState.reset(new RootState(_state.release()));
	return queueEvent(context, _e.release());
}

HRESULT AsyncContextHandle::stop(AsyncContext& context)
{
	// Queue NULL event to terminate worker thread.
	HR_ASSERT_OK(queueEvent(context, nullptr));
	if(workerThread.joinable()) {
		workerThread.join();
		LOG4CPLUS_INFO(logger, __FUNCTION__ ": Worker thread has terminated.");
	}
	return S_OK;
}

HRESULT AsyncContextHandle::handleEvent(AsyncContext& context, Event& e)
{
	e.m_context = &context;
	return stateMachine->handleEvent(e);
}

HRESULT AsyncContextHandle::queueEvent(AsyncContext& context, Event* e)
{
	if(e) e->m_context = &context;
	{
		std::lock_guard<std::mutex> _lock(eventQueueLock);
		eventQueue.push_back(std::unique_ptr<Event>(e));
	}
	WIN32_ASSERT(SetEvent(hEventAvailable));
	return S_OK;
}

void AsyncContextHandle::handleEvent()
{
	LOG4CPLUS_INFO(logger, __FUNCTION__ ": started.");

	// Set running flag and it will be reset when this method exits.
	ScopedStore<bool> _running(isWorkerThreadRunning, false, true);

	while(true) {
		auto wait = WaitForSingleObject(hEventAvailable, INFINITE);
		switch(wait) {
		case WAIT_OBJECT_0:
			ResetEvent(hEventAvailable);
			break;
		default:
			LOG4CPLUS_ERROR(logger, "Waiting event fails by " << wait << ", error=" << GetLastError());
			return;
		}

		std::unique_ptr<Event> e;
		{
			std::lock_guard<std::mutex> _lock(eventQueueLock);
			if(eventQueue.empty()) continue;
			e.reset(eventQueue.front().release());
			eventQueue.pop_front();
		}
		if(!e) {
			// stop() method sets NULL event.
			LOG4CPLUS_INFO(logger, __FUNCTION__ ": exit.");
			return;
		}

		HR_EXPECT_OK(stateMachine->handleEvent(*e));
	}
}
