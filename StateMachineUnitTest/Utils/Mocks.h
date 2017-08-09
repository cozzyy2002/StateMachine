#pragma once

#include <StateMachine/Event.h>
#include <StateMachine/State.h>

#include <map>

enum class MockObjectId{
	UNKNOWN = -1,
	EVENT = 1,
	CURRENT_STATE,
	NEXT_STATE,
	MASTER_STATE1,
	MASTER_STATE2,
	OTHER_STATE,
};

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
	MockEvent() : MockObject() {}
	MockEvent(MockObjectId id) : MockObject(id) {}

	virtual LPCTSTR toString() const { return _T(""); }
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

	// Implementation of Object::getObject()
	virtual LPCTSTR toString();
	mutable std::tstring m_string;

	using State::backToMaster;
};
