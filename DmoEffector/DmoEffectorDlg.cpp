
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
}

BEGIN_MESSAGE_MAP(CDmoEffectorDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(ID_BUTTON_START, &CDmoEffectorDlg::OnBnClickedButtonStart)
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

	// Get input devices and show them in the input devices list of UI.
	HRESULT hr = CDevice::createDeviceList(CLSID_AudioInputDeviceCategory, m_inputDeviceList);
	for (CDevice::device_list_t::const_iterator i = m_inputDeviceList.begin(); i != m_inputDeviceList.end(); i++) {
		CDevice* dev = i->get();
		m_inputDeviceSel.AddString(dev->getName());
	}
	if (!m_inputDeviceList.empty()) m_inputDeviceSel.SetCurSel(0);

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

#include <initguid.h>
#include <../../ASIOSDK2.3/common/iasiodrv.h>

DEFINE_GUID(clsidAsio, 0x232685C6, 0x6548, 0x49D8, 0x84, 0x6D, 0x41, 0x41, 0xA3, 0xEF, 0x75, 0x60);
//DEFINE_GUID(clsidAsio, 0xa91eaba1, 0xcf4c, 0x11d3, 0xb9, 0x6a, 0x00, 0xa0, 0xc9, 0xc7, 0xb6, 0x1a);

HRESULT loadAsio(IASIO** ppAsio)
{
	HMODULE hModule = LoadLibraryA("C:\\Program Files (x86)\\ASIO4ALL v2\\asio4all.dll");
	WIN32_ASSERT(hModule);
	LPFNGETCLASSOBJECT DllGetClassObject;
	WIN32_ASSERT(DllGetClassObject = (LPFNGETCLASSOBJECT)GetProcAddress(hModule, "DllGetClassObject"));

	CComPtr<IClassFactory> factory;
	HR_ASSERT_OK(DllGetClassObject(clsidAsio, IID_PPV_ARGS(&factory)));
	CComPtr<IASIO> asio;
	HR_ASSERT_OK(factory->CreateInstance(NULL, clsidAsio, (void**)&asio));
	char driverName[100] = "";
	asio->getDriverName(driverName);
	LOG4CPLUS_INFO(logger, "Loaded '" << driverName << "' version " << asio->getDriverVersion());
	*ppAsio = asio.Detach();

	return S_OK;
}

void CDmoEffectorDlg::OnBnClickedButtonStart()
{
	CComPtr<IASIO> asio;
	if (SUCCEEDED(loadAsio(&asio))) {
		LOG4CPLUS_INFO(logger, "IASIO::init(): " << (asio->init(m_hWnd) ? "OK" : "NG"));
	}
	return;

	UpdateData(TRUE);

	int sel = m_inputDeviceSel.GetCurSel();
	if ((0 <= sel) && (sel < (int)m_inputDeviceList.size())) {
		m_mainController.start(m_inputDeviceList[sel].get());
	} else {
		LOG4CPLUS_ERROR(logger, "Invalid device selection: " << sel);
	}
}
