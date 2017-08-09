#pragma once

#include "Object.h"
#include <memory>

namespace state_machine {

class Context;

class Event : public Object
{
public:
	Event();
	virtual ~Event();
	virtual log4cplus::LogLevel getLogLevel() const { return log4cplus::INFO_LOG_LEVEL; }

	Context* context;
};

} // namespace state_machine
