#pragma once

namespace state_machine {

class Context;
class State;

class StateMachine
{
public:
	static StateMachine* createInstance();
	virtual ~StateMachine() {};
};

} // namespace state_machine
