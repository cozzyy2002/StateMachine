#pragma once

#include <StateMachine/Context.h>
#include <StateMachine/State.h>
#include <StateMachine/Event.h>
#include <StateMachine/StateMachine.h>

using namespace state_machine;

class CAppContext : public Context
{
public:
	CAppContext(LPCTSTR name, StateMachine* stateMachine);
	~CAppContext();

protected:
	CString m_name;

	virtual void modifyString(std::tstring& _string) override { _string = m_name.GetString(); }
};
