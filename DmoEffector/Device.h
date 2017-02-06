#pragma once
class CDevice
{
public:
	typedef std::vector<std::unique_ptr<CDevice>> list_t;
	static HRESULT createDeviceList(REFGUID category, list_t& devices);

	CDevice(IMoniker* moniker);
	CDevice(LPCTSTR name, LPCTSTR devicePath);
	virtual ~CDevice();

	HRESULT getBaseFilter(IBaseFilter** ppBaseFilter);
	LPCTSTR getName();
	LPCTSTR getDevicePath();

protected:
	IBindCtx* getBindCtx();

	tstring m_name;
	tstring m_devicePath;
	CComPtr<IBaseFilter> m_baseFilter;
	CComPtr<IMoniker> m_moniker;
	CComPtr<IBindCtx> m_bindCtx;
};
