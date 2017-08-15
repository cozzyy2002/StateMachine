#include "stdafx.h"
#include "State.h"
#include "Handles.h"

using namespace state_machine;

/*static*/ HRESULT State::S_EVENT_IGNORED = S_FALSE + 1;

StateHandle::StateHandle(bool isSubState)
	: m_isSubState(isSubState)
{
}

StateHandle::~StateHandle()
{
}

State::State(bool isSubState /*= false*/)
	: m_hState(new StateHandle(isSubState))
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

State* State::getRawMasterState() const
{
	return m_hState->getMasterState();
}

#pragma endregion
