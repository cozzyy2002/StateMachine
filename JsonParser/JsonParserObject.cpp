#include "stdafx.h"
#include "JsonParserObject.h"

using namespace json_parser;

CParserContext::CParserContext(state_machine::StateMachine & stateMachine)
	: Context(stateMachine)
{
}

HRESULT CParserContext::start(const CJsonParser::Option& option, state_machine::State * initialState)
{
	outStream = std::make_unique<std::tostringstream>();
	previousCharacter = '\0';
	this->option = &option;

	return Context::start(initialState);
}

HRESULT CParserContext::stop(std::tstring& out)
{
	*outStream << std::ends;
	out = outStream->str();
	return Context::stop();
}

/*
	Writes character to output stream.
*/
void CParserContext::out(TCHAR character)
{
	if(option->removeEol) {
		switch(character) {
		case '\r':
		case '\n':
			// Remove EOL.
			return;
		}
	}
	*outStream << character;
}

/*
	Handles characters exept for comment and literal.

	This state is created as initial state on CParserContext::start().
*/
HRESULT CParserState::handleEvent(state_machine::Event & e, State & currentState, State ** nextState)
{
	auto context = e.getContext<CParserContext>();
	auto _e(e.cast<CParserEvent>());
	auto isOut(true);
	switch(_e->character) {
	case '*':
		if(context->previousCharacter == '/') {
			// "/*": Start of comment.
			*nextState = new CCommentState();
			isOut = false;
		}
		break;
	case '/':
		if(context->previousCharacter == '/') {
			// "//": Start of single line comment.
			*nextState = new CSingleLineCommentState();
		}
		isOut = false;
		break;
	case '\"':
		// Start of literal string.
		*nextState = new CLiteralState();
		break;
	default:
		break;
	}
	if(isOut) {
		if(context->previousCharacter == '/') {
			// In case previous '/' is not start of comment.
			context->out('/');
		}
		context->out(_e->character);
	}
	return S_OK;
}

/*
	Handles characters in comment.

	Recognaizes '*' followed by '/' as end of the comment.
	Returns back to parent state on end of the comment.
	Other characters than end of line(EOL) are ignored.
*/
HRESULT CCommentState::handleEvent(state_machine::Event & e, State & currentState, State ** nextState)
{
	auto context = e.getContext<CParserContext>();
	auto _e(e.cast<CParserEvent>());
	switch(_e->character) {
	case '/':
		if(context->previousCharacter == '*') {
			// End of comment
			*nextState = backToMaster();

			// Avoid to out '/' when processing next caracter in CPerserState::handleEvent().
			_e->character = '\0';
		}
		break;
	case '\r':
	case '\n':
		// EOL characters.
		context->out(_e->character);
		break;
	default:
		break;
	}
	return S_OK;
}

/*
	Handles characters in single line comment.

	Recognaizes end of line(EOL) as end of the comment.
	Other characters than above are ignored.
	Returns back to parent state on end of the comment.
*/
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

/*
	Handles characters in literal string.

	Recognizes escape sequence '\x' and end of literal '"'.
	Returns back to parent state on end of literal.
*/
HRESULT CLiteralState::handleEvent(state_machine::Event & e, State & currentState, State ** nextState)
{
	static const TCHAR escapeChar('\\');
	auto context = e.getContext<CParserContext>();
	auto _e(e.cast<CParserEvent>());
	auto character(_e->character);
	if(context->previousCharacter == escapeChar) {
		// Character following '\' is treated as ordinary character.
		// e.g. '\"' is '\"' not end of literal.
		context->out(escapeChar);
	} else {
		if(character == escapeChar) {
			// On start of escape, do not out now.
			return S_OK;
		} else if(character == '\"') {
			// End of literal string.
			*nextState = backToMaster();
		}
	}
	context->out(character);
	return S_OK;
}
