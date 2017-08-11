#include "stdafx.h"
#include "TestUtils/Mocks.h"
#include "TestUtils/TestStateMachine.h"

#include <StateMachine/Context.h>
#include <StateMachine/StateMachine.h>

using namespace state_machine;
using namespace testing;

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
	EXPECT_CALL(*state, entry(Property(&Event::getContext<Testee>, Eq(testee)), _)).Times(1);
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

	EXPECT_EQ(testee, e.getContext());
	EXPECT_EQ(state, stateMachine.getCurrentState(testee));
	EXPECT_FALSE(MockObject::deleted(MockObjectId::CURRENT_STATE));

	ASSERT_HRESULT_SUCCEEDED(testee->stop());

	EXPECT_EQ(nullptr, stateMachine.getCurrentState(testee));
	EXPECT_TRUE(MockObject::deleted(MockObjectId::CURRENT_STATE));
}

class ContextHandleEventUnitTest : public ContextUnitTest
{
public:
	void SetUp() {
		state = new MockState(MockObjectId::CURRENT_STATE);
		EXPECT_CALL(*state, entry(_, _)).Times(1);
		ASSERT_HRESULT_SUCCEEDED(testee->start(state));
	}
	void TearDown() {
		ASSERT_HRESULT_SUCCEEDED(testee->stop());
	}

	MockState* state;
};

TEST_F(ContextHandleEventUnitTest, recursive_call_check)
{
	EXPECT_CALL(*state, handleEvent(&e, state, _))
		.WillOnce(Invoke([this](Event* _e, State*, State**)
	{
		EXPECT_TRUE(testee->isEventHandling());

		// Call Context::handleEvent() in State::handleEvent().
		EXPECT_EQ(E_ILLEGAL_METHOD_CALL, testee->handleEvent(_e));
		return S_OK;
	}));

	ASSERT_HRESULT_SUCCEEDED(testee->handleEvent(&e));
	EXPECT_FALSE(testee->isEventHandling());

	EXPECT_EQ(state, stateMachine.getCurrentState(testee));
	EXPECT_FALSE(MockObject::deleted(MockObjectId::CURRENT_STATE));
	EXPECT_TRUE(e.isHandled);
}

class MultiContextHandleEventUnitTest : public ContextHandleEventUnitTest
{
public:
	MultiContextHandleEventUnitTest() : testee1(new Testee(&stateMachine)) {}

	void SetUp() {
		ContextHandleEventUnitTest::SetUp();

		EXPECT_CALL(*state, entry(_, _)).Times(1);
		ASSERT_HRESULT_SUCCEEDED(testee1->start(state));
	}
	void TearDown() {
		ContextHandleEventUnitTest::TearDown();
	}

	Testee* testee1;
};

TEST_F(MultiContextHandleEventUnitTest, recursive_call_check)
{
	EXPECT_CALL(*state, handleEvent(&e, state, _))
		.WillOnce(Invoke([this](Event* _e, State*, State**)
		{
			EXPECT_TRUE(testee->isEventHandling());

			// Call another Context::handleEvent() in State::handleEvent().
			EXPECT_HRESULT_SUCCEEDED(testee1->handleEvent(_e));
			return S_OK;
		}))
		.WillOnce(Return(S_OK));

	ASSERT_HRESULT_SUCCEEDED(testee->handleEvent(&e));
	EXPECT_FALSE(testee->isEventHandling());

	EXPECT_EQ(state, stateMachine.getCurrentState(testee));
	EXPECT_EQ(state, stateMachine.getCurrentState(testee1));
	EXPECT_FALSE(MockObject::deleted(MockObjectId::CURRENT_STATE));
	EXPECT_TRUE(e.isHandled);
}
