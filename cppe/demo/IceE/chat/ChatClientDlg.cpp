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
#include "ChatConfigDlg.h"
#include "Router.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CChatClientDlg::CChatClientDlg(const Ice::CommunicatorPtr& communicator, const LogIPtr& log,
			       CWnd* pParent /*=NULL*/) :
    CDialog(CChatClientDlg::IDD, pParent),
    _communicator(communicator), 
    _chat(0), 
    _log(log),
    _user(""),
    _password(""),
    _host(""),
    _port("10005")
{
    _hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void
CChatClientDlg::setSession(const Demo::ChatSessionPrx& chat, CString user, CString password, CString host,
			   CString port)
{
    _chat = chat;
    _user = user;
    _password = password;
    _host = host;
    _port = port;
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
    ON_BN_CLICKED(IDC_CONFIG, OnLogin)
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
    _edit->EnableWindow(FALSE);

    ((CButton*)GetDlgItem(IDC_SEND))->EnableWindow(FALSE);

    //
    // Retrieve the chat display edit control.
    //
    CEdit* disp = (CEdit*)GetDlgItem(IDC_LOG2);
    disp->EnableWindow(FALSE);
    _log->setControl(disp);

    //
    // Set the focus to the text input
    //
    ((CButton*)GetDlgItem(IDC_LOGIN))->SetFocus();
 
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
    if(_chat == 0)
    {
        AfxMessageBox(CString("You must login first."), MB_OK|MB_ICONEXCLAMATION);
	return;
    }

    CString text;
    _edit->GetWindowText(text);

    try
    {
        _chat->say(std::string(text));
    }
    catch(const Ice::ConnectionLostException&)
    {
        AfxMessageBox(CString("Login timed out due to inactivity"), MB_OK|MB_ICONEXCLAMATION);
        EndDialog(0);
    }

    //
    // Clear text input and reset focus.
    //
    _edit->SetWindowText(CString(""));
    ((CButton*)GetDlgItem(IDC_LOG))->SetFocus();
}

void
CChatClientDlg::OnLogin()
{
    if(_chat == 0)
    {
        CChatConfigDlg dlg(_communicator, _log, this, _user, _password, _host, _port);
        dlg.DoModal();
    }
    else
    {
    	try
	{
	    Glacier2::RouterPrx::uncheckedCast(_communicator->getDefaultRouter())->destroySession();
	}
	catch(const Ice::Exception& ex)
	{
	    AfxMessageBox(CString(ex.toString().c_str()), MB_OK|MB_ICONEXCLAMATION);
	}
	_chat = 0;
    }

    if(_chat == 0)
    {
        _edit->EnableWindow(FALSE);
        ((CButton*)GetDlgItem(IDC_SEND))->EnableWindow(FALSE);
	(CEdit*)GetDlgItem(IDC_LOG2)->EnableWindow(FALSE);
        ((CButton*)GetDlgItem(IDC_CONFIG))->SetWindowText("Login");
    }
    else
    {
        _edit->EnableWindow(TRUE);
        ((CButton*)GetDlgItem(IDC_SEND))->EnableWindow(TRUE);
	(CEdit*)GetDlgItem(IDC_LOG2)->EnableWindow(TRUE);
        ((CButton*)GetDlgItem(IDC_CONFIG))->SetWindowText("Logout");
        ((CEdit*)GetDlgItem(IDC_LOG))->SetFocus();
    }
}
