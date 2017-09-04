#include "stdafx.h"
#include "TestUtils/Mocks.h"

#include <StateMachine/Context.h>

using namespace state_machine;
using namespace testing;

class ContextBaseUnitTest : public Test
{
public:
	class ContextTestee : public Context {};
	class AsyncContextTestee : public AsyncContext {};

	std::unique_ptr<Context> testee;
};

// Context::start(State*, Event*) is not implemented.
// Object passed by pointer should be deleted.
TEST_F(ContextBaseUnitTest, Context_start_notimpl)
{
	testee.reset(new ContextTestee());

	auto state(new MockState(MockObjectId::CURRENT_STATE));
	auto e(new MockEvent(MockObjectId::EVENT));

	ASSERT_EQ(E_NOTIMPL, testee->start(state, e));

	EXPECT_TRUE(MockObject::deleted(MockObjectId::CURRENT_STATE));
	EXPECT_TRUE(MockObject::deleted(MockObjectId::EVENT));
}

// AsyncContext::start(State*, Event&) is not implemented.
// Object passed by pointer should be deleted.
TEST_F(ContextBaseUnitTest, AsyncContext_start_notimpl)
{
	testee.reset(new AsyncContextTestee());

	auto state(new MockState(MockObjectId::CURRENT_STATE));
	MockEvent e;

	ASSERT_EQ(E_NOTIMPL, testee->start(state, e));

	EXPECT_TRUE(MockObject::deleted(MockObjectId::CURRENT_STATE));
}

class ContextUnitTest : public Test
{
public:
	class Testee : public Context
	{
	public:
	};

	Testee testee;
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
	auto state = new MockState(MockObjectId::CURRENT_STATE);

	EXPECT_CALL(*state, handleEvent(_, _, _)).Times(0);
	EXPECT_CALL(*state, entry(Property(&Event::getContext<Testee>, Eq(&testee)), _)).Times(1);
	EXPECT_CALL(*state, exit(_, _)).Times(0);

	ASSERT_HRESULT_SUCCEEDED(testee.start(state));

	EXPECT_TRUE(testee.isStarted());
	EXPECT_EQ(state, testee.getCurrentState());
	EXPECT_FALSE(MockObject::deleted(MockObjectId::CURRENT_STATE));

	ASSERT_HRESULT_SUCCEEDED(testee.stop());

	EXPECT_FALSE(testee.isStarted());
	EXPECT_EQ(nullptr, testee.getCurrentState());
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
	auto state = new MockState(MockObjectId::CURRENT_STATE);

	EXPECT_CALL(*state, handleEvent(_, _, _)).Times(0);
	EXPECT_CALL(*state, entry(Ref(e), _)).Times(1);
	EXPECT_CALL(*state, exit(_, _)).Times(0);

	ASSERT_HRESULT_SUCCEEDED(testee.start(state, e));

	EXPECT_EQ(&testee, e.getContext());
	EXPECT_TRUE(testee.isStarted());
	EXPECT_EQ(state, testee.getCurrentState());
	EXPECT_FALSE(MockObject::deleted(MockObjectId::CURRENT_STATE));

	ASSERT_HRESULT_SUCCEEDED(testee.stop());

	EXPECT_FALSE(testee.isStarted());
	EXPECT_EQ(nullptr, testee.getCurrentState());
	EXPECT_TRUE(MockObject::deleted(MockObjectId::CURRENT_STATE));
}

TEST_F(ContextUnitTest, start_null_initialState)
{
	EXPECT_EQ(E_POINTER, testee.start(nullptr));

	EXPECT_FALSE(testee.isStarted());
	EXPECT_EQ(nullptr, testee.getCurrentState());
}

TEST_F(ContextUnitTest, start_on_started)
{
	auto state = new MockState(MockObjectId::CURRENT_STATE);
	auto state1 = new MockState(MockObjectId::NEXT_STATE);

	EXPECT_CALL(*state, entry(_, _)).Times(1);

	EXPECT_HRESULT_SUCCEEDED(testee.start(state));

	EXPECT_TRUE(testee.isStarted());
	EXPECT_EQ(state, testee.getCurrentState());

	EXPECT_EQ(E_ILLEGAL_METHOD_CALL, testee.start(state1));

	EXPECT_FALSE(MockObject::deleted(MockObjectId::CURRENT_STATE));
	EXPECT_TRUE(MockObject::deleted(MockObjectId::NEXT_STATE));
}

class ContextHandleEventUnitTest : public ContextUnitTest
{
public:
	void SetUp() {
		state = new MockState(MockObjectId::CURRENT_STATE);
		EXPECT_CALL(*state, entry(_, _)).Times(1);
		e.logLevel = log4cplus::INFO_LOG_LEVEL;
		ASSERT_HRESULT_SUCCEEDED(testee.start(state, e));
	}
	void TearDown() {
		ASSERT_HRESULT_SUCCEEDED(testee.stop());
	}

	MockState* state;
};

TEST_F(ContextHandleEventUnitTest, recursive_call_check)
{
	EXPECT_CALL(*state, handleEvent(Ref(e), Ref(*state), _))
		.WillOnce(Invoke([this](Event& _e, State&, State**)
	{
		EXPECT_TRUE(testee.isEventHandling());

		// Call Context::handleEvent() in State::handleEvent().
		EXPECT_EQ(E_ILLEGAL_METHOD_CALL, testee.handleEvent(_e));
		return S_OK;
	}));

	ASSERT_HRESULT_SUCCEEDED(testee.handleEvent(e));
	EXPECT_FALSE(testee.isEventHandling());

	EXPECT_EQ(state, testee.getCurrentState());
	EXPECT_FALSE(MockObject::deleted(MockObjectId::CURRENT_STATE));
	EXPECT_TRUE(e.isHandled);
}

class MultiContextHandleEventUnitTest : public ContextHandleEventUnitTest
{
public:
	void SetUp() {
		ContextHandleEventUnitTest::SetUp();

		state1 = new MockState(MockObjectId::OTHER_STATE);
		EXPECT_CALL(*state1, entry(_, _)).Times(1);
		ASSERT_HRESULT_SUCCEEDED(testee1.start(state1));
	}
	void TearDown() {
		ASSERT_HRESULT_SUCCEEDED(testee1.stop());

		ContextHandleEventUnitTest::TearDown();
	}

	Testee testee1;
	MockState* state1;
};

TEST_F(MultiContextHandleEventUnitTest, recursive_call_check)
{
	EXPECT_CALL(*state, handleEvent(Ref(e), Ref(*state), _))
		.WillOnce(Invoke([this](Event& _e, State&, State**)
		{
			EXPECT_TRUE(testee.isEventHandling());

			// Call another Context::handleEvent() in State::handleEvent().
			EXPECT_HRESULT_SUCCEEDED(testee1.handleEvent(_e));
			return S_OK;
		}));
	EXPECT_CALL(*state1, handleEvent(Ref(e), Ref(*state1), _))
		.WillOnce(Return(S_OK));

	ASSERT_HRESULT_SUCCEEDED(testee.handleEvent(e));
	EXPECT_FALSE(testee.isEventHandling());

	EXPECT_EQ(state, testee.getCurrentState());
	EXPECT_EQ(state1, testee1.getCurrentState());
	EXPECT_FALSE(MockObject::deleted(MockObjectId::CURRENT_STATE));
	EXPECT_FALSE(MockObject::deleted(MockObjectId::OTHER_STATE));
	EXPECT_TRUE(e.isHandled);
}
