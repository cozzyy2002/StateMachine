#include <StateMachine/stdafx.h>
#include <StateMachine/Event.h>
#include <StateMachine/State.h>
#include <StateMachine/Context.h>

#include "StateMachineImpl.h"
#include "Handles.h"

static log4cplus::Logger logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("state_machine.StateMachine"));

using namespace state_machine;

/*static*/ StateMachine* StateMachine::createInstance()
{
	LOG4CPLUS_DEBUG(logger, __FUNCTION__ ": Creating instance");
	return new StateMachineImpl();
}

StateMachineImpl::StateMachineImpl()
{
}

StateMachineImpl::~StateMachineImpl()
{
	LOG4CPLUS_DEBUG(logger, __FUNCTION__ ": Deleting instance");
}

State* StateMachineImpl::getCurrentState(Context * context) const
{
	return context->getHandle()->currentState.get();
}

HRESULT StateMachineImpl::handleEvent(Event* e)
{
	Context* context = e->getContext();
	ContextHandle* hContext = context->getHandle();

	if(logger.isEnabledFor(e->getLogLevel())) {
		// Suppress low level log output.
		std::tstringstream stream;
		stream << "Handling " << e->toString() << " in " << context->toString();
		logger.forcedLog(e->getLogLevel(), stream.str());
	}

	// Recursive call check.
	HR_ASSERT(!hContext->isEventHandling(), E_ILLEGAL_METHOD_CALL);
	ScopedStore<bool> _recursive_guard(hContext->m_isEventHandling, false, true);

	// Lock this scope(If necessary)
	std::unique_ptr<std::lock_guard<std::mutex>> _lock(context->getStateLock());

	// Current state is contained by Context object in the Event.
	std::shared_ptr<State>& currentState(hContext->currentState);

	State* pNextState = nullptr;
	std::shared_ptr<State> nextState;
	bool backToMaster = false;
	HRESULT hr = State::S_EVENT_IGNORED;
	e->isHandled = false;

	// Call State::handleEvent()
	// If event is ignored and the state is sub state, delegate handling event to master state.
	for(State* pCurrentState = currentState.get();
		pCurrentState && !e->isHandled;
		pCurrentState = pCurrentState->getHandle()->getMasterState())
	{
		LOG4CPLUS_DEBUG(logger, "Calling " << pCurrentState->toString() << "::handleEvent()");
		hr = HR_EXPECT_OK(pCurrentState->handleEvent(e, currentState.get(), &pNextState));
		// Set Event::isHandled.
		// If state transition occurs, assume that the event is handled even if S_EVENT_IGNORED was returned.
		// Setting true by State::handleEvent() is prior to above condition.
		if(!e->isHandled) e->isHandled = ((hr != State::S_EVENT_IGNORED) || pNextState);
		if(pNextState) {
			std::shared_ptr<State>* nextMasterState = findState(currentState, pNextState);
			if(nextMasterState) {
				// Back to existing master state.
				HR_ASSERT(pCurrentState->isSubState(), E_UNEXPECTED);
				nextState = *nextMasterState;
				backToMaster = true;
			} else {
				// Exit to newly created state.
				// Note: Object returned to pNextState might be deleted,
				//       if nextState goes out of scope before it is set as current state.
				nextState.reset(pNextState);
			}
		}
		if(FAILED(hr)) {
			LOG4CPLUS_DEBUG(logger, "Calling " << pCurrentState->toString() << "::handleError()");
			HR_ASSERT_OK(pCurrentState->handleError(e, hr));
		}
		if(!e->isHandled) {
			LOG4CPLUS_DEBUG(logger, "Calling " << pCurrentState->toString() << "::handleIgnoredEvent()");
			HR_ASSERT_OK(pCurrentState->handleIgnoredEvent(e));
		}
	}

	if(SUCCEEDED(hr) && pNextState) {
		LOG4CPLUS_INFO(logger, "Next state is " << pNextState->toString());
		// State transition occurred.
		if(pNextState->isSubState() && !backToMaster) {
			// Transition from master state to sub state.
			// Don't call exit() of master state.
			SubStateHandle* hSubState = pNextState->getHandle<SubStateHandle>();
			HR_ASSERT(hSubState, E_UNEXPECTED);
			hSubState->m_masterState = currentState;
		} else {
			// Transition to other state or master state of current state.
			// Call exit() of current state and master state if any.
			HR_ASSERT_OK(for_each_state(currentState, [e, pNextState](std::shared_ptr<State>& state)
			{
				if(state.get() != pNextState) {
					LOG4CPLUS_DEBUG(logger, "Calling " << state->toString() << "::exit()");
					HR_ASSERT_OK(state->exit(e, pNextState));
					return S_OK;
				} else {
					// If the state is next state, don't call it's exit().
					return S_FALSE;
				}
			}));
		}
		// Preserve current state as previous state until calling entry() of next stete.
		// Note: Current state and it's master state(which is not next state) will be deleted when current state is updated.
		std::shared_ptr<State> previousState(currentState);
		currentState = nextState;
		if(!backToMaster) {
			LOG4CPLUS_DEBUG(logger, "Calling " << currentState->toString() << "::entry()");
			HR_ASSERT_OK(currentState->entry(e, previousState.get()));
		}
	}

	LOG4CPLUS_DEBUG(logger, e->toString()
							<< " is " << (e->isHandled ? "handled" : "ignored")
							<< ". HRESULT=0x" << std::hex << hr);

	return hr;
}

std::shared_ptr<State>* StateMachineImpl::findState(std::shared_ptr<State>& currentState, State* pState)
{
	std::shared_ptr<State>* ret = nullptr;
	for_each_state(currentState, [this, pState, &ret](std::shared_ptr<State>& state)
	{
		if(pState == state.get()) {
			ret = &state;
			return S_FALSE;
		}
		return S_OK;
	});
	return ret;
}

HRESULT StateMachineImpl::for_each_state(std::shared_ptr<State>& currentState, std::function<HRESULT(std::shared_ptr<State>& state)> func)
{
	HRESULT hr;
	if(currentState->isSubState()) {
		for(std::shared_ptr<State>* state(&currentState);
			state && state->get();
			state = (*state)->isSubState() ? &((*state)->getHandle<SubStateHandle>()->m_masterState) : nullptr) {
			hr = func(*state);
			if(hr != S_OK) return hr;
		}
	} else {
		hr = func(currentState);
	}
	return hr;
}
