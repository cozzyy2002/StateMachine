#include "stdafx.h"
#include "Mocks.h"

static log4cplus::Logger logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("Test.Mocks"));

/*static*/ MockObject::MockObjects_t MockObject::m_mockObjects;

MockObject::MockObject() : m_id(MockObjectId::UNKNOWN)
{
	LOG4CPLUS_DEBUG(logger, "Creating MockObject: " << m_id.toString());
}

MockObject::MockObject(MockObjectId id) : m_id(id)
{
	LOG4CPLUS_DEBUG(logger, "Creating MockObject: " << m_id.toString());
	if(m_mockObjects.find(id) == m_mockObjects.end()) {
		m_mockObjects[id] = this;
	} else {
		ADD_FAILURE() << "MockObject constructor: " << id << " exists already.";
	}
}


MockObject::~MockObject()
{
	LOG4CPLUS_DEBUG(logger, "Deleting MockObject: " << m_id.toString());
	if(m_id != MockObjectId::UNKNOWN) {
		auto it = m_mockObjects.find(m_id);
		if(it != m_mockObjects.end()) {
			it->second = nullptr;
		} else {
			ADD_FAILURE() << "MockObject destructor: " << m_id.toString() << " does not exist in the list.";
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

/*static*/ bool MockObject::created(MockObjectId id)
{
	return (m_mockObjects.find(id) != m_mockObjects.end());
}

/*static*/ bool MockObject::deleted(MockObjectId id)
{
	auto it = m_mockObjects.find(id);
	return (it != m_mockObjects.end()) && !it->second;
}

LPCTSTR MockState::toString()
{
	if(m_string.empty()) {
		std::tstringstream stream;
		stream << Object::toString() << _T("(") << m_id.toString() << _T(")");
		m_string = stream.str();
	}
	return m_string.c_str();
}
