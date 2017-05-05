#include "stdafx.h"
#include "AsioHandlerState.h"
#include "AsioDriver.h"

static log4cplus::Logger logger = log4cplus::Logger::getInstance(_T("AsioHandler.State"));

CAsioHandlerState::CAsioHandlerState(Types type, CAsioHandlerState* previousState)
	: type(type), context(previousState ? previousState->context : NULL)
{
}

/*static*/ CAsioHandlerState * CAsioHandlerState::createInitialState(CAsioHandlerContext* context)
{
	return new NotInitializedState(context);
}

CAsioHandlerState::~CAsioHandlerState()
{
}

/**
 * Handles events that should be handled in any state.
 *
 * handleEvent() method of each state calls this method when unrecognized event.
 */
HRESULT CAsioHandlerState::handleEvent(const CAsioHandlerEvent * event, CAsioHandlerState ** nextState)
{
	switch (event->type) {
	case CAsioHandlerEvent::Types::Shutdown:
		break;
	case CAsioHandlerEvent::Types::AsioResetRequest:
		HR_ASSERT_OK(event->checkType<AsioResetRequestEvent>());
		break;
	case CAsioHandlerEvent::Types::AsioResyncRequest:
		HR_ASSERT_OK(event->checkType<AsioResyncRequestEvent>());
		break;
	case CAsioHandlerEvent::Types::AsioLatenciesChanged:
		HR_ASSERT_OK(event->checkType<AsioLatenciesChangedEvent>());
		break;
	default:
		return handleUnexpectedEvent(event, nextState);
	}

	return S_OK;
}

HRESULT CAsioHandlerState::handleUnexpectedEvent(const CAsioHandlerEvent* event, CAsioHandlerState** nextState)
{
	LOG4CPLUS_FATAL(logger, "Unexpected event " << event->toString() << " in state " << this->toString());
	return E_UNEXPECTED;
}

NotInitializedState::NotInitializedState(CAsioHandlerContext * context)
	: CAsioHandlerState(Types::NotInitialized, NULL)
{
	HR_EXPECT(context, E_POINTER);

	this->context = context;
}

HRESULT NotInitializedState::handleEvent(const CAsioHandlerEvent * event, CAsioHandlerState ** nextState)
{
	switch (event->type) {
	case CAsioHandlerEvent::Types::Setup:
		{
			const SetupEvent* setupEvent;
			HR_ASSERT_OK(event->cast(&setupEvent));
			HR_ASSERT_OK(setup(setupEvent));
			*nextState = new StandbyState(this);
		}
		break;
	default:
		return handleUnexpectedEvent(event, nextState);
	}
	return S_OK;
}

HRESULT NotInitializedState::setup(const SetupEvent * event)
{
	// Initialize ASIO object.
	IASIO* asio = context->asio = event->asio;
	ASIO_ASSERT(asio->init(event->hwnd), E_ABORT);

	// Show name and version of ASIO driver created.
	char driverName[100];
	asio->getDriverName(driverName);
	LOG4CPLUS_INFO(logger, "Loaded '" << driverName << "' version=" << asio->getDriverVersion());

	// Get number of channels and assert that we have enough channels.
	int numChannels = event->numChannels;
	long numInputChannels, numOutputChannels;
	ASIO_ASSERT_OK(asio->getChannels(&numInputChannels, &numOutputChannels));
	LOG4CPLUS_INFO(logger, "Input " << numInputChannels << " channels, Output " << numOutputChannels << " channels");
	if (0 < numChannels) {
		HR_ASSERT((numChannels <= numInputChannels) && (numChannels <= numOutputChannels), E_INVALIDARG);
	} else {
		numChannels = min(numInputChannels, numOutputChannels);
	}
	context->numChannels = numChannels;

	// Initialize all ASIOBufferInfo prior to calling IASIO::createBuffers().
	// Buffers are prepared for each in/out, channel and double buffer index 0/1
	context->forInChannels([this](long channel, ASIOBufferInfo&in, ASIOBufferInfo&out) {
		in.isInput = ASIOTrue;
		out.isInput = ASIOFalse;
		in.channelNum = out.channelNum = channel;
		in.buffers[0] = in.buffers[1] = out.buffers[0] = out.buffers[1] = NULL;
		return S_OK;
	});

	context->driverInfo.isOutputReadySupported = (asio->outputReady() == ASE_OK);

	// Create buffers for input and output.
	long minSize, maxSize, preferredSize, granularity;
	ASIO_ASSERT_OK(asio->getBufferSize(&minSize, &maxSize, &preferredSize, &granularity));
	context->bufferSize = preferredSize;
	ASIO_ASSERT_OK(asio->createBuffers(context->asioBufferInfos.get(), numChannels * 2, context->bufferSize, context->getAsioCallbacks()));
	LOG4CPLUS_INFO(logger, "Created buffers: " << numChannels << "channels, Prepared buffer size=" << context->bufferSize);

	// Set 0 to all buffers.
	context->forInChannels([this](long channel, ASIOBufferInfo&in, ASIOBufferInfo&out) {
		ZeroMemory(in.buffers[0], context->bufferSize);
		ZeroMemory(in.buffers[1], context->bufferSize);
		ZeroMemory(out.buffers[0], context->bufferSize);
		ZeroMemory(out.buffers[1], context->bufferSize);

		return S_OK;
	});

	return S_OK;
}

HRESULT StandbyState::handleEvent(const CAsioHandlerEvent * event, CAsioHandlerState ** nextState)
{
	switch (event->type) {
	case CAsioHandlerEvent::Types::Start:
		HR_ASSERT_OK(event->checkType<StartEvent>());
		ASIO_ASSERT_OK(context->asio->start());
		*nextState = new RunningState(this);
		break;
	default:
		return CAsioHandlerState::handleEvent(event, nextState);
	}
	return S_OK;
}

HRESULT RunningState::handleEvent(const CAsioHandlerEvent * event, CAsioHandlerState ** nextState)
{
	switch (event->type) {
	case CAsioHandlerEvent::Types::Stop:
		HR_ASSERT_OK(event->checkType<StopEvent>());
		ASIO_ASSERT_OK(context->asio->stop());

		// TODO: Notify CAsioHandlerContext::Statistics to user.

		*nextState = new StandbyState(this);
		break;
	case CAsioHandlerEvent::Types::Data:
		{
			const DataEvent* ev;
			HR_ASSERT_OK(event->cast(&ev));
			HR_ASSERT_OK(handleData(ev->params, ev->doubleBufferIndex));
		}
		break;
	default:
		return CAsioHandlerState::handleEvent(event, nextState);
	}
	return S_OK;
}

HRESULT RunningState::handleData(const ASIOTime & params, long doubleBufferIndex)
{
	context->forInChannels([this, doubleBufferIndex](long /*channel*/, ASIOBufferInfo&in, ASIOBufferInfo&out) {
		CopyMemory(out.buffers[doubleBufferIndex], in.buffers[doubleBufferIndex], context->bufferSize);
		return S_OK;
	});

	// Notify the driver that output data is available if supported.
	if (context->driverInfo.isOutputReadySupported) {
		ASIO_EXPECT_OK(context->asio->outputReady());
	}

	return S_OK;
}
