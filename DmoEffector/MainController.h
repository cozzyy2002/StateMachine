#pragma once

class CDevice;

class CMainController
{
public:
	CMainController();
	~CMainController();

	HRESULT setup(HWND hwnd);
	HRESULT shutdown();
	HRESULT start(CDevice* inputDevice, CDevice* outputDevice);
	HRESULT stop();

protected:
	CComPtr<ICaptureGraphBuilder2> m_graphBuilder;
	CComPtr<IMediaControl> m_mediaControl;
	CComPtr<IBasicAudio> m_basicAudio;
};
