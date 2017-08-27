#include "stdafx.h"
#include "JsonParser.h"
#include "JsonParserObject.h"

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
	auto len(strlen(source));
	auto strOut(new char[len]);
	CParserContext context(*m_stateMachine);
	context.start(new CParserState());
	for(size_t i = 0; i < len; i++) {
		auto charactor(source[i]);
		CParserEvent e(charactor);
		context.handleEvent(e);
		context.m_previousCharacter = charactor;
	}
	context.stop();
	out.assign(strOut, strlen(strOut));
	return std::string();
}
