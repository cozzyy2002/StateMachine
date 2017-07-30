#include "stdafx.h"
#include "Mocks.h"

/*static*/ MockObject::MockObjects_t MockObject::m_mockObjects;

MockObject::MockObject() : m_id(-1)
{
}

MockObject::MockObject(int id) : m_id(id)
{
	if(m_mockObjects.find(id) == m_mockObjects.end()) {
		m_mockObjects[id] = this;
	} else {
		ADD_FAILURE() << "MockObject constructor: ID " << id << " exists already.";
	}
}


MockObject::~MockObject()
{
	if(m_id != -1) {
		MockObjects_t::iterator it = m_mockObjects.find(m_id);
		if(it != m_mockObjects.end()) {
			it->second = nullptr;
		} else {
			ADD_FAILURE() << "MockObject destructor: ID " << m_id << " does not exist in the list.";
		}
	}
}

/*static*/ void MockObject::clear()
{
	for each (auto item in m_mockObjects) {
		MockObject* obj = item.second;
		if(obj) {
			obj->m_id = -1;		// Suppress ADD_FFAILURE() in destructor.
			delete obj;
		}
	}
	m_mockObjects.clear();
}

/*static*/ bool MockObject::create(int id)
{
	return (m_mockObjects.find(id) != m_mockObjects.end());
}

/*static*/ bool MockObject::deleted(int id)
{
	MockObjects_t::const_iterator it = m_mockObjects.find(id);
	return (it != m_mockObjects.end()) && !it->second;
}
