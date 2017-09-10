#pragma once

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
	NEXT_STATE,
	MASTER_STATE1,
	MASTER_STATE2,
	OTHER_STATE,
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
	typedef std::map<MockObjectId, MockObject*> MockObjects_t;
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
	MockEvent(state_machine::Context& context, Priority priority = Priority::Normal)
		: state_machine::Event(context, priority), MockObject() {}
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

	MOCK_METHOD3(handleEvent, HRESULT(state_machine::Event& e, state_machine::State& currentState, state_machine::State** nextState));
	MOCK_METHOD1(handleIgnoredEvent, HRESULT(state_machine::Event& e));
	MOCK_METHOD2(handleError, HRESULT(state_machine::Event& e, HRESULT hr));
	MOCK_METHOD2(entry, HRESULT(state_machine::Event& e, state_machine::State& previousState));
	MOCK_METHOD2(exit, HRESULT(state_machine::Event& e, state_machine::State& nextState));

	using State::eventIsIgnored;

protected:
	virtual void modifyString(std::tstring& _string) override { MockObject::modifyString(_string); }
};

class MockSubState : public state_machine::SubState, public MockObject
{
public:
	MockSubState(MockObjectId id) : MockObject(id) {}

	MOCK_METHOD3(handleEvent, HRESULT(state_machine::Event& e, state_machine::State& currentState, state_machine::State** nextState));
	MOCK_METHOD1(handleIgnoredEvent, HRESULT(state_machine::Event& e));
	MOCK_METHOD2(handleError, HRESULT(state_machine::Event& e, HRESULT hr));
	MOCK_METHOD2(entry, HRESULT(state_machine::Event& e, state_machine::State& previousState));
	MOCK_METHOD2(exit, HRESULT(state_machine::Event& e, state_machine::State& nextState));

	using SubState::backToMaster;
	using State::eventIsIgnored;

protected:
	virtual void modifyString(std::tstring& _string) override { MockObject::modifyString(_string); }
};
