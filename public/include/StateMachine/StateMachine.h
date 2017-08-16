#pragma once

namespace state_machine {

class Context;
class State;

class StateMachine
{
public:
	static StateMachine* createInstance();
	virtual ~StateMachine() {};

	/*
		Returns State object in current state.
	*/
	virtual State* getCurrentState(Context* context) const = 0;
};

} // namespace state_machine
