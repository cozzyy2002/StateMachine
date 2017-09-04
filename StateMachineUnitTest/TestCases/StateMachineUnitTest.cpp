#include "stdafx.h"
#include "TestUtils/Mocks.h"
#include "TestUtils/TestStateMachine.h"

#include <StateMachine/Context.h>

using namespace state_machine;
using namespace testing;

class TestContext : public Context
{
public:
	TestContext() : Context() {}
};

class StateMacineUnitTest : public Test
{
public:
	typedef TestStateMachine Testee;

	StateMacineUnitTest()
		: context(new TestContext())
		, e(new MockEvent(*context)) {}

	void SetUp() {
		currentState = new MockState(MockObjectId::CURRENT_STATE);
		testee.setNextState(*context, currentState);
	}
	void TearDown() {
		context.reset();
	}

	Testee testee;
	std::unique_ptr<TestContext> context;
	MockState* currentState;
	std::unique_ptr<MockEvent> e;
};

TEST_F(StateMacineUnitTest, no_transition)
{
	EXPECT_CALL(*currentState, handleEvent(Ref(*e), Ref(*currentState), _))
		.WillOnce(Return(S_OK));
	EXPECT_CALL(*currentState, entry(_, _)).Times(0);
	EXPECT_CALL(*currentState, exit(_, _)).Times(0);

	ASSERT_HRESULT_SUCCEEDED(testee.handleEvent(*e));

	EXPECT_EQ(currentState, context->getCurrentState());
	EXPECT_FALSE(MockObject::deleted(MockObjectId::CURRENT_STATE));
	EXPECT_TRUE(e->isHandled);
}

TEST_F(StateMacineUnitTest, no_transition_error)
{
	EXPECT_CALL(*currentState, handleEvent(Ref(*e), Ref(*currentState), _))
		.WillOnce(Return(E_NOTIMPL));
	EXPECT_CALL(*currentState, handleError(Ref(*e), E_NOTIMPL))
		.WillOnce(Invoke([this](Event& e, HRESULT hr)
		{
			// Call default error handler.
			 return currentState->State::handleError(e, hr);
		}));
	EXPECT_CALL(*currentState, entry(_, _)).Times(0);
	EXPECT_CALL(*currentState, exit(_, _)).Times(0);

	ASSERT_EQ(E_NOTIMPL, testee.handleEvent(*e));

	EXPECT_EQ(currentState, context->getCurrentState());
	EXPECT_FALSE(MockObject::deleted(MockObjectId::CURRENT_STATE));
	EXPECT_TRUE(e->isHandled);
}

TEST_F(StateMacineUnitTest, transition)
{
	auto nextState = new MockState(MockObjectId::NEXT_STATE);
	EXPECT_CALL(*currentState, handleEvent(Ref(*e), Ref(*currentState), _))
		.WillOnce(DoAll(SetArgPointee<2>(nextState), Return(S_OK)));
	EXPECT_CALL(*currentState, entry(_, _)).Times(0);
	EXPECT_CALL(*currentState, exit(Ref(*e), Ref(*nextState))).Times(1);
	EXPECT_CALL(*nextState, entry(Ref(*e), Ref(*currentState))).Times(1);
	EXPECT_CALL(*nextState, exit(_, _)).Times(0);

	ASSERT_HRESULT_SUCCEEDED(testee.handleEvent(*e));

	EXPECT_EQ(nextState, context->getCurrentState());
	EXPECT_TRUE(MockObject::deleted(MockObjectId::CURRENT_STATE));
	EXPECT_FALSE(MockObject::deleted(MockObjectId::NEXT_STATE));
	EXPECT_TRUE(e->isHandled);
}

TEST_F(StateMacineUnitTest, transition_error)
{
	auto nextState = new MockState(MockObjectId::NEXT_STATE);
	EXPECT_CALL(*currentState, handleEvent(Ref(*e), Ref(*currentState), _))
		.WillOnce(DoAll(SetArgPointee<2>(nextState), Return(E_NOTIMPL)));
	EXPECT_CALL(*currentState, handleError(Ref(*e), E_NOTIMPL))
		.WillOnce(Invoke([this](Event& e, HRESULT hr)
		{
			// Call default error handler.
			 return currentState->State::handleError(e, hr);
		}));
	EXPECT_CALL(*currentState, entry(_, _)).Times(0);
	EXPECT_CALL(*currentState, exit(Ref(*e), Ref(*nextState))).Times(0);
	EXPECT_CALL(*nextState, entry(Ref(*e), Ref(*currentState))).Times(0);
	EXPECT_CALL(*nextState, exit(_, _)).Times(0);

	ASSERT_EQ(E_NOTIMPL, testee.handleEvent(*e));

	EXPECT_EQ(currentState, context->getCurrentState());
	EXPECT_FALSE(MockObject::deleted(MockObjectId::CURRENT_STATE));
	EXPECT_TRUE(MockObject::deleted(MockObjectId::NEXT_STATE));
	EXPECT_TRUE(e->isHandled);
}

class StateMacineSubStateUnitTest : public StateMacineUnitTest
{
public:
	void SetUp() {
		StateMacineUnitTest::SetUp();

		// Clear current state to change to sub state.
		testee.clearCurrentState(*context);
		currentState = new MockSubState(MockObjectId::CURRENT_STATE);
		masterState1 = new MockSubState(MockObjectId::MASTER_STATE1);
		masterState2 = new MockState(MockObjectId::MASTER_STATE2);
		testee.setNextState(*context, masterState2);
		testee.setNextState(*context, masterState1);
		testee.setNextState(*context, currentState);
		otherState = new MockState(MockObjectId::OTHER_STATE);

		// Check master state setter/getter of State class.
		ASSERT_EQ(masterState1, currentState->getMasterState<MockSubState>());
		ASSERT_EQ(masterState2, masterState1->getMasterState<MockState>());
	}
	void TearDown() {
		StateMacineUnitTest::TearDown();
	}

	MockSubState* currentState;	// Sub state(Different from currentState ot base class).
	MockSubState* masterState1;	// Master of current state
	MockState* masterState2;	// Master of master1 state
	MockState* otherState;
};

TEST_F(StateMacineSubStateUnitTest, back_to_parent1)
{
	EXPECT_CALL(*currentState, handleEvent(Ref(*e), Ref(*currentState), _))
		.WillOnce(DoAll(SetArgPointee<2>(currentState->backToMaster()), Return(S_OK)));
	EXPECT_CALL(*currentState, entry(_, _)).Times(0);
	EXPECT_CALL(*currentState, exit(_, _)).Times(1);
	EXPECT_CALL(*masterState1, entry(_, _)).Times(0);
	EXPECT_CALL(*masterState1, exit(_, _)).Times(0);
	EXPECT_CALL(*masterState2, entry(_, _)).Times(0);
	EXPECT_CALL(*masterState2, exit(_, _)).Times(0);

	ASSERT_HRESULT_SUCCEEDED(testee.handleEvent(*e));

	EXPECT_EQ(masterState1, context->getCurrentState());
	EXPECT_TRUE(MockObject::deleted(MockObjectId::CURRENT_STATE));
	EXPECT_FALSE(MockObject::deleted(MockObjectId::MASTER_STATE1));
	EXPECT_FALSE(MockObject::deleted(MockObjectId::MASTER_STATE2));
	EXPECT_TRUE(e->isHandled);
}

TEST_F(StateMacineSubStateUnitTest, back_to_parent2)
{
	EXPECT_CALL(*currentState, handleEvent(Ref(*e), Ref(*currentState), _))
		.WillOnce(DoAll(SetArgPointee<2>(masterState2), Return(S_OK)));
	EXPECT_CALL(*currentState, entry(_, _)).Times(0);
	EXPECT_CALL(*currentState, exit(_, _)).Times(1);
	EXPECT_CALL(*masterState1, entry(_, _)).Times(0);
	EXPECT_CALL(*masterState1, exit(_, _)).Times(1);
	EXPECT_CALL(*masterState2, entry(_, _)).Times(0);
	EXPECT_CALL(*masterState2, exit(_, _)).Times(0);

	ASSERT_HRESULT_SUCCEEDED(testee.handleEvent(*e));

	EXPECT_EQ(masterState2, context->getCurrentState());
	EXPECT_TRUE(MockObject::deleted(MockObjectId::CURRENT_STATE));
	EXPECT_TRUE(MockObject::deleted(MockObjectId::MASTER_STATE1));
	EXPECT_FALSE(MockObject::deleted(MockObjectId::MASTER_STATE2));
	EXPECT_TRUE(e->isHandled);
}

TEST_F(StateMacineSubStateUnitTest, exit_to_other_state)
{
	EXPECT_CALL(*currentState, handleEvent(Ref(*e), Ref(*currentState), _))
		.WillOnce(DoAll(SetArgPointee<2>(otherState), Return(S_OK)));
	EXPECT_CALL(*currentState, entry(_, _)).Times(0);
	EXPECT_CALL(*currentState, exit(Ref(*e), _)).Times(1);
	EXPECT_CALL(*masterState1, entry(_, _)).Times(0);
	EXPECT_CALL(*masterState1, exit(Ref(*e), _)).Times(1);
	EXPECT_CALL(*masterState2, entry(_, _)).Times(0);
	EXPECT_CALL(*masterState2, exit(Ref(*e), _)).Times(1);
	EXPECT_CALL(*otherState, entry(Ref(*e), _)).Times(1);
	EXPECT_CALL(*otherState, exit(_, _)).Times(0);

	ASSERT_HRESULT_SUCCEEDED(testee.handleEvent(*e));

	EXPECT_EQ(otherState, context->getCurrentState());
	EXPECT_TRUE(MockObject::deleted(MockObjectId::CURRENT_STATE));
	EXPECT_TRUE(MockObject::deleted(MockObjectId::MASTER_STATE1));
	EXPECT_TRUE(MockObject::deleted(MockObjectId::MASTER_STATE2));
	EXPECT_FALSE(MockObject::deleted(MockObjectId::OTHER_STATE));
	EXPECT_TRUE(e->isHandled);
}

TEST_F(StateMacineSubStateUnitTest, event_ignored)
{
	State::S_EVENT_IGNORED = 16;

	EXPECT_CALL(*currentState, handleEvent(Ref(*e), Ref(*currentState), _))
		.WillOnce(Return(currentState->eventIsIgnored()));
	EXPECT_CALL(*currentState, handleIgnoredEvent(Ref(*e)))
		.WillOnce(Return(currentState->eventIsIgnored()));
	EXPECT_CALL(*currentState, entry(_, _)).Times(0);
	EXPECT_CALL(*currentState, exit(_, _)).Times(0);
	EXPECT_CALL(*masterState1, handleEvent(Ref(*e), Ref(*currentState), _))
		.WillOnce(Return(masterState1->eventIsIgnored()));
	EXPECT_CALL(*masterState1, handleIgnoredEvent(Ref(*e)))
		.WillOnce(Return(masterState1->eventIsIgnored()));
	EXPECT_CALL(*masterState1, entry(_, _)).Times(0);
	EXPECT_CALL(*masterState1, exit(_, _)).Times(0);
	EXPECT_CALL(*masterState2, handleEvent(Ref(*e), Ref(*currentState), _))
		.WillOnce(Return(masterState2->eventIsIgnored()));
	EXPECT_CALL(*masterState2, handleIgnoredEvent(Ref(*e)))
		.WillOnce(Return(masterState2->eventIsIgnored()));
	EXPECT_CALL(*masterState2, entry(_, _)).Times(0);
	EXPECT_CALL(*masterState2, exit(_, _)).Times(0);

	ASSERT_EQ(State::S_EVENT_IGNORED, testee.handleEvent(*e));

	EXPECT_EQ(currentState, context->getCurrentState());
	EXPECT_FALSE(MockObject::deleted(MockObjectId::CURRENT_STATE));
	EXPECT_FALSE(MockObject::deleted(MockObjectId::MASTER_STATE1));
	EXPECT_FALSE(MockObject::deleted(MockObjectId::MASTER_STATE2));
	EXPECT_FALSE(e->isHandled);
}
