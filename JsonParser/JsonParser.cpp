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

void CJsonParser::removeComment(LPCTSTR source, std::tstring & out)
{
	auto len(_tcslen(source));
	std::unique_ptr<TCHAR[]> outStr(new TCHAR[len + 1]);
	CParserContext context(*m_stateMachine);
	context.start(outStr.get(), new CParserState());
	for(size_t i = 0; i < len; i++) {
		CParserEvent e(source[i]);
		context.handleEvent(e);
		// Note: e.character might be modified by state.
		context.m_previousCharacter = e.character;
	}
	context.stop();
	out.assign(outStr.get(), _tcslen(outStr.get()));
}
