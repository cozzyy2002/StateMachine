#include "stdafx.h"
#include "State.h"

using namespace state_machine;

// Pointer to dummy State object that does not match actual State object nor nullptr.
/*static*/ State* State::RETURN_TO_MASTER = (State*)&State::RETURN_TO_MASTER;

State::State(State* previousState /*= nullptr*/, bool isSubState /*= false*/)
	: m_masterState(isSubState ? previousState : nullptr)
	, m_entryCalled(false)
{
}


State::~State()
{
}
