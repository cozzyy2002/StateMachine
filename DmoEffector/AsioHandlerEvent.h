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

		Event type is declared in devived class as `static const Types Type` member.

		Note: Do not release returned object,
		      because this method does cast, not QueryInterface() nor AddRef().
	 */
	template<class T>
	HRESULT cast(const T** ppEvent) const {
		if (!ppEvent) return E_POINTER;
		*ppEvent = dynamic_cast<const T*>(this);
		HR_ASSERT(*ppEvent, E_INVALIDARG);			// dynamic_cast failed.
		HR_ASSERT_OK(checkType<T>());				// Type mismatch.
		return S_OK;
	}

	/**
		Check Event type
		Call this method if cast() method is not been called.
	*/
	template<class T>
	HRESULT checkType() const { return (this->type == T::Type) ? S_OK : E_NOINTERFACE; }

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

/**
	Template base class for events.
 */
template<CAsioHandlerEvent::Types::Values _type, bool _isUserEvent>
class EventBase : public CAsioHandlerEvent
{
public:
	EventBase() : CAsioHandlerEvent(_type, _isUserEvent) {}

	static const Types::Values Type = _type;
};

typedef EventBase<CAsioHandlerEvent::Types::Shutdown, true> ShutdownEvent;
typedef EventBase<CAsioHandlerEvent::Types::Start, true> StartEvent;
typedef EventBase<CAsioHandlerEvent::Types::Stop, true> StopEvent;
typedef EventBase<CAsioHandlerEvent::Types::AsioResetRequest, false> AsioResetRequestEvent;
typedef EventBase<CAsioHandlerEvent::Types::AsioResyncRequest, false> AsioResyncRequestEvent;
typedef EventBase<CAsioHandlerEvent::Types::AsioLatenciesChanged, false> AsioLatenciesChangedEvent;

class SetupEvent : public EventBase<CAsioHandlerEvent::Types::Setup, true>
{
public:
	SetupEvent(IASIO* asio, HWND hwnd, int numChannels)
		: EventBase()
		, asio(asio), hwnd(hwnd), numChannels(numChannels) {}

	CComPtr<IASIO> asio;
	const HWND hwnd;
	const int numChannels;
};

class DataEvent : public EventBase<CAsioHandlerEvent::Types::Data, false>
{
public:
	DataEvent(const ASIOTime * params, long doubleBufferIndex)
		: EventBase()
		, params(*params), doubleBufferIndex(doubleBufferIndex) {}

	const ASIOTime params;
	const long doubleBufferIndex;
};
