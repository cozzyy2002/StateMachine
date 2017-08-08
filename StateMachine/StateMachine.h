#pragma once

#include "Context.h"
#include "State.h"
#include "Event.h"

#include <functional>

namespace state_machine {

class StateMachine
{
public:
	StateMachine();
	~StateMachine();

	HRESULT handleEvent(Event* e);

protected:
	std::shared_ptr<State>* findState(std::shared_ptr<State>& currentState, State* pState);
	HRESULT for_each_state(std::shared_ptr<State>& currentState, std::function<HRESULT(std::shared_ptr<State>& state)> func);

#pragma region Used by unit test.
	void setCurrentState(Context* context, State* currentState) { context->currentState.reset(currentState); }
	State* getCurrentState(Context* context) const { return context->currentState.get(); }
#pragma endregion
};

} // namespace state_machine
