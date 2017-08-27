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
	std::unique_ptr<char[]> outStr(new char[len + 1]);
	CParserContext context(*m_stateMachine);
	context.start(outStr.get(), new CParserState());
	for(size_t i = 0; i < len; i++) {
		CParserEvent e(source[i]);
		context.handleEvent(e);
		// Note: e.character might be modified by state.
		context.m_previousCharacter = e.character;
	}
	context.stop();
	out.assign(outStr.get(), strlen(outStr.get()));
	return std::string();
}
