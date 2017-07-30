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
	State* nextState = nullptr;
	HRESULT hr = m_currentState->handleEvent(e, m_currentState.get(), &nextState);
	if(SUCCEEDED(hr) && nextState) {
		m_currentState->exit(e, nextState);
		std::shared_ptr<State> previousState(m_currentState);
		m_currentState.reset(nextState);
		m_currentState->entry(e, previousState.get());
	}
	return S_OK;
}
