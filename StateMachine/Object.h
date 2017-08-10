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
	virtual const Object* getObject() const { return this; }
	std::tstring m_string;
};

// Class to reset value when it goes out of the scope.
template<typename T>
class ScopedStore
{
public:
	// Current value will be restored on destruct.
	ScopedStore(T& store) : store(store) { finaleValue = store; }
	// Specified final value will be stored on destruct.
	ScopedStore(T& store, T finalValue ) : store(store), finalValue(finalValue) {}
	// Specified final value will be stored on destruct, and initial value can be specified.
	ScopedStore(T& store, T finalValue, T initialValue) : store(store), finalValue(finalValue) { store = initialValue; }
	~ScopedStore() { store = finalValue; }

protected:
	T& store;
	T finalValue;
};

}
