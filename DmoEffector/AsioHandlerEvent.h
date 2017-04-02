#pragma once

#include <guiddef.h>

MIDL_INTERFACE("2A8782E9-2869-442B-9EEC-DDE68415B6D2")
CAsioHandlerEvent : public IUnknown, public CUnknownImpl
{
	DISALLOW_COPY_AND_ASSIGN(CAsioHandlerEvent);

public:
	ENUM(Types, Setup, Shutdown, Start, Stop, Data);

public:
	CAsioHandlerEvent();
	virtual ~CAsioHandlerEvent();

	LPCTSTR toString() const { return type.toString(); }

	const Types type;

	IUNKNOWN_METHODS;

protected:
	IUNKNOWN_INTERFACES(QITABENT(CAsioHandlerEvent, CAsioHandlerEvent));
};
