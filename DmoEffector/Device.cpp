#include "stdafx.h"
#include "Device.h"

#pragma comment(lib, "Strmiids.lib")

static log4cplus::Logger logger = log4cplus::Logger::getInstance(_T("CDevice"));

class CSafeVariant {
public:
	CSafeVariant() { VariantInit(&m_variant); }
	~CSafeVariant() { VariantClear(&m_variant); }
	VARIANT* operator &() { return &m_variant; }
	operator LPCWSTR() { return m_variant.bstrVal; }

protected:
	VARIANT m_variant;
};

/*static*/ HRESULT CDevice::createDeviceList(device_list_t& devices)
{
	CComPtr<ICreateDevEnum> devEnum;
	HR_ASSERT_OK(devEnum.CoCreateInstance(CLSID_SystemDeviceEnum));

	CComPtr<IEnumMoniker> enumMoniker;
	HR_ASSERT_OK(devEnum->CreateClassEnumerator(CLSID_AudioInputDeviceCategory, &enumMoniker, 0));
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

LPCTSTR CDevice::getName()
{
	if (m_name.empty()) {
		CComPtr<IPropertyBag> prop;
		if (SUCCEEDED(HR_EXPECT_OK(m_moniker->BindToStorage(NULL, NULL, IID_PPV_ARGS(&prop))))) {
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
		HR_EXPECT_OK(m_moniker->GetDisplayName(NULL, NULL, &wstr));
		CW2T str(wstr);
		CoTaskMemFree(wstr);
		m_devicePath = (LPCTSTR)str;
	}
	return m_devicePath.c_str();
}
