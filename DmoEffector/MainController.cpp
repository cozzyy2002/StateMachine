#include "stdafx.h"
#include "MainController.h"

#include "Device.h"

#include <initguid.h>
DEFINE_GUID(clsidAsio, 0x232685C6, 0x6548, 0x49D8, 0x84, 0x6D, 0x41, 0x41, 0xA3, 0xEF, 0x75, 0x60);

static log4cplus::Logger logger = log4cplus::Logger::getInstance(_T("MainController"));

CMainController::CMainController()
{
}


CMainController::~CMainController()
{
	HR_EXPECT_OK(shutdown());
}

HRESULT CMainController::setup(HWND hwnd)
{
	m_asio.Release();
	HR_ASSERT_OK(CoCreateInstance(clsidAsio, NULL, CLSCTX_INPROC_SERVER, clsidAsio, (LPVOID*)&m_asio));
	HR_ASSERT(m_asio->init(hwnd), E_ABORT);

	char driverName[100];
	m_asio->getDriverName(driverName);
	LOG4CPLUS_INFO(logger, "Loaded '" << driverName << "' version=" << m_asio->getDriverVersion());
	return S_OK;
}

HRESULT CMainController::shutdown()
{
	HR_EXPECT_OK(stop());

	m_asio.Release();
	return S_OK;
}

HRESULT CMainController::start(CDevice * inputDevice, CDevice* outputDevice)
{
	CT2A in(inputDevice->getName());
	CT2A out(outputDevice->getName());
	LOG4CPLUS_INFO(logger, __FUNCTION__ "('" << (LPCSTR)in << "','" << (LPCSTR)out << "')");

	HR_ASSERT_OK(stop());

	CComPtr<IGraphBuilder> graph;

	// Setup FilterGraph and GraphBuilder
	HR_ASSERT_OK(graph.CoCreateInstance(CLSID_FilterGraph));
	HR_ASSERT_OK(m_graphBuilder.CoCreateInstance(CLSID_CaptureGraphBuilder2));
	HR_ASSERT_OK(m_graphBuilder->SetFiltergraph(graph));

	// Construct filter graph that contains input device as source filter.
	CComPtr<IBaseFilter> inputFilter;
	HR_ASSERT_OK(inputDevice->getBaseFilter(&inputFilter));
	HR_ASSERT_OK(graph->AddFilter(inputFilter, NULL));
	CComPtr<IBaseFilter> outputFilter;
	HR_ASSERT_OK(outputDevice->getBaseFilter(&outputFilter));
	HR_ASSERT_OK(graph->AddFilter(outputFilter, NULL));
	HR_ASSERT_OK(m_graphBuilder->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Audio, inputFilter, NULL, outputFilter));

	HR_ASSERT_OK(graph.QueryInterface(&m_mediaControl));
	HR_ASSERT_OK(graph.QueryInterface(&m_basicAudio));

	HR_ASSERT_OK(m_mediaControl->Run());

	return S_OK;
}

HRESULT CMainController::stop()
{
	LOG4CPLUS_INFO(logger, __FUNCTION__ << "(" << (m_mediaControl ? "Running" : "Stopped") << ")");

	if (m_mediaControl) {
		HR_EXPECT_OK(m_mediaControl->Stop());
		m_mediaControl.Release();
	}

	m_graphBuilder.Release();
	m_basicAudio.Release();

	return S_OK;
}
