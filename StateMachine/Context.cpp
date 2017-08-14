#include "stdafx.h"
#include "Context.h"
#include "Event.h"
#include "StateMachine.h"
#include "Handles.h"

using namespace state_machine;

ContextHandle::ContextHandle(StateMachine* stateMachine)
	: stateMachine(stateMachine)
	, m_isEventHandling(false)
{
}

ContextHandle::~ContextHandle()
{
}

Context::Context(StateMachine* stateMachine)
	: m_hContext(new ContextHandle(stateMachine))
{
}

Context::~Context()
{
	delete m_hContext;
}

HRESULT ContextHandle::start(Context* context, State * initialState, Event* userEvent)
{
	Event* e = userEvent ? userEvent : new Event();
	return stateMachine->start(context, initialState, e);
}

HRESULT ContextHandle::stop(Context* context)
{
	return stateMachine->stop(context);
}

HRESULT ContextHandle::handleEvent(Context* context, Event * e)
{
	e->m_context = context;
	return stateMachine->handleEvent(e);
}

std::lock_guard<std::mutex>* ContextHandle::getStateLock()
{
	return isStateLockEnabled() ? new std::lock_guard<std::mutex>(stateLock) : nullptr;
}

#pragma region Context methods which invode ContextHandle methods.

HRESULT Context::start(State * initialState, Event* userEvent /*= nullptr*/)
{
	return m_hContext->start(this, initialState, userEvent);
}

HRESULT Context::stop()
{
	return m_hContext->stop(this);
}

HRESULT Context::handleEvent(Event * e)
{
	return m_hContext->handleEvent(this, e);
}

std::lock_guard<std::mutex>* Context::getStateLock()
{
	return 	m_hContext->getStateLock();
}

bool Context::isEventHandling() const
{
	return m_hContext->isEventHandling();
}

#pragma endregion
