#include "stdafx.h"
#include "JsonParserObject.h"

using namespace json_parser;

json_parser::CParserContext::CParserContext(state_machine::StateMachine & stateMachine)
	: Context(stateMachine)
{
}
