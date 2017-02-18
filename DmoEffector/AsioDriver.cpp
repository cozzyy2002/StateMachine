#include "stdafx.h"
#include "AsioDriver.h"

static log4cplus::Logger logger = log4cplus::Logger::getInstance(_T("AsioDriver"));

/*static*/ const HKEY CAsioDriver::RegRootHandle = HKEY_LOCAL_MACHINE;
/*static*/ LPCTSTR CAsioDriver::RegTopKeyName = _T("SOFTWARE\\ASIO");

#define CREGKEY_ASSERT(exp, result) HR_ASSERT(exp, HRESULT_FROM_WIN32(result))
#define CREGKEY_ASSERT_OK(exp) HR_ASSERT_OK(HRESULT_FROM_WIN32(exp))

// Creates list for the drivers registered in the registry.
HRESULT CAsioDriver::createDriverList(list_t & drivers)
{
	// Open root registry key of Asio
	CRegKey rootKey;
	CREGKEY_ASSERT_OK(rootKey.Open(RegRootHandle, RegTopKeyName, KEY_READ));

	DWORD index = 0;
	while(true) {
		TCHAR name[MAX_PATH];
		DWORD nameLength = ARRAYSIZE(name);
		long result = rootKey.EnumKey(index++, name, &nameLength);
		if(result == ERROR_SUCCESS) {
			// Open key of each driver contains CLSID and description.
			CRegKey key;
			CREGKEY_ASSERT_OK(key.Open(rootKey, name, KEY_READ));

			// Read values in the key.
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

			// Create CAsioDriver instance and add it to the list.
			drivers.push_back(list_t::value_type(new CAsioDriver(_clsid, description)));
		} else {
			CREGKEY_ASSERT(result == ERROR_NO_MORE_ITEMS, result);
			break;
		}
	}

	LOG4CPLUS_DEBUG(logger, "Found " << drivers.size() << " Asio driver(s).");
	return S_OK;
}

CAsioDriver::CAsioDriver(const CLSID& clsid, LPCTSTR description)
	: m_clsid(clsid), m_description(description)
{
	LOG4CPLUS_DEBUG(logger, __FUNCTION__ "('" << m_description.c_str() << "')");
}

CAsioDriver::~CAsioDriver()
{
	LOG4CPLUS_DEBUG(logger, __FUNCTION__ "('" << m_description.c_str() << "')");
}

// Creates instance of ASIO driver using CLSID.
HRESULT CAsioDriver::create(IASIO** ppAsio) const
{
	LOG4CPLUS_INFO(logger, "Creating '" << m_description.c_str() << "'");
	HR_ASSERT_OK(CoCreateInstance(m_clsid, NULL, CLSCTX_INPROC_SERVER, m_clsid, (LPVOID*)ppAsio));
	return S_OK;
}
