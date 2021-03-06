// StateMachineUnitTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "TestUtils/Mocks.h"
#include <log4cplus/configurator.h>

static log4cplus::Logger logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("Test.main"));

class TestEventListener : public testing::EmptyTestEventListener
{
public:
	virtual void OnTestCaseStart(const testing::TestCase& test_case) {
		LOG4CPLUS_INFO(logger, "==== " << test_case.name() << ": Test case start ====");
	}
	virtual void OnTestStart(const testing::TestInfo& test_info) {
		LOG4CPLUS_INFO(logger, "---- " << test_info.test_case_name() << "." << test_info.name() << ": Test start ----");
	}
	virtual void OnTestEnd(const testing::TestInfo& test_info) {
		MockObject::clear();

		LOG4CPLUS_INFO(logger, "---- " << test_info.test_case_name() << "." << test_info.name() << ": "
										<< (test_info.result()->Passed() ? "PASSED" : "FAILED") << " ----");
	}
	virtual void OnTestCaseEnd(const testing::TestCase& test_case) {
		LOG4CPLUS_INFO(logger, "==== " << test_case.name() << ": Test case end ====");
	}
};

int _tmain(int argc, TCHAR** argv)
{
	log4cplus::PropertyConfigurator::doConfigure(LOG4CPLUS_TEXT("log4cplus.properties"));

	testing::InitGoogleTest(&argc, argv);
	testing::InitGoogleMock(&argc, argv);
	testing::TestEventListeners& listeners = testing::UnitTest::GetInstance()->listeners();
	listeners.Append(new TestEventListener());

	return RUN_ALL_TESTS();
}
