#pragma once

#include "Object.h"

namespace state_machine {

class Context;

class Event : public Object
{
public:
	Event(Context* context = nullptr);
	virtual ~Event();
	virtual log4cplus::LogLevel getLogLevel() const { return log4cplus::INFO_LOG_LEVEL; }

	// Returns this pointer as user event type.
	template<class T>
	T* cast() const { return dynamic_cast<T*>(this); }

	// Returns context as user context type.
	template<class T = Context>
	T* getContext() const { return dynamic_cast<T*>(context); }

private:
	// Derived class(User event) can not modify members of this class.
	friend class Context;

	Context* context;
};

} // namespace state_machine
