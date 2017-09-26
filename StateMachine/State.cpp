#include "stdafx_local.h"
#include <StateMachine/State.h>

#include "Handles.h"

using namespace state_machine;

/*static*/ HRESULT State::S_EVENT_IGNORED = S_FALSE + 1;

State::State(State* masterState /*= nullptr*/)
{
	if(!masterState) {
		m_hState.reset(new StateHandle());
	} else {
		m_hState.reset(new SubStateHandle(masterState));
	}
}

State::~State()
{
}

bool State::isSubState() const
{
	return m_hState->isSubState();
}

#pragma region State/SubState methods which invode ContextHandle methods.

State* State::backToMaster()
{
	return m_hState->backToMaster();
}

State* State::getRawMasterState() const
{
	return m_hState->getRawMasterState();
}

#pragma endregion
