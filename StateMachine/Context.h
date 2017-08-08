#pragma once

#include <memory>

namespace state_machine {

class State;
class StateMachine;

class Context
{
	friend class StateMachine;

public:
	Context();
	virtual ~Context();

private:	// Prevent members from being modified by derived class(User context).
	std::shared_ptr<State> currentState;
};

} // namespace state_machine
