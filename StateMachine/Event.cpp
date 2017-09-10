#include "stdafx_local.h"
#include <StateMachine/Event.h>

#include "Handles.h"

using namespace state_machine;

Event::Event(Priority priority /*= Priority::Normal*/)
	: m_context(nullptr)
	, isHandled(false)
	, priority(priority), isInternal(false)
{
}

Event::Event(Context& context, Priority priority /*= Priority::Normal*/)
	: m_context(&context)
	, isHandled(false)
	, priority(priority), isInternal(false)
{
}

Event::~Event()
{
}

Event::LogLevel state_machine::Event::getLogLevel() const
{
	return log4cplus::INFO_LOG_LEVEL;
}
