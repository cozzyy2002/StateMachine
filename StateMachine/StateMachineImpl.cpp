#include "stdafx_local.h"
#include <StateMachine/Event.h>
#include <StateMachine/State.h>
#include <StateMachine/Context.h>

#include "StateMachineImpl.h"
#include "Handles.h"

#include <log4cplus/configurator.h>
#include <Shlwapi.h>

using namespace state_machine;

auto globalLogger = log4cplus::Logger::getInstance(globalLoggerName);

static std::tstring logConfigFileName;

void state_machine::configureLog(LPCTSTR configFileName)
{
	if(logConfigFileName.empty()) {
		logConfigFileName = configFileName;
		log4cplus::PropertyConfigurator::doConfigure(configFileName);
	} else if(logConfigFileName != configFileName) {
		LOG4CPLUS_ERROR(globalLogger, configFileName << _T(": Already configured by ") << logConfigFileName.c_str());
	} else { /* Already configured by same file. Do nothing */ }
}

StateMachineImpl::StateMachineImpl(Context& context)
	: context(context)
{
	if(logConfigFileName.empty()) {
		// Log is not configured yet.
		logConfigFileName = defaultLogConigFileName;
		auto logConfig(logConfigFileName.c_str());
		if(PathFileExists(logConfig) && !PathIsDirectory(logConfig)) {
			log4cplus::PropertyConfigurator::doConfigure(logConfig);
		} else {
			log4cplus::BasicConfigurator::doConfigure();
		}
	}

	logger = log4cplus::Logger::getInstance(stateMachineDefaultLoggerName);
}

StateMachineImpl::~StateMachineImpl()
{
}

HRESULT StateMachineImpl::handleEvent(Event& e)
{
	auto hContext = context.getHandle();

	if(logger.isEnabledFor(e.getLogLevel())) {
		// Suppress low level log output.
		std::tstringstream stream;
		stream << "Handling " << e.toString() << " in " << context.toString();
		logger.forcedLog(e.getLogLevel(), stream.str());
	}

	// Recursive call check.
	HR_ASSERT(!hContext->isEventHandling(), E_ILLEGAL_METHOD_CALL);
	ScopedStore<bool> _recursive_guard(hContext->m_isEventHandling, false, true);

	// Lock this scope(If necessary)
	std::unique_ptr<std::lock_guard<std::mutex>> _lock(context.getStateLock());

	// Current state is contained by Context object in the Event.
	auto& currentState(hContext->currentState);

	std::shared_ptr<State> nextState;
	bool callEntry(false);
	auto hr(State::S_EVENT_IGNORED);
	e.isHandled = false;

	// Call State::handleEvent()
	// If event is ignored and the state is sub state, delegate handling event to master state.
	// Note: Do not use HR_ASSERT_OK() macro to prevent log from long source code when error occurs.
	auto hr_for = for_each_state(currentState, [&](std::shared_ptr<State>& state)
	{
		LOG4CPLUS_DEBUG(logger, "Calling " << state->toString() << "::handleEvent()");
		State* pNextState = nullptr;
		hr = HR_EXPECT_OK(state->handleEvent(context, e, *currentState, &pNextState));
		// Set Event::isHandled.
		// If state transition occurs, assume that the event is handled even if S_EVENT_IGNORED was returned.
		// Setting true by State::handleEvent() is prior to above condition.
		if(!e.isHandled) e.isHandled = ((hr != State::S_EVENT_IGNORED) || pNextState);
		if(pNextState) {
			auto nextMasterState = findState(currentState, pNextState);
			if(nextMasterState) {
				// Back to existing master state.
				HR_ASSERT(state->isSubState(), E_UNEXPECTED);
				nextState = *nextMasterState;
			} else {
				// Exit to newly created state.
				// Note: Object returned to pNextState might be deleted,
				//       if nextState goes out of scope before it is set as current state.
				nextState.reset(pNextState);
				callEntry = true;
			}
		}
		if(FAILED(hr)) {
			LOG4CPLUS_DEBUG(logger, "Calling " << state->toString() << "::handleError()");
			HR_ASSERT_OK(state->handleError(context, e, hr));
		}
		if(!e.isHandled) {
			LOG4CPLUS_DEBUG(logger, "Calling " << state->toString() << "::handleIgnoredEvent()");
			HR_ASSERT_OK(state->handleIgnoredEvent(context, e));
			return S_FOR_EACH_CONTINUE;
		} else {
			return S_FOR_EACH_BREAK;
		}
	});
	if(FAILED(hr_for)) return hr_for;

	if(SUCCEEDED(hr) && nextState) {
		LOG4CPLUS_INFO(logger, "Next state is " << nextState->toString());
		// State transition occurred.
		// Transition to other state or master state of current state.
		// Call exit() of current state and master state if any.
		hr = for_each_state(currentState, [this, &e, &nextState](std::shared_ptr<State>& state)
		{
			// If the state is next state or master state of next state,
			// don't call it's exit().
			if((state.get() == nextState.get()) ||
			   (state.get() == nextState->getMasterState()))
			{
				return S_FOR_EACH_BREAK;
			}

			LOG4CPLUS_DEBUG(logger, "Calling " << state->toString() << "::exit()");
			HR_ASSERT_OK(state->exit(context, e, *nextState));
			return S_FOR_EACH_CONTINUE;
		});
		if(FAILED(hr)) return hr;

		// Preserve current state as previous state until calling entry() of next stete.
		// Note: Current state and it's master state(which is not next state) will be deleted when current state is updated.
		auto previousState(currentState);
		currentState = nextState;
		if(callEntry) {
			LOG4CPLUS_DEBUG(logger, "Calling " << currentState->toString() << "::entry()");
			HR_ASSERT_OK(currentState->entry(context, e, *previousState));
		}
	}

	LOG4CPLUS_DEBUG(logger, e.toString()
							<< " is " << (e.isHandled ? "handled" : "ignored")
							<< ". HRESULT=0x" << std::hex << hr);

	return hr;
}

// Find state in master state of current state.
std::shared_ptr<State>* StateMachineImpl::findState(std::shared_ptr<State>& currentState, State* pState)
{
	std::shared_ptr<State>* ret = nullptr;
	for_each_state(currentState, [pState, &ret](std::shared_ptr<State>& state)
	{
		if(pState == state.get()) {
			ret = &state;
			return S_FOR_EACH_BREAK;
		}
		return S_FOR_EACH_CONTINUE;
	});
	return ret;
}

// Applies specified function to each state within master state(s) of current state.
HRESULT StateMachineImpl::for_each_state(std::shared_ptr<State>& currentState, std::function<HRESULT(std::shared_ptr<State>& state)> func)
{
	auto hr(S_FALSE);
	for(auto state(&currentState);
		state && state->get();
		state = (*state)->isSubState() ? &((*state)->getHandle<SubStateHandle>()->masterState) : nullptr)
	{
		hr = func(*state);
		if(hr != S_FOR_EACH_CONTINUE) return hr;
	}
	return hr;
}
