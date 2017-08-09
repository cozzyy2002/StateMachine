#include "stdafx.h"
#include "Utils/Mocks.h"

#include <StateMachine/Context.h>
#include <StateMachine/StateMachine.h>

using namespace state_machine;
using namespace testing;

class TestStateMachine : public StateMachine
{
public:
	using StateMachine::setCurrentState;
	using StateMachine::getCurrentState;
};

class ContextUnitTest : public Test
{
public:
	class Testee : public Context
	{
	public:
		Testee(TestStateMachine* stateMachine) : Context(stateMachine) {}
	};

	ContextUnitTest() : testee(new Testee(&stateMachine)) {}

	TestStateMachine stateMachine;
	Testee* testee;
	MockEvent e;
};

TEST_F(ContextUnitTest, start_stop)
{
	MockState* state = new MockState(MockObjectId::CURRENT_STATE);

	EXPECT_CALL(*state, handleEvent(_, _, _)).Times(0);
	EXPECT_CALL(*state, entry(_, _)).Times(1);
	EXPECT_CALL(*state, exit(_, _)).Times(0);

	ASSERT_HRESULT_SUCCEEDED(testee->start(state));

	EXPECT_EQ(state, stateMachine.getCurrentState(testee));
	EXPECT_FALSE(MockObject::deleted(MockObjectId::CURRENT_STATE));

	ASSERT_HRESULT_SUCCEEDED(testee->stop());

	EXPECT_EQ(nullptr, stateMachine.getCurrentState(testee));
	EXPECT_TRUE(MockObject::deleted(MockObjectId::CURRENT_STATE));
}

TEST_F(ContextUnitTest, start_with_user_event_stop)
{
	MockState* state = new MockState(MockObjectId::CURRENT_STATE);

	EXPECT_CALL(*state, handleEvent(_, _, _)).Times(0);
	EXPECT_CALL(*state, entry(&e, _)).Times(1);
	EXPECT_CALL(*state, exit(_, _)).Times(0);

	ASSERT_HRESULT_SUCCEEDED(testee->start(state, &e));

	EXPECT_EQ(state, stateMachine.getCurrentState(testee));
	EXPECT_FALSE(MockObject::deleted(MockObjectId::CURRENT_STATE));

	ASSERT_HRESULT_SUCCEEDED(testee->stop());

	EXPECT_EQ(nullptr, stateMachine.getCurrentState(testee));
	EXPECT_TRUE(MockObject::deleted(MockObjectId::CURRENT_STATE));
}
