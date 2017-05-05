#pragma once

#include <functional>

struct CAsioHandlerEvent;

class CAsioHandlerContext
{
protected:
	CAsioHandlerContext();

public:
	virtual ~CAsioHandlerContext();

	struct Statistics {
		long bufferSwitch[2];	// Count of bufferSwitchTimeInfo() called for each doubleBufferIndex.
	};

	// State of this class.
	ENUM(State,
		NotLoaded,		// Asio is not loaded or has been released.
		Prepared,		// Asio has been loaded, initialized and prepared. Ready to run.
		Running			// Asio is running.
	);

	State m_state;

	inline const State& getState() const { return m_state; }

	struct Property {
		State state;
		int numChannels;
		long bufferSize;
	};

	virtual HRESULT triggerEvent(CAsioHandlerEvent* event) = 0;
	virtual ASIOCallbacks* getAsioCallbacks() const = 0;

	HRESULT getProperty(Property* pProperty);
	ASIOBufferInfo& getInputBufferInfo(int channel) { return asioBufferInfos.get()[channel]; }
	ASIOBufferInfo& getOutputBufferInfo(int channel) { return asioBufferInfos.get()[channel + numChannels]; }
	HRESULT initializeChannelInfo(long channel);

	HRESULT forInChannels(std::function<HRESULT(long channel, ASIOBufferInfo& in, ASIOBufferInfo& out)> func);

	// ASIO4All
	CComPtr<IASIO> asio;

	struct DriverInfo {
		bool isOutputReadySupported;
	};

	DriverInfo driverInfo;

	int numChannels;
	std::unique_ptr<ASIOBufferInfo[]> asioBufferInfos;
	long bufferSize;
	Statistics statistics;

	CHandle shutDownEvent;
};

#define ASIO_ASSERT HR_ASSERT
#define ASIO_EXPECT HR_EXPECT
#define ASIO_ASSERT_OK(exp) do { HRESULT _hr = ASIO_EXPECT_OK(exp); if(FAILED(_hr)) return _hr; } while(0)
#define ASIO_EXPECT_OK(exp) asioCheck(exp, _T(#exp), _T(__FILE__), __LINE__)
extern HRESULT asioCheck(ASIOError expr, LPCTSTR exprStr, LPCTSTR src, int line);
inline bool asioIsOk(ASIOError e) { return (e == ASE_OK) || (e == ASE_SUCCESS); }
