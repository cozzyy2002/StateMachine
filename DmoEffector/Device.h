#pragma once
class CDevice
{
public:
	typedef std::vector<std::unique_ptr<CDevice>> device_list_t;
	static HRESULT createDeviceList(device_list_t& devices);

	CDevice(IMoniker* moniker);
	CDevice(LPCTSTR name, LPCTSTR devicePath);
	virtual ~CDevice();

	IMoniker* getMoniker() { return m_moniker; }
	LPCTSTR getName();
	LPCTSTR getDevicePath();

protected:
	tstring m_name;
	tstring m_devicePath;
	CComPtr<IMoniker> m_moniker;
};
