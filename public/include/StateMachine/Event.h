#pragma once

#include "Object.h"

namespace state_machine {

class Context;
class ContextHandle;

class Event : public Object
{
public:
	Event(Context* context = nullptr);
	virtual ~Event();
	virtual INLINE log4cplus::LogLevel getLogLevel() const { return log4cplus::INFO_LOG_LEVEL; }

	// Returns this pointer as user event type.
	template<class T>
	INLINE T* cast() const { return dynamic_cast<T*>(this); }

	// Returns context as user context type.
	template<class T = Context>
	INLINE T* getContext() const { return dynamic_cast<T*>(m_context); }

	// true if the event is handled by the State::handleEvent().
	// This value is set to true by state machine,
	// when State::handleEvent() returns other than S_EVENT_IGNORED or state transition occurs.
	// State class could set this to true in order to
	// suppress calling it's handleIgnoredEvent() and handleEvent() of master state.
	// But setting false affect nothing.
	bool isHandled;

private:
	// Derived class(User event) can not modify members of this class.
	friend class ContextHandle;
	Context* m_context;
};

} // namespace state_machine
