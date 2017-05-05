#include "stdafx.h"
#include "AsioHandlerContext.h"

static log4cplus::Logger logger = log4cplus::Logger::getInstance(_T("AsioHandler.Context"));

static long getSampleSize(ASIOSampleType type);
static void logChannelInfo(const ASIOChannelInfo& info);

CAsioHandlerContext::CAsioHandlerContext()
	: m_state(State::NotLoaded), numChannels(numChannels)
	, shutDownEvent(CreateEvent(NULL, FALSE, FALSE, NULL))
{
	WIN32_EXPECT(NULL != (HANDLE)shutDownEvent);
}


CAsioHandlerContext::~CAsioHandlerContext()
{
}

HRESULT CAsioHandlerContext::getProperty(Property * pProperty)
{
	HR_ASSERT(pProperty, E_POINTER);

	pProperty->state = m_state;
	pProperty->numChannels = numChannels;
	pProperty->bufferSize = bufferSize;
	return S_OK;
}

HRESULT CAsioHandlerContext::initializeChannelInfo(long channel)
{
	ASIOChannelInfo input, output;
	ZeroMemory(&input, sizeof(input));
	ZeroMemory(&output, sizeof(output));
	input.channel = output.channel = channel;
	input.isInput = ASIOTrue;
	output.isInput = ASIOFalse;
	ASIO_ASSERT_OK(asio->getChannelInfo(&input));
	logChannelInfo(input);
	ASIO_ASSERT_OK(asio->getChannelInfo(&output));
	logChannelInfo(output);

	// Input and output type should equal.
	ASIO_ASSERT(input.type == output.type, E_ABORT);

	long sampleSize = getSampleSize(input.type);
	ASIO_ASSERT(0 < sampleSize, E_INVALIDARG);

	return S_OK;
}

/*
Call function for each channels(from 0 to m_numChannels - 1).
*/
HRESULT CAsioHandlerContext::forInChannels(std::function<HRESULT(long channel, ASIOBufferInfo&in, ASIOBufferInfo&out)> func)
{
	for (long channel = 0; channel < numChannels; channel++) {
		ASIOBufferInfo&in = getInputBufferInfo(channel);
		ASIOBufferInfo&out = getOutputBufferInfo(channel);
		HR_ASSERT_OK(func(channel, in, out));
	}

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
