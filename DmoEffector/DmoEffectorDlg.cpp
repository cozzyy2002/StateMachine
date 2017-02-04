
// DmoEffectorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DmoEffector.h"
#include "DmoEffectorDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static log4cplus::Logger logger = log4cplus::Logger::getInstance(_T("DmoEffectorDlg"));

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CDmoEffectorDlg dialog



CDmoEffectorDlg::CDmoEffectorDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DMOEFFECTOR_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDmoEffectorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_INPUT_DEVIES, m_inputDeviceSel);
	DDX_Control(pDX, IDC_COMBO_OUTPUT_DEVIES, m_outputDeviceSel);
}

BEGIN_MESSAGE_MAP(CDmoEffectorDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(ID_BUTTON_START, &CDmoEffectorDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(ID_BUTTON_STOP, &CDmoEffectorDlg::OnBnClickedButtonStop)
END_MESSAGE_MAP()


// CDmoEffectorDlg message handlers

BOOL CDmoEffectorDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	// Get input/output devices and show them in the device list of UI.
	static LPCTSTR noDeiceMessage = _T("<No device found>");
	int index;
	HR_EXPECT_OK(CDevice::createDeviceList(CLSID_AudioInputDeviceCategory, m_inputDeviceList));
	for (CDevice::device_list_t::const_iterator i = m_inputDeviceList.begin(); i != m_inputDeviceList.end(); i++) {
		CDevice* dev = i->get();
		index = m_inputDeviceSel.AddString(dev->getName());
		m_inputDeviceSel.SetItemDataPtr(index, dev);
	}
	if (0 == m_inputDeviceSel.GetCount()) {
		m_inputDeviceSel.AddString(noDeiceMessage);
		m_inputDeviceSel.SetItemDataPtr(0, NULL);
	}
	m_inputDeviceSel.SetCurSel(0);

	HR_EXPECT_OK(CDevice::createDeviceList(CLSID_AudioRendererCategory, m_outputDeviceList));
	for (CDevice::device_list_t::const_iterator i = m_outputDeviceList.begin(); i != m_outputDeviceList.end(); i++) {
		CDevice* dev = i->get();
		index = m_outputDeviceSel.AddString(dev->getName());
		m_outputDeviceSel.SetItemDataPtr(index, dev);
	}
	if (0 == m_outputDeviceSel.GetCount()) {
		m_outputDeviceSel.AddString(noDeiceMessage);
		m_outputDeviceSel.SetItemDataPtr(0, NULL);
	}
	m_outputDeviceSel.SetCurSel(0);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDmoEffectorDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CDmoEffectorDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CDmoEffectorDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CDmoEffectorDlg::OnBnClickedButtonStart()
{
	UpdateData(TRUE);

	m_mainController.setup(m_hWnd);

	CDevice* inputDevice = (CDevice*)m_inputDeviceSel.GetItemDataPtr(m_inputDeviceSel.GetCurSel());
	CDevice* outputDevice = (CDevice*)m_outputDeviceSel.GetItemDataPtr(m_outputDeviceSel.GetCurSel());

	if (inputDevice && outputDevice) {
		m_mainController.start(inputDevice, outputDevice);
	}
}


void CDmoEffectorDlg::OnBnClickedButtonStop()
{
	m_mainController.stop();

	m_mainController.shutdown();
}
