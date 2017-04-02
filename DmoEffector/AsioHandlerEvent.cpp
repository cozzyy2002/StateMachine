#include "stdafx.h"
#include "AsioHandlerEvent.h"

CAsioHandlerEvent::CAsioHandlerEvent(Types type, bool isUserEvent)
	: type(type), isUserEvent(isUserEvent)
{
}


CAsioHandlerEvent::~CAsioHandlerEvent()
{
}
