#include "stdafx.h"
#include "AppObjects.h"
#include "StateMachineDoc.h"
#include "JsonUtil.h"

static log4cplus::Logger logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("AppObjects"));

CAppContext::CAppContext(bool isAsync, CStateMachineDoc* doc, LPCTSTR name)
	: Context(isAsync)
	, doc(doc)
	, CAppObject(name)
{
	LOG4CPLUS_DEBUG(logger, __FUNCTION__ ": Creating instance");
}


CAppContext::~CAppContext()
{
	LOG4CPLUS_DEBUG(logger, __FUNCTION__ ": Deleting instance");
}

HRESULT CAppState::handleEvent(Context& context, Event& e, State& currentState, State** nextState)
{
	HRESULT hr = S_OK;
	auto _context = context.cast<CAppContext>();
	auto ev = e.cast<CAppEvent>();
	_context->doc->outputMessage(_T("%s::handleEvent('%s', '%s')"), toString(), e.toString(), currentState.toString());
	return hr;
}

HRESULT CAppState::handleIgnoredEvent(Context& context, Event& e)
{
	auto _context = context.cast<CAppContext>();
	_context->doc->outputMessage(_T("%s::handleIgnoredEvent('%s')"), toString(), e.toString());
	return S_OK;
}

HRESULT CAppState::handleError(Context& context, Event& e, HRESULT hr)
{
	auto _context = context.cast<CAppContext>();
	_context->doc->outputMessage(_T("%s::handleError('%s', HRESULT=0x%lx)"), toString(), e.toString(), hr);
	return S_OK;
}

HRESULT CAppState::entry(Context& context, Event& e, State& previousState)
{
	auto _context = context.cast<CAppContext>();
	_context->doc->outputMessage(_T("%s::entry('%s', '%s')"), toString(), e.toString(), previousState.toString());
	_context->doc->onStateEntryCalled(this);
	return S_OK;
}

HRESULT CAppState::exit(Context& context, Event& e, State& nextState)
{
	auto _context = context.cast<CAppContext>();
	_context->doc->outputMessage(_T("%s::exit('%s', '%s')"), toString(), e.toString(), nextState.toString());
	_context->doc->onStateExitCalled(this);
	return S_OK;
}

CStateConfig::CStateConfig(const picojson::value & config)
{
	CJsonObject jsonObj(config);
	// Construct Do information.
	auto doMethod(jsonObj.getObject("do"));
	Do.Return = doMethod.getHRESULT();
	auto nextState(doMethod.getObject("next_state"));
	Do.NextState.Name = nextState.getString("name");
	Do.Return = nextState.getHRESULT();
	// Construct Entry information.
	auto entryMethod(jsonObj.getObject("entry"));
	Entry.Return = entryMethod.getHRESULT();
	// Construct Exit information.
	auto exitMethod(jsonObj.getObject("exit"));
	Exit.Return = exitMethod.getHRESULT();
}

CAppEvent::CAppEvent(LPCTSTR name, const picojson::value& config)
	: CAppObject(name), config(config)
{
}
