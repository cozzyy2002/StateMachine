#include "stdafx.h"
#include "AsioHandler.h"


#include <initguid.h>
DEFINE_GUID(clsidAsio, 0x232685C6, 0x6548, 0x49D8, 0x84, 0x6D, 0x41, 0x41, 0xA3, 0xEF, 0x75, 0x60);

static log4cplus::Logger logger = log4cplus::Logger::getInstance(_T("AsioHandler"));
static HRESULT logChannelInfo(IASIO* asio, long channel, ASIOBool isInput);

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

	m_instance = this;
}


CAsioHandler::~CAsioHandler()
{
	m_instance = NULL;
}

/*
	Returns singleton CAioHandler object.
*/
CAsioHandler * CAsioHandler::getInstance(int numChannels)
{
	if (!m_instance) m_instance = new CAsioHandler(numChannels);
	return m_instance;
}

HRESULT CAsioHandler::setup(HWND hwnd)
{
	HR_ASSERT(m_state == State::NotLoaded, E_ILLEGAL_METHOD_CALL);

	// Create IASIO object and initialize it.
	m_asio.Release();
	HR_ASSERT_OK(CoCreateInstance(clsidAsio, NULL, CLSCTX_INPROC_SERVER, clsidAsio, (LPVOID*)&m_asio));
	ASIO_ASSERT(m_asio->init(hwnd), E_ABORT);

	// Show name and version of ASIO driver created.
	char driverName[100];
	m_asio->getDriverName(driverName);
	LOG4CPLUS_INFO(logger, "Loaded '" << driverName << "' version=" << m_asio->getDriverVersion());

	// Initialize all ASIOBufferInfo prior to calling IASIO::createBuffers().
	// Buffers are prepared for each in/out, channel and double buffer index 0/1
	forInChannels([this](long channel, ASIOBufferInfo&in, ASIOBufferInfo&out) {
		in.isInput = ASIOTrue;
		out.isInput = ASIOFalse;
		in.channelNum = out.channelNum = channel;
		in.buffers[0] = in.buffers[1] = out.buffers[0] = out.buffers[1] = NULL;
		return S_OK;
	});

	// Create buffers.
	long minSize, maxSize, preferredSize, granularity;
	ASIO_ASSERT_OK(m_asio->getBufferSize(&minSize, &maxSize, &preferredSize, &granularity));
	m_bufferSize = preferredSize;
	ASIO_ASSERT_OK(m_asio->createBuffers(m_asioBufferInfos.get(), m_numChannels * 2, m_bufferSize, &m_callbacks));
	LOG4CPLUS_INFO(logger, "Prepared ASIO: " << m_numChannels << "channels, Buffer size=" << m_bufferSize);

	// Set 0 to all buffers.
	forInChannels([this](long channel, ASIOBufferInfo&in, ASIOBufferInfo&out) {
		ZeroMemory(in.buffers[0], m_bufferSize);
		ZeroMemory(in.buffers[1], m_bufferSize);
		ZeroMemory(out.buffers[0], m_bufferSize);
		ZeroMemory(out.buffers[1], m_bufferSize);

		logChannelInfo(m_asio, channel, ASIOTrue);
		logChannelInfo(m_asio, channel, ASIOFalse);

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
	// Returning S_FALSE means that this method has done nothing.
	if(m_state != State::Running) return S_FALSE;

	ASIO_ASSERT_OK(m_asio->stop());

	if (ppStatistics) *ppStatistics = &m_statistics;

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

/*static*/ HRESULT logChannelInfo(IASIO* asio, long channel, ASIOBool isInput)
{
	ASIOChannelInfo info;
	ZeroMemory(&info, sizeof(info));
	info.channel = channel;
	info.isInput = isInput;

	ASIO_ASSERT_OK(asio->getChannelInfo(&info));
	LOG4CPLUS_INFO(logger, "Channel " << channel << (isInput ? ":IN " : ":OUT")
							<< "=Group " << info.channelGroup << ",Sample type=" << info.type
							<< " '" << info.name << "'");

	return S_OK;
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
