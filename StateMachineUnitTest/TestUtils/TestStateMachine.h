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
	/*
		Set current state of context.

		If currentState is sub state, 
	*/
	void setNextState(Context* context, State* nextState) {
		std::shared_ptr<State>& currentState = context->getHandle()->currentState;
		if(nextState && nextState->isSubState()) {
			ASSERT_NE(currentState.get(), nullptr) << "No current state to be master state in the context.";
			nextState->getHandle<SubStateHandle>()->m_masterState = currentState;
		}
		currentState.reset(nextState);
	}
	void clearCurrentState(Context* context) {
		setNextState(context, nullptr);
	}

	State* getCurrentState(Context* context) const {
		return context->getHandle()->currentState.get();
	}
};
