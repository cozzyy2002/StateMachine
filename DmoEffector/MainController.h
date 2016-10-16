#pragma once

class CDevice;

class CMainController
{
public:
	CMainController();
	~CMainController();

	HRESULT start(CDevice* inputDevice);
	HRESULT stop();

protected:
	CComPtr<ICaptureGraphBuilder2> m_graphBuilder;
	CComPtr<IMediaControl> m_mediaControl;
	CComPtr<IBasicAudio> m_basicAudio;
};
