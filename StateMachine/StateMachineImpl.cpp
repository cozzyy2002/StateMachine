#include "stdafx.h"
#include "StateMachineImpl.h"
#include "Event.h"
#include "State.h"
#include "Context.h"
#include "Handles.h"

static log4cplus::Logger logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("state_machine.StateMachine"));

using namespace state_machine;

/*static*/ StateMachine* StateMachine::createInstance()
{
	return new StateMachineImpl();
}

StateMachineImpl::StateMachineImpl()
{
}

StateMachineImpl::~StateMachineImpl()
{
}

HRESULT StateMachineImpl::handleEvent(Event* e)
{
	Context* context = e->getContext();
	ContextHandle* hContext = context->getHadle();

	if(logger.isEnabledFor(e->getLogLevel())) {
		// Suppress low level log output.
		LOG4CPLUS_INFO(logger, "Handling " << e->toString() << " in " << context->toString());
	}

	// Recursive call check.
	HR_ASSERT(!hContext->m_isEventHandling, E_ILLEGAL_METHOD_CALL);
	ScopedStore<bool> _recursive_guard(hContext->m_isEventHandling, false, true);

	// Lock this scope(If necessary)
	std::unique_ptr<std::lock_guard<std::mutex>> _lock(hContext->getStateLock());

	// Current state is contained by Context object in the Event.
	std::shared_ptr<State>& currentState(hContext->currentState);

	// Call State::handleEvent()
	// If event is ignored and the state is sub state, delegate handling event to master state.
	State* pNextState = nullptr;
	std::shared_ptr<State> nextState;
	bool backToMaster = false;
	HRESULT hr;
	for(State* pCurrentState = currentState.get();
		pCurrentState && !e->isHandled;
		pCurrentState = pCurrentState->getHadle()->m_masterState.get())
	{
		LOG4CPLUS_DEBUG(logger, "Calling " << pCurrentState->toString() << "::handleEvent()");
		hr = HR_EXPECT_OK(pCurrentState->handleEvent(e, currentState.get(), &pNextState));
		// Set Event::isHandled.
		// If state transition occurs, assume that the event is handled even if S_EVENT_IGNORED was returned.
		// Setting true by State::handleEvent() is prior to above condition.
		e->isHandled = e->isHandled ? e->isHandled : ((hr != S_EVENT_IGNORED) || pNextState);
		if(pNextState) {
			// Note: Object returned to pNextState might be deleted,
			//       if nextState goes out of scope before it is set as current state.
			std::shared_ptr<State>* nextMasterState = findState(currentState, pNextState);
			if(nextMasterState) {
				// Back to existing master state.
				nextState = *nextMasterState;
				backToMaster = true;
			} else {
				// Exit to newly created state.
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
		LOG4CPLUS_INFO(logger, "Next state is " << pNextState->toString() << (pNextState->isSubState() ? "(Sub state)" : ""));
		// State transition occurred.
		if(pNextState->isSubState() && !backToMaster) {
			// Transition from master state to sub state.
			// Don't call exit() of master state.
			pNextState->getHadle()->m_masterState = currentState;
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
	for(std::shared_ptr<State>* state(&currentState); state->get(); state = &(state->get()->getHadle()->m_masterState)) {
		hr = func(*state);
		if(hr != S_OK) return hr;
	}
	return hr;
}

#pragma region Used by unit test.
void StateMachineImpl::setCurrentState(Context* context, State* currentState)
{
	context->getHadle()->currentState.reset(currentState);
}

State* StateMachineImpl::getCurrentState(Context* context) const
{
	return context->getHadle()->currentState.get();
}

void StateMachineImpl::setMasterState(State * state, State * masterState)
{
	state->getHadle()->m_masterState.reset(masterState);
}

State * StateMachineImpl::getMasterState(State * state) const
{
	return state->getHadle()->m_masterState.get();
}
#pragma endregion
