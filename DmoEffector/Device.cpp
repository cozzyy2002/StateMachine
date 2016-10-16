#include "stdafx.h"
#include "Device.h"

#pragma comment(lib, "Strmiids.lib")

static log4cplus::Logger logger = log4cplus::Logger::getInstance(_T("Device"));

class CSafeVariant {
public:
	CSafeVariant() { VariantInit(&m_variant); }
	~CSafeVariant() { VariantClear(&m_variant); }
	VARIANT* operator &() { return &m_variant; }
	operator LPCWSTR() { return m_variant.bstrVal; }

protected:
	VARIANT m_variant;
};

/*static*/ HRESULT CDevice::createDeviceList(REFGUID category, device_list_t& devices)
{
	CComPtr<ICreateDevEnum> devEnum;
	HR_ASSERT_OK(devEnum.CoCreateInstance(CLSID_SystemDeviceEnum));

	CComPtr<IEnumMoniker> enumMoniker;
	HR_ASSERT_OK(devEnum->CreateClassEnumerator(category, &enumMoniker, 0));
	HRESULT hr;
	while (true) {
		CComPtr<IMoniker> moniker;
		hr = HR_EXPECT_OK(enumMoniker->Next(1, &moniker, NULL));
		if (hr != S_OK) break;
		devices.push_back(device_list_t::value_type(new CDevice(moniker)));
	}

	return S_OK;
}

CDevice::CDevice(IMoniker* moniker) : m_moniker(moniker) {}

CDevice::CDevice(LPCTSTR name, LPCTSTR devicePath)
	: m_name(name), m_devicePath(devicePath) {}

CDevice::~CDevice() {}

HRESULT CDevice::getBaseFilter(IBaseFilter ** ppBaseFilter)
{
	HR_ASSERT(ppBaseFilter, E_POINTER);
	HR_ASSERT(m_moniker, E_ILLEGAL_METHOD_CALL);

	if (!m_baseFilter) {
		HR_ASSERT_OK(m_moniker->BindToObject(getBindCtx(), NULL, IID_PPV_ARGS(&m_baseFilter)));
	}
	*ppBaseFilter = m_baseFilter;
	(*ppBaseFilter)->AddRef();

	return S_OK;
}

LPCTSTR CDevice::getName()
{
	if (m_name.empty()) {
		CComPtr<IPropertyBag> prop;
		if (SUCCEEDED(HR_EXPECT_OK(m_moniker->BindToStorage(getBindCtx(), NULL, IID_PPV_ARGS(&prop))))) {
			CSafeVariant wstr;
			HR_EXPECT_OK(prop->Read(L"FriendlyName", &wstr, 0));
			CW2T str(wstr);
			m_name = (LPCTSTR)str;
		}
	}
	return m_name.c_str();
}

LPCTSTR CDevice::getDevicePath()
{
	if (m_devicePath.empty()) {
		LPOLESTR wstr = L"";
		HR_EXPECT_OK(m_moniker->GetDisplayName(getBindCtx(), NULL, &wstr));
		CW2T str(wstr);
		CoTaskMemFree(wstr);
		m_devicePath = (LPCTSTR)str;
	}
	return m_devicePath.c_str();
}

IBindCtx* CDevice::getBindCtx()
{
	if (!m_bindCtx) {
		HR_EXPECT_OK(CreateBindCtx(0, &m_bindCtx));
	}
	return m_bindCtx;
}
