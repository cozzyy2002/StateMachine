#include "stdafx.h"
#include "AppObjects.h"
#include "StateMachineDoc.h"

static log4cplus::Logger logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("AppObjects"));

CAppContext::CAppContext(CStateMachineDoc* doc, LPCTSTR name, StateMachine& stateMachine)
	: Context(stateMachine)
	, CAppObject(doc, name)
{
	LOG4CPLUS_DEBUG(logger, __FUNCTION__ ": Creating instance");
}


CAppContext::~CAppContext()
{
	LOG4CPLUS_DEBUG(logger, __FUNCTION__ ": Deleting instance");
}

HRESULT CAppState::handleEvent(Event& e, State& currentState, State** nextState)
{
	HRESULT hr = S_OK;
	doc->outputMessage(_T("%s::handleEvent('%s', '%s')"), toString(), e.toString(), currentState.toString());
	return hr;
}

HRESULT CAppState::handleIgnoredEvent(Event& e)
{
	doc->outputMessage(_T("%s::handleIgnoredEvent('%s')"), toString(), e.toString());
	return S_OK;
}

HRESULT CAppState::handleError(Event& e, HRESULT hr)
{
	doc->outputMessage(_T("%s::handleError('%s', HRESULT=0x%lx)"), toString(), e.toString(), hr);
	return S_OK;
}

HRESULT CAppState::entry(Event& e, State& previousState)
{
	doc->outputMessage(_T("%s::entry('%s', '%s')"), toString(), e.toString(), previousState.toString());
	doc->onStateEntryCalled(this);
	return S_OK;
}

HRESULT CAppState::exit(Event& e, State& nextState)
{
	doc->outputMessage(_T("%s::exit('%s', '%s')"), toString(), e.toString(), nextState.toString());
	doc->onStateExitCalled(this);
	return S_OK;
}

CAppEvent::CAppEvent(CStateMachineDoc* doc, LPCTSTR name, const picojson::value& config)
	: CAppObject(doc, name), config(config)
{
}
