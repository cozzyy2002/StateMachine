#pragma once

struct IASIO;

/*
	Class that manages ASIO driver registered in registry.

	Registry key HKLM\SOFTARE\ASIO contains keys for each driver.
	Each driver has CLSID and Description value.
*/
class CAsioDriver
{
public:
	static const HKEY RegRootHandle;
	static LPCTSTR RegTopKeyName;

	typedef std::vector<std::unique_ptr<CAsioDriver>> list_t;
	static HRESULT createDriverList(list_t& drivers);

	~CAsioDriver();

	// Creates COM object of Asio driver.
	HRESULT create(IASIO** ppAsio);

	// Returns description.
	// Note: setupComboBox() template function calls getName() method to get string which appears in the combo-box.
	//       See DmoEffectorDlg.cpp
	LPCTSTR getName() const { return m_description.c_str(); }
	REFCLSID getClsId() const { return m_clsid; }

protected:
	// Constructor
	// Call createDriverList() method to create instance of this class.
	CAsioDriver(const CLSID& clsid, LPCTSTR descriptio);

	CLSID m_clsid;
	tstring m_description;
	CComPtr<IASIO> m_asio;
};
