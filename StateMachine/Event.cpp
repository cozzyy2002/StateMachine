#include "stdafx.h"
#include "Event.h"
#include "Handles.h"

using namespace state_machine;

Event::Event(Context* context /*= nullptr*/)
	: m_context(context)
{
}

Event::~Event()
{
}
