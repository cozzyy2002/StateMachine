#pragma once

#include <StateMachine/Event.h>
#include <StateMachine/State.h>
#include <win32/Enum.h>
#include <map>

ENUM(MockObjectId,
	UNKNOWN,
	EVENT,
	CURRENT_STATE,
	NEXT_STATE,
	MASTER_STATE1,
	MASTER_STATE2,
	OTHER_STATE
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
};

class MockEvent : public state_machine::Event, public MockObject
{
public:
	MockEvent(state_machine::Context* context = nullptr) : state_machine::Event(context), MockObject() {}
	MockEvent(MockObjectId id) : MockObject(id) {}
	virtual log4cplus::LogLevel getLogLevel() const { return log4cplus::DEBUG_LOG_LEVEL; }
};

class MockState : public state_machine::State, public MockObject
{
public:
	MockState() : MockObject() {}
	MockState(MockObjectId id) : MockObject(id) {}

	MOCK_METHOD3(handleEvent, HRESULT(const state_machine::Event* e, const state_machine::State* currentState, state_machine::State** nextState));
	MOCK_METHOD1(handleIgnoredEvent, HRESULT(const state_machine::Event* e));
	MOCK_METHOD2(handleError, HRESULT(const state_machine::Event* e, HRESULT hr));
	MOCK_METHOD2(entry, HRESULT(const state_machine::Event* e, const state_machine::State* previousState));
	MOCK_METHOD2(exit, HRESULT(const state_machine::Event* e, const state_machine::State* nextState));

	virtual LPCTSTR toString();

	using State::backToMaster;
};
