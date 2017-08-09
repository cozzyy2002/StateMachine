#include "stdafx.h"
#include "Object.h"

using namespace state_machine;

LPCTSTR Object::toString()
{
	if(m_string.empty()) {
		CA2T _string(typeid(*getObject()).name());
		m_string = _string;
	}
	return m_string.c_str();
}
