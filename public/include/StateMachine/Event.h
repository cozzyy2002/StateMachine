#pragma once

#include "Object.h"

#include <memory>

namespace state_machine {

class Context;
class ContextHandle;
class AsyncContextHandle;

class Event : public Object
{
public:
	// Queueing priority of async Context.
	// Application can not use value greater than Highest.
	enum class Priority {
		// Application use.
		Lowest,
		Lower,
		Normal,
		Higher,
		Highest,
		// Internal use.
		Internal,		// Application should use priority value less than this.
		StopContext,	// Terminates worker thread of async context.
	};

	Event(Priority priority = Priority::Normal);
	virtual ~Event();

	Priority getPriority() const { return priority; }
	using LogLevel = int;
	virtual LogLevel getLogLevel() const;

	// Returns this pointer as user event type.
	// Do NOT delete returned object.
	template<class T>
	INLINE T* cast() { return dynamic_cast<T*>(this); }

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
	friend class AsyncContextHandle;
	const Priority priority;
	bool isInternal;

	// Comparator used by sorting event queue.
	struct HigherPriority {
		using event_t = std::unique_ptr<Event>;
		bool operator()(const event_t& _Left, const event_t& _Right) const {
			return _Left->priority > _Right->priority;
		}
	};

	// Internal event is always legal.
	INLINE bool isLegalEvent() const { return isInternal || (priority < Priority::Internal); }
};

} // namespace state_machine
