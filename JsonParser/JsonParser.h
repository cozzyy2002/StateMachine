#pragma once

#include <memory>

namespace json_parser {

class CJsonParser
{
public:
	CJsonParser();
	~CJsonParser();

	struct Option {
		// Constructor for remove space.
		Option(bool removeSpace, bool removeEol)
			: removeComment(true), removeSpace(removeSpace), removeEol(removeEol)
			, expandTab(false), tabStop(0) {}
		// Constructor for expand tab.
		Option(unsigned int tabStop, bool removeComment = false, bool removeEol = false)
			: removeComment(removeComment), removeSpace(false), removeEol(removeEol)
			, expandTab(true), tabStop(tabStop) {}

		bool removeComment;		// Remove comment.
		bool removeSpace;		// Remove space(' '), tab('\t')
		bool removeEol;			// Remove CR('\r'), LF('\n')
		bool expandTab;			// Expand tab.
		unsigned int tabStop;	// Tab stop position(Used if expandTab == true).
	};

	void removeComment(LPCTSTR source, bool preserveEol, std::tstring& out);
	void preprocess(LPCTSTR source, const Option& option, std::tstring& out);

protected:
	std::unique_ptr<state_machine::StateMachine> m_stateMachine;
};

}
