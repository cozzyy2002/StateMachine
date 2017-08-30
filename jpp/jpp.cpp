// jpp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <Shlwapi.h>

namespace std {
#ifdef _UNICODE
typedef wcout tcout;
#else
typedef cout tcout;
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

static int checkOptions(TCHAR* options);

int _tmain(int argc, TCHAR *argv[])
{
	int ret = 0;
	int i;
	for(i = 1; i < argc; i++) {
		if(*argv[i] == _T('-')) {
			ret = checkOptions(argv[i] + 1);
			if(ret) return;
		}
	}

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
	return ret;
}

/*static*/ int checkOptions(TCHAR* options)
{
	for(auto p = options; *p; p++) {
		switch(*p) {
		case 'c':
		case 's':
		case 'r':
		case 'e':
		default:
			std::tcout << _T("Unknown option: ") << *p << std::endl;
			return 1;
		}
	}
}
