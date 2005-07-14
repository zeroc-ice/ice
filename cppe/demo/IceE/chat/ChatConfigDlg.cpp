// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


#include "stdafx.h"
#include "ChatClient.h"
#include "ChatConfigDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CChatConfigDlg::CChatConfigDlg(const Ice::CommunicatorPtr& communicator, const Demo::ChatSessionPrx& chat,
                               const LogIPtr& log, CWnd* pParent /*=NULL*/) :
    CDialog(CChatConfigDlg::IDD, pParent), _communicator(communicator), _chat(chat), _log(log)
{
    _hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void
CChatConfigDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CChatConfigDlg, CDialog)
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    //}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_LOGIN, OnLogin)
END_MESSAGE_MAP()

BOOL
CChatConfigDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    // Set the icon for this dialog.  The framework does this automatically
    // when the application's main window is not a dialog
    SetIcon(_hIcon, TRUE);         // Set big icon
    SetIcon(_hIcon, FALSE);        // Set small icon

    //
    // Retrieve the text input edit control.
    //
    _edit = (CEdit*)GetDlgItem(IDC_LOG);

    //
    // Retrieve the chat display edit control.
    //
    CEdit* pass = (CEdit*)GetDlgItem(IDC_USER);

    //
    // Set the focus to the username input
    //
    ((CEdit*)GetDlgItem(IDC_USER))->SetFocus();
 
    return FALSE; // return FALSE because we explicitly set the focus
}

void
CChatConfigDlg::OnCancel()
{
    _log->setControl(0);
    CDialog::OnCancel();
}

// If you add a minimize button to your dialog, you will need the code below
// to draw the icon.  For MFC applications using the document/view model,
// this is automatically done for you by the framework.

void
CChatConfigDlg::OnPaint() 
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
CChatConfigDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(_hIcon);
}

/*
void
CChatConfigDlg::OnSend()
{
    CString strText;

    int len = _edit->LineLength();
    _edit->GetLine(0, strText.GetBuffer(len), len);
    try
    {
        _chat->say(std::string(strText));
    }
    catch(const Ice::ConnectionLostException&)
    {
        AfxMessageBox(CString("Login timed out due to inactivity"), MB_OK|MB_ICONEXCLAMATION);
        OnShutdown();
    }
    strText.ReleaseBuffer(len);

    //
    // Clear text input and reset focus.
    //
    _edit->SetWindowText(CString(""));
    ((CButton*)GetDlgItem(IDC_LOG))->SetFocus();
}
*/

void
CChatConfigDlg::OnLogin()
{
    EndDialog(0);
}
