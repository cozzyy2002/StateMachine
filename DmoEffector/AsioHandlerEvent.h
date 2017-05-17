#pragma once

#include <guiddef.h>

class CAsioDriver;

ENUM(EventTypes,
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

MIDL_INTERFACE("2A8782E9-2869-442B-9EEC-DDE68415B6D2")
CAsioHandlerEvent : public IUnknown, public CUnknownImpl
{
	DISALLOW_COPY_AND_ASSIGN(CAsioHandlerEvent);

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
		HR_ASSERT(*ppEvent, E_INVALIDARG);					// dynamic_cast failed.
		HR_ASSERT(this->type == T::Type, E_NOINTERFACE);	// Type mismatch.
		return S_OK;
	}

	virtual LPCTSTR toString() const { return type.toString(); }

	const EventTypes type;

	// Indicates where the event is caused by user.
	// If true, user is notified error occurred in handling state.
	const bool isUserEvent;

	IUNKNOWN_METHODS;

protected:
	/**
		Protected constructor.

		Public constructor is exposed by EventBase template class.
		Actual event classes are derived from EventBase template class.
	 */
	CAsioHandlerEvent(EventTypes type, bool isUserEvent);

	IUNKNOWN_INTERFACES(QITABENT(CAsioHandlerEvent, CAsioHandlerEvent));
};

/**
	Template base class for events.
 */
template<EventTypes::Values _type, bool _isUserEvent>
class EventBase : public CAsioHandlerEvent
{
public:
	EventBase() : CAsioHandlerEvent(_type, _isUserEvent) {}

	// Associated event type used by CAsioHandlerEvent::cast() method.
	static const EventTypes::Values Type = _type;
};

typedef EventBase<EventTypes::Shutdown, true> ShutdownEvent;
typedef EventBase<EventTypes::Start, true> StartEvent;
typedef EventBase<EventTypes::Stop, true> StopEvent;
typedef EventBase<EventTypes::AsioResetRequest, false> AsioResetRequestEvent;
typedef EventBase<EventTypes::AsioResyncRequest, false> AsioResyncRequestEvent;
typedef EventBase<EventTypes::AsioLatenciesChanged, false> AsioLatenciesChangedEvent;

class SetupEvent : public EventBase<EventTypes::Setup, true>
{
public:
	SetupEvent(IASIO* asio, HWND hwnd, int numChannels)
		: EventBase()
		, asio(asio), hwnd(hwnd), numChannels(numChannels) {}

	CComPtr<IASIO> asio;
	const HWND hwnd;
	const int numChannels;
};

class DataEvent : public EventBase<EventTypes::Data, false>
{
public:
	DataEvent(const ASIOTime * params, long doubleBufferIndex)
		: EventBase()
		, params(*params), doubleBufferIndex(doubleBufferIndex) {}

	const ASIOTime params;
	const long doubleBufferIndex;
};
