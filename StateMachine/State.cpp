#include "stdafx.h"
#include "State.h"

using namespace state_machine;

State::State(State* previousState /*= nullptr*/, bool isSubState /*= false*/)
	: m_masterState(isSubState ? previousState : nullptr)
	, m_entryCalled(false)
{
}

State::~State()
{
}

State * state_machine::State::backToMaster()
{
	HR_EXPECT(isSubState(), E_UNEXPECTED);
	return m_masterState.get();
}
