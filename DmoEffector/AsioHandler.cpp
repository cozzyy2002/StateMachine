#include "stdafx.h"
#include "AsioHandler.h"
#include "AsioDriver.h"

static log4cplus::Logger logger = log4cplus::Logger::getInstance(_T("AsioHandler"));
static long getSampleSize(ASIOSampleType type);
static void logChannelInfo(const ASIOChannelInfo& info);

CAsioHandler* CAsioHandler::m_instance = NULL;

/*
	Protected constructor.

	Call getInstance() static method to create or get CAsioHandler object.
*/
CAsioHandler::CAsioHandler(int numChannels)
	: m_state(State::NotLoaded), m_numChannels(numChannels)
{
	// Allocate buffer infos for channel * 2(in and out).
	m_asioBufferInfos.reset(new ASIOBufferInfo[numChannels * 2]);

	ZeroMemory(&m_statistics, sizeof(m_statistics));
	ZeroMemory(&m_driverInfo, sizeof(m_driverInfo));

	m_instance = this;
}


CAsioHandler::~CAsioHandler()
{
	m_instance = NULL;
}

/*
	Returns singleton CAioHandler object.
*/
CAsioHandler * CAsioHandler::getInstance(int numChannels /*= 0*/)
{
	if (!m_instance) m_instance = new CAsioHandler(numChannels);
	return m_instance;
}

HRESULT CAsioHandler::setup(const CAsioDriver* pAsioDriver, HWND hwnd)
{
	HR_ASSERT(pAsioDriver, E_POINTER);
	HR_ASSERT(m_state == State::NotLoaded, E_ILLEGAL_METHOD_CALL);

	// Create IASIO object and initialize it.
	m_asio.Release();
	HR_ASSERT_OK(pAsioDriver->create(&m_asio));
	ASIO_ASSERT(m_asio->init(hwnd), E_ABORT);

	// Show name and version of ASIO driver created.
	char driverName[100];
	m_asio->getDriverName(driverName);
	LOG4CPLUS_INFO(logger, "Loaded '" << driverName << "' version=" << m_asio->getDriverVersion());

	// Get number of channels and assert that we have enough channels.
	long numInputChannels, numOutputChannels;
	ASIO_ASSERT_OK(m_asio->getChannels(&numInputChannels, &numOutputChannels));
	LOG4CPLUS_INFO(logger, "Input " << numInputChannels << " channels, Output " << numOutputChannels << " channels");
	if (0 < m_numChannels) {
		HR_ASSERT((m_numChannels <= numInputChannels) && (m_numChannels <= numOutputChannels), E_INVALIDARG);
	} else {
		m_numChannels = min(numInputChannels, numOutputChannels);
	}

	// Initialize all ASIOBufferInfo prior to calling IASIO::createBuffers().
	// Buffers are prepared for each in/out, channel and double buffer index 0/1
	forInChannels([this](long channel, ASIOBufferInfo&in, ASIOBufferInfo&out) {
		in.isInput = ASIOTrue;
		out.isInput = ASIOFalse;
		in.channelNum = out.channelNum = channel;
		in.buffers[0] = in.buffers[1] = out.buffers[0] = out.buffers[1] = NULL;
		return S_OK;
	});

	m_driverInfo.isOutputReadySupported = (m_asio->outputReady() == ASE_OK);

	// Create buffers for input and output.
	long minSize, maxSize, preferredSize, granularity;
	ASIO_ASSERT_OK(m_asio->getBufferSize(&minSize, &maxSize, &preferredSize, &granularity));
	m_bufferSize = preferredSize;
	ASIO_ASSERT_OK(m_asio->createBuffers(m_asioBufferInfos.get(), m_numChannels * 2, m_bufferSize, &m_callbacks));
	LOG4CPLUS_INFO(logger, "Created buffers: " << m_numChannels << "channels, Prepared buffer size=" << m_bufferSize);

	// Set 0 to all buffers.
	forInChannels([this](long channel, ASIOBufferInfo&in, ASIOBufferInfo&out) {
		ZeroMemory(in.buffers[0], m_bufferSize);
		ZeroMemory(in.buffers[1], m_bufferSize);
		ZeroMemory(out.buffers[0], m_bufferSize);
		ZeroMemory(out.buffers[1], m_bufferSize);

		return S_OK;
	});

	m_state = State::Prepared;
	return S_OK;
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

	if (m_asio) {
		hr = ASIO_EXPECT_OK(m_asio->disposeBuffers());
		m_asio.Release();
	}

	m_state = State::NotLoaded;
	return hr;
}

HRESULT CAsioHandler::start()
{
	HR_ASSERT(m_state == State::Prepared, E_ILLEGAL_METHOD_CALL);

	ASIO_ASSERT_OK(m_asio->start());

	m_state = State::Running;
	return S_OK;
}

HRESULT CAsioHandler::stop(const Statistics** ppStatistics /*= NULL*/)
{
	if (ppStatistics) *ppStatistics = &m_statistics;

	// Returning S_FALSE means that this method has done nothing.
	if(m_state != State::Running) return S_FALSE;

	ASIO_ASSERT_OK(m_asio->stop());

	m_state = State::Prepared;
	return S_OK;
}

HRESULT CAsioHandler::getProperty(Property * pProperty)
{
	HR_ASSERT(pProperty, E_POINTER);

	pProperty->state = m_state;
	pProperty->numChannels = m_numChannels;
	pProperty->bufferSize = m_bufferSize;
	return S_OK;
}

HRESULT CAsioHandler::handleEvent(const CAsioHandlerEvent* event)
{
	CAsioHandlerState* nextState = NULL;
	m_currentState->handleEvent(event, &nextState);
	if (nextState) {
		m_currentState->exit(event, nextState);
		nextState->entry(event, m_currentState.get());
		m_currentState.reset(nextState);
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

/*
	Call function for each channels(from 0 to m_numChannels - 1).
*/
HRESULT CAsioHandler::forInChannels(std::function<HRESULT(long channel, ASIOBufferInfo&in, ASIOBufferInfo&out)> func)
{
	for (long channel = 0; channel < m_numChannels; channel++) {
		ASIOBufferInfo&in = getInputBufferInfo(channel);
		ASIOBufferInfo&out = getOutputBufferInfo(channel);
		HR_ASSERT_OK(func(channel, in, out));
	}

	return S_OK;
}

void CAsioHandler::bufferSwitch(long doubleBufferIndex, ASIOBool directProcess)
{
	if (FAILED(HR_EXPECT(m_asio, E_ILLEGAL_METHOD_CALL))) return;

	ASIOTime time;
	ZeroMemory(&time, sizeof(time));
	HRESULT hr = ASIO_EXPECT_OK(m_asio->getSamplePosition(&time.timeInfo.samplePosition, &time.timeInfo.systemTime));
	if (SUCCEEDED(hr)) {
		time.timeInfo.flags = kSystemTimeValid | kSamplePositionValid;
	}

	bufferSwitchTimeInfo(&time, doubleBufferIndex, directProcess);
}

ASIOTime * CAsioHandler::bufferSwitchTimeInfo(ASIOTime * params, long doubleBufferIndex, ASIOBool directProcess)
{
	if (FAILED(HR_EXPECT(m_asio, E_ILLEGAL_METHOD_CALL))) return NULL;

	m_statistics.bufferSwitch[doubleBufferIndex]++;

	if (directProcess) {
		forInChannels([this, doubleBufferIndex](long /*channel*/, ASIOBufferInfo&in, ASIOBufferInfo&out) {
			CopyMemory(out.buffers[doubleBufferIndex], in.buffers[doubleBufferIndex], m_bufferSize);
			return S_OK;
		});

		// Notify the driver that output data is available if supported.
		if (m_driverInfo.isOutputReadySupported) {
			ASIO_EXPECT_OK(m_asio->outputReady());
		}
	} else {
		// Process should be deferred.
	}

	return nullptr;
}

void CAsioHandler::sampleRateDidChange(ASIOSampleRate sRate)
{
	if (FAILED(HR_EXPECT(m_asio, E_ILLEGAL_METHOD_CALL))) return;

}

long CAsioHandler::asioMessage(long selector, long value, void * message, double * opt)
{
	if (FAILED(HR_EXPECT(m_asio, E_ILLEGAL_METHOD_CALL))) return 0;

	long ret = ASIOFalse;
	LPCSTR strSelector = "UNKNOWN";
	switch (selector) {
	case kAsioSelectorSupported:
		strSelector = "kAsioSelectorSupported";
		switch (value) {
		case kAsioSupportsTimeInfo:
		case kAsioEngineVersion:
			ret = ASIOTrue;
			break;
		}
		break;
	case kAsioEngineVersion:
		strSelector = "kAsioEngineVersion";
		ret = 2;
		break;
	case kAsioResetRequest:
	case kAsioBufferSizeChange:
	case kAsioResyncRequest:
	case kAsioLatenciesChanged:
		break;
	case kAsioSupportsTimeInfo:
		// Tell the driver that we support bufferSwitchTimeInfo() callback.
		strSelector = "kAsioSupportsTimeInfo";
		ret = ASIOTrue;
		break;
	case kAsioSupportsTimeCode:
	case kAsioMMCCommand:
	case kAsioSupportsInputMonitor:
	case kAsioSupportsInputGain:
	case kAsioSupportsInputMeter:
	case kAsioSupportsOutputGain:
	case kAsioSupportsOutputMeter:
	case kAsioOverload:
		break;
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

HRESULT CAsioHandler::initializeChannelInfo(long channel)
{
	ASIOChannelInfo input, output;
	ZeroMemory(&input, sizeof(input));
	ZeroMemory(&output, sizeof(output));
	input.channel = output.channel = channel;
	input.isInput = ASIOTrue;
	output.isInput = ASIOFalse;
	ASIO_ASSERT_OK(m_asio->getChannelInfo(&input));
	logChannelInfo(input);
	ASIO_ASSERT_OK(m_asio->getChannelInfo(&output));
	logChannelInfo(output);

	// Input and output type should equal.
	ASIO_ASSERT(input.type == output.type, E_ABORT);

	long sampleSize = getSampleSize(input.type);
	ASIO_ASSERT(0 < sampleSize, E_INVALIDARG);

	return S_OK;
}

/*static*/ long getSampleSize(ASIOSampleType type)
{
	long size = 0;
	switch (type) {
	case ASIOSTDSDInt8LSB1:
	case ASIOSTDSDInt8MSB1:
	case ASIOSTDSDInt8NER8:
		size = 1;
		break;
	case ASIOSTInt16MSB:
	case ASIOSTInt16LSB:
		size = 2;
		break;
	case ASIOSTInt24MSB:
	case ASIOSTInt24LSB:
		size = 3;
		break;
	case ASIOSTInt32MSB:
	case ASIOSTFloat32MSB:
	case ASIOSTInt32MSB16:
	case ASIOSTInt32MSB18:
	case ASIOSTInt32MSB20:
	case ASIOSTInt32MSB24:
	case ASIOSTInt32LSB:
	case ASIOSTFloat32LSB:
	case ASIOSTInt32LSB16:
	case ASIOSTInt32LSB18:
	case ASIOSTInt32LSB20:
	case ASIOSTInt32LSB24:
		size = 4;
		break;
	case ASIOSTFloat64MSB:
	case ASIOSTFloat64LSB:
		size = 8;
		break;
	}

	return size;
}

/*static*/ void logChannelInfo(const ASIOChannelInfo& info)
{
	LOG4CPLUS_INFO(logger, "Channel " << info.channel << (info.isInput ? ":IN " : ":OUT")
							<< "=Group " << info.channelGroup << ",Sample type=" << info.type
							<< " '" << info.name << "'");
}

struct ErrorMessage {
	LPCTSTR symbol;
	LPCTSTR description;
};

static const ErrorMessage errorMessages[] = {
	//	ASE_OK = 0,             // This value will be returned whenever the call succeeded
	//	ASE_SUCCESS = 0x3f4847a0,	// unique success return value for ASIOFuture calls
		_T("ASE_NotPresent"),		_T("hardware input or output is not present or available"),
		_T("ASE_HWMalfunction"),	_T("hardware is malfunctioning (can be returned by any ASIO function)"),
		_T("ASE_InvalidParameter"),	_T("input parameter invalid"),
		_T("ASE_InvalidMode"),		_T("hardware is in a bad mode or used in a bad mode"),
		_T("ASE_SPNotAdvancing"),	_T("hardware is not running when sample position is inquired"),
		_T("ASE_NoClock"),			_T("sample clock or rate cannot be determined or is not present"),
		_T("ASE_NoMemory"),			_T("not enough memory for completing the request"),
};

HRESULT asioCheck(ASIOError expr, LPCTSTR exprStr, LPCTSTR src, int line)
{
	HRESULT hr;
	switch (expr) {
	case ASE_OK:
	case ASE_SUCCESS:
		hr = S_OK;
		break;
	default:
		hr = E_FAIL;
		{
			static const int ASE_top = ASE_NotPresent;
			int i = expr - ASE_top;
			LPCTSTR symbol = _T("UNKNOWN");
			LPCTSTR description = _T("Unknown ASIOError value");
			if (i < ARRAYSIZE(errorMessages)) {
				const ErrorMessage msg = errorMessages[i];
				symbol = msg.symbol;
				description = msg.description;
			}
			LOG4CPLUS_ERROR(logger,
				exprStr << _T(" failed. ") << symbol << _T(": ") << description << _T("(") << expr
				<< _T(") at:\n") << src << _T("(") << std::dec << line << _T(")"));
		}
		break;
	}
	return hr;
}
