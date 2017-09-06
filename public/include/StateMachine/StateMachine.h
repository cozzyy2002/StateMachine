#pragma once

#include "Common.h"

namespace state_machine {

static LPCTSTR defaultLogConigFileName(_T("log4cplus.properties"));
static LPCTSTR globalLoggerName = _T("state_machine.global");
static LPCTSTR stateMachineDefaultLoggerName = _T("state_machine.StateMachine");

class State;

class StateMachine
{
public:
	virtual void setLoggerName(LPCTSTR loggerName) = 0;
	virtual const std::tstring& getLoggerName() const = 0;
};

} // namespace state_machine
