#include "stdafx.h"
#include "Utils/Mocks.h"

#include <StateMachine/StateMachine.h>

using namespace state_machine;
using namespace testing;

class StateMacineStateUnitTest : public Test
{
public:
	class Testee : public StateMachine
	{
	public:
		using StateMachine::m_currentState;
	};

	void SetUp() {
		currentState = new MockState(CURRENT_STATE_ID);
		testee.m_currentState.reset(currentState);
	}
	void TearDown() {
		testee.m_currentState.reset();
		MockObject::clear();
	}

	enum {
		EVENT_ID = 1,
		CURRENT_STATE_ID,
		NEXT_STATE_ID,
		MASTER_STATE1_ID,
		MASTER_STATE2_ID,
		OTHER_STATE_ID,
	};
	Testee testee;
	MockState* currentState;
};

TEST_F(StateMacineStateUnitTest, no_transition)
{
	MockEvent e;
	EXPECT_CALL(*currentState, handleEvent(&e, currentState, _))
		.WillOnce(Return(S_OK));
	EXPECT_CALL(*currentState, entry(_, _)).Times(0);
	EXPECT_CALL(*currentState, exit(_, _)).Times(0);

	ASSERT_HRESULT_SUCCEEDED(testee.handleEvent(&e));

	EXPECT_EQ(currentState, testee.m_currentState.get());
	EXPECT_FALSE(MockObject::deleted(CURRENT_STATE_ID));
}

TEST_F(StateMacineStateUnitTest, no_transition_error)
{
	MockEvent e;
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

	EXPECT_EQ(currentState, testee.m_currentState.get());
	EXPECT_FALSE(MockObject::deleted(CURRENT_STATE_ID));
}

TEST_F(StateMacineStateUnitTest, transition)
{
	MockEvent e;
	MockState* nextState = new MockState(NEXT_STATE_ID);
	EXPECT_CALL(*currentState, handleEvent(&e, currentState, _))
		.WillOnce(DoAll(SetArgPointee<2>(nextState), Return(S_OK)));
	EXPECT_CALL(*currentState, entry(_, _)).Times(0);
	EXPECT_CALL(*currentState, exit(&e, nextState)).Times(1);
	EXPECT_CALL(*nextState, entry(&e, currentState)).Times(1);
	EXPECT_CALL(*nextState, exit(_, _)).Times(0);

	ASSERT_HRESULT_SUCCEEDED(testee.handleEvent(&e));

	EXPECT_EQ(nextState, testee.m_currentState.get());
	EXPECT_TRUE(MockObject::deleted(CURRENT_STATE_ID));
	EXPECT_FALSE(MockObject::deleted(NEXT_STATE_ID));
}

TEST_F(StateMacineStateUnitTest, transition_error)
{
	MockEvent e;
	MockState* nextState = new MockState(NEXT_STATE_ID);
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

	EXPECT_EQ(currentState, testee.m_currentState.get());
	EXPECT_FALSE(MockObject::deleted(CURRENT_STATE_ID));
	EXPECT_FALSE(MockObject::deleted(NEXT_STATE_ID));
}

class StateMacineSubStateUnitTest : public StateMacineStateUnitTest
{
public:
	void SetUp() {
		StateMacineStateUnitTest::SetUp();
		masterState1 = new MockState(MASTER_STATE1_ID);
		masterState2 = new MockState(MASTER_STATE2_ID);
		std::shared_ptr<State> m1(masterState1), m2(masterState2);
		currentState->setMasterState(m1);
		masterState1->setMasterState(m2);
		otherState = new MockState(OTHER_STATE_ID);
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
	MockEvent e;

	EXPECT_CALL(*currentState, handleEvent(&e, currentState, _))
		.WillOnce(DoAll(SetArgPointee<2>(State::RETURN_TO_MASTER), Return(S_OK)));
	EXPECT_CALL(*currentState, entry(_, _)).Times(0);
	EXPECT_CALL(*currentState, exit(_, _)).Times(1);
	EXPECT_CALL(*masterState1, entry(_, _)).Times(0);
	EXPECT_CALL(*masterState1, exit(_, _)).Times(0);
	EXPECT_CALL(*masterState2, entry(_, _)).Times(0);
	EXPECT_CALL(*masterState2, exit(_, _)).Times(0);

	masterState1->setEntryCalled(true);
	ASSERT_HRESULT_SUCCEEDED(testee.handleEvent(&e));

	EXPECT_EQ(masterState1, testee.m_currentState.get());
	EXPECT_TRUE(MockObject::deleted(CURRENT_STATE_ID));
	EXPECT_FALSE(MockObject::deleted(MASTER_STATE1_ID));
	EXPECT_FALSE(MockObject::deleted(MASTER_STATE2_ID));
}
