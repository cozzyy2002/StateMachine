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
	LPTSTR outStr;
	size_t outPos;
	bool preserveEol;
};

class CParserState : public state_machine::State
{
public:
	virtual HRESULT handleEvent(state_machine::Event& e, State& currentState, State** nextState) override;
};

// Comment state started by "/*"
class CCommentState : public state_machine::SubState
{
public:
	virtual HRESULT handleEvent(state_machine::Event& e, State& currentState, State** nextState) override;
};

// Comment state started by "//"
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
	// On EOF, this value is '\0'
	TCHAR character;
};

}
