
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
{
	// TODO: add one-time construction code here

}

CStateMachineDoc::~CStateMachineDoc()
{
}

CAppContext * CStateMachineDoc::createContext(LPCTSTR name)
{
	auto it = m_context_list.find(name);
	if(it == m_context_list.end()) {
		CAppContext* context = new CAppContext(name, m_stateMachine);
		m_context_list[name].reset(context);
		LOG4CPLUS_INFO(logger, "Created context: " << context->toString());
		return context;
	} else{
		return it->second.get();
	}
}

BOOL CStateMachineDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)
	m_stateMachine = m_app->getStateMachine();

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
		// TODO: add loading code here
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


//void CStateMachineDoc::OnButtonContextCreate()
//{
//	GetView
//	m_context.reset(new CAppContext(m_stateMachine));
//}


//void CStateMachineDoc::OnUpdateButtonContextCreate(CCmdUI *pCmdUI)
//{
//	// TODO: Add your command update UI handler code here
//}
