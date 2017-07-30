#pragma once

namespace state_machine {

class Event
{
public:
	Event();
	virtual ~Event();

	virtual LPCTSTR toString() = 0;
};

} // namespace state_machine
