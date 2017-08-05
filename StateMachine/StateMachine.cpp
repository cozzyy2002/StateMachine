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
		if(S_EVENT_IGNORED == hr) hr = HR_EXPECT_OK(currentState->handleIgnoredEvent(e));
		currentState = currentState->getMasterState();
	} while(currentState && (S_EVENT_IGNORED == hr));

	if(m_currentState->isSubState() && (nextState == State::RETURN_TO_MASTER)) {
		// If sub state returns RETURN_TO_MASTER constant as next state,
		// next state is it's master state.
		nextState = m_currentState->getMasterState();
	}
	if(SUCCEEDED(hr) && nextState) {
		// State transition occurred.
		if(nextState->isSubState()) {
			// Transition from master state to sub state.
			// Don't call exit() of master state.
			nextState->setMasterState(m_currentState);
		} else {
			// Transition to other state or master state of current state.
			// Call exit() of current state and master state if any.
			for(State* currentState = m_currentState.get();
				currentState && (currentState != nextState);
				currentState = currentState->getMasterState())
			{
				currentState->setEntryCalled(false);
				HR_ASSERT_OK(currentState->exit(e, nextState));
			}
		}
		// Preserve current state as previous state until calling entry() of next stete.
		// Note: Current state and it's master state(which is not next state) will be deleted on out of scope.
		std::shared_ptr<State> previousState(m_currentState);
		m_currentState.reset(nextState);
		if(!m_currentState->isEntryCalled()) {
			m_currentState->setEntryCalled(true);
			HR_ASSERT_OK(m_currentState->entry(e, previousState.get()));
		}
	}
	return hr;
}
