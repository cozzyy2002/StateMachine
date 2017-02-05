#include "stdafx.h"
#include "AsioDriver.h"

static log4cplus::Logger logger = log4cplus::Logger::getInstance(_T("AsioDriver"));

/*static*/ const HKEY CAsioDriver::RegRootHandle = HKEY_LOCAL_MACHINE;
/*static*/ LPCTSTR CAsioDriver::RegTopKeyName = _T("SOFTWARE\\ASIO");

#define CREGKEY_ASSERT_OK(exp) HR_ASSERT_OK(HRESULT_FROM_WIN32(exp))
#define CREGKEY_EXPECT_OK(exp) HR_EXPECT_OK(HRESULT_FROM_WIN32(exp))

HRESULT CAsioDriver::createDriverList(driver_list_t & drivers)
{
	CRegKey rootKey;
	CREGKEY_ASSERT_OK(rootKey.Open(RegRootHandle, RegTopKeyName, KEY_READ));

	DWORD index = 0;
	long result;
	do {
		TCHAR name[MAX_PATH];
		DWORD nameLength = ARRAYSIZE(name);
		result = CREGKEY_EXPECT_OK(rootKey.EnumKey(index++, name, &nameLength));
		if(result == ERROR_SUCCESS) {
			CRegKey key;
			CREGKEY_ASSERT_OK(key.Open(rootKey, name, KEY_READ));
			ULONG len;
			TCHAR clsid[50];
			TCHAR description[MAX_PATH];
			len = ARRAYSIZE(clsid);
			CREGKEY_ASSERT_OK(key.QueryStringValue(_T("CLSID"), clsid, &len));
			CT2W wclsid(clsid);
			CLSID _clsid;
			HR_ASSERT_OK(CLSIDFromString((LPCOLESTR)wclsid, &_clsid));
			len = ARRAYSIZE(description);
			CREGKEY_ASSERT_OK(key.QueryStringValue(_T("Description"), description, &len));

			drivers.push_back(driver_list_t::value_type(new CAsioDriver(_clsid, description)));
		}
	} while (result == ERROR_SUCCESS);

	LOG4CPLUS_INFO(logger, "Found " << drivers.size() << " Asio drivers.");
	return S_OK;
}

CAsioDriver::CAsioDriver(const CLSID& clsid, LPCTSTR description)
	: m_clsid(clsid), m_description(description)
{
}

CAsioDriver::~CAsioDriver()
{
}

HRESULT CAsioDriver::create()
{
	return E_NOTIMPL;
}
