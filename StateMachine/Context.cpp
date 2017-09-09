#include "stdafx_local.h"
#include <StateMachine/Event.h>
#include <StateMachine/State.h>
#include <StateMachine/Context.h>

#include "StateMachineImpl.h"
#include "Handles.h"

using namespace state_machine;

static auto logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("state_machine.Context"));

Context::Context(bool isAsync)
	: m_isAsync(isAsync)
{
	if(!isAsync) {
		m_hContext.reset(new ContextHandle());
	} else {
		m_hContext.reset(new AsyncContextHandle());
	}
}

Context::~Context()
{
	HR_EXPECT_OK(stop());
}

State* Context::getCurrentRawState() const
{
	return getHandle<ContextHandle>()->currentState.get();
}

#pragma region Context methods which invode ContextHandle/AsyncContextHandle methods.

HRESULT Context::start(State* initialState, Event& userEvent)
{
	return m_hContext->start(*this, initialState, userEvent);
}

HRESULT Context::start(State* initialState, Event* userEvent)
{
	return m_hContext->start(*this, initialState, userEvent);
}

HRESULT Context::start(State * initialState)
{
	return m_hContext->start(*this, initialState, Event());
}

HRESULT Context::stop()
{
	return m_hContext->stop(*this);
}

bool Context::isStarted() const
{
	return m_hContext->isStarted();
}

HRESULT Context::handleEvent(Event& e)
{
	return m_hContext->handleEvent(*this, e);
}

HRESULT Context::queueEvent(Event* e)
{
	return m_hContext->queueEvent(*this, e);
}

std::lock_guard<std::mutex>* Context::getStateLock()
{
	return 	m_hContext->getStateLock(*this);
}

bool Context::isEventHandling() const
{
	return m_hContext->isEventHandling();
}

StateMachine* Context::getStateMachine()
{
	return m_hContext->getStateMachine();
}

#pragma endregion
