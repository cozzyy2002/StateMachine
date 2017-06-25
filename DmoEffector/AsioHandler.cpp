#include "stdafx.h"
#include "AsioHandler.h"
#include "AsioDriver.h"

static log4cplus::Logger logger = log4cplus::Logger::getInstance(_T("AsioHandler"));

CAsioHandler* CAsioHandler::m_instance = NULL;

/*
	Protected constructor.

	Call getInstance() static method to create or get CAsioHandler object.
*/
CAsioHandler::CAsioHandler(int numChannels)
	: CAsioHandlerContext(numChannels), m_workQueueId(0)
{
	// Allocate buffer infos for channel * 2(in and out).
	asioBufferInfos.reset(new ASIOBufferInfo[numChannels * 2]);

	ZeroMemory(&statistics, sizeof(statistics));
	ZeroMemory(&driverInfo, sizeof(driverInfo));

	m_instance = this;
}


CAsioHandler::~CAsioHandler()
{
	HR_EXPECT_OK(this->shutdown());

	m_instance = NULL;
}

/*
	Returns singleton CAioHandler object.
*/
CAsioHandler * CAsioHandler::getInstance(int numChannels /*= 0*/)
{
	return (m_instance) ? m_instance : new CAsioHandler(numChannels);
}

HRESULT CAsioHandler::setup(IASIO* asio, HWND hwnd)
{
	HR_ASSERT(asio, E_POINTER);
	HR_ASSERT_OK(MFStartup(MF_VERSION));

	this->asio = asio;

	// Create work queue and initial state object.
	HR_ASSERT_OK(MFAllocateWorkQueue(&m_workQueueId));
	m_currentState.reset(CAsioHandlerState::createInitialState(this));

	// Trigger setup event.
	CComPtr<CAsioHandlerEvent> event(new SetupEvent(asio, hwnd, numChannels));
	return triggerEvent(event);
}
/*
	Shutdown the ASIO driver.

	Note: This method processes until the end of all steps
		  even if one or more steps are failed.
*/
HRESULT CAsioHandler::shutdown()
{
	// shutdown() method could not be called while running.
	// Call stop() prior this method.
	HR_ASSERT(m_state != State::Running, E_ILLEGAL_METHOD_CALL);

	// Returning S_FALSE means that this method has done nothing.
	HRESULT hr = S_FALSE;

	if (m_workQueueId) {
		// Shutdown and wait for state to shutdown.
		CComPtr<CAsioHandlerEvent> event(new ShutdownEvent());
		HR_ASSERT_OK(triggerEvent(event));
		WIN32_EXPECT(WAIT_OBJECT_0 == WaitForSingleObject(shutDownEvent, 100));

		ASIO_EXPECT_OK(MFUnlockWorkQueue(m_workQueueId));
		HR_EXPECT_OK(MFShutdown());
		m_workQueueId = 0;
	}

	if (asio) {
		hr = ASIO_EXPECT_OK(asio->disposeBuffers());
		asio.Release();
	}

	m_state = State::NotLoaded;
	return hr;
}

HRESULT CAsioHandler::start()
{
	CComPtr<CAsioHandlerEvent> event(new StartEvent());
	return triggerEvent(event);
}

HRESULT CAsioHandler::stop()
{
	CComPtr<CAsioHandlerEvent> event(new StopEvent());
	return triggerEvent(event);
}

HRESULT CAsioHandler::triggerEvent(CAsioHandlerEvent * event)
{
	HR_ASSERT_OK(MFPutWorkItem(m_workQueueId, this, event));
	return S_OK;
}

// Set result of exp to hr1 unless hr1 is error.
#define HR_PRESERVE_ERROR(hr1, exp) do { HRESULT hr2 = HR_EXPECT_OK(exp); if(SUCCEEDED(hr1)) hr1 = hr2; } while(false)

HRESULT CAsioHandler::handleEvent(const CAsioHandlerEvent* event)
{
	CAsioHandlerState* nextState = NULL;
	HRESULT hr = HR_EXPECT_OK(m_currentState->handleEvent(event, &nextState));
	if (nextState) {
		// State transition
		HR_PRESERVE_ERROR(hr, m_currentState->exit(event, nextState));
		HR_PRESERVE_ERROR(hr, nextState->entry(event, m_currentState.get()));
		m_currentState.reset(nextState);
	}

	if (FAILED(hr) && event->isUserEvent) {
		// Failed to handle user event.
		// TODO: notify error to user.
	}

	if (event->type == EventTypes::Shutdown) {
		// Notify main thread that evnt handling completed.
		WIN32_EXPECT(SetEvent(shutDownEvent));
	}

	return S_OK;
}

/*
	Implementation of IMFAsyncCallback::GetParameters().
 */
HRESULT STDMETHODCALLTYPE CAsioHandler::GetParameters(DWORD *pdwFlags, DWORD *pdwQueue)
{
	return E_NOTIMPL;
}

/*
	Implementation of IMFAsyncCallback::Invoke().

	Calls handleEvent() method with CAsioEvent object.
	CAsioEvent object is obtained from state of IMFAsyncResult.
*/
HRESULT STDMETHODCALLTYPE CAsioHandler::Invoke(IMFAsyncResult *pAsyncResult)
{
	CComPtr<IUnknown> unkState;
	HR_ASSERT_OK(pAsyncResult->GetState(&unkState));
	CComPtr<CAsioHandlerEvent> event;
	HR_ASSERT_OK(unkState->QueryInterface(&event));
	return handleEvent(event);
}

void CAsioHandler::bufferSwitch(long doubleBufferIndex, ASIOBool directProcess)
{
	if (FAILED(HR_EXPECT(asio, E_ILLEGAL_METHOD_CALL))) return;

	ASIOTime time;
	ZeroMemory(&time, sizeof(time));
	HRESULT hr = ASIO_EXPECT_OK(asio->getSamplePosition(&time.timeInfo.samplePosition, &time.timeInfo.systemTime));
	if (SUCCEEDED(hr)) {
		time.timeInfo.flags = kSystemTimeValid | kSamplePositionValid;
	}

	bufferSwitchTimeInfo(&time, doubleBufferIndex, directProcess);
}

ASIOTime * CAsioHandler::bufferSwitchTimeInfo(ASIOTime * params, long doubleBufferIndex, ASIOBool directProcess)
{
	statistics.bufferSwitch[doubleBufferIndex]++;

	CComPtr<CAsioHandlerEvent> event(new DataEvent(params, doubleBufferIndex));
	HR_EXPECT_OK(triggerEvent(event));
	return nullptr;
}

void CAsioHandler::sampleRateDidChange(ASIOSampleRate sRate)
{
	if (FAILED(HR_EXPECT(asio, E_ILLEGAL_METHOD_CALL))) return;

}

long CAsioHandler::asioMessage(long selector, long value, void * message, double * opt)
{
	if (FAILED(HR_EXPECT(asio, E_ILLEGAL_METHOD_CALL))) return 0;

	long ret = ASIOFalse;
	CComPtr<CAsioHandlerEvent> event;
	LPCSTR strSelector = "UNKNOWN";

#define CASE(x) case x: strSelector=#x;

	switch (selector) {
	CASE(kAsioSelectorSupported)
		switch (value) {
		case kAsioEngineVersion:
		case kAsioResetRequest:
		case kAsioResyncRequest:
		case kAsioLatenciesChanged:
		case kAsioSupportsTimeInfo:
			ret = ASIOTrue;
			break;
		}
		break;
	CASE(kAsioEngineVersion)
		ret = 2;
		break;
	CASE(kAsioResetRequest)
		event = new AsioResetRequestEvent();
		break;
	CASE(kAsioBufferSizeChange) break;
	CASE(kAsioResyncRequest)
		event = new AsioResyncRequestEvent();
		break;
	CASE(kAsioLatenciesChanged)
		event = new AsioLatenciesChangedEvent();
		break;
	CASE(kAsioSupportsTimeInfo)
		// Tell the driver that we support bufferSwitchTimeInfo() callback.
		ret = ASIOTrue;
		break;
	CASE(kAsioSupportsTimeCode) break;
	CASE(kAsioMMCCommand) break;
	CASE(kAsioSupportsInputMonitor) break;
	CASE(kAsioSupportsInputGain) break;
	CASE(kAsioSupportsInputMeter) break;
	CASE(kAsioSupportsOutputGain) break;
	CASE(kAsioSupportsOutputMeter) break;
	CASE(kAsioOverload) break;
	}

	if (event) {
		if (SUCCEEDED(HR_EXPECT_OK(triggerEvent(event)))) {
			ret = ASIOTrue;
		}
	}

	LOG4CPLUS_INFO(logger, __FUNCTION__ "(" << strSelector << ":" << selector << ",value=" << value << ") returned " << ret);
	return ret;
}

void CAsioHandler::s_bufferSwitch(long doubleBufferIndex, ASIOBool directProcess)
{
	if (m_instance) m_instance->bufferSwitch(doubleBufferIndex, directProcess);
	else LOG4CPLUS_WARN(logger, "No instance: " __FUNCTION__ "(" << doubleBufferIndex << "," << directProcess << ")");
}

ASIOTime * CAsioHandler::s_bufferSwitchTimeInfo(ASIOTime * params, long doubleBufferIndex, ASIOBool directProcess)
{
	ASIOTime* ret = params;
	if(m_instance) ret = m_instance->bufferSwitchTimeInfo(params, doubleBufferIndex, directProcess);
	else LOG4CPLUS_WARN(logger, "No instance: " __FUNCTION__ "(" << doubleBufferIndex << "," << directProcess << ")");
	return ret;
}

void CAsioHandler::s_sampleRateDidChange(ASIOSampleRate sRate)
{
	if (m_instance) m_instance->sampleRateDidChange(sRate);
	else LOG4CPLUS_WARN(logger, "No instance: " __FUNCTION__ "(" << sRate << ")");
}

long CAsioHandler::s_asioMessage(long selector, long value, void * message, double * opt)
{
	long ret = 0;
	if(m_instance) ret = m_instance->asioMessage(selector, value, message, opt);
	else LOG4CPLUS_WARN(logger, "No instance: " __FUNCTION__ "(" << selector << "," << value << ",...)");
	return ret;
}

ASIOCallbacks CAsioHandler::m_callbacks = {
	s_bufferSwitch, s_sampleRateDidChange, s_asioMessage, s_bufferSwitchTimeInfo
};
