#pragma once

namespace std {
#if defined(_UNICODE)
	typedef wstring tstring;
	typedef wstringstream tstringstream;
#else
	typedef string tstring;
	typedef stringstream tstringstream;
#endif
}

namespace state_machine {

class Object
{
public:
	virtual LPCTSTR toString();

protected:
	virtual const Object* getObject() const = 0;
	std::tstring m_string;
};

}
