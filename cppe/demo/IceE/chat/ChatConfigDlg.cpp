// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <stdafx.h>
#include <ChatConfigDlg.h>

#ifdef ICEE_HAS_ROUTER

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace std;

CChatConfigDlg::CChatConfigDlg(const CString& user, const CString& password,
			       const CString& host, const CString& port, CWnd* pParent /*=NULL*/) :
    CDialog(CChatConfigDlg::IDD, pParent),
    _user(user),
    _password(password),
    _host(host),
    _port(port)
{
    _hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CString
CChatConfigDlg::getUser() const
{
    return _user;
}

CString
CChatConfigDlg::getPassword() const
{
    return _password;
}

CString
CChatConfigDlg::getHost() const
{
    return _host;
}

CString
CChatConfigDlg::getPort() const
{
    return _port;
}

void
CChatConfigDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CChatConfigDlg, CDialog)
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
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
    _useredit = (CEdit*)GetDlgItem(IDC_USER);
    _passedit = (CEdit*)GetDlgItem(IDC_PASSWORD);
    _hostedit = (CEdit*)GetDlgItem(IDC_HOST);
    _portedit = (CEdit*)GetDlgItem(IDC_PORT);

    //
    // Fill the windows with last info entered.
    //
    _useredit->SetWindowText(_user);
    _passedit->SetWindowText(_password);
    _hostedit->SetWindowText(_host);
    _portedit->SetWindowText(_port);

    //
    // Set the focus to the username input
    //
    _useredit->SetFocus();
 
    return FALSE; // return FALSE because we explicitly set the focus
}

void
CChatConfigDlg::OnCancel()
{
    CDialog::OnCancel();
}

//
// If you add a minimize button to your dialog, you will need the code below
// to draw the icon.  For MFC applications using the document/view model,
// this is automatically done for you by the framework.
//
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

//
// The system calls this function to obtain the cursor to display while the user drags
// the minimized window.
//
HCURSOR
CChatConfigDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(_hIcon);
}


void
CChatConfigDlg::OnLogin()
{
    //
    // Retrieve the entered info from the edit controls.
    //
    _useredit->GetWindowText(_user);
    _passedit->GetWindowText(_password);
    _hostedit->GetWindowText(_host);
    _portedit->GetWindowText(_port);
    EndDialog(IDOK);
}

#endif // ICEE_HAS_ROUTER
