#include "stdafx.h"
#include "MainController.h"

#include "Device.h"

static log4cplus::Logger logger = log4cplus::Logger::getInstance(_T("MainController"));

CMainController::CMainController()
{
}


CMainController::~CMainController()
{
	HR_EXPECT_OK(stop());
}

HRESULT CMainController::start(CDevice * inputDevice, CDevice* outputDevice)
{
	LOG4CPLUS_INFO(logger, __FUNCTION__ << "('" << inputDevice->getDevicePath() << "','" << outputDevice->getDevicePath() << "')");

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
