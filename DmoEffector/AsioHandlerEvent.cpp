#include "stdafx.h"
#include "AsioHandlerEvent.h"

CAsioHandlerEvent::CAsioHandlerEvent(EventTypes type, bool isUserEvent)
	: type(type), isUserEvent(isUserEvent)
{
}


CAsioHandlerEvent::~CAsioHandlerEvent()
{
}
