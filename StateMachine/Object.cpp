#include "stdafx.h"
#include "Object.h"

#if defined(_DEBUG)
#include <iomanip>
#endif

using namespace state_machine;

LPCTSTR Object::toString()
{
	if(m_string.empty()) {
		CA2T _string(typeid(*getObject()).name());
#if defined(_DEBUG)
		// Creates "0xXXXXXXXX:class C..C".
		// Where XXXXXXXX is this pointer and C...C is class name.
		std::tstringstream stream;
		stream << _T("0x") << std::hex << std::setw(8) << (void*)this
				<< _T(":") << (LPCTSTR)_string;
		m_string = stream.str();
#else
		m_string = _string;
#endif
	}
	return m_string.c_str();
}
