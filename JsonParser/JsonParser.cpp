#include "stdafx.h"
#include "JsonParser.h"

using namespace json_parser;

CJsonParser::CJsonParser()
	: m_stateMachine(state_machine::StateMachine::createInstance())
{
}


CJsonParser::~CJsonParser()
{
}

std::string CJsonParser::removeComment(LPCSTR source, std::string & out)
{
	return std::string();
}
