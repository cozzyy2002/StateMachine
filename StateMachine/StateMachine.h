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

	virtual HRESULT start(Context* context, State* initialState, Event* userEvent = nullptr) = 0;
	virtual HRESULT stop(Context* context) = 0;
	virtual HRESULT handleEvent(Event* e) = 0;
};

} // namespace state_machine
