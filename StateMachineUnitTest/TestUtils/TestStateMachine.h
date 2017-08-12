#pragma once

#include <StateMachine/StateMachineImpl.h>

using namespace state_machine;
using namespace testing;

class TestStateMachine : public StateMachineImpl
{
public:
	using StateMachineImpl::setCurrentState;
	using StateMachineImpl::getCurrentState;
	using StateMachineImpl::setMasterState;
	using StateMachineImpl::getMasterState;
};
