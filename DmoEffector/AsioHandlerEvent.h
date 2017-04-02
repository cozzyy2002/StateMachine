#pragma once

#include <guiddef.h>

class CAsioDriver;

MIDL_INTERFACE("2A8782E9-2869-442B-9EEC-DDE68415B6D2")
CAsioHandlerEvent : public IUnknown, public CUnknownImpl
{
	DISALLOW_COPY_AND_ASSIGN(CAsioHandlerEvent);

public:
	ENUM(Types, Setup, Shutdown, Start, Stop, Data);

public:
	virtual ~CAsioHandlerEvent();

	LPCTSTR toString() const { return type.toString(); }

	const Types type;

	// Indicates where the event is caused by user.
	// If true, user is notified error occurred in handling state.
	const bool isUserEvent;

	IUNKNOWN_METHODS;

protected:
	CAsioHandlerEvent(Types type, bool isUserEvent);

	IUNKNOWN_INTERFACES(QITABENT(CAsioHandlerEvent, CAsioHandlerEvent));
};

class InternalEvent : public CAsioHandlerEvent
{
public:
	InternalEvent(Types type) : CAsioHandlerEvent(type, false) {}
};

class UserEvent : public CAsioHandlerEvent
{
public:
	UserEvent(Types type) : CAsioHandlerEvent(type, true) {}
};

class SetupEvent : public UserEvent
{
public:
	SetupEvent(const CAsioDriver* pAsioDriver, HWND hwnd, int numChannels)
		: UserEvent(Types::Setup)
		, pAsioDriver(pAsioDriver), hwnd(hwnd), numChannels(numChannels) {}

	const CAsioDriver* pAsioDriver;
	const HWND hwnd;
	const int numChannels;
};