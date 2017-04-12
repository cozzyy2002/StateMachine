#pragma once

#include "AsioHandlerEvent.h"
#include "AsioHandlerState.h"
#include "AsioHandlerContext.h"

class CAsioDriver;

class CAsioHandler : public CAsioHandlerContext, public IMFAsyncCallback, public CUnknownImpl
{
protected:
	static CAsioHandler* m_instance;
	CAsioHandler(int numChannels);

public:
	~CAsioHandler();

	static CAsioHandler* getInstance(int numChannels = 0);

	HRESULT setup(const CAsioDriver* pAsioDriver, HWND hwnd);
	HRESULT shutdown();
	HRESULT start();

	HRESULT stop();

#pragma region CAsioHandlerContext
	virtual HRESULT triggerEvent(CAsioHandlerEvent* event);
	virtual ASIOCallbacks* getAsioCallbacks() const { return &m_callbacks; };
#pragma endregion

#pragma region IMFAsyncCallback
	virtual HRESULT STDMETHODCALLTYPE GetParameters(
		/* [out] */ __RPC__out DWORD *pdwFlags,
		/* [out] */ __RPC__out DWORD *pdwQueue);

	virtual HRESULT STDMETHODCALLTYPE Invoke(
		/* [in] */ __RPC__in_opt IMFAsyncResult *pAsyncResult);
#pragma endregion

	// Calls CUnknownImpl methods.
	IUNKNOWN_METHODS;

public:
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

	std::unique_ptr<CAsioHandlerState> m_currentState;
	DWORD m_workQueueId;

	HRESULT handleEvent(const CAsioHandlerEvent* event);

#pragma warning(push)
#pragma warning(disable: 4838)
	IUNKNOWN_INTERFACES(QITABENT(CAsioHandler, IMFAsyncCallback));
#pragma warning(pop)
};
