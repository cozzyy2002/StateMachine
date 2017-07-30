// StateMachineUnitTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

int main(int argc, TCHAR** argv)
{
	testing::InitGoogleTest(&argc, argv);
	testing::InitGoogleMock(&argc, argv);

	return RUN_ALL_TESTS();
}
