#pragma once
class CAsioDriver
{
public:
	static const HKEY RegRootHandle;
	static LPCTSTR RegTopKeyName;

	typedef std::vector<std::unique_ptr<CAsioDriver>> driver_list_t;
	static HRESULT createDriverList(driver_list_t& drivers);

	~CAsioDriver();

	// Creates COM object of Asio driver.
	HRESULT create();

protected:
	// Constructor
	// Only createDriverList() method can create instance of this class.
	CAsioDriver(const CLSID& clsid, LPCTSTR descriptio);

	CLSID m_clsid;
	tstring m_description;
};
