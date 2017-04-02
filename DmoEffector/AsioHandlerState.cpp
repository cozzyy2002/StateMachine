#include "stdafx.h"
#include "AsioHandlerState.h"
#include "AsioHandler.h"

static log4cplus::Logger logger = log4cplus::Logger::getInstance(_T("AsioHandler.State"));

CAsioHandlerState::CAsioHandlerState(CAsioHandlerState* previousState)
	: m_asioHandler(previousState ? previousState->m_asioHandler : NULL)
{
}

HRESULT CAsioHandlerState::onUnexpectedEvent(CAsioHandlerEvent * event)
{
	LOG4CPLUS_FATAL(logger, "Unexpected event " << event->toString() << " in state " << this->toString());
	return E_UNEXPECTED;
}

/*static*/ CAsioHandlerState * CAsioHandlerState::createInitialState(CAsioHandler* asioHandler)
{
	return new NotInitializedState(asioHandler);
}

CAsioHandlerState::~CAsioHandlerState()
{
}

HRESULT NotInitializedState::handleEvent(CAsioHandlerEvent * event, CAsioHandlerState ** nextState)
{
	switch (event->type) {
	case CAsioHandlerEvent::Types::Setup:
		{
			SetupEvent* setupEvent = dynamic_cast<SetupEvent*>(event);
			HR_ASSERT_OK(setup(setupEvent));
		}
		break;
	default:
		return onUnexpectedEvent(event);
	}
	return S_OK;
}

HRESULT NotInitializedState::setup(SetupEvent * event)
{
	// Create IASIO object and initialize it.
	m_asioHandler->m_asio.Release();
	event->pAsioDriver->create(&m_asioHandler->m_asio);
	IASIO* asio = m_asioHandler->m_asio;
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
	m_asioHandler->m_numChannels = numChannels;

	// Initialize all ASIOBufferInfo prior to calling IASIO::createBuffers().
	// Buffers are prepared for each in/out, channel and double buffer index 0/1
	forInChannels([this](long channel, ASIOBufferInfo&in, ASIOBufferInfo&out) {
		in.isInput = ASIOTrue;
		out.isInput = ASIOFalse;
		in.channelNum = out.channelNum = channel;
		in.buffers[0] = in.buffers[1] = out.buffers[0] = out.buffers[1] = NULL;
		return S_OK;
	});

	m_asioHandler->m_driverInfo.isOutputReadySupported = (asio->outputReady() == ASE_OK);

	// Create buffers for input and output.
	long minSize, maxSize, preferredSize, granularity;
	ASIO_ASSERT_OK(asio->getBufferSize(&minSize, &maxSize, &preferredSize, &granularity));
	m_asioHandler->m_bufferSize = preferredSize;
	ASIO_ASSERT_OK(asio->createBuffers(m_asioHandler->m_asioBufferInfos.get(), numChannels * 2, m_asioHandler->m_bufferSize, &m_callbacks));
	LOG4CPLUS_INFO(logger, "Created buffers: " << numChannels << "channels, Prepared buffer size=" << m_asioHandler->m_bufferSize);

	// Set 0 to all buffers.
	forInChannels([this](long channel, ASIOBufferInfo&in, ASIOBufferInfo&out) {
		ZeroMemory(in.buffers[0], m_bufferSize);
		ZeroMemory(in.buffers[1], m_bufferSize);
		ZeroMemory(out.buffers[0], m_bufferSize);
		ZeroMemory(out.buffers[1], m_bufferSize);

		return S_OK;
	});

	return S_OK;
}
