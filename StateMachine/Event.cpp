#include "stdafx.h"
#include "Event.h"

using namespace state_machine;

Event::Event(Context* context /*= nullptr*/)
	: context(context)
{
}


Event::~Event()
{
}
