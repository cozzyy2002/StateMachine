#pragma once

namespace json_parser {

class CParserContext : public state_machine::Context
{
public:
	CParserContext(state_machine::StateMachine& stateMachine);

	HRESULT start(LPTSTR outStr, bool preserveEol, state_machine::State* initialState);
	HRESULT stop();
	void out(TCHAR character);

	TCHAR m_previousCharacter;
	TCHAR m_startQuotationMark;

protected:
	// Output string written by out() method.
	LPTSTR outStr;
	size_t outPos;
	bool preserveEol;
};

// State that processes ordinary characters.
// Ordinary characters are other than comment nor literal string.
// This state is also created as initial state.
class CParserState : public state_machine::State
{
public:
	virtual HRESULT handleEvent(state_machine::Event& e, State& currentState, State** nextState) override;
};

// Comment state started by "/*" and ended by "*/"
class CCommentState : public state_machine::SubState
{
public:
	virtual HRESULT handleEvent(state_machine::Event& e, State& currentState, State** nextState) override;
};

// Comment state started by "//" and ended by end of line(EOL)
class CSingleLineCommentState : public state_machine::SubState
{
public:
	virtual HRESULT handleEvent(state_machine::Event& e, State& currentState, State** nextState) override;
};

// Literal state enclosed by single/double quotation mark.
class CLiteralState : public state_machine::SubState
{
public:
	virtual HRESULT handleEvent(state_machine::Event& e, State& currentState, State** nextState) override;
};

class CParserEvent : public state_machine::Event
{
public:
	CParserEvent(TCHAR character) : character(character) {}

	// Character to parse.
	TCHAR character;
};

}