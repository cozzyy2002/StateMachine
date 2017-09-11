#pragma once

#include "Common.h"

#if defined(NO_INLINE_METHOD)
#define INLINE
#else
#define INLINE inline
#endif

namespace state_machine {

/*
	Base class for Context, State and Event
*/
class Object
{
	// Disallow copy constructor and assignment operator.
	Object(const Object&) = delete;
	Object& operator=(const Object&) = delete;

public:
	// Returns this pointer as user class type.
	// Do NOT delete returned object.
	template<class T>
	INLINE T* cast() { return dynamic_cast<T*>(this); }

	// Returns string representation.
	// If modify the string in derived class, override modifyString() method.
	// Do NOT delete returned string.
	LPCTSTR toString();

protected:
	Object() {}
	// Do NOT delete returned object.
	INLINE virtual const Object* getObject() const { return this; }

	// Modify m_string in derived class.
	// This method is called by toString().
	virtual void modifyString(std::tstring& _string) {}

private:
	mutable std::tstring m_string;
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
