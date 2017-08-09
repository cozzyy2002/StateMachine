#include "stdafx.h"
#include "TestUtils/Mocks.h"

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

/*
	Start/Stop state machine.

	Context::start()
		entry() of user state is called once.
		Event::context is equal to user context.
		Current state is equal to user state.
	Context::stop():
		User state is deleted.
*/
TEST_F(ContextUnitTest, start_stop)
{
	MockState* state = new MockState(MockObjectId::CURRENT_STATE);

	EXPECT_CALL(*state, handleEvent(_, _, _)).Times(0);
	EXPECT_CALL(*state, entry(Field(&Event::context, Eq(testee)), _)).Times(1);
	EXPECT_CALL(*state, exit(_, _)).Times(0);

	ASSERT_HRESULT_SUCCEEDED(testee->start(state));

	EXPECT_EQ(state, stateMachine.getCurrentState(testee));
	EXPECT_FALSE(MockObject::deleted(MockObjectId::CURRENT_STATE));

	ASSERT_HRESULT_SUCCEEDED(testee->stop());

	EXPECT_EQ(nullptr, stateMachine.getCurrentState(testee));
	EXPECT_TRUE(MockObject::deleted(MockObjectId::CURRENT_STATE));
}

/*
	Start/Stop state machine with user event.

	Context::start(User event)
		entry(User event) of user state is called once.
		context of user event is equal to user context.
		Current state is equal to user state.
	Context::stop():
		User state is deleted.
*/
TEST_F(ContextUnitTest, start_with_user_event_stop)
{
	MockState* state = new MockState(MockObjectId::CURRENT_STATE);

	EXPECT_CALL(*state, handleEvent(_, _, _)).Times(0);
	EXPECT_CALL(*state, entry(&e, _)).Times(1);
	EXPECT_CALL(*state, exit(_, _)).Times(0);

	ASSERT_HRESULT_SUCCEEDED(testee->start(state, &e));

	EXPECT_EQ(testee, e.context);
	EXPECT_EQ(state, stateMachine.getCurrentState(testee));
	EXPECT_FALSE(MockObject::deleted(MockObjectId::CURRENT_STATE));

	ASSERT_HRESULT_SUCCEEDED(testee->stop());

	EXPECT_EQ(nullptr, stateMachine.getCurrentState(testee));
	EXPECT_TRUE(MockObject::deleted(MockObjectId::CURRENT_STATE));
}
