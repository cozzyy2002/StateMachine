#include "stdafx.h"
#include "Utils/Mocks.h"

#include <StateMachine/StateMachine.h>

using namespace state_machine;
using namespace testing;

class TestContext : public Context {};

class StateMacineStateUnitTest : public Test
{
public:
	class Testee : public StateMachine
	{
	public:
		using StateMachine::setCurrentState;
		using StateMachine::getCurrentState;
	};

	void SetUp() {
		currentState = new MockState(MockObjectId::CURRENT_STATE);
		context.reset(new TestContext());
		testee.setCurrentState(context.get(), currentState);
		e.context = context.get();
	}
	void TearDown() {
		context.reset();
		MockObject::clear();
	}

	std::unique_ptr<TestContext> context;
	Testee testee;
	MockState* currentState;
	MockEvent e;
};

TEST_F(StateMacineStateUnitTest, no_transition)
{
	EXPECT_CALL(*currentState, handleEvent(&e, currentState, _))
		.WillOnce(Return(S_OK));
	EXPECT_CALL(*currentState, entry(_, _)).Times(0);
	EXPECT_CALL(*currentState, exit(_, _)).Times(0);

	ASSERT_HRESULT_SUCCEEDED(testee.handleEvent(&e));

	EXPECT_EQ(currentState, testee.getCurrentState(context.get()));
	EXPECT_FALSE(MockObject::deleted(MockObjectId::CURRENT_STATE));
}

TEST_F(StateMacineStateUnitTest, no_transition_error)
{
	EXPECT_CALL(*currentState, handleEvent(&e, currentState, _))
		.WillOnce(Return(E_NOTIMPL));
	EXPECT_CALL(*currentState, handleError(&e, E_NOTIMPL))
		.WillOnce(Invoke([&](const Event* e, HRESULT hr)
		{
			// Call default error handler.
			return currentState->State::handleError(e, hr);
		}));
	EXPECT_CALL(*currentState, entry(_, _)).Times(0);
	EXPECT_CALL(*currentState, exit(_, _)).Times(0);

	ASSERT_EQ(E_NOTIMPL, testee.handleEvent(&e));

	EXPECT_EQ(currentState, testee.getCurrentState(context.get()));
	EXPECT_FALSE(MockObject::deleted(MockObjectId::CURRENT_STATE));
}

TEST_F(StateMacineStateUnitTest, transition)
{
	MockState* nextState = new MockState(MockObjectId::NEXT_STATE);
	EXPECT_CALL(*currentState, handleEvent(&e, currentState, _))
		.WillOnce(DoAll(SetArgPointee<2>(nextState), Return(S_OK)));
	EXPECT_CALL(*currentState, entry(_, _)).Times(0);
	EXPECT_CALL(*currentState, exit(&e, nextState)).Times(1);
	EXPECT_CALL(*nextState, entry(&e, currentState)).Times(1);
	EXPECT_CALL(*nextState, exit(_, _)).Times(0);

	ASSERT_HRESULT_SUCCEEDED(testee.handleEvent(&e));

	EXPECT_EQ(nextState, testee.getCurrentState(context.get()));
	EXPECT_TRUE(MockObject::deleted(MockObjectId::CURRENT_STATE));
	EXPECT_FALSE(MockObject::deleted(MockObjectId::NEXT_STATE));
}

TEST_F(StateMacineStateUnitTest, transition_error)
{
	MockState* nextState = new MockState(MockObjectId::NEXT_STATE);
	EXPECT_CALL(*currentState, handleEvent(&e, currentState, _))
		.WillOnce(DoAll(SetArgPointee<2>(nextState), Return(E_NOTIMPL)));
	EXPECT_CALL(*currentState, handleError(&e, E_NOTIMPL))
		.WillOnce(Invoke([&](const Event* e, HRESULT hr)
		{
			// Call default error handler.
			return currentState->State::handleError(e, hr);
		}));
	EXPECT_CALL(*currentState, entry(_, _)).Times(0);
	EXPECT_CALL(*currentState, exit(&e, nextState)).Times(0);
	EXPECT_CALL(*nextState, entry(&e, currentState)).Times(0);
	EXPECT_CALL(*nextState, exit(_, _)).Times(0);

	ASSERT_EQ(E_NOTIMPL, testee.handleEvent(&e));

	EXPECT_EQ(currentState, testee.getCurrentState(context.get()));
	EXPECT_FALSE(MockObject::deleted(MockObjectId::CURRENT_STATE));
	EXPECT_TRUE(MockObject::deleted(MockObjectId::NEXT_STATE));
}

class StateMacineSubStateUnitTest : public StateMacineStateUnitTest
{
public:
	void SetUp() {
		StateMacineStateUnitTest::SetUp();
		masterState1 = new MockState(MockObjectId::MASTER_STATE1);
		masterState2 = new MockState(MockObjectId::MASTER_STATE2);
		currentState->masterState().reset(masterState1);
		masterState1->masterState().reset(masterState2);
		otherState = new MockState(MockObjectId::OTHER_STATE);
	}
	void TearDown() {
		StateMacineStateUnitTest::TearDown();
	}

	MockState* masterState1;	// Master of sub state
	MockState* masterState2;	// Master of master1 state
	MockState* otherState;
};

TEST_F(StateMacineSubStateUnitTest, back_to_parent1)
{
	EXPECT_CALL(*currentState, handleEvent(&e, currentState, _))
		.WillOnce(DoAll(SetArgPointee<2>(currentState->backToMaster()), Return(S_OK)));
	EXPECT_CALL(*currentState, entry(_, _)).Times(0);
	EXPECT_CALL(*currentState, exit(_, _)).Times(1);
	EXPECT_CALL(*masterState1, entry(_, _)).Times(0);
	EXPECT_CALL(*masterState1, exit(_, _)).Times(0);
	EXPECT_CALL(*masterState2, entry(_, _)).Times(0);
	EXPECT_CALL(*masterState2, exit(_, _)).Times(0);

	ASSERT_HRESULT_SUCCEEDED(testee.handleEvent(&e));

	EXPECT_EQ(masterState1, testee.getCurrentState(context.get()));
	EXPECT_TRUE(MockObject::deleted(MockObjectId::CURRENT_STATE));
	EXPECT_FALSE(MockObject::deleted(MockObjectId::MASTER_STATE1));
	EXPECT_FALSE(MockObject::deleted(MockObjectId::MASTER_STATE2));
}

TEST_F(StateMacineSubStateUnitTest, back_to_parent2)
{
	EXPECT_CALL(*currentState, handleEvent(&e, currentState, _))
		.WillOnce(DoAll(SetArgPointee<2>(masterState2), Return(S_OK)));
	EXPECT_CALL(*currentState, entry(_, _)).Times(0);
	EXPECT_CALL(*currentState, exit(_, _)).Times(1);
	EXPECT_CALL(*masterState1, entry(_, _)).Times(0);
	EXPECT_CALL(*masterState1, exit(_, _)).Times(1);
	EXPECT_CALL(*masterState2, entry(_, _)).Times(0);
	EXPECT_CALL(*masterState2, exit(_, _)).Times(0);

	ASSERT_HRESULT_SUCCEEDED(testee.handleEvent(&e));

	EXPECT_EQ(masterState2, testee.getCurrentState(context.get()));
	EXPECT_TRUE(MockObject::deleted(MockObjectId::CURRENT_STATE));
	EXPECT_TRUE(MockObject::deleted(MockObjectId::MASTER_STATE1));
	EXPECT_FALSE(MockObject::deleted(MockObjectId::MASTER_STATE2));
}
