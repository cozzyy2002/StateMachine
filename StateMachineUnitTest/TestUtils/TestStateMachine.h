#pragma once

#include <StateMachine/StateMachineImpl.h>
#include <StateMachine/Context.h>
#include <StateMachine/State.h>
#include <StateMachine/Handles.h>

using namespace state_machine;
using namespace testing;

class TestStateMachine : public StateMachineImpl
{
public:
	void setCurrentState(Context* context, State* currentState) {
		context->getHandle()->currentState.reset(currentState);
	}
	State* getCurrentState(Context* context) const {
		return context->getHandle()->currentState.get();
	}
	void setMasterState(State* state, State* masterState) {
		state->getHandle()->m_masterState.reset(masterState);
	}
	State* getMasterState(State* state) const {
		return state->getHandle()->m_masterState.get();
	}
};
