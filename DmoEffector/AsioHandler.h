#pragma once

#include <functional>

class CAsioHandler
{
protected:
	static CAsioHandler* m_instance;
	CAsioHandler(int numChannels);

public:
	~CAsioHandler();

	static CAsioHandler* getInstance(int numChannels);

	HRESULT setup(HWND hwnd);
	HRESULT shutdown();
	HRESULT start();

	struct Statistics {
		long bufferSwitch[2];	// Count of bufferSwitchTimeInfo() called for each doubleBufferIndex.
	};
	HRESULT stop(const Statistics** ppStatistics = NULL);

	struct Property {

	};

	HRESULT getProperty(Property* pProperty);

protected:
	CComPtr<IASIO> m_asio;

	int m_numChannels;
	std::unique_ptr<ASIOBufferInfo[]> m_asioBufferInfos;
	long m_bufferSize;
	Statistics m_statistics;

	ASIOBufferInfo& getInputBufferInfo(int channel) { return m_asioBufferInfos.get()[channel]; }
	ASIOBufferInfo& getOutputBufferInfo(int channel) { return m_asioBufferInfos.get()[channel + m_numChannels]; }

	HRESULT forInChannels(std::function<HRESULT(long channel, ASIOBufferInfo& in, ASIOBufferInfo& out)> func);

	// ASIO callbacks
	void bufferSwitch(long doubleBufferIndex, ASIOBool directProcess);
	ASIOTime* bufferSwitchTimeInfo(ASIOTime* params, long doubleBufferIndex, ASIOBool directProcess);
	void sampleRateDidChange(ASIOSampleRate sRate);
	long asioMessage(long selector, long value, void* message, double* opt);

	static void s_bufferSwitch(long doubleBufferIndex, ASIOBool directProcess);
	static ASIOTime* s_bufferSwitchTimeInfo(ASIOTime* params, long doubleBufferIndex, ASIOBool directProcess);
	static void s_sampleRateDidChange(ASIOSampleRate sRate);
	static long s_asioMessage(long selector, long value, void* message, double* opt);

	static ASIOCallbacks m_callbacks;
};

#define ASIO_ASSERT HR_ASSERT
#define ASIO_EXPECT HR_EXPECT
#define ASIO_ASSERT_OK(exp) do { HRESULT _hr = ASIO_EXPECT_OK(exp); if(FAILED(_hr)) return _hr; } while(0)
#define ASIO_EXPECT_OK(exp) asioCheck(exp, _T(#exp), _T(__FILE__), __LINE__)
extern HRESULT asioCheck(ASIOError expr, LPCTSTR exprStr, LPCTSTR src, int line);
inline bool asioIsOk(ASIOError e) { return (e == ASE_OK) || (e == ASE_SUCCESS); }
