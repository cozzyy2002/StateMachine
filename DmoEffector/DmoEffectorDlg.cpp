
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
	DDX_Control(pDX, IDC_COMBO_ASIO_DRIVER, m_asioDriverSel);
}

BEGIN_MESSAGE_MAP(CDmoEffectorDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(ID_BUTTON_START, &CDmoEffectorDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(ID_BUTTON_STOP, &CDmoEffectorDlg::OnBnClickedButtonStop)
END_MESSAGE_MAP()

template<class T>
static void setupComboBox(const T& list, LPCTSTR emptyItemName, CComboBox& combo)
{
	if (!list.empty()) {
		for (T::const_iterator i = list.begin(); i != list.end(); i++) {
			T::value_type::pointer item = i->get();
			int index = combo.AddString(item->getName());
			combo.SetItemDataPtr(index, item);
		}
	} else {
		combo.AddString(emptyItemName);
		combo.SetItemDataPtr(0, NULL);
	}
	combo.SetCurSel(0);
}

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

	// Get Asio driver list and show them in the ASIO driver list of UI.
	static LPCTSTR noDriverMessage = _T("<No ASIO driver found>");
	HR_EXPECT_OK(CAsioDriver::createDriverList(m_asioDriverList));
	setupComboBox(m_asioDriverList, noDriverMessage, m_asioDriverSel);

	// Get input/output devices and show them in the device list of UI.
	static LPCTSTR noDeiceMessage = _T("<No device found>");
	HR_EXPECT_OK(CDevice::createDeviceList(CLSID_AudioInputDeviceCategory, m_inputDeviceList));
	setupComboBox(m_inputDeviceList, noDeiceMessage, m_inputDeviceSel);

	HR_EXPECT_OK(CDevice::createDeviceList(CLSID_AudioRendererCategory, m_outputDeviceList));
	setupComboBox(m_outputDeviceList, noDeiceMessage, m_outputDeviceSel);

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

	CAsioDriver* pAsioDriver = (CAsioDriver*)m_asioDriverSel.GetItemDataPtr(m_asioDriverSel.GetCurSel());
	m_mainController.setup(pAsioDriver, m_hWnd);

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
