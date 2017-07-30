#pragma once

namespace state_machine {

class Event
{
public:
	Event();
	virtual ~Event();

	virtual LPCTSTR toString() const = 0;
};

} // namespace state_machine
