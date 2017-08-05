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
		NEXT_STATE_ID
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
