#include "stdafx.h"
#include "Mocks.h"

static log4cplus::Logger logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("Test.Mocks"));

/*static*/ MockObject::MockObjects_t MockObject::m_mockObjects;

MockObject::MockObject() : m_id(-1)
{
	LOG4CPLUS_DEBUG(logger, "Creating MockObject: ID=" << m_id);
}

MockObject::MockObject(int id) : m_id(id)
{
	LOG4CPLUS_DEBUG(logger, "Creating MockObject: ID=" << m_id);
	if(m_mockObjects.find(id) == m_mockObjects.end()) {
		m_mockObjects[id] = this;
	} else {
		ADD_FAILURE() << "MockObject constructor: ID " << id << " exists already.";
	}
}


MockObject::~MockObject()
{
	LOG4CPLUS_DEBUG(logger, "Deleting MockObject: ID=" << m_id);
	if(m_id != -1) {
		auto it = m_mockObjects.find(m_id);
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
		auto obj = item.second;
		if(obj) delete obj;
	}
	m_mockObjects.clear();
}

/*static*/ bool MockObject::create(int id)
{
	return (m_mockObjects.find(id) != m_mockObjects.end());
}

/*static*/ bool MockObject::deleted(int id)
{
	auto it = m_mockObjects.find(id);
	return (it != m_mockObjects.end()) && !it->second;
}

LPCTSTR MockState::toString() const
{
	if(m_string.empty()) {
		CA2CT _string(typeid(*this).name());
		m_string = (LPCTSTR)_string;
	}
	return m_string.c_str();
}
