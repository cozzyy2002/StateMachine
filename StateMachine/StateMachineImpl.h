#pragma once

#include "StateMachine.h"

#include <functional>

namespace state_machine {

class StateMachineImpl : public StateMachine
{
public:
	StateMachineImpl();
	virtual ~StateMachineImpl();

	HRESULT handleEvent(Event* e);

protected:
	std::shared_ptr<State>* findState(std::shared_ptr<State>& currentState, State* pState);
	HRESULT for_each_state(std::shared_ptr<State>& currentState, std::function<HRESULT(std::shared_ptr<State>& state)> func);
};

} // namespace state_machine
