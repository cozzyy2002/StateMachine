// jpp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../JsonParser/JsonParser.h"
#include <iostream>
#include <Shlwapi.h>
#include <locale>

using namespace json_parser;

namespace std {
#ifdef _UNICODE
static auto& tcout(wcout);
#else
static auto& tcout(cout);
#endif
}

static auto usage =
"JSON preprocessor\n"
"Usage: jpp [-csre [N]] [json]\n"
"       -c:      Remove comment(default)\n"
"       -s:      Remove Space and Tab\n"
"       -r:      Remove EOL(CR and LF)\n"
"       -e [N]:  Expand tab for each N columns(default=2)\n"
"       Note: -s and -e are not specified with each other.\n"
"       json: JSON file to preprocess.\n"
"             If `json` is existing file, jpp reads JSON text from the file.\n"
"             Else jpp recognizes `json` as JSON text.\n"
"             If this argument is omitted, jpp reads STDIN.\n"
;

static const int defaultTabStop = 2;
static int checkArgs(int argc, TCHAR* argv[], CJsonParser::Option& option);
static int checkOptions(TCHAR* options, CJsonParser::Option& option);

int _tmain(int argc, TCHAR *argv[])
{
	CJsonParser::Option option;
	auto i = checkArgs(argc, argv, option);
	if(i < argc) {
		auto fileName = argv[i];
		if(PathFileExists(fileName) && !PathIsDirectory(fileName)) {
			// Existing file
		} else {
			// JSON text in command line.
		}
	} else {
		// Read JSON from STDIN.
	}
	return 0;
}

enum {
	CHECK_OPTIONS_OK,
	CHECK_OPTIONS_TAB_STOP,
	CHECK_OPTIONS_ERROR,
};

/*static*/ int checkArgs(int argc, TCHAR* argv[], CJsonParser::Option& option)
{
	int i;
	for(i = 1; i < argc; i++) {
		if(*argv[i] == _T('-')) {
			switch(checkOptions(argv[i] + 1, option)) {
			case CHECK_OPTIONS_OK:
				break;
			case CHECK_OPTIONS_TAB_STOP: {
					auto str = argv[i + 1];
					std::locale loc;
					if(isdigit(*str, loc)) {
						// Next argument should be legal number as tab stop.
						auto tabStop = _tstoi(argv[i]);
						if((1 < tabStop) || (tabStop <= 8) && (errno == 0)) {
							option.tabStop = (unsigned int)tabStop;
						} else {
							std::tcout << _T("Illegal tab stop: ") << argv[i] << std::endl;
							return -1;
						}
					} else {
						option.tabStop = defaultTabStop;
					}
				}
				break;
			default:
				// Error
				return -1;
			}
		}
	}
	return i;
}

/*static*/ int checkOptions(TCHAR* options, CJsonParser::Option& option)
{
	int ret = CHECK_OPTIONS_OK;
	for(auto p = options; *p; p++) {
		switch(*p) {
		case 'c': option.removeComment = true; break;
		case 's': option.removeSpace = true; break;
		case 'r': option.removeEol = true; break;
		case 'e': option.expandTab = true; ret = CHECK_OPTIONS_TAB_STOP; break;
		default:
			std::tcout << _T("Unknown option: ") << *p << std::endl;
			return CHECK_OPTIONS_ERROR;
		}
	}
	return ret;
}
