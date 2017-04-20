#pragma once

#include <guiddef.h>

class CAsioDriver;

MIDL_INTERFACE("2A8782E9-2869-442B-9EEC-DDE68415B6D2")
CAsioHandlerEvent : public IUnknown, public CUnknownImpl
{
	DISALLOW_COPY_AND_ASSIGN(CAsioHandlerEvent);

public:
	ENUM(Types,
		Setup,						/// CAsioHandler::setup() method has been called by user.
		Shutdown,					/// CAsioHandler::shutdonw() method has been called by user.
		Start,						/// CAsioHandler::start() method has been called by user.
		Stop,						/// CAsioHandler::stop() method has been called by user.
		Data,						/// CAsioHandler::bufferSwitchTimeInfo() callback has been called by ASIO driver.
		AsioResetRequest,			/// ASIO driver requests a reset.
		//AsioBufferSizeChange,		/// ASIO buffer sizes will change, issued by the user. - Done by AsioRestRequest
		AsioResyncRequest,			/// ASIO driver detected underruns and requires a resynchronization.
		AsioLatenciesChanged		/// ASIO driver detected a latancy change.
	);

public:
	virtual ~CAsioHandlerEvent();

	/**
		Cast 'this' to derived class and check event type.
	 */
	template<class T>
	HRESULT cast(const Types& type, const T** ppEvent) const {
		if (!ppEvent) return E_POINTER;
		const T* obj = dynamic_cast<const T*>(this);
		if (obj && (obj->type == type)) {
			*ppEvent = obj;
			return S_OK;
		} else {
			return E_INVALIDARG;
		}
	}

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

class DataEvent : public InternalEvent
{
public:
	DataEvent(const ASIOTime * params, long doubleBufferIndex)
		: InternalEvent(Types::Data)
		, params(*params), doubleBufferIndex(doubleBufferIndex) {}

	const ASIOTime params;
	const long doubleBufferIndex;
};
