#pragma once

#include <StateMachine/Context.h>
#include <StateMachine/State.h>
#include <../StateMachine/Handles.h>
#include <../StateMachine/StateMachineImpl.h>

using namespace state_machine;
using namespace testing;

class TestStateMachine : public StateMachineImpl
{
public:
	TestStateMachine(Context& context) : StateMachineImpl(context) {}

	/*
		Set current state of context.

		If currentState is sub state, current state becomes master of next state.
		If next state is nullptr, current state is reset and the object is deleted.
	*/
	void setNextState(State* nextState) {
		auto& currentState = context.getHandle()->currentState;
		if(nextState && nextState->isSubState()) {
			ASSERT_NE(currentState.get(), nullptr) << "No current state to be master state in the context.";
			nextState->getHandle<SubStateHandle>()->m_masterState = currentState;
		}
		currentState.reset(nextState);
	}
	void clearCurrentState(Context& context) {
		setNextState(nullptr);
	}
};
