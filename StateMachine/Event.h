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

protected:
	// Implementation of Object::getObject()
	virtual const Object* getObject() const { return this; }
};

} // namespace state_machine
