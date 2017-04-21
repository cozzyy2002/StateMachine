#pragma once

#include "AsioHandler.h"

class CDevice;

class CMainController
{
public:
	CMainController();
	~CMainController();

	HRESULT setup(IASIO* asio, HWND hwnd);
	HRESULT shutdown();
	HRESULT start(CDevice* inputDevice, CDevice* outputDevice);
	HRESULT stop();

protected:
	std::unique_ptr<CAsioHandler> m_asioHandler;
};
