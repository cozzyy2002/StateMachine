
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
	ON_BN_CLICKED(IDC_BUTTON_PARSE, &CStateMachineView::OnClickedButtonParse)
	ON_BN_CLICKED(IDC_BUTTON_HANDLE_EVENT, &CStateMachineView::OnClickedButtonHandleEvent)
	ON_BN_CLICKED(IDC_BUTTON_POST_EVENT, &CStateMachineView::OnClickedButtonPostEvent)
END_MESSAGE_MAP()

// CStateMachineView construction/destruction

CStateMachineView::CStateMachineView()
	: CFormView(IDD_STATEMACHINEAPP_FORM)
	, m_contextName(_T(""))
	, m_config(_T(""))
{
}

CStateMachineView::~CStateMachineView()
{
}

void CStateMachineView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_CONTEXT_NAME, m_contextName);
	DDX_Text(pDX, IDC_EDIT_CONFIG, m_config);
	DDX_Control(pDX, IDC_LIST_ACTIVE_STATES, m_activeStates);
	DDX_Control(pDX, IDC_COMBO_EVENT_NAMES, m_eventNames);
}

BOOL CStateMachineView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CFormView::PreCreateWindow(cs);
}

enum class ActiveStatesColumn {
	Name,
	IsSubState,
};

void CStateMachineView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	ResizeParentToFit();

	m_activeStates.InsertColumn((int)ActiveStatesColumn::Name, _T("name"));
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
	m_context = doc->createContext(m_contextName);
}


void CStateMachineView::OnClickedButtonContextStart()
{
	CStateMachineDoc* doc = GetDocument();
	UpdateData();
	CAppEvent* e = nullptr;
	auto index = m_eventNames.GetCurSel();
	if(0 <= index) {
		CString name;
		m_eventNames.GetLBText(index, name);
		auto eventConfig = (const picojson::value*)m_eventNames.GetItemDataPtr(index);
		e = new CAppEvent(doc, name, *eventConfig);
	}
	auto state = doc->getConfigString(doc->getConfig(), "initial_state");
	doc->start(state.c_str(), e);
}


void CStateMachineView::OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* pHint)
{
	auto doc = (CStateMachineDoc*)pHint;
	if(doc) {
		const picojson::value& config = doc->getConfig();

		// Set context name.
		std::tstring contextName(doc->getConfigString(config, "name"));
		if(!contextName.empty()) {
			m_contextName = contextName.c_str();
		}

		// Create contents of event name list.
		int selected = m_eventNames.GetCurSel();
		m_eventNames.ResetContent();
		const picojson::array& events = doc->getConfigObject<picojson::array>(config, "events");
		for each(auto event in events) {
			auto index = m_eventNames.AddString(doc->getConfigString(event, "name").c_str());
			m_eventNames.SetItemDataPtr(index, &event);
		}
		int count = m_eventNames.GetCount();
		if(0 < count) {
			if(selected < 0) selected = 0;
			selected = (selected < count) ? selected : (count - 1);
			m_eventNames.SetCurSel(selected);
		}

		// Create contents of active state list.
		m_activeStates.DeleteAllItems();
		for each(auto state in doc->m_stateStack) {
			m_activeStates.InsertItem(0, state->getName());
		}

		UpdateData(FALSE);
	}
}


void CStateMachineView::OnClickedButtonParse()
{
	CStateMachineDoc* doc = GetDocument();
	UpdateData();

	doc->parse(m_config);
}


void CStateMachineView::OnClickedButtonHandleEvent()
{
	// TODO: Add your control notification handler code here
}


void CStateMachineView::OnClickedButtonPostEvent()
{
	// TODO: Add your control notification handler code here
}
