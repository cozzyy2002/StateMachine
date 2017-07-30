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

	static const int CURRENT_STATE_ID = 1;
	Testee testee;
	MockState* currentState;
};

TEST_F(StateMacineStateUnitTest, a)
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

TEST_F(StateMacineStateUnitTest, b)
{
	MockEvent e;
	MockState* nextState = new MockState();
	EXPECT_CALL(*currentState, handleEvent(&e, currentState, _))
		.WillOnce(DoAll(SetArgPointee<2>(nextState), Return(S_OK)));
	EXPECT_CALL(*currentState, entry(_, _)).Times(0);
	EXPECT_CALL(*currentState, exit(&e, nextState)).Times(1);
	EXPECT_CALL(*nextState, entry(&e, currentState)).Times(1);
	EXPECT_CALL(*nextState, exit(_, _)).Times(0);

	ASSERT_HRESULT_SUCCEEDED(testee.handleEvent(&e));

	EXPECT_EQ(nextState, testee.m_currentState.get());
	EXPECT_TRUE(MockObject::deleted(CURRENT_STATE_ID));
}
