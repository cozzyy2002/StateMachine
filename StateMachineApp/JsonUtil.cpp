#include "stdafx.h"
#include "JsonUtil.h"

/*static*/ const picojson::object CJsonObject::emptyObject;
/*static*/ const picojson::array CJsonObject::emptyArray;

std::tstring CJsonObject::getString(LPCSTR key, LPCTSTR defaultValue /*=_T("")*/) const
{
	std::tstring ret(defaultValue);
	auto it = m_object.find(key);
	if(it != m_object.end()) {
		CA2T _ret(it->second.get<std::string>().c_str());
		ret = (LPCTSTR)_ret;
	}
	return ret;
}

HRESULT CJsonObject::getHRESULT(LPCSTR key, HRESULT defaultValue) const
{
	return (HRESULT)getJsonValue<double>(key, defaultValue);
}

bool CJsonObject::getBool(LPCSTR key, bool defaultValue) const
{
	return getJsonValue<bool>(key, defaultValue);
}

const picojson::array & CJsonObject::getArray(LPCSTR key) const
{
	return getJsonObject<picojson::array>(key, emptyArray);
}

CJsonObject CJsonObject::getObject(LPCSTR key) const
{
	return CJsonObject(getJsonObject<picojson::object>(key, emptyObject));
}
