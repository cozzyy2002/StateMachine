#pragma once

namespace state_machine {

class Event;
class State;
class Context;

class StateMachine
{
public:
	static StateMachine* createInstance();
	virtual ~StateMachine() {};
};

} // namespace state_machine
