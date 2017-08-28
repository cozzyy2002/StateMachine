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

void CJsonParser::removeComment(LPCTSTR source, bool preserveEol, std::tstring & out)
{
	auto len(_tcslen(source));
	CParserContext context(*m_stateMachine);
	context.start(preserveEol, new CParserState());
	for(size_t i = 0; i < len; i++) {
		CParserEvent e(source[i]);
		context.handleEvent(e);
		// Note: e.character might be modified by state.
		context.previousCharacter = e.character;
	}
	context.stop(out);
}
