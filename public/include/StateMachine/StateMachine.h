#pragma once

#include "Common.h"

namespace state_machine {

class State;

class StateMachine
{
protected:
	// Do not delete.
	~StateMachine() {}

public:
	virtual void setLoggerName(LPCTSTR loggerName) = 0;
	virtual const std::tstring& getLoggerName() const = 0;
};

} // namespace state_machine
