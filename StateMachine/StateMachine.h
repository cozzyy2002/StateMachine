#pragma once

#include <memory>

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
	std::shared_ptr<State> m_currentState;
};

} // namespace state_machine
