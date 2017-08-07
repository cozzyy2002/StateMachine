#pragma once

#include <memory>
#include <functional>

namespace state_machine {

class Event;
class State;

class StateMachine
{
public:
	StateMachine();
	~StateMachine();

	HRESULT handleEvent(const Event* e);

protected:
	std::shared_ptr<State>* findState(State* pState);
	HRESULT for_each_state(std::function<HRESULT(std::shared_ptr<State>& state)> func);

	std::shared_ptr<State> m_currentState;
};

} // namespace state_machine
