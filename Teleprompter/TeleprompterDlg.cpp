
// TeleprompterDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Teleprompter.h"
#include "TeleprompterDlg.h"
#include "afxdialogex.h"
#include "SM3Tool.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#define MEMBASESIZE	10240
#define MEMRNDSIZE	10
#else
#define MEMBASESIZE	1073741824
#define MEMRNDSIZE	1048576
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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


// CTeleprompterDlg 对话框



CTeleprompterDlg::CTeleprompterDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_TELEPROMPTER_DIALOG, pParent), m_pThread(NULL), m_needEXIT(false)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTeleprompterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON1, m_startbtn);
	DDX_Control(pDX, IDC_MEMUSEGB, m_editmem);
	DDX_Control(pDX, IDC_CHECK1, m_ckRandom);
	DDX_Control(pDX, IDC_CHECK2, m_ckRead);
}

BEGIN_MESSAGE_MAP(CTeleprompterDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CTeleprompterDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// CTeleprompterDlg 消息处理程序

BOOL CTeleprompterDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
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

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CTeleprompterDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CTeleprompterDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CTeleprompterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CTeleprompterDlg::MyThreadProc()
{
	int memgb = getInputMemGB();
	std::vector<unsigned char *> vecMemBuffer;
	for (int r = 0; r < memgb; r++)
	{
		unsigned char * buffer = new unsigned char[MEMBASESIZE];
		vecMemBuffer.push_back(buffer);
	}
	while (1)
	{
		Sleep(10);
		if (hasCheckedEnableRandom())
		{
			for (int r = 0; r < vecMemBuffer.size(); r++)
			{
				unsigned char * buffer = vecMemBuffer[r];
				for (int i = 0; i < 1024; i++)
				{
					memset(buffer+i*MEMRNDSIZE,rand()%255, MEMRNDSIZE);
				}
			}
		}
		if (hasCheckedEnableRead())
		{
			SM3Tool sm3;
			unsigned char output[32];
			for (int r = 0; r < vecMemBuffer.size(); r++)
			{
				unsigned char * buffer = vecMemBuffer[r];
				for (int i = 0; i < 1024; i++)
				{
					sm3.sm3(buffer + i*MEMRNDSIZE, MEMRNDSIZE, output);
				}
			}
		}
		if (isNeedEXIT())
		{
			resetEXITflag();
			break;
		} 
	}
	for (int r = 0; r <vecMemBuffer.size(); r++)
	{
		delete[] vecMemBuffer[r];
	}
	resetBtn();
	return ;
}

void CTeleprompterDlg::resetBtn()
{
	m_startbtn.EnableWindow(TRUE);
	m_editmem.SetReadOnly(FALSE);
	delete(m_pThread);
	m_pThread = NULL;
	m_startbtn.SetWindowText("开始");
}

void CTeleprompterDlg::OnBnClickedButton1()
{
	if (m_pThread == NULL)
	{
		m_startbtn.SetWindowText("停止");
		m_editmem.SetReadOnly(TRUE);
		m_pThread= new boost::thread(boost::bind(&CTeleprompterDlg::MyThreadProc, this));
	}
	else
	{
		m_startbtn.EnableWindow(FALSE);
		setEXITflag();
		//while(1)
		{
			//if (isNeedEXIT() == false)

		}
	}
	
}


