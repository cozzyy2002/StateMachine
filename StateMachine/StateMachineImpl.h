#pragma once

#include <StateMachine/Common.h>
#include <StateMachine/StateMachine.h>

#include <functional>

namespace state_machine {

class StateMachineImpl : public StateMachine
{
public:
	StateMachineImpl(Context& context);
	virtual ~StateMachineImpl();

	HRESULT handleEvent(Event& e);

	virtual void setLoggerName(LPCTSTR loggerName) override { logger = log4cplus::Logger::getInstance(loggerName); }
	virtual const std::tstring& getLoggerName() const override { return logger.getName(); }

protected:
	Context& context;

	std::shared_ptr<State>* findState(std::shared_ptr<State>& currentState, State* pState);
	HRESULT for_each_state(std::shared_ptr<State>& currentState, std::function<HRESULT(std::shared_ptr<State>& state)> func);

	log4cplus::Logger logger;

#define S_FOR_EACH_CONTINUE S_OK
#define S_FOR_EACH_BREAK S_FALSE
};

} // namespace state_machine
