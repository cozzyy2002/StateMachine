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
	// Calling Context::queueEvent() with event that has priority other than Normal
	// causes sorting event queue by priority.
	enum class Priority {
		Normal,
		Higher,
		Highest,
		// Internal use.
		Internal,		// Application should use priority value less than this.
		StopContext,	// Terminates worker thread of async context.
	};

	Event(Priority priority = Priority::Normal);
	Event(Context& context, Priority priority = Priority::Normal);
	virtual ~Event();
	typedef int LogLevel;
	virtual LogLevel getLogLevel() const;

	// Returns this pointer as user event type.
	// Do NOT delete returned object.
	template<class T>
	INLINE T* cast() { return dynamic_cast<T*>(this); }

	// Returns context as user context type.
	// Do NOT delete returned object.
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
	friend class AsyncContextHandle;
	Context* m_context;
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
