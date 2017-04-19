#pragma once

#include "AsioHandlerEvent.h"
#include "AsioHandlerContext.h"

#include <win32/Enum.h>

class CAsioHandlerState
{
	DISALLOW_COPY_AND_ASSIGN(CAsioHandlerState);

public:
	ENUM(Types, NotInitialized, Standby, Running);

public:
	static CAsioHandlerState* createInitialState(CAsioHandlerContext* context);

	virtual ~CAsioHandlerState();

	virtual HRESULT handleEvent(const CAsioHandlerEvent* event, CAsioHandlerState** nextState);
	virtual HRESULT entry(const CAsioHandlerEvent* event, const CAsioHandlerState* previousState) { return S_OK; }
	virtual HRESULT exit(const CAsioHandlerEvent* event, const CAsioHandlerState* nextState) { return S_OK; }
	HRESULT handleUnexpectedEvent(const CAsioHandlerEvent* event, CAsioHandlerState** nextState);

	LPCTSTR toString() const { return type.toString(); }

	const Types type;

protected:
	CAsioHandlerState(Types type, CAsioHandlerState* previousState);

	// CAsioHandler object that holds context values.
	CAsioHandlerContext* context;
};

class NotInitializedState : public CAsioHandlerState
{
public:
	NotInitializedState(CAsioHandlerContext* context);
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
