
// StateMachineDoc.cpp : implementation of the CStateMachineDoc class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "StateMachineApp.h"
#endif

#include "StateMachineDoc.h"
#include "AppObjects.h"
#include "MainFrm.h"
#include "../JsonParser/JsonParser.h"

#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static log4cplus::Logger logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("StateMachineDoc"));

// CStateMachineDoc

IMPLEMENT_DYNCREATE(CStateMachineDoc, CDocument)

BEGIN_MESSAGE_MAP(CStateMachineDoc, CDocument)
//	ON_COMMAND(IDC_BUTTON_CONTEXT_CREATE, &CStateMachineDoc::OnButtonContextCreate)
//	ON_UPDATE_COMMAND_UI(IDC_BUTTON_CONTEXT_CREATE, &CStateMachineDoc::OnUpdateButtonContextCreate)
END_MESSAGE_MAP()


// CStateMachineDoc construction/destruction

CStateMachineDoc::CStateMachineDoc()
	: m_app((CStateMachineApp*)AfxGetApp())
	, m_config(new CJsonObject())
{
	// TODO: add one-time construction code here

}

CStateMachineDoc::~CStateMachineDoc()
{
}

CAppContext * CStateMachineDoc::createContext(LPCTSTR name)
{
	auto isAsync(m_config->getBool("is_async"));
	CAppContext* context = new CAppContext(isAsync, this, name);
	m_context.reset(context);
	outputMessage(_T("Created CAppContext: '%s'(%s)"),
					context->toString(), context->isAsync() ? _T("AsyncContext") : _T("Context"));
	SetTitle(nullptr);
	return context;
}

CAppState * CStateMachineDoc::createState(LPCTSTR name, BOOL isSubState)
{
	CAppState* state = new CAppState(name, isSubState ? true : false);
	outputMessage(_T("Created CAppState: '%s'"), state->toString());
	return state;
}

HRESULT CStateMachineDoc::start(LPCTSTR stateName, CAppEvent* e /*= nullptr*/)
{
	// Own Event* in this scope.
	std::unique_ptr<CAppEvent> _e(e);

	CAppState* state = createState(stateName, false);

	// Get name of state object.
	// state object might be deleted after calling Context::start() in async Config.
	std::tstring _stateName(state->toString());
	HRESULT hr;
	if(_e) {
		if(!m_context->isAsync()) {
			hr = m_context->start(state, *_e);
		} else {
			hr = m_context->start(state, _e.release());
		}
	} else {
		hr = m_context->start(state);
	}
	outputMessage(_T("Started CAppContext '%s': Initial state is '%s', HRESULT=0x%lx"),
					m_context->toString(), _stateName.c_str(), hr);
	SetTitle(nullptr);
	return hr;
}

bool CStateMachineDoc::parse(LPCTSTR source)
{
	std::tstring preParsed;
	json_parser::CJsonParser jsonParser;
	jsonParser.removeComment(source, true, preParsed);
	CT2A _source(preParsed.c_str());
	std::string error = picojson::parse(m_configJson, (LPCSTR)_source);
	std::tstring _error;
	if(error.empty()) {
		m_configSource = source;
		m_config.reset(new CJsonObject(m_configJson));
		UpdateAllViews(nullptr, (LPARAM)UpdateViewHint::ConfigParsed, this);
		return true;
	} else {
		CA2T _error(error.c_str());
		outputMessage(_T("Parsing JSON failed: %s"), (LPCTSTR)_error);
		return false;
	}
}

void CStateMachineDoc::onStateEntryCalled(CAppState * state)
{
	m_stateStack.clear();
	for(auto st = state; st; st = st->getMasterState()) {
		m_stateStack.push_back(st);
	}
	UpdateAllViews(nullptr, (LPARAM)UpdateViewHint::StateChanged, this);
}

void CStateMachineDoc::onStateExitCalled(CAppState * state)
{
}

/*
	Format message and output to debug window.
*/
void CStateMachineDoc::outputMessage(LPCTSTR format, ...)
{
	va_list arg;
	va_start(arg, format);
	TCHAR message[256];
	_vstprintf_s(message, format, arg);
	LOG4CPLUS_DEBUG(logger, message);
	CMainFrame* frame = (CMainFrame*)AfxGetApp()->GetMainWnd();
	if(frame) {
		frame->OutputMessage(message);
	}
}

BOOL CStateMachineDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}




// CStateMachineDoc serialization

void CStateMachineDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// Read ANSI text file.
		m_configSource.Empty();
		auto f = ar.GetFile();
		auto len = (size_t)f->GetLength();
		auto buf(std::make_unique<char[]>(len + 1));
		f->Read(buf.get(), len);
		buf[len] = '\0';
		m_configSource = buf.get();
	}
}

#ifdef SHARED_HANDLERS

// Support for thumbnails
void CStateMachineDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// Modify this code to draw the document's data
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// Support for Search Handlers
void CStateMachineDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// Set search contents from document's data. 
	// The content parts should be separated by ";"

	// For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CStateMachineDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CStateMachineDoc diagnostics

#ifdef _DEBUG
void CStateMachineDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CStateMachineDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CStateMachineDoc commands


void CStateMachineDoc::SetTitle(LPCTSTR lpszTitle)
{
	std::tstring title;
	if(lpszTitle) {
		// Called by framework on [File]->[Open]
		// Show opened file name.
		// lpszTitle already points file name.
	} else {
		// Called by this when configuration is changed.
		// Show configuration status(started or stopped).
		if(m_context) {
			title = m_context->getName();
			title += m_context->isStarted() ? _T(":Started") : _T(":Stopped");
		} else {
			title = _T("<Unknown>");
		}
		lpszTitle = title.c_str();
	}
	CDocument::SetTitle(lpszTitle);
}
