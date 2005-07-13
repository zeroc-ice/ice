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
#include "ChatClientDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CChatClientDlg::CChatClientDlg(const Ice::CommunicatorPtr& communicator, const Demo::ChatSessionPrx& chat,
                               const LogIPtr& log, CWnd* pParent /*=NULL*/) :
    CDialog(CChatClientDlg::IDD, pParent), _communicator(communicator), _chat(chat), _log(log)
{
    _hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void
CChatClientDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CChatClientDlg, CDialog)
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    //}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_SHUTDOWN, OnShutdown)
    ON_BN_CLICKED(IDC_SEND, OnSend)
END_MESSAGE_MAP()

BOOL
CChatClientDlg::OnInitDialog()
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
    CEdit* disp = (CEdit*)GetDlgItem(IDC_LOG2);
    _log->setControl(disp);

    //
    // Set the focus to the text input
    //
    ((CButton*)GetDlgItem(IDC_LOG))->SetFocus();
 
    return FALSE; // return FALSE because we explicitly set the focus
}

void
CChatClientDlg::OnCancel()
{
    _log->setControl(0);
    CDialog::OnCancel();
}

// If you add a minimize button to your dialog, you will need the code below
// to draw the icon.  For MFC applications using the document/view model,
// this is automatically done for you by the framework.

void
CChatClientDlg::OnPaint() 
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
CChatClientDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(_hIcon);
}

void
CChatClientDlg::OnSend()
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

void
CChatClientDlg::OnShutdown()
{
    EndDialog(0);
}
