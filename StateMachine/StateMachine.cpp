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
	State* pCurrentState = m_currentState.get();
	State* pNextState = nullptr;
	std::shared_ptr<State> nextState;
	HRESULT hr;
	do {
		hr = HR_EXPECT_OK(pCurrentState->handleEvent(e, pCurrentState, &pNextState));
		nextState.reset(pNextState);
		if(FAILED(hr)) {
			HR_ASSERT_OK(pCurrentState->handleError(e, hr));
		}
		if(S_EVENT_IGNORED == hr) hr = HR_EXPECT_OK(pCurrentState->handleIgnoredEvent(e));
		pCurrentState = pCurrentState->masterState().get();
	} while(pCurrentState && (S_EVENT_IGNORED == hr));

	if(SUCCEEDED(hr) && pNextState) {
		// State transition occurred.
		if(pNextState->isSubState() && !pNextState->isEntryCalled()) {
			// Transition from master state to sub state.
			// Don't call exit() of master state.
			pNextState->masterState() = m_currentState;
		} else {
			// Transition to other state or master state of current state.
			// Call exit() of current state and master state if any.
			for(State* pCurrentState = m_currentState.get();
				pCurrentState && (pCurrentState != pNextState);
				pCurrentState = pCurrentState->masterState().get())
			{
				pCurrentState->setEntryCalled(false);
				HR_ASSERT_OK(pCurrentState->exit(e, pNextState));
			}
		}
		// Preserve current state as previous state until calling entry() of next stete.
		// Note: Current state and it's master state(which is not next state) will be deleted on out of scope.
		std::shared_ptr<State> previousState(m_currentState);
		m_currentState = nextState;
		if(!m_currentState->isEntryCalled()) {
			m_currentState->setEntryCalled(true);
			HR_ASSERT_OK(m_currentState->entry(e, previousState.get()));
		}
	}
	return hr;
}
