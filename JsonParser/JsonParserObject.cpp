#include "stdafx.h"
#include "JsonParserObject.h"

using namespace json_parser;

CParserContext::CParserContext(state_machine::StateMachine & stateMachine)
	: Context(stateMachine)
{
}

HRESULT CParserContext::start(bool preserveEol, state_machine::State * initialState)
{
	outStream = std::make_unique<std::tostringstream>();
	m_previousCharacter = '\0';
	m_startQuotationMark = '\0';
	this->preserveEol = preserveEol;

	return Context::start(initialState);
}

HRESULT CParserContext::stop(std::tstring& out)
{
	*outStream << std::ends;
	out = outStream->str();
	return Context::stop();
}

void CParserContext::out(TCHAR character)
{
	if(!preserveEol) {
		switch(character) {
		case '\r':
		case '\n':
			return;
		}
	}
	*outStream << character;
}

HRESULT CParserState::handleEvent(state_machine::Event & e, State & currentState, State ** nextState)
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

HRESULT CCommentState::handleEvent(state_machine::Event & e, State & currentState, State ** nextState)
{
	auto context = e.getContext<CParserContext>();
	auto _e(e.cast<CParserEvent>());
	switch(_e->character) {
	case '/':
		if(context->m_previousCharacter == '*') {
			// End of comment
			*nextState = backToMaster();

			// Avoid to out '/' when processing next caracter in CPerserState::handleEvent().
			_e->character = '\0';
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

HRESULT CSingleLineCommentState::handleEvent(state_machine::Event & e, State & currentState, State ** nextState)
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

HRESULT CLiteralState::handleEvent(state_machine::Event & e, State & currentState, State ** nextState)
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
