#include <StateMachine/stdafx.h>
#include <StateMachine/Event.h>

#include "Handles.h"

using namespace state_machine;

Event::Event(Context* context /*= nullptr*/)
	: m_context(context)
	, isHandled(false)
{
}

Event::~Event()
{
}
