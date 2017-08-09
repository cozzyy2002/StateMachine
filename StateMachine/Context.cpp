#include "stdafx.h"
#include "Context.h"
#include "Event.h"
#include "StateMachine.h"

using namespace state_machine;

Context::Context(StateMachine* stateMachine)
	: stateMachine(stateMachine)
{
}

Context::~Context()
{
}

HRESULT Context::start(State * initialState, Event* userEvent /*= nullptr*/)
{
	return stateMachine->start(this, initialState, userEvent);
}

HRESULT Context::stop()
{
	return stateMachine->stop(this);
}

HRESULT Context::handleEvent(Event * e)
{
	e->context = this;
	return stateMachine->handleEvent(e);
}

std::lock_guard<std::mutex>* Context::geStatetLock()
{
	return isStateLockEnabled() ? new std::lock_guard<std::mutex>(eventQueueLock) : nullptr;
}
