#pragma once

#include <StateMachine/Event.h>
#include <StateMachine/State.h>

#include <map>

class MockObject
{
public:
	MockObject();
	MockObject(int id);
	virtual ~MockObject();

	static void clear();
	static bool create(int id);
	static bool deleted(int id);

	int m_id;
	typedef std::map<int, MockObject*> MockObjects_t;
	static MockObjects_t m_mockObjects;
};

class MockEvent : public state_machine::Event, public MockObject
{
public:
	MockEvent() : MockObject() {}
	MockEvent(int id) : MockObject(id) {}

	virtual LPCTSTR toString() { return _T(""); }
};

class MockState : public state_machine::State, public MockObject
{
public:
	MockState() : MockObject() {}
	MockState(int id) : MockObject(id) {}

	MOCK_METHOD3(handleEvent, HRESULT(const state_machine::Event* e, const state_machine::State* currentState, state_machine::State** nextState));
	MOCK_METHOD2(entry, HRESULT(const state_machine::Event* e, const state_machine::State* previousState));
	MOCK_METHOD2(exit, HRESULT(const state_machine::Event* e, const state_machine::State* nextState));

	virtual LPCTSTR toString() { return _T(""); }
};
