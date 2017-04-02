#pragma once

#include "AsioHandlerEvent.h"

#include <win32/Enum.h>

class CAsioHandlerState
{
	DISALLOW_COPY_AND_ASSIGN(CAsioHandlerState);

public:
	ENUM(Types, NotInitialized, Standby, Running);

public:
	virtual ~CAsioHandlerState();

	virtual HRESULT handleEvent(const CAsioHandlerEvent* event, CAsioHandlerState** nextState) = 0;
	virtual HRESULT entry(const CAsioHandlerEvent* event, const CAsioHandlerState* previousState) { return S_OK; }
	virtual HRESULT exit(const CAsioHandlerEvent* event, const CAsioHandlerState* nextState) { return S_OK; }

	LPCTSTR toString() const { return type.toString(); }

	const Types type;

protected:
	CAsioHandlerState();
};
