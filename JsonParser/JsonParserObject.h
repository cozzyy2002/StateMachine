#pragma once

namespace json_parser {

class CParserContext : public state_machine::Context
{
public:
	CParserContext(state_machine::StateMachine& stateMachine);

	virtual HRESULT start(state_machine::State* initialState) override;
	void out(char character);

	char m_previousCharacter;
	char m_startQuotationMark;

protected:
	
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
	CParserEvent(char character) : character(character) {}

	// Character to parse.
	// On EOF, this value is '\0'
	char character;
};

}
