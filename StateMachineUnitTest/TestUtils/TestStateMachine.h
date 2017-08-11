#pragma once

#include <StateMachine/StateMachine.h>

using namespace state_machine;
using namespace testing;

class TestStateMachine : public StateMachine
{
public:
	using StateMachine::setCurrentState;
	using StateMachine::getCurrentState;
};
