#pragma once

namespace state_machine {

class Event;

class State
{
public:
	State();
	virtual ~State();

	virtual HRESULT handleEvent(const Event* e, const State* currentState, State** nextState) = 0;
	virtual HRESULT entry(const Event* e, const State* previousState) = 0;
	virtual HRESULT exit(const Event* e, const State* nextState) = 0;

	virtual LPCTSTR toString() = 0;
};

} // namespace state_machine
