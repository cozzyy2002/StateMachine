#pragma once

namespace std {
#if defined(_UNICODE)
typedef wstring tstring;
typedef wstringstream tstringstream;
typedef wostringstream tostringstream;
typedef wistringstream tistringstream;
typedef wistream tistream;
typedef wostream tostream;
#else
typedef string tstring;
typedef stringstream tstringstream;
typedef ostringstream tostringstream;
typedef istringstream tistringstream;
typedef istream tistream;
typedef ostream tostream;
#endif
}

namespace state_machine {

static LPCTSTR defaultLogConigFileName(_T("log4cplus.properties"));
static LPCTSTR globalLoggerName = _T("state_machine.global");
static LPCTSTR stateMachineDefaultLoggerName = _T("state_machine.StateMachine");

extern void configureLog(LPCTSTR configFileName);

}
