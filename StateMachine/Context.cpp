#include "stdafx_local.h"
#include <StateMachine/Event.h>
#include <StateMachine/State.h>
#include <StateMachine/Context.h>

#include "StateMachineImpl.h"
#include "Handles.h"

using namespace state_machine;

static auto logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("state_machine.Context"));

Context::Context(StateMachine& stateMachine)
	: m_hContext(new ContextHandle(stateMachine))
{
}

state_machine::Context::Context(ContextHandle * hContext)
{
}

Context::~Context()
{
}

State* Context::getCurrentRawState() const
{
	return getHandle()->currentState.get();
}

#pragma region Context methods which invode ContextHandle methods.

HRESULT Context::start(State* initialState, Event& userEvent)
{
	HR_ASSERT(initialState, E_POINTER);

	return m_hContext->start(*this, initialState, userEvent);
}

HRESULT Context::start(State * initialState)
{
	Event e;
	return start(initialState, e);
}

HRESULT Context::stop()
{
	return m_hContext->stop(*this);
}

HRESULT Context::handleEvent(Event& e)
{
	return m_hContext->handleEvent(*this, e);
}

std::lock_guard<std::mutex>* Context::getStateLock()
{
	return 	m_hContext->getStateLock(*this);
}

bool Context::isEventHandling() const
{
	return m_hContext->isEventHandling();
}

#pragma endregion

AsyncContext::AsyncContext(StateMachine& stateMachine)
	: Context(nullptr), m_hAsyncContext(new AsyncContextHandle(stateMachine))
{

}

#pragma region AsyncContext methods which invode AsyncContextHandle methods.

HRESULT AsyncContext::start(State* initialState, Event& userEvent)
{
	LOG4CPLUS_FATAL(logger, __FUNCTION__ "(Event&) is not emplemented.");
	return E_NOTIMPL;
}
HRESULT AsyncContext::start(State* initialState, Event* userEvent /*= nullptr*/)
{
	HR_ASSERT(initialState, E_POINTER);

	if(!userEvent) userEvent = new Event();
	return m_hAsyncContext->start(*this, initialState, userEvent);
}

HRESULT AsyncContext::stop()
{
	return m_hAsyncContext->stop(*this);
}

HRESULT AsyncContext::handleEvent(Event& e)
{
	return m_hAsyncContext->handleEvent(*this, e);
}

HRESULT AsyncContext::queueEvent(Event* e)
{
	return m_hAsyncContext->queueEvent(*this, e);
}

#pragma endregion
