#pragma once

#include "StateMachine.h"

#include <functional>

namespace state_machine {

class StateMachineImpl : public StateMachine
{
public:
	StateMachineImpl();
	virtual ~StateMachineImpl();

	virtual HRESULT start(Context* context, State* initialState, Event* userEvent = nullptr) override;
	virtual HRESULT stop(Context* context) override;
	virtual HRESULT handleEvent(Event* e) override;

protected:
	std::shared_ptr<State>* findState(std::shared_ptr<State>& currentState, State* pState);
	HRESULT for_each_state(std::shared_ptr<State>& currentState, std::function<HRESULT(std::shared_ptr<State>& state)> func);
	
#pragma region Used by unit test.
	void setCurrentState(Context* context, State* currentState);
	State* getCurrentState(Context* context) const;
	void setMasterState(State* state, State* masterState);
	State* getMasterState(State* state) const;
#pragma endregion
};

} // namespace state_machine
