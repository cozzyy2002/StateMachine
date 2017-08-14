#include "stdafx.h"
#include "State.h"
#include "Handles.h"

using namespace state_machine;

StateHandle::StateHandle(State* previousState, bool isSubState /*= false*/)
	: m_masterState(isSubState ? previousState : nullptr)
{
}

StateHandle::~StateHandle()
{
}

State::State(State* previousState /*= nullptr*/, bool isSubState /*= false*/)
	: m_hState(new StateHandle(previousState, isSubState))
{
}

State::~State()
{
	delete m_hState;
}

State * StateHandle::backToMaster()
{
	HR_EXPECT(isSubState(), E_UNEXPECTED);
	return m_masterState.get();
}

#pragma region Context methods which invode ContextHandle methods.

bool State::isSubState() const
{
	return m_hState->isSubState();
}

State * State::backToMaster()
{
	return m_hState->backToMaster();
}

HRESULT State::eventIsIgnored() const
{
	return m_hState->eventIsIgnored();
}

State* State::getRawMasterState() const
{
	return m_hState->getMasterState();
}

#pragma endregion
