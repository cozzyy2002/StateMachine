#include "stdafx.h"
#include "AsioDriver.h"

static log4cplus::Logger logger = log4cplus::Logger::getInstance(_T("AsioDriver"));

/*static*/ const HKEY CAsioDriver::RegRootHandle = HKEY_LOCAL_MACHINE;
/*static*/ LPCTSTR CAsioDriver::RegTopKeyName = _T("SOFTWARE\\ASIO");

static HRESULT getString(const CRegKey& key, LPCTSTR name, tstring& string, DWORD maxSize = MAX_PATH);

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
		long error = rootKey.EnumKey(index++, name, &nameLength);
		if(error == ERROR_SUCCESS) {
			// Open key of each driver contains CLSID and description.
			CRegKey key;
			CREGKEY_ASSERT_OK(key.Open(rootKey, name, KEY_READ));

			// Read values in the key.
			tstring clsid;
			HR_ASSERT_OK(getString(key, _T("CLSID"), clsid, 38));
			CT2W wclsid(clsid.c_str());
			CLSID _clsid;
			HR_ASSERT_OK(CLSIDFromString((LPCOLESTR)wclsid, &_clsid));
			tstring description;
			HR_ASSERT_OK(getString(key, _T("Description"), description));

			// Create CAsioDriver instance and add it to the list.
			LOG4CPLUS_DEBUG(logger, "Creating CAsioDriver: Registry key=" << name << ", " << clsid.c_str() << ", '" << description.c_str() << "'");
			drivers.push_back(list_t::value_type(new CAsioDriver(_clsid, description.c_str())));
		} else {
			HR_ASSERT(error == ERROR_NO_MORE_ITEMS, HRESULT_FROM_WIN32(error));
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

// Returns instance of ASIO driver using CLSID.
HRESULT CAsioDriver::create(IASIO** ppAsio)
{
	if (!m_asio) {
		LOG4CPLUS_INFO(logger, "Creating '" << m_description.c_str() << "'");
		HR_ASSERT_OK(CoCreateInstance(m_clsid, NULL, CLSCTX_INPROC_SERVER, m_clsid, (LPVOID*)ppAsio));
	} else {
		LOG4CPLUS_INFO(logger, "Querying '" << m_description.c_str() << "'");
		HR_ASSERT_OK(m_asio->QueryInterface(m_clsid, (void**)ppAsio));
	}
	return S_OK;
}

/*static*/ HRESULT getString(const CRegKey& key, LPCTSTR name, tstring& string, DWORD maxLength /*= MAX_PATH*/)
{
	DWORD type = REG_NONE;
	DWORD size = 0;
	CREGKEY_ASSERT_OK(RegQueryValueEx((HKEY)key, name, NULL, &type, NULL, &size));
	HR_ASSERT(type == REG_SZ, E_INVALIDARG);
	size_t strLength = (size / sizeof(TCHAR)) - 1;	// String lenght excluding null terminater.
	HR_ASSERT(strLength <= maxLength, E_UNEXPECTED);
	std::unique_ptr<BYTE[]> data(new BYTE[size]);
	CREGKEY_ASSERT_OK(RegQueryValueEx((HKEY)key, name, NULL, &type, data.get(), &size));
	string.assign((LPCTSTR)data.get(), strLength);
	return S_OK;
}
