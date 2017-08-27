#include "stdafx_local.h"
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

Event::LogLevel state_machine::Event::getLogLevel() const
{
	return log4cplus::INFO_LOG_LEVEL;
}
