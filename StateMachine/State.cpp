#include "stdafx.h"
#include "State.h"

using namespace state_machine;

// Pointer to dummy State object that does not match actual State object nor nullptr.
/*static*/ const State* State::RETURN_TO_MASTER = (State*)&State::RETURN_TO_MASTER;

State::State(State* previousState /*= nullptr*/, bool isSubState /*= false*/)
	: m_masterState(isSubState ? previousState : nullptr)
{
}


State::~State()
{
}
