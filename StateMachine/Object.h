#pragma once

#include <mutex>

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

/*
	Base class for Context, State and Event
*/
class Object
{
public:
	// Returns string representation.
	// If modify the string in derived class, override modifyString() method.
	LPCTSTR toString() const;

protected:
	virtual const Object* getObject() const { return this; }

	// Modify m_string in derived class.
	// This method is called by toString().
	virtual void modifyString(std::tstring& _string) const {}

private:
#pragma region mutable members used in toString() const method.
	mutable std::tstring m_string;
	mutable std::mutex m_stringLock;
#pragma endregion
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
