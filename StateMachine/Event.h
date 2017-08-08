#pragma once

#include <memory>

namespace state_machine {

class Context;

class Event
{
public:
	Event();
	virtual ~Event();

	virtual LPCTSTR toString() const = 0;

	Context* context;
};

} // namespace state_machine
