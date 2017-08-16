
// StateMachineView.cpp : implementation of the CStateMachineView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "StateMachineApp.h"
#endif

#include "StateMachineDoc.h"
#include "StateMachineView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static log4cplus::Logger logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("StateMachineView"));

// CStateMachineView

IMPLEMENT_DYNCREATE(CStateMachineView, CFormView)

BEGIN_MESSAGE_MAP(CStateMachineView, CFormView)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_BN_CLICKED(IDC_BUTTON_CONTEXT_CREATE, &CStateMachineView::OnClickedButtonContextCreate)
	ON_BN_CLICKED(IDC_BUTTON_CONTEXT_START, &CStateMachineView::OnClickedButtonContextStart)
END_MESSAGE_MAP()

// CStateMachineView construction/destruction

CStateMachineView::CStateMachineView()
	: CFormView(IDD_STATEMACHINEAPP_FORM)
	, m_contextName(_T(""))
	, m_StateName(_T(""))
	, m_isSubState(FALSE)
{
}

CStateMachineView::~CStateMachineView()
{
}

void CStateMachineView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_CONTEXT_NAME, m_contextName);
	DDX_Text(pDX, IDC_EDIT_STATE_NAME, m_StateName);
	DDX_Check(pDX, IDC_CHECK_IS_SUB__STATE, m_isSubState);
}

BOOL CStateMachineView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CFormView::PreCreateWindow(cs);
}

void CStateMachineView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	ResizeParentToFit();

}

void CStateMachineView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CStateMachineView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CStateMachineView diagnostics

#ifdef _DEBUG
void CStateMachineView::AssertValid() const
{
	CFormView::AssertValid();
}

void CStateMachineView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

CStateMachineDoc* CStateMachineView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CStateMachineDoc)));
	return (CStateMachineDoc*)m_pDocument;
}
#endif //_DEBUG


// CStateMachineView message handlers


void CStateMachineView::OnClickedButtonContextCreate()
{
	CStateMachineDoc* doc = GetDocument();
	UpdateData();
	doc->m_context.reset(new CAppContext(m_contextName, doc->m_stateMachine));

	LOG4CPLUS_INFO(logger, "Created context: " << doc->m_context->toString());
}


void CStateMachineView::OnClickedButtonContextStart()
{
	CStateMachineDoc* doc = GetDocument();
	UpdateData();
}
