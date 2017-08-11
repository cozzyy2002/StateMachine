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

	// true if the event is handled by the methods of State class.
	// State machine doesn't depend on this value, it just sets this value.
	// This value is set to true by state machine,
	// when State::handleEvent() returns other than S_EVENT_IGNORED.
	bool isHandled;

private:
	// Derived class(User event) can not modify members of this class.
	friend class Context;

	Context* context;
};

} // namespace state_machine
