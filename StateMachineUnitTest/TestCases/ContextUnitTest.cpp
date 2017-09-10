#include "stdafx.h"
#include "TestUtils/Mocks.h"

#include <StateMachine/Context.h>

using namespace state_machine;
using namespace testing;

class ContextBaseUnitTest : public Test
{
public:
	class Testee : public Context
	{
	public:
		Testee(bool isAsync) : Context(isAsync) {}
	};

	std::unique_ptr<Context> testee;
};

// Context::start(State*, Event*) is not implemented.
// Object passed by pointer should be deleted.
TEST_F(ContextBaseUnitTest, Context_start_notimpl)
{
	testee.reset(new Testee(false));

	auto state(new MockState(MockObjectId::CURRENT_STATE));
	auto e(new MockEvent(MockObjectId::EVENT));

	ASSERT_EQ(E_NOTIMPL, testee->start(state, e));

	EXPECT_TRUE(MockObject::deleted(MockObjectId::CURRENT_STATE));
	EXPECT_TRUE(MockObject::deleted(MockObjectId::EVENT));
}

// Context::queueEvent() is not implemented.
// Object passed by pointer should be deleted.
TEST_F(ContextBaseUnitTest, Context_queueEvent_notimpl)
{
	testee.reset(new Testee(false));

	auto state(new MockState(MockObjectId::CURRENT_STATE));
	MockEvent e;

	EXPECT_CALL(*state, entry(_, _)).WillOnce(Return(S_OK));
	ASSERT_HRESULT_SUCCEEDED(testee->start(state, e));

	ASSERT_EQ(E_NOTIMPL, testee->queueEvent(new MockEvent(MockObjectId::EVENT)));

	ASSERT_HRESULT_SUCCEEDED(testee->stop());
	EXPECT_TRUE(MockObject::deleted(MockObjectId::CURRENT_STATE));
	EXPECT_TRUE(MockObject::deleted(MockObjectId::EVENT));
}

// AsyncContext::start(State*, Event&) is not implemented.
// Object passed by pointer should be deleted.
TEST_F(ContextBaseUnitTest, AsyncContext_start_notimpl)
{
	testee.reset(new Testee(true));

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
		Testee() : Context(false) {}
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

class AsyncContextPriorityTest : public Test
{
public:
	class TestEvent : public Event
	{
	public:
		TestEvent(Priority priority, int sequence)
			: Event(priority), sequence(sequence) {}

		// Sequence number to be handled.
		const int sequence;

		virtual void modifyString(std::tstring& _string) override {
			TCHAR str[100];
			_stprintf_s(str, _T(": priority=%d, sequence=%d"), (int)getPriority(), sequence);
			_string += str;
		}
	};

	class Testee : public Context
	{
	public:
		Testee() : Context(true) {}
	};

	AsyncContextPriorityTest()
		: state(new MockState(MockObjectId::CURRENT_STATE)) {}

	void SetUp() {
		EXPECT_CALL(*state, entry(_, _)).WillOnce(Return(S_OK));
		ASSERT_HRESULT_SUCCEEDED(testee.start(state));
		Sleep(100);
		ASSERT_TRUE(testee.isStarted());
	}
	void TearDown() {
		Sleep(100);
		ASSERT_HRESULT_SUCCEEDED(testee.stop());
	}

	Testee testee;
	MockState* state;
	using P = Event::Priority;
};

TEST_F(AsyncContextPriorityTest, illegal_priority)
{
	ASSERT_EQ(E_INVALIDARG, testee.queueEvent(new Event(P::StopContext)));
}

// Events should be handled by it's priority order.
TEST_F(AsyncContextPriorityTest, priority_order)
{
	// Sequence of events are priority order.
	auto e0(new TestEvent(P::Highest, 0));
	auto e1(new TestEvent(P::Higher, 1));
	auto e2(new TestEvent(P::Normal, 3));	// Queued after e3 that has same priority.
	auto e3(new TestEvent(P::Normal, 2));
	auto e4(new TestEvent(P::Lower, 4));
	auto e5(new TestEvent(P::Lowest, 5));

	int sequence = 0;
	EXPECT_CALL(*state, handleEvent(_, _, _))
		.WillRepeatedly(Invoke([&sequence](Event& e, State&, State**)
		{
			auto _e(e.cast<TestEvent>());
			if(_e->sequence < 0) {
				// Dummy event
				Sleep(100);
			} else {
				// Test if TestEvent::sequence is right.
				EXPECT_EQ(sequence++, _e->sequence);
			}
			return S_OK;
		}));

	// Dummy event to wait for all events to be queued.
	ASSERT_HRESULT_SUCCEEDED(testee.queueEvent(new TestEvent(P::Highest, -1)));

	// Queue events reverse priority order.
	ASSERT_HRESULT_SUCCEEDED(testee.queueEvent(e5));
	ASSERT_HRESULT_SUCCEEDED(testee.queueEvent(e4));
	ASSERT_HRESULT_SUCCEEDED(testee.queueEvent(e3));
	ASSERT_HRESULT_SUCCEEDED(testee.queueEvent(e2));
	ASSERT_HRESULT_SUCCEEDED(testee.queueEvent(e1));
	ASSERT_HRESULT_SUCCEEDED(testee.queueEvent(e0));

	Sleep(3000);
	EXPECT_EQ(6, sequence);
}

// Events should not be handled.
// Context::stop() should be processed prior to other events.
TEST_F(AsyncContextPriorityTest, stop)
{
	auto e0(new TestEvent(P::Highest, 0));
	auto e1(new TestEvent(P::Higher, 1));

	EXPECT_CALL(*state, handleEvent(_, _, _))
		.WillRepeatedly(Invoke([](Event& e, State&, State**)
	{
		auto _e(e.cast<TestEvent>());
		if(_e->sequence < 0) {
			// Dummy event
			Sleep(100);
		} else {
			// Event should not be handled.
			ADD_FAILURE() << "Event handled: Priority=" << (int)_e->getPriority();
		}
		return S_OK;
	}));

	// Dummy event to wait for all events to be queued.
	ASSERT_HRESULT_SUCCEEDED(testee.queueEvent(new TestEvent(P::Highest, -1)));

	// Queue events reverse priority order.
	ASSERT_HRESULT_SUCCEEDED(testee.queueEvent(e1));
	ASSERT_HRESULT_SUCCEEDED(testee.queueEvent(e0));

	ASSERT_HRESULT_SUCCEEDED(testee.stop());

	Sleep(1000);
}
