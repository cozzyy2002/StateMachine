#include "stdafx.h"
#include "AppObjects.h"
#include "StateMachineDoc.h"

static log4cplus::Logger logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("AppObjects"));

CAppContext::CAppContext(LPCTSTR name, StateMachine* stateMachine)
	: Context(stateMachine)
	, CAppObject(name)
{
	LOG4CPLUS_DEBUG(logger, __FUNCTION__ ": Creating instance");
}


CAppContext::~CAppContext()
{
	LOG4CPLUS_DEBUG(logger, __FUNCTION__ ": Deleting instance");
}

HRESULT CAppState::handleEvent(Event* e, State* currentState, State** nextState)
{
	HRESULT hr = S_OK;
	m_doc->outputMessage(_T("%s::handleEvent('%s', '%s')"), toString(), e->toString(), currentState->toString());
	return hr;
}

HRESULT CAppState::handleIgnoredEvent(Event* e)
{
	m_doc->outputMessage(_T("%s::handleIgnoredEvent('%s')"), toString(), e->toString());
	return S_OK;
}

HRESULT CAppState::handleError(Event* e, HRESULT hr)
{
	m_doc->outputMessage(_T("%s::handleError('%s', HRESULT=0x%lx)"), toString(), e->toString(), hr);
	return S_OK;
}

HRESULT CAppState::entry(Event* e, State* previousState)
{
	m_doc->outputMessage(_T("%s::entry('%s', '%s')"), toString(), e->toString(), previousState->toString());
	m_doc->onStateEntryCalled(this);
	return S_OK;
}

HRESULT CAppState::exit(Event* e, State* nextState)
{
	m_doc->outputMessage(_T("%s::exit('%s', '%s')"), toString(), e->toString(), nextState->toString());
	m_doc->onStateExitCalled(this);
	return S_OK;
}

CAppEvent::CAppEvent()
{
}

LPCTSTR CAppEvent::getName()
{
	if(m_name.empty()) {
		if(data.is<picojson::object>()) {
			const picojson::object& obj = data.get<picojson::object>();
			const std::string& name = obj.at("name").get<std::string>();
			CA2T _name(name.c_str());
			m_name = (LPCTSTR)_name;
		}
	}
	return m_name.c_str();
}
