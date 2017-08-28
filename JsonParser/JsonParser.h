#pragma once

#include <memory>

namespace json_parser {

class CJsonParser
{
public:
	CJsonParser();
	~CJsonParser();
	void removeComment(LPCTSTR source, bool preserveEol, std::tstring& out);

protected:
	std::unique_ptr<state_machine::StateMachine> m_stateMachine;
};

}
