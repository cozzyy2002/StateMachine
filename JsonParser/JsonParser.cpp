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

HRESULT CJsonParser::removeComment(LPCTSTR source, bool preserveEol, std::tstring & out)
{
	Option option(true, !preserveEol);
	return preprocess(source, option, out);
}

HRESULT json_parser::CJsonParser::preprocess(LPCTSTR source, const Option & option, std::tstring & out)
{
	// Remove space and expand tab can not be specified with each other.
	if(option.removeSpace && option.expandTab) return E_INVALIDARG;
	// Tab stop should be greater than 1.
	if(option.expandTab && (option.tabStop < 2)) return E_INVALIDARG;

	auto len(_tcslen(source));
	CParserContext context(*m_stateMachine);
	context.start(option, new CParserState());
	for(size_t i = 0; i < len; i++) {
		CParserEvent e(source[i]);
		context.handleEvent(e);
		// Note: e.character might be modified by state.
		context.previousCharacter = e.character;
	}
	context.stop(out);
	return S_OK;
}
