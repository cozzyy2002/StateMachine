#pragma once

#include "Common.h"

namespace state_machine {

class Context;
class State;

class StateMachine
{
public:
	static void configureLog(LPCTSTR configFileName);
	static StateMachine* createInstance();
	virtual ~StateMachine() {};

protected:
	static std::tstring logConfigFileName;
};

} // namespace state_machine
