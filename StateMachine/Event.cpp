#include <StateMachine/stdafx.h>
#include <StateMachine/Event.h>

#include "Handles.h"

using namespace state_machine;

Event::Event()
	: m_context(nullptr)
	, isHandled(false)
{
}

Event::Event(Context& context)
	: m_context(&context)
	, isHandled(false)
{
}

Event::~Event()
{
}
