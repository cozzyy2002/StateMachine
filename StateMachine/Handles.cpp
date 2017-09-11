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
	virtual HRESULT handleEvent(Context&, Event&, State&, State** nextState) override {
		*nextState = userState.release();
		return S_OK;
	}

protected:
	std::unique_ptr<State> userState;
};

std::lock_guard<std::mutex>* ContextHandle::getStateLock()
{
	return context.isStateLockEnabled() ? new std::lock_guard<std::mutex>(stateLock) : nullptr;
}

StateMachine* ContextHandle::getStateMachine()
{
	return stateMachine.get();
}

ContextHandle::ContextHandle(Context& context)
	: context(context)
	, stateMachine(new StateMachineImpl(context))
	, m_isEventHandling(false)
{
}

ContextHandle::~ContextHandle()
{
}

HRESULT ContextHandle::start(State* initialState, Event& userEvent)
{
	// Avoid memory leak even if exit before handle event.
	std::unique_ptr<State> _state(initialState);

	HR_ASSERT(initialState, E_POINTER);
	HR_ASSERT(userEvent.isLegalEvent(), E_INVALIDARG);
	HR_ASSERT(!currentState, E_ILLEGAL_METHOD_CALL);
	HR_ASSERT(!isStarted(), E_ILLEGAL_METHOD_CALL);

	currentState.reset(new RootState(_state.release()));
	return handleEvent(userEvent);
}

HRESULT ContextHandle::start(State* initialState, Event* userEvent)
{
	delete initialState;
	delete userEvent;
	LOG4CPLUS_FATAL(logger, __FUNCTION__ "(Event*) is not implemented.");
	return E_NOTIMPL;
}

HRESULT ContextHandle::start(State* initialState)
{
	Event e;
	return this->start(initialState, e);
}

HRESULT ContextHandle::stop()
{
	// Note: stop() can be called even if stopped.
	currentState.reset();
	return S_OK;
}

HRESULT ContextHandle::handleEvent(Event& e)
{
	HR_ASSERT(e.isLegalEvent(), E_INVALIDARG);
	HR_ASSERT(isStarted(), E_ILLEGAL_METHOD_CALL);

	return stateMachine->handleEvent(e);
}

HRESULT ContextHandle::queueEvent(Event* e)
{
	delete e;
	LOG4CPLUS_FATAL(logger, __FUNCTION__ "() is not implemented.");
	return E_NOTIMPL;
}

static HANDLE createWin32Event()
{
	return CreateEvent(nullptr, TRUE, FALSE, nullptr);
}

AsyncContextHandle::AsyncContextHandle(Context& context)
	: ContextHandle(context)
	, isWorkerThreadRunning(false)
	, hWorkerThreadStarted(createWin32Event())
	, hWorkerThreadTerminated(createWin32Event())
	, hEventAvailable(createWin32Event())
{
}

AsyncContextHandle::~AsyncContextHandle()
{
}

HRESULT AsyncContextHandle::start(State* initialState, Event& userEvent)
{
	delete initialState;
	LOG4CPLUS_FATAL(logger, __FUNCTION__ "(Event&) is not implemented.");
	return E_NOTIMPL;
}

HRESULT AsyncContextHandle::start(State* initialState, Event* userEvent)
{
	// Avoid memory leak even if exit before queue event.
	std::unique_ptr<State> _state(initialState);
	std::unique_ptr<Event> _e(userEvent);

	HR_ASSERT(initialState, E_POINTER);
	HR_ASSERT(!isStarted(), E_ILLEGAL_METHOD_CALL);

	HR_ASSERT(userEvent, E_POINTER);
	HR_ASSERT(userEvent->isLegalEvent(), E_INVALIDARG);
	HR_ASSERT(!currentState, E_ILLEGAL_METHOD_CALL);

	// Start event handling in worker thread.
	HR_ASSERT_OK(context.onStartThread(workerThreadProc, hWorkerThreadStarted));

	// Set initial state and queue event to be handled.
	currentState.reset(new RootState(_state.release()));
	return queueEvent(_e.release());
}

HRESULT AsyncContextHandle::start(State* initialState)
{
	return this->start(initialState, new Event());
}

HRESULT AsyncContextHandle::stop()
{
	// Note: stop() can be called even if stopped.
	if(isStarted()) {
		// Queue event with Shutdown priority to terminate worker thread.
		auto e(new Event(Event::Priority::StopContext));
		e->isInternal = true;
		HR_ASSERT_OK(queueEvent(e));
		HR_ASSERT_OK(context.onStopThread(hWorkerThreadTerminated));

		ContextHandle::stop();
	}

	return S_OK;
}

HRESULT AsyncContextHandle::handleEvent(Event& e)
{
	HR_ASSERT(e.isLegalEvent(), E_INVALIDARG);
	HR_ASSERT(isStarted(), E_ILLEGAL_METHOD_CALL);

	return stateMachine->handleEvent(e);
}

HRESULT AsyncContextHandle::queueEvent(Event* e)
{
	// Avoid memory leak even if exit before event is queued.
	std::unique_ptr<Event> _e(e);
	HR_ASSERT(e, E_POINTER);
	HR_ASSERT(e->isLegalEvent(), E_INVALIDARG);

	// When start() calls this method, worker thread is not running yet.
	// So we check only ContextHandle::isStarted() not this->isStarted().
	HR_ASSERT(ContextHandle::isStarted(), E_ILLEGAL_METHOD_CALL);

	{
		std::lock_guard<std::mutex> _lock(eventQueueLock);

#ifdef USE_SORT
		// Queue event and sort by priority order.
		eventQueue.push_back(std::move(_e));
		eventQueue.sort(Event::HigherPriority());
#else
		// Queue event by priority order
		auto it(eventQueue.begin());
		while(it != eventQueue.end()) {
			if((*it)->getPriority() < e->getPriority()) break;
			it++;
		}
		eventQueue.insert(it, std::move(_e));
#endif
	}
	WIN32_ASSERT(SetEvent(hEventAvailable));
	return S_OK;
}

void AsyncContextHandle::handleEvent()
{
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

		while(!eventQueue.empty()) {
			std::unique_ptr<Event> e;
			{
				std::lock_guard<std::mutex> _lock(eventQueueLock);
				if(eventQueue.empty()) break;
				e.reset(eventQueue.front().release());
				eventQueue.pop_front();
			}
			if(e->priority == Event::Priority::StopContext) {
				return;
			}

			HR_EXPECT_OK(stateMachine->handleEvent(*e));
		}
	}
}

// Worker thread procedure.
// Called by Context::onStartThread() as WorkerThreadProc.
/*static*/ void AsyncContextHandle::workerThreadProc(Context & context)
{
	auto h = context.getHandle<AsyncContextHandle>();
	if(!h) {
		LOG4CPLUS_FATAL(logger, __FUNCTION__ ": Illegal Context pointer.");
		return;
	}
	LOG4CPLUS_INFO(logger, __FUNCTION__ ": Starting worker thread.");

	// Setup method to be called when worker thread is terminated.
	using _Ty = AsyncContextHandle;
	std::unique_ptr<_Ty, void(*)(_Ty*)> _deleter(h, onWorkerThreadTerminated);

	h->isWorkerThreadRunning = true;
	WIN32_EXPECT(SetEvent(h->hWorkerThreadStarted));
	h->handleEvent();
	LOG4CPLUS_INFO(logger, __FUNCTION__ ": Terminating worker thread. Queued events=" << h->eventQueue.size());
}

/*static*/ void AsyncContextHandle::onWorkerThreadTerminated(AsyncContextHandle* h)
{
	h->isWorkerThreadRunning = false;
	WIN32_EXPECT(SetEvent(h->hWorkerThreadTerminated));
}
