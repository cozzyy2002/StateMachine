#pragma once

namespace std {
#if defined(_UNICODE)
typedef wstring tstring;
typedef wstringstream tstringstream;
typedef wostringstream tostringstream;
typedef wistringstream tistringstream;
typedef wistream tistream;
typedef wostream tostream;
#else
typedef string tstring;
typedef stringstream tstringstream;
typedef ostringstream tostringstream;
typedef istringstream tistringstream;
typedef istream tistream;
typedef ostream tostream;
#endif
}
