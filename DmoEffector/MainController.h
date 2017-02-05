#pragma once

#include "AsioHandler.h"

class CAsioDriver;
class CDevice;

class CMainController
{
public:
	CMainController();
	~CMainController();

	HRESULT setup(const CAsioDriver* pAsioDriver, HWND hwnd);
	HRESULT shutdown();
	HRESULT start(CDevice* inputDevice, CDevice* outputDevice);
	HRESULT stop();

protected:
	std::unique_ptr<CAsioHandler> m_asioHandler;
};
