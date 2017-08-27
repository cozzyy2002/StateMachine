#include "stdafx.h"
#include "JsonParserObject.h"

using namespace json_parser;

json_parser::CParserContext::CParserContext(state_machine::StateMachine & stateMachine)
	: Context(stateMachine)
	, m_previousCharacter('\0'), m_startQuotationMark('\0')
{
}

HRESULT json_parser::CParserContext::start(state_machine::State * initialState)
{
	return E_NOTIMPL;
}

void json_parser::CParserContext::out(char character)
{
}

HRESULT json_parser::CParserState::handleEvent(state_machine::Event & e, State & currentState, State ** nextState)
{
	auto context = e.getContext<CParserContext>();
	auto _e(e.cast<CParserEvent>());
	auto isOut(true);
	switch(_e->character) {
	case '*':
		if(context->m_previousCharacter == '/') {
			// "/*": Start of comment.
			*nextState = new CCommentState();
			isOut = false;
		}
		break;
	case '/':
		if(context->m_previousCharacter == '/') {
			// "//": Start of single line comment.
			*nextState = new CSingleLineCommentState();
		}
		isOut = false;
		break;
	case '\'':
	case '\"':
		// Start of literal string.
		context->m_startQuotationMark = _e->character;
		*nextState = new CLiteralState();
		break;
	default:
		break;
	}
	if(isOut) {
		if(context->m_previousCharacter == '/') {
			context->out('/');
		}
		context->out(_e->character);
	}
	return S_OK;
}

HRESULT json_parser::CCommentState::handleEvent(state_machine::Event & e, State & currentState, State ** nextState)
{
	auto context = e.getContext<CParserContext>();
	auto _e(e.cast<CParserEvent>());
	switch(_e->character) {
	case '/':
		if(context->m_previousCharacter == '*') {
			// End of comment
			*nextState = backToMaster();
		}
		break;
	case '\r':
	case '\n':
		context->out(_e->character);
		break;
	default:
		break;
	}
	return S_OK;
}

HRESULT json_parser::CSingleLineCommentState::handleEvent(state_machine::Event & e, State & currentState, State ** nextState)
{
	auto context = e.getContext<CParserContext>();
	auto _e(e.cast<CParserEvent>());
	switch(_e->character) {
	case '\r':
	case '\n':
		// End of comment
		*nextState = backToMaster();
		context->out(_e->character);
		break;
	}
	return S_OK;
}

HRESULT json_parser::CLiteralState::handleEvent(state_machine::Event & e, State & currentState, State ** nextState)
{
	auto context = e.getContext<CParserContext>();
	auto _e(e.cast<CParserEvent>());
	if(_e->character == context->m_startQuotationMark) {
		// End of literal string.
		*nextState = backToMaster();
	}
	context->out(_e->character);
	return S_OK;
}
