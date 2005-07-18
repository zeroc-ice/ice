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

    _ping = new SessionPingThread(_chat);
    _ping->start();
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


    //
    // Retrieve the chat display edit control for
    // log output.
    //
    CEdit* disp = (CEdit*)GetDlgItem(IDC_LOG2);
    _log->setControl(disp);

    //
    // Disable the input, output and send as we are
    // not logged in yet.
    //
    _edit->EnableWindow(FALSE);
    disp->EnableWindow(FALSE);
    ((CButton*)GetDlgItem(IDC_SEND))->EnableWindow(FALSE);

    //
    // Set the focus to the login button
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
    //
    // Get text from the input edit box and forward it
    // on to the chat server.
    //
    CString text;
    _edit->GetWindowText(text);

    try
    {
#ifdef _WIN32_WCE
	char buffer[256];
	wcstombs(buffer, text, 256);
	_chat->say(buffer);
#else
        _chat->say(std::string(text));
#endif
    }
    catch(const Ice::Exception& e)
    {
        AfxMessageBox(CString(e.toString().c_str()), MB_OK|MB_ICONEXCLAMATION);

	_ping->destroy();
	_ping->getThreadControl().join();

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
        //
	// Login: Create and display login dialog.
	//
        CChatConfigDlg dlg(_communicator, _log, this, _user, _password, _host, _port);
        dlg.DoModal();
    }
    else
    {
        //
	// Logout: Destroy session and stop ping thread.
	//
	_chat = 0;
	_ping->destroy();
	_ping->getThreadControl().join();

    	try
	{
	    Glacier2::RouterPrx::uncheckedCast(_communicator->getDefaultRouter())->destroySession();
	}
	catch(const Ice::Exception& ex)
	{
	    AfxMessageBox(CString(ex.toString().c_str()), MB_OK|MB_ICONEXCLAMATION);
	}
    }

    //
    // Reset window state appropriate to logged in state.
    //
    if(_chat == 0)
    {
        _edit->EnableWindow(FALSE);
        ((CButton*)GetDlgItem(IDC_SEND))->EnableWindow(FALSE);
	(CEdit*)GetDlgItem(IDC_LOG2)->EnableWindow(FALSE);
#ifdef _WIN32_WCE
        ((CButton*)GetDlgItem(IDC_CONFIG))->SetWindowText(L"Login");
#else
        ((CButton*)GetDlgItem(IDC_CONFIG))->SetWindowText("Login");
#endif
    }
    else
    {
        _edit->EnableWindow(TRUE);
        ((CButton*)GetDlgItem(IDC_SEND))->EnableWindow(TRUE);
	(CEdit*)GetDlgItem(IDC_LOG2)->EnableWindow(TRUE);
#ifdef _WIN32_WCE
        ((CButton*)GetDlgItem(IDC_CONFIG))->SetWindowText(L"Logout");
#else
        ((CButton*)GetDlgItem(IDC_CONFIG))->SetWindowText("Logout");
#endif
        ((CEdit*)GetDlgItem(IDC_LOG))->SetFocus();
    }
}
