#include "stdafx.h"
#include "TestUtils/Mocks.h"
#include "TestUtils/TestStateMachine.h"

#include <StateMachine/Context.h>

using namespace state_machine;
using namespace testing;

class TestContext : public Context
{
public:
	TestContext() : Context(false) {}
};

class StateMachineUnitTest : public Test
{
public:
	class Testee : public TestStateMachine
	{
	public:
		Testee(Context& context) : TestStateMachine(context) {}
	};

	StateMachineUnitTest()
		: testee(context), e(new MockEvent()) {}

	void SetUp() {
		currentState = new MockState(MockObjectId::CURRENT_STATE);
		testee.setNextState(currentState);
	}
	void TearDown() {
	}

	Testee testee;
	TestContext context;
	MockState* currentState;
	std::unique_ptr<MockEvent> e;
};

TEST_F(StateMachineUnitTest, no_transition)
{
	EXPECT_CALL(*currentState, handleEvent(Ref(context), Ref(*e), Ref(*currentState), _))
		.WillOnce(Return(S_OK));
	EXPECT_CALL(*currentState, entry(_, _, _)).Times(0);
	EXPECT_CALL(*currentState, exit(_, _, _)).Times(0);

	ASSERT_HRESULT_SUCCEEDED(testee.handleEvent(*e));

	EXPECT_EQ(currentState, context.getCurrentState());
	EXPECT_FALSE(MockObject::deleted(MockObjectId::CURRENT_STATE));
	EXPECT_TRUE(e->isHandled);
}

TEST_F(StateMachineUnitTest, no_transition_error)
{
	EXPECT_CALL(*currentState, handleEvent(Ref(context), Ref(*e), Ref(*currentState), _))
		.WillOnce(Return(E_NOTIMPL));
	EXPECT_CALL(*currentState, handleError(Ref(context), Ref(*e), E_NOTIMPL))
		.WillOnce(Invoke([this](Context& context, Event& e, HRESULT hr)
		{
			// Call default error handler.
			 return currentState->State::handleError(context, e, hr);
		}));
	EXPECT_CALL(*currentState, entry(_, _, _)).Times(0);
	EXPECT_CALL(*currentState, exit(_, _, _)).Times(0);

	ASSERT_EQ(E_NOTIMPL, testee.handleEvent(*e));

	EXPECT_EQ(currentState, context.getCurrentState());
	EXPECT_FALSE(MockObject::deleted(MockObjectId::CURRENT_STATE));
	EXPECT_TRUE(e->isHandled);
}

TEST_F(StateMachineUnitTest, transition)
{
	auto nextState = new MockState(MockObjectId::NEXT_STATE);
	EXPECT_CALL(*currentState, handleEvent(Ref(context), Ref(*e), Ref(*currentState), _))
		.WillOnce(DoAll(SetArgPointee<3>(nextState), Return(S_OK)));
	EXPECT_CALL(*currentState, entry(_, _, _)).Times(0);
	EXPECT_CALL(*currentState, exit(Ref(context), Ref(*e), Ref(*nextState))).Times(1);
	EXPECT_CALL(*nextState, entry(Ref(context), Ref(*e), Ref(*currentState))).Times(1);
	EXPECT_CALL(*nextState, exit(_, _, _)).Times(0);

	ASSERT_HRESULT_SUCCEEDED(testee.handleEvent(*e));

	EXPECT_EQ(nextState, context.getCurrentState());
	EXPECT_TRUE(MockObject::deleted(MockObjectId::CURRENT_STATE));
	EXPECT_FALSE(MockObject::deleted(MockObjectId::NEXT_STATE));
	EXPECT_TRUE(e->isHandled);
}

TEST_F(StateMachineUnitTest, transition_to_sub_state_and_back)
{
	auto nextState = new MockSubState(currentState, MockObjectId::NEXT_STATE);
	EXPECT_CALL(*currentState, handleEvent(Ref(context), Ref(*e), Ref(*currentState), _))
		.WillOnce(DoAll(SetArgPointee<3>(nextState), Return(S_OK)));
	EXPECT_CALL(*nextState, handleEvent(Ref(context), Ref(*e), Ref(*nextState), _))
		.WillOnce(DoAll(SetArgPointee<3>(nextState->backToMaster()), Return(S_OK)));
	EXPECT_CALL(*currentState, entry(_, _, _)).Times(0);
	EXPECT_CALL(*currentState, exit(_, _, _)).Times(0);
	EXPECT_CALL(*nextState, entry(Ref(context), Ref(*e), Ref(*currentState))).Times(1);
	EXPECT_CALL(*nextState, exit(Ref(context), Ref(*e), Ref(*currentState))).Times(1);

	// Master state -> Sub state
	ASSERT_HRESULT_SUCCEEDED(testee.handleEvent(*e));
	EXPECT_TRUE(e->isHandled);

	// Sub state -> Master state.
	e->isHandled = false;
	ASSERT_HRESULT_SUCCEEDED(testee.handleEvent(*e));
	EXPECT_TRUE(e->isHandled);

	EXPECT_EQ(currentState, context.getCurrentState());
	EXPECT_FALSE(MockObject::deleted(MockObjectId::CURRENT_STATE));
	EXPECT_TRUE(MockObject::deleted(MockObjectId::NEXT_STATE));
}

TEST_F(StateMachineUnitTest, transition_error)
{
	auto nextState = new MockState(MockObjectId::NEXT_STATE);
	EXPECT_CALL(*currentState, handleEvent(Ref(context), Ref(*e), Ref(*currentState), _))
		.WillOnce(DoAll(SetArgPointee<3>(nextState), Return(E_NOTIMPL)));
	EXPECT_CALL(*currentState, handleError(Ref(context), Ref(*e), E_NOTIMPL))
		.WillOnce(Invoke([this](Context& context, Event& e, HRESULT hr)
		{
			// Call default error handler.
			 return currentState->State::handleError(context, e, hr);
		}));
	EXPECT_CALL(*currentState, entry(_, _, _)).Times(0);
	EXPECT_CALL(*currentState, exit(_, _, _)).Times(0);
	EXPECT_CALL(*nextState, entry(_, _, _)).Times(0);
	EXPECT_CALL(*nextState, exit(_, _, _)).Times(0);

	ASSERT_EQ(E_NOTIMPL, testee.handleEvent(*e));

	EXPECT_EQ(currentState, context.getCurrentState());
	EXPECT_FALSE(MockObject::deleted(MockObjectId::CURRENT_STATE));
	EXPECT_TRUE(MockObject::deleted(MockObjectId::NEXT_STATE));
	EXPECT_TRUE(e->isHandled);
}

class StateMachineSubStateUnitTest : public StateMachineUnitTest
{
public:
	void SetUp() {
		StateMachineUnitTest::SetUp();

		// Clear current state to change to sub state.
		testee.clearCurrentState(context);
		masterState2 = new MockState(MockObjectId::MASTER_STATE2);
		masterState1 = new MockSubState(masterState2, MockObjectId::MASTER_STATE1);
		currentState = new MockSubState(masterState1, MockObjectId::CURRENT_STATE);
		testee.setNextState(currentState);
		otherState = new MockState(MockObjectId::OTHER_STATE);

		// Check master state setter/getter of State class.
		ASSERT_EQ(masterState1, currentState->getMasterState<MockSubState>());
		ASSERT_EQ(masterState2, masterState1->getMasterState<MockState>());
	}
	void TearDown() {
		StateMachineUnitTest::TearDown();
	}

	MockSubState* currentState;	// Sub state(Different from currentState ot base class).
	MockSubState* masterState1;	// Master of current state
	MockState* masterState2;	// Master of master1 state
	MockState* otherState;
};

/*
	before: MASTER_STATE2 - MASTER_STATE1 - CURRENT_STATE
	after : MASTER_STATE2 - MASTER_STATE1
*/
TEST_F(StateMachineSubStateUnitTest, back_to_master1)
{
	EXPECT_CALL(*currentState, handleEvent(Ref(context), Ref(*e), Ref(*currentState), _))
		.WillOnce(DoAll(SetArgPointee<3>(currentState->backToMaster()), Return(S_OK)));
	EXPECT_CALL(*currentState, entry(_, _, _)).Times(0);
	EXPECT_CALL(*currentState, exit(Ref(context), _, _)).Times(1);
	EXPECT_CALL(*masterState1, entry(_, _, _)).Times(0);
	EXPECT_CALL(*masterState1, exit(_, _, _)).Times(0);
	EXPECT_CALL(*masterState2, entry(_, _, _)).Times(0);
	EXPECT_CALL(*masterState2, exit(_, _, _)).Times(0);

	ASSERT_HRESULT_SUCCEEDED(testee.handleEvent(*e));

	EXPECT_EQ(masterState1, context.getCurrentState());
	EXPECT_TRUE(MockObject::deleted(MockObjectId::CURRENT_STATE));
	EXPECT_FALSE(MockObject::deleted(MockObjectId::MASTER_STATE1));
	EXPECT_FALSE(MockObject::deleted(MockObjectId::MASTER_STATE2));
	EXPECT_TRUE(e->isHandled);
}

/*
	before: MASTER_STATE2 - MASTER_STATE1 - CURRENT_STATE
	after : MASTER_STATE2
*/
TEST_F(StateMachineSubStateUnitTest, back_to_master2)
{
	EXPECT_CALL(*currentState, handleEvent(Ref(context), Ref(*e), Ref(*currentState), _))
		.WillOnce(DoAll(SetArgPointee<3>(masterState2), Return(S_OK)));
	EXPECT_CALL(*currentState, entry(_, _, _)).Times(0);
	EXPECT_CALL(*currentState, exit(Ref(context), _, _)).Times(1);
	EXPECT_CALL(*masterState1, entry(_, _, _)).Times(0);
	EXPECT_CALL(*masterState1, exit(Ref(context), _, _)).Times(1);
	EXPECT_CALL(*masterState2, entry(_, _, _)).Times(0);
	EXPECT_CALL(*masterState2, exit(_, _, _)).Times(0);

	ASSERT_HRESULT_SUCCEEDED(testee.handleEvent(*e));

	EXPECT_EQ(masterState2, context.getCurrentState());
	EXPECT_TRUE(MockObject::deleted(MockObjectId::CURRENT_STATE));
	EXPECT_TRUE(MockObject::deleted(MockObjectId::MASTER_STATE1));
	EXPECT_FALSE(MockObject::deleted(MockObjectId::MASTER_STATE2));
	EXPECT_TRUE(e->isHandled);
}

/*
	before: MASTER_STATE2 - MASTER_STATE1 - CURRENT_STATE
	after : OTHER_STATE
*/
TEST_F(StateMachineSubStateUnitTest, exit_to_other_state)
{
	EXPECT_CALL(*currentState, handleEvent(Ref(context), Ref(*e), Ref(*currentState), _))
		.WillOnce(DoAll(SetArgPointee<3>(otherState), Return(S_OK)));
	EXPECT_CALL(*currentState, entry(_, _, _)).Times(0);
	EXPECT_CALL(*currentState, exit(Ref(context), Ref(*e), _)).Times(1);
	EXPECT_CALL(*masterState1, entry(_, _, _)).Times(0);
	EXPECT_CALL(*masterState1, exit(Ref(context), Ref(*e), _)).Times(1);
	EXPECT_CALL(*masterState2, entry(_, _, _)).Times(0);
	EXPECT_CALL(*masterState2, exit(Ref(context), Ref(*e), _)).Times(1);
	EXPECT_CALL(*otherState, entry(Ref(context), Ref(*e), _)).Times(1);
	EXPECT_CALL(*otherState, exit(_, _, _)).Times(0);

	ASSERT_HRESULT_SUCCEEDED(testee.handleEvent(*e));

	EXPECT_EQ(otherState, context.getCurrentState());
	EXPECT_TRUE(MockObject::deleted(MockObjectId::CURRENT_STATE));
	EXPECT_TRUE(MockObject::deleted(MockObjectId::MASTER_STATE1));
	EXPECT_TRUE(MockObject::deleted(MockObjectId::MASTER_STATE2));
	EXPECT_FALSE(MockObject::deleted(MockObjectId::OTHER_STATE));
	EXPECT_TRUE(e->isHandled);
}

/*
	before: MASTER_STATE2 - MASTER_STATE1 - CURRENT_STATE
			CURRENT_STATE returns SUB_STATE as next state.
	after : MASTER_STATE2 - MASTER_STATE1 - SUB_STATE
 */
TEST_F(StateMachineSubStateUnitTest, exit_to_sub_state)
{
	MockSubState* subState = new MockSubState(masterState1, MockObjectId::SUB_STATE);

	EXPECT_CALL(*currentState, handleEvent(Ref(context), Ref(*e), Ref(*currentState), _))
		.WillOnce(DoAll(SetArgPointee<3>(subState), Return(S_OK)));
	EXPECT_CALL(*currentState, entry(_, _, _)).Times(0);
	EXPECT_CALL(*currentState, exit(Ref(context), Ref(*e), Ref(*subState))).Times(1);
	EXPECT_CALL(*masterState1, entry(_, _, _)).Times(0);
	EXPECT_CALL(*masterState1, exit(_, _, _)).Times(0);
	EXPECT_CALL(*masterState2, entry(_, _, _)).Times(0);
	EXPECT_CALL(*masterState2, exit(_, _, _)).Times(0);
	EXPECT_CALL(*subState, entry(Ref(context), Ref(*e), Ref(*currentState))).Times(1);
	EXPECT_CALL(*subState, exit(_, _, _)).Times(0);

	ASSERT_HRESULT_SUCCEEDED(testee.handleEvent(*e));

	EXPECT_EQ(subState, context.getCurrentState());
	EXPECT_TRUE(MockObject::deleted(MockObjectId::CURRENT_STATE));
	EXPECT_FALSE(MockObject::deleted(MockObjectId::MASTER_STATE1));
	EXPECT_FALSE(MockObject::deleted(MockObjectId::MASTER_STATE2));
	EXPECT_FALSE(MockObject::deleted(MockObjectId::SUB_STATE));
	EXPECT_TRUE(e->isHandled);
}

/*
	before: MASTER_STATE2 - MASTER_STATE1 - CURRENT_STATE
			CURRENT_STATE returns SUB_STATE as next state.
	after : MASTER_STATE2 - SUB_STATE
*/
TEST_F(StateMachineSubStateUnitTest, exit_to_sub_state_1)
{
	MockSubState* subState = new MockSubState(masterState2, MockObjectId::SUB_STATE);

	EXPECT_CALL(*currentState, handleEvent(Ref(context), Ref(*e), Ref(*currentState), _))
		.WillOnce(DoAll(SetArgPointee<3>(subState), Return(S_OK)));
	EXPECT_CALL(*currentState, entry(_, _, _)).Times(0);
	EXPECT_CALL(*currentState, exit(Ref(context), Ref(*e), Ref(*subState))).Times(1);
	EXPECT_CALL(*masterState1, entry(_, _, _)).Times(0);
	EXPECT_CALL(*masterState1, exit(Ref(context), Ref(*e), Ref(*subState))).Times(1);
	EXPECT_CALL(*masterState2, entry(_, _, _)).Times(0);
	EXPECT_CALL(*masterState2, exit(_, _, _)).Times(0);
	EXPECT_CALL(*subState, entry(Ref(context), Ref(*e), Ref(*currentState))).Times(1);
	EXPECT_CALL(*subState, exit(_, _, _)).Times(0);

	ASSERT_HRESULT_SUCCEEDED(testee.handleEvent(*e));

	EXPECT_EQ(subState, context.getCurrentState());
	EXPECT_TRUE(MockObject::deleted(MockObjectId::CURRENT_STATE));
	EXPECT_TRUE(MockObject::deleted(MockObjectId::MASTER_STATE1));
	EXPECT_FALSE(MockObject::deleted(MockObjectId::MASTER_STATE2));
	EXPECT_FALSE(MockObject::deleted(MockObjectId::SUB_STATE));
	EXPECT_TRUE(e->isHandled);
}

/*
	before: MASTER_STATE2 - MASTER_STATE1 - CURRENT_STATE
			MASTER_STATE1 returns SUB_STATE as next state.
	after : MASTER_STATE2 - SUB_STATE
*/
TEST_F(StateMachineSubStateUnitTest, exit_to_sub_state_2)
{
	MockSubState* subState = new MockSubState(masterState2, MockObjectId::SUB_STATE);

	EXPECT_CALL(*currentState, handleEvent(Ref(context), Ref(*e), Ref(*currentState), _))
		.WillOnce(Return(State::S_EVENT_IGNORED));
	EXPECT_CALL(*currentState, handleIgnoredEvent(Ref(context), Ref(*e)))
		.WillOnce(Return(currentState->S_EVENT_IGNORED));
	EXPECT_CALL(*currentState, entry(_, _, _)).Times(0);
	EXPECT_CALL(*currentState, exit(Ref(context), Ref(*e), Ref(*subState))).Times(1);
	EXPECT_CALL(*masterState1, handleEvent(Ref(context), Ref(*e), Ref(*currentState), _))
		.WillOnce(DoAll(SetArgPointee<3>(subState), Return(S_OK)));
	EXPECT_CALL(*masterState1, entry(_, _, _)).Times(0);
	EXPECT_CALL(*masterState1, exit(Ref(context), Ref(*e), Ref(*subState))).Times(1);
	EXPECT_CALL(*masterState2, entry(_, _, _)).Times(0);
	EXPECT_CALL(*masterState2, exit(_, _, _)).Times(0);
	EXPECT_CALL(*subState, entry(Ref(context), Ref(*e), Ref(*currentState))).Times(1);
	EXPECT_CALL(*subState, exit(_, _, _)).Times(0);

	ASSERT_HRESULT_SUCCEEDED(testee.handleEvent(*e));

	EXPECT_EQ(subState, context.getCurrentState());
	EXPECT_TRUE(MockObject::deleted(MockObjectId::CURRENT_STATE));
	EXPECT_TRUE(MockObject::deleted(MockObjectId::MASTER_STATE1));
	EXPECT_FALSE(MockObject::deleted(MockObjectId::MASTER_STATE2));
	EXPECT_FALSE(MockObject::deleted(MockObjectId::SUB_STATE));
	EXPECT_TRUE(e->isHandled);
}

TEST_F(StateMachineSubStateUnitTest, event_ignored)
{
	State::S_EVENT_IGNORED = 16;

	EXPECT_CALL(*currentState, handleEvent(Ref(context), Ref(*e), Ref(*currentState), _))
		.WillOnce(Return(currentState->eventIsIgnored()));
	EXPECT_CALL(*currentState, handleIgnoredEvent(Ref(context), Ref(*e)))
		.WillOnce(Return(currentState->eventIsIgnored()));
	EXPECT_CALL(*currentState, entry(_, _, _)).Times(0);
	EXPECT_CALL(*currentState, exit(_, _, _)).Times(0);
	EXPECT_CALL(*masterState1, handleEvent(Ref(context), Ref(*e), Ref(*currentState), _))
		.WillOnce(Return(masterState1->eventIsIgnored()));
	EXPECT_CALL(*masterState1, handleIgnoredEvent(Ref(context), Ref(*e)))
		.WillOnce(Return(masterState1->eventIsIgnored()));
	EXPECT_CALL(*masterState1, entry(_, _, _)).Times(0);
	EXPECT_CALL(*masterState1, exit(_, _, _)).Times(0);
	EXPECT_CALL(*masterState2, handleEvent(Ref(context), Ref(*e), Ref(*currentState), _))
		.WillOnce(Return(masterState2->eventIsIgnored()));
	EXPECT_CALL(*masterState2, handleIgnoredEvent(Ref(context), Ref(*e)))
		.WillOnce(Return(masterState2->eventIsIgnored()));
	EXPECT_CALL(*masterState2, entry(_, _, _)).Times(0);
	EXPECT_CALL(*masterState2, exit(_, _, _)).Times(0);

	ASSERT_EQ(State::S_EVENT_IGNORED, testee.handleEvent(*e));

	EXPECT_EQ(currentState, context.getCurrentState());
	EXPECT_FALSE(MockObject::deleted(MockObjectId::CURRENT_STATE));
	EXPECT_FALSE(MockObject::deleted(MockObjectId::MASTER_STATE1));
	EXPECT_FALSE(MockObject::deleted(MockObjectId::MASTER_STATE2));
	EXPECT_FALSE(e->isHandled);
}
