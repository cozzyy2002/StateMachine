#include "stdafx.h"
#include "AppObjects.h"

static log4cplus::Logger logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("AppObjects"));

CAppContext::CAppContext(LPCTSTR name, StateMachine* stateMachine)
	: Context(stateMachine)
	, m_name(name)
{
	LOG4CPLUS_DEBUG(logger, __FUNCTION__ ": Creating instance");
}


CAppContext::~CAppContext()
{
	LOG4CPLUS_DEBUG(logger, __FUNCTION__ ": Deleting instance");
}
