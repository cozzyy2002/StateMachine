#pragma once

#include "AsioHandlerEvent.h"

#include <win32/Enum.h>

class CAsioHandler;

class CAsioHandlerState
{
	DISALLOW_COPY_AND_ASSIGN(CAsioHandlerState);

public:
	ENUM(Types, NotInitialized, Standby, Running);

public:
	static CAsioHandlerState* createInitialState(CAsioHandler* asioHandler);

	virtual ~CAsioHandlerState();

	virtual HRESULT handleEvent(CAsioHandlerEvent* event, CAsioHandlerState** nextState) = 0;
	virtual HRESULT entry(CAsioHandlerEvent* event, const CAsioHandlerState* previousState) { return S_OK; }
	virtual HRESULT exit(CAsioHandlerEvent* event, const CAsioHandlerState* nextState) { return S_OK; }

	LPCTSTR toString() const { return type.toString(); }

	const Types type;

protected:
	CAsioHandlerState(CAsioHandlerState* previousState);

	HRESULT onUnexpectedEvent(CAsioHandlerEvent* event);

	// CAsioHandler object that holds context values.
	CAsioHandler* m_asioHandler;
};

class NotInitializedState : public CAsioHandlerState
{
public:
	NotInitializedState(CAsioHandler* asioHandler) : CAsioHandlerState(NULL) { m_asioHandler = asioHandler; }
	virtual HRESULT handleEvent(CAsioHandlerEvent* event, CAsioHandlerState** nextState);

protected:
	HRESULT setup(SetupEvent* event);
};
