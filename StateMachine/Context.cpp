#include "stdafx.h"
#include "Context.h"
#include "StateMachine.h"

using namespace state_machine;

Context::Context(StateMachine* stateMachine)
	: stateMachine(stateMachine)
{
}

Context::~Context()
{
}

HRESULT state_machine::Context::start(State * initialState)
{
	return stateMachine->start(this, initialState);
}

HRESULT state_machine::Context::stop()
{
	return stateMachine->stop(this);
}

HRESULT state_machine::Context::handleEvent(Event * e)
{
	return stateMachine->handleEvent(e);
}

std::lock_guard<std::mutex>* Context::geStatetLock()
{
	return isStateLockEnabled() ? new std::lock_guard<std::mutex>(eventQueueLock) : nullptr;
}
