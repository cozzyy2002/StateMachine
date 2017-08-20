#include <StateMachine/stdafx.h>
#include <StateMachine/State.h>

#include "Handles.h"

using namespace state_machine;

/*static*/ HRESULT State::S_EVENT_IGNORED = S_FALSE + 1;

State::State()
{
	if(!isSubState()) {
		m_hState.reset(new StateHandle());
	}
}

State::~State()
{
}

SubState::SubState()
{
	m_hState.reset(new SubStateHandle());
}

SubState::~SubState()
{
}

State* SubStateHandle::backToMaster()
{
	return m_masterState.get();
}

#pragma region State/SubState methods which invode ContextHandle methods.

State* SubState::backToMaster()
{
	return getHandle<SubStateHandle>()->backToMaster();
}

State* SubState::getRawMasterState() const
{
	return getHandle<SubStateHandle>()->getMasterState();
}

#pragma endregion
