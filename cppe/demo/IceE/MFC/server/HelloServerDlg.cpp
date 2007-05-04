// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************


#include "stdafx.h"
#include "HelloServer.h"
#include "HelloServerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CHelloServerDlg::CHelloServerDlg(const Ice::CommunicatorPtr& communicator, const LogIPtr& log,
                                 CWnd* pParent /*=NULL*/) :
    CDialog(CHelloServerDlg::IDD, pParent), _communicator(communicator), _log(log)
{
    _hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void
CHelloServerDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CHelloServerDlg, CDialog)
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    //}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_SHUTDOWN, OnShutdown)
    ON_BN_CLICKED(IDC_CLEAR, OnClear)
    ON_MESSAGE(WM_USER, OnLog)
END_MESSAGE_MAP()

BOOL
CHelloServerDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    // Set the icon for this dialog.  The framework does this automatically
    // when the application's main window is not a dialog
    SetIcon(_hIcon, TRUE);            // Set big icon
    SetIcon(_hIcon, FALSE);        // Set small icon

    //
    // Retrieve the edit control.
    //
    _edit = (CEdit*)GetDlgItem(IDC_LOG);
    _log->setHandle(m_hWnd);

    //
    // Set the focus to the shutdown button, so that the text in the log
    // is not initially highlighted.
    //
    ((CButton*)GetDlgItem(IDC_SHUTDOWN))->SetFocus();
 
    return FALSE; // return FALSE because we explicitly set the focus
}

void
CHelloServerDlg::OnCancel()
{
    _log->setHandle(0);
    CDialog::OnCancel();
}

// If you add a minimize button to your dialog, you will need the code below
// to draw the icon.  For MFC applications using the document/view model,
// this is automatically done for you by the framework.

void
CHelloServerDlg::OnPaint() 
{
#ifdef _WIN32_WCE
    CDialog::OnPaint();
#else
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
        dc.DrawIcon(x, y, _hIcon);
    }
    else
    {
        CDialog::OnPaint();
    }
#endif
}

// The system calls this function to obtain the cursor to display while the user drags
// the minimized window.
HCURSOR
CHelloServerDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(_hIcon);
}

void
CHelloServerDlg::OnShutdown()
{
    EndDialog(0);
}

void
CHelloServerDlg::OnClear()
{
    _edit->SetWindowText(CString(""));
}

LRESULT
CHelloServerDlg::OnLog(WPARAM wParam, LPARAM lParam)
{
    char* text = (char*)lParam;

    _edit->SetSel(-1, -1);
    _edit->ReplaceSel(CString(text));

    delete[] text;

    return 0;
}
