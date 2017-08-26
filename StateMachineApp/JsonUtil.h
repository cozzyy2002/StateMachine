#pragma once

#include <StateMachine/Object.h>

class CJsonUtil
{
public:
	CJsonUtil() {}
	virtual ~CJsonUtil() {}
};

class CJsonObject : public CJsonUtil
{
public:
	CJsonObject() : m_object(emptyObject) {}
	CJsonObject(const picojson::object& obj)
		: m_object(obj) {}
	CJsonObject(const picojson::value& value)
		: m_object(value.is<picojson::object>() ? value.get<picojson::object>() : emptyObject) {}

	std::tstring getString(LPCSTR key, LPCTSTR defaultValue = _T("")) const;
	HRESULT getHRESULT(LPCSTR key = "return", HRESULT defaultValue = S_OK) const;
	bool getBool(LPCSTR key, bool defaultValue = false) const;
	const picojson::array& getArray(LPCSTR key) const;
	CJsonObject getObject(LPCSTR key) const;

	// Returns integral type value associated with key.
	template<typename T>
	T getJsonValue(LPCSTR key, const T defaultValue) const;

	// Returns object/array of picojson associated with key.
	template<typename T>
	const T& getJsonObject(LPCSTR key, const T& defaultObject) const;

protected:
	// Empty object returned when specified key does not exist.
	static const picojson::object emptyObject;
	// Empty array returned when specified key does not exist.
	static const picojson::array emptyArray;

	const picojson::object& m_object;
};

template<typename T>
inline T CJsonObject::getJsonValue(LPCSTR key, const T defaultValue) const
{
	auto ret(defaultValue);
	auto it = m_object.find(key);
	if(it != m_object.end() && it->second.is<T>()) {
		ret = it->second.get<T>();
	}
	return ret;
}

template<typename T>
inline const T & CJsonObject::getJsonObject(LPCSTR key, const T& defaultObject) const
{
	auto it = m_object.find(key);
	if(it != m_object.end() && it->second.is<T>()) {
		return it->second.get<T>();
	}
	return defaultObject;
}
