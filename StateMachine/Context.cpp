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
		m_hContext.reset(new ContextHandle(*this));
	} else {
		m_hContext.reset(new AsyncContextHandle(*this));
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
	return m_hContext->start(initialState, userEvent);
}

HRESULT Context::start(State* initialState, Event* userEvent)
{
	return m_hContext->start(initialState, userEvent);
}

HRESULT Context::start(State * initialState)
{
	if(!isAsync()) {
		return getHandle<ContextHandle>()->start(initialState);
	} else {
		return getHandle<AsyncContextHandle>()->start(initialState);
	}
}

HRESULT Context::stop()
{
	return m_hContext->stop();
}

bool Context::isStarted() const
{
	return m_hContext->isStarted();
}

HRESULT Context::handleEvent(Event& e)
{
	return m_hContext->handleEvent(e);
}

HRESULT Context::queueEvent(Event* e)
{
	return m_hContext->queueEvent(e);
}

HRESULT Context::waitForEvent(HANDLE hEvent, DWORD timeout)
{
	auto wait(WaitForSingleObject(hEvent, timeout));
	switch(wait) {
	case WAIT_OBJECT_0:	return S_OK;
	case WAIT_TIMEOUT:	return HRESULT_FROM_WIN32(WAIT_TIMEOUT);
	default:			return HRESULT_FROM_WIN32(GetLastError());
	}
}

HANDLE Context::getWorkerThreadStartEvent() const
{
	if(!isAsync()) {
		LOG4CPLUS_FATAL(logger, __FUNCTION__ "() is not implemented.");
		return NULL;
	}
	return getHandle<AsyncContextHandle>()->hWorkerThreadStarted;
}

HANDLE Context::getWorkerThreadTerminateEvent() const
{
	if(!isAsync()) {
		LOG4CPLUS_FATAL(logger, __FUNCTION__ "() is not implemented.");
		return NULL;
	}
	return getHandle<AsyncContextHandle>()->hWorkerThreadTerminated;
}

std::lock_guard<std::mutex>* Context::getStateLock()
{
	return 	m_hContext->getStateLock();
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
