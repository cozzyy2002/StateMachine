#pragma once

namespace json_parser {

class CParserContext : public state_machine::Context
{
public:
	CParserContext(state_machine::StateMachine& stateMachine);

	char m_previousCharacter;
};

class CParserState : public state_machine::State
{
public:
	virtual HRESULT handleEvent(state_machine::Event& e, State& currentState, State** nextState) override;
};

class CParserEvent : public state_machine::Event
{
public:
};

}
