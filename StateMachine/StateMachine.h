#pragma once

#include <functional>

namespace state_machine {

class Event;
class State;
class Context;

class StateMachine
{
public:
	StateMachine();
	~StateMachine();

	HRESULT start(Context* context, State* initialState, Event* userEvent = nullptr);
	HRESULT stop(Context* context);
	HRESULT handleEvent(Event* e);

protected:
	std::shared_ptr<State>* findState(std::shared_ptr<State>& currentState, State* pState);
	HRESULT for_each_state(std::shared_ptr<State>& currentState, std::function<HRESULT(std::shared_ptr<State>& state)> func);
	
	bool m_isHandlingState;

#pragma region Used by unit test.
	void setCurrentState(Context* context, State* currentState);
	State* getCurrentState(Context* context) const;
#pragma endregion
};

} // namespace state_machine
