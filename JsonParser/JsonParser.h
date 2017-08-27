#pragma once

#include <memory>

namespace json_parser {

class CJsonParser
{
public:
	CJsonParser();
	~CJsonParser();

	void removeComment(LPCTSTR source, std::tstring& out, bool preserveEof);

protected:
	std::unique_ptr<state_machine::StateMachine> m_stateMachine;
};

}
