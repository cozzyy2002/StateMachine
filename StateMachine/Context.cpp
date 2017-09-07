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

state_machine::Context::Context(bool isAsync, ContextHandleBase* hContext)
	: m_isAsync(isAsync), m_hContext(hContext)
{
}

Context::~Context()
{
}

State* Context::getCurrentRawState() const
{
	return getHandle<ContextHandle>()->currentState.get();
}

#pragma region Context methods which invode ContextHandle/AsyncContextHandle methods.

HRESULT Context::start(State* initialState, Event& userEvent)
{
	if(!isAsync()) {
		return getHandle<ContextHandle>()->start(*this, initialState, userEvent);
	} else {
		delete initialState;
		LOG4CPLUS_FATAL(logger, __FUNCTION__ "(Event&) is not implemented.");
		return E_NOTIMPL;
	}
}

HRESULT Context::start(State* initialState, Event* userEvent)
{
	if(!isAsync()) {
		delete initialState;
		delete userEvent;

		LOG4CPLUS_FATAL(logger, __FUNCTION__ "(Event*) is not implemented.");
		return E_NOTIMPL;
	} else {
		return getHandle<AsyncContextHandle>()->start(*this, initialState, userEvent);
	}
}

HRESULT Context::start(State * initialState)
{
	if(!isAsync()) {
		return getHandle<ContextHandle>()->start(*this, initialState, Event());
	} else {
		return getHandle<AsyncContextHandle>()->start(*this, initialState, new Event());
	}
}

HRESULT Context::stop()
{
	if(!isAsync()) {
		return getHandle<ContextHandle>()->stop(*this);
	} else {
		return getHandle<AsyncContextHandle>()->stop(*this);
	}
}

bool Context::isStarted() const
{
	return m_hContext->isStarted();
}

HRESULT Context::handleEvent(Event& e)
{
	if(!isAsync()) {
		return getHandle<ContextHandle>()->handleEvent(*this, e);
	} else {
		LOG4CPLUS_FATAL(logger, __FUNCTION__ "(Event&) is not implemented.");
		return E_NOTIMPL;
	}
}

HRESULT Context::queueEvent(Event* e)
{
	if(!isAsync()) {
		delete e;
		LOG4CPLUS_FATAL(logger, __FUNCTION__ "(Event*) is not implemented.");
		return E_NOTIMPL;
	} else {
		return getHandle<AsyncContextHandle>()->queueEvent(*this, e);
	}
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
