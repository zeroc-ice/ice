// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include "stdafx.h"
#include "HelloClient.h"
#include "HelloClientDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_AMI_CALLBACK             (WM_USER + 1)

BEGIN_MESSAGE_MAP(CHelloClientApp, CWinApp)
    ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

using namespace std;

CHelloClientApp::CHelloClientApp()
{
    // Place all significant initialization in InitInstance
}

// The one and only CHelloClientApp object

CHelloClientApp theApp;

BOOL
CHelloClientApp::InitInstance()
{
    // InitCommonControls() is required on Windows XP if an application
    // manifest specifies use of ComCtl32.dll version 6 or later to enable
    // visual styles.  Otherwise, any window creation will fail.
    InitCommonControls();

    CWinApp::InitInstance();

    try
    {
        CHelloClientDlg dlg;
        m_pMainWnd = &dlg;
        dlg.DoModal();
    }
    catch(const IceUtil::Exception& ex)
    {
        ostringstream ostr;
        ostr << ex;
        string s = ostr.str();
        AfxMessageBox(CString(s.c_str()), MB_OK|MB_ICONEXCLAMATION);
    }

    // Since the dialog has been closed, return FALSE so that we exit the
    // application, rather than start the application's message pump.
    return FALSE;
}

