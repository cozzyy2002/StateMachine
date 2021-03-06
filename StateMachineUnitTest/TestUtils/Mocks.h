#pragma once

#include <StateMachine/Context.h>
#include <StateMachine/Event.h>
#include <StateMachine/State.h>
#include <win32/Enum.h>
#include <map>

ENUM(MockObjectId,
	UNKNOWN,
	EVENT,
	EVENT_0, EVENT_1, EVENT_2, EVENT_3, EVENT_4,
	EVENT_5, EVENT_6, EVENT_7, EVENT_8, EVENT_9,
	CURRENT_STATE,
	NEXT_STATE, NEXT_STATE1, NEXT_STATE2,
	MASTER_STATE, MASTER_STATE1, MASTER_STATE2,
	SUB_STATE, SUB_STATE1, SUB_STATE2,
	OTHER_STATE, OTHER_STATE1, OTHER_STATE2,
	STATE,
	STATE_0, STATE_1, STATE_2, STATE_3, STATE_4,
	STATE_5, STATE_6, STATE_7, STATE_8, STATE_9
	);

class MockObject
{
public:
	MockObject();
	MockObject(MockObjectId id);
	virtual ~MockObject();

	static void clear();
	static bool created(MockObjectId id);
	static bool deleted(MockObjectId id);

	MockObjectId m_id;
	using MockObjects_t = std::map<MockObjectId, MockObject*>;
	static MockObjects_t m_mockObjects;

protected:
	// Implementation of Object::modifyString() to be called by derived classes.
	void modifyString(std::tstring& _string);
};

class MockEvent : public state_machine::Event, public MockObject
{
public:
	MockEvent(Priority priority = Priority::Normal)
		: state_machine::Event(priority), MockObject() {}
	MockEvent(MockObjectId id, Priority priority = Priority::Normal)
		: state_machine::Event(priority), MockObject(id) {}
	virtual log4cplus::LogLevel getLogLevel() const override { return logLevel; }

	// Default loglevel
	log4cplus::LogLevel logLevel = log4cplus::DEBUG_LOG_LEVEL;

protected:
	virtual void modifyString(std::tstring& _string) override { MockObject::modifyString(_string); }
};

class MockState : public state_machine::State, public MockObject
{
public:
	MockState(MockObjectId id) : MockObject(id) {}

	MOCK_METHOD4(handleEvent, HRESULT(state_machine::Context& context, state_machine::Event& e, state_machine::State& currentState, state_machine::State** nextState));
	MOCK_METHOD2(handleIgnoredEvent, HRESULT(state_machine::Context& context, state_machine::Event& e));
	MOCK_METHOD3(handleError, HRESULT(state_machine::Context& context, state_machine::Event& e, HRESULT hr));
	MOCK_METHOD3(entry, HRESULT(state_machine::Context& context, state_machine::Event& e, state_machine::State& previousState));
	MOCK_METHOD3(exit, HRESULT(state_machine::Context& context, state_machine::Event& e, state_machine::State& nextState));

	using State::eventIsIgnored;

protected:
	virtual void modifyString(std::tstring& _string) override { MockObject::modifyString(_string); }
};

class MockSubState : public state_machine::State, public MockObject
{
public:
	MockSubState(State* masterState, MockObjectId id) : State(masterState), MockObject(id) {}

	MOCK_METHOD4(handleEvent, HRESULT(state_machine::Context& context, state_machine::Event& e, state_machine::State& currentState, state_machine::State** nextState));
	MOCK_METHOD2(handleIgnoredEvent, HRESULT(state_machine::Context& context, state_machine::Event& e));
	MOCK_METHOD3(handleError, HRESULT(state_machine::Context& context, state_machine::Event& e, HRESULT hr));
	MOCK_METHOD3(entry, HRESULT(state_machine::Context& context, state_machine::Event& e, state_machine::State& previousState));
	MOCK_METHOD3(exit, HRESULT(state_machine::Context& context, state_machine::Event& e, state_machine::State& nextState));

	using State::backToMaster;
	using State::eventIsIgnored;

protected:
	virtual void modifyString(std::tstring& _string) override { MockObject::modifyString(_string); }
};
