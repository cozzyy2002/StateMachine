#pragma once

#include <memory>

namespace state_machine {

class Context;

class Event
{
public:
	Event();
	virtual ~Event();

	virtual LPCTSTR toString() const { return _T("Event"); }

	Context* context;
};

} // namespace state_machine
