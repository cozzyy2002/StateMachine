#include "stdafx.h"
#include "TestUtils/Mocks.h"
#include "../StateMachine/Handles.h"
#include <StateMachine/Context.h>

using namespace state_machine;
using namespace testing;

class StateUnitTest : public Test
{
public:
	class Testee : public State, public MockObject
	{
	public:
		Testee() : State(), MockObject(MockObjectId::MASTER_STATE) {}
		Testee(Testee* testee)
			: State(true), MockObject(MockObjectId::SUB_STATE) {
			getHandle<SubStateHandle>()->m_masterState.reset(testee);
		}

		using State::backToMaster;
	};

	void SetUp() {
		masterState = new Testee();
		subState = new Testee(masterState);
	}
	void TearDown() {
		// Note: masterState is deleted along with subState.
		delete subState;
	}

	Testee* masterState;
	Testee* subState;
};

TEST_F(StateUnitTest, master_state)
{
	EXPECT_FALSE(masterState->isSubState());
	EXPECT_NE(masterState->getHandle<StateHandle>(), nullptr);
	EXPECT_EQ(masterState->getHandle<SubStateHandle>(), nullptr);
	EXPECT_EQ(masterState->getMasterState(), nullptr);
	EXPECT_EQ(masterState->backToMaster(), nullptr);
}

TEST_F(StateUnitTest, sub_state)
{
	EXPECT_TRUE(subState->isSubState());
	EXPECT_NE(subState->getHandle<StateHandle>(), nullptr);
	EXPECT_NE(subState->getHandle<SubStateHandle>(), nullptr);
	EXPECT_EQ(subState->getMasterState(), masterState);
	EXPECT_EQ(subState->backToMaster(), masterState);
}
