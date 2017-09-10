#pragma once

namespace std {
#if defined(_UNICODE)
using tstring = wstring;
using tstringstream = wstringstream;
using tostringstream = wostringstream;
using tistringstream = wistringstream;
using tistream = wistream;
using tostream = wostream;
#else
using tstring = string;
using tstringstream = stringstream;
using tostringstream = ostringstream;
using tistringstream = istringstream;
using tistream = istream;
using tostream = ostream;
#endif
}

namespace state_machine {

static LPCTSTR defaultLogConigFileName(_T("log4cplus.properties"));
static LPCTSTR globalLoggerName = _T("state_machine.global");
static LPCTSTR stateMachineDefaultLoggerName = _T("state_machine.StateMachine");

extern void configureLog(LPCTSTR configFileName);

}
