#pragma once

#include <memory>

namespace json_parser {

class CJsonParser
{
public:
	CJsonParser();
	~CJsonParser();

	std::string removeComment(LPCSTR source, std::string& out);

protected:
	std::unique_ptr<state_machine::StateMachine> m_stateMachine;
};

}
