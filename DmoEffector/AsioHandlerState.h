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

	virtual HRESULT handleEvent(const CAsioHandlerEvent* event, CAsioHandlerState** nextState);
	virtual HRESULT entry(const CAsioHandlerEvent* event, const CAsioHandlerState* previousState) { return S_OK; }
	virtual HRESULT exit(const CAsioHandlerEvent* event, const CAsioHandlerState* nextState) { return S_OK; }

	LPCTSTR toString() const { return type.toString(); }

	const Types type;

protected:
	CAsioHandlerState(Types type, CAsioHandlerState* previousState);

	// CAsioHandler object that holds context values.
	CAsioHandler* m_asioHandler;
};

class NotInitializedState : public CAsioHandlerState
{
public:
	NotInitializedState(CAsioHandler* asioHandler) : CAsioHandlerState(Types::NotInitialized, NULL) { m_asioHandler = asioHandler; }
	virtual HRESULT handleEvent(const CAsioHandlerEvent* event, CAsioHandlerState** nextState);

protected:
	HRESULT setup(const SetupEvent* event);
};

class StandbyState : public CAsioHandlerState
{
public:
	StandbyState(CAsioHandlerState* previousState) : CAsioHandlerState(Types::Standby, previousState) {}

	virtual HRESULT handleEvent(const CAsioHandlerEvent* event, CAsioHandlerState** nextState);
};

class RunningState : public CAsioHandlerState
{
public:
	RunningState(CAsioHandlerState* previousState) : CAsioHandlerState(Types::Running, previousState) {}

	virtual HRESULT handleEvent(const CAsioHandlerEvent* event, CAsioHandlerState** nextState);

protected:
	HRESULT handleData(const ASIOTime& params, long doubleBufferIndex);
};
