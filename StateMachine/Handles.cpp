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

ContextHandle::ContextHandle(StateMachine& stateMachine)
	: stateMachine(dynamic_cast<StateMachineImpl*>(&stateMachine))
	, m_isEventHandling(false)
{
	// Check if dynamic_cast<>() worked.
	HR_EXPECT(this->stateMachine, E_ABORT);
}

ContextHandle::~ContextHandle()
{
}

HRESULT ContextHandle::start(Context& context, State* initialState, Event& userEvent)
{
	HR_ASSERT(!currentState, E_ILLEGAL_METHOD_CALL);

	currentState.reset(new RootState(initialState));
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

std::lock_guard<std::mutex>* ContextHandle::getStateLock(Context& context)
{
	return context.isStateLockEnabled() ? new std::lock_guard<std::mutex>(stateLock) : nullptr;
}

AsyncContextHandle::AsyncContextHandle(StateMachine& stateMachine)
	: ContextHandle(stateMachine)
{
}

HRESULT AsyncContextHandle::start(AsyncContext& context, State* initialState, Event* userEvent)
{
	return S_OK;
}

HRESULT AsyncContextHandle::stop(AsyncContext& context)
{
	return S_OK;
}

HRESULT AsyncContextHandle::handleEvent(AsyncContext& context, Event& e)
{
	return S_OK;
}

HRESULT AsyncContextHandle::queueEvent(AsyncContext& context, Event* e)
{
	return S_OK;
}
