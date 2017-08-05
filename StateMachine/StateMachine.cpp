#include "stdafx.h"
#include "StateMachine.h"
#include "Event.h"
#include "State.h"

using namespace state_machine;

StateMachine::StateMachine()
{
}


StateMachine::~StateMachine()
{
}

HRESULT state_machine::StateMachine::handleEvent(const Event* e)
{
	// Call State::handleEvent()
	// If event is ignored and the state is sub state, delegate handling event to master state.
	State* currentState = m_currentState.get();
	State* nextState = nullptr;
	HRESULT hr;
	do {
		hr = HR_EXPECT_OK(currentState->handleEvent(e, currentState, &nextState));
		if(FAILED(hr)) {
			HR_ASSERT_OK(currentState->handleError(e, hr));
		}
		if(S_EVENT_IGNORED == hr) hr = currentState->handleIgnoredEvent(e);
		currentState = currentState->getMasterState().get();
	} while(currentState && (S_EVENT_IGNORED == hr));

	if(m_currentState->isSubState() && (nextState == State::RETURN_TO_MASTER)) {
		// If sub state returns RETURN_TO_MASTER constant,
		// next state is master state.
		nextState = m_currentState->getMasterState().get();
	}
	if(SUCCEEDED(hr) && nextState) {
		if(nextState->isSubState()) {
			// Master state to sub state.
			// Don't call exit() of master state.
			nextState->getMasterState() = m_currentState;
		} else {
			// Current state might be sub state or not.
			std::shared_ptr<State> masterState(m_currentState);
			do {
				std::shared_ptr<State> currentState(masterState);
				currentState->exit(e, nextState);
				masterState = currentState->getMasterState();
			} while(masterState && (masterState.get() != nextState));
		}
		std::shared_ptr<State> previousState(m_currentState);
		m_currentState.reset(nextState);
		m_currentState->entry(e, previousState.get());
	}
	return hr;
}
