// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <stdafx.h>
#include <ChatClient.h>
#include <ChatClientDlg.h>
#include <Chat.h>
#include <LogI.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif



BEGIN_MESSAGE_MAP(CChatClientApp, CWinApp)
END_MESSAGE_MAP()

CChatClientApp::CChatClientApp()
{
    // Place all significant initialization in InitInstance
}

// The one and only CChatClientApp object

CChatClientApp theApp;

#ifdef ICEE_HAS_ROUTER

BOOL CChatClientApp::InitInstance()
{
    //
    // InitCommonControls() is required on Windows XP if an application
    // manifest specifies use of ComCtl32.dll version 6 or later to enable
    // visual styles.  Otherwise, any window creation will fail.
    //
    InitCommonControls();

    CWinApp::InitInstance();

    //
    // Create a communicator.
    //
    Ice::CommunicatorPtr communicator;
    LogIPtr log;
    try
    {
	Ice::InitializationData initData;
        log = new LogI;
	initData.logger = log;

        int argc = 0;
        communicator = Ice::initialize(argc, 0, initData);
    }
    catch(const Ice::Exception& ex)
    {
        AfxMessageBox(CString(ex.toString().c_str()), MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    //
    // Create the main dialog.
    //
    CChatClientDlg dlg(communicator, log);

    //
    // Show dialog and wait until it is closed.
    //
    m_pMainWnd = &dlg;
    dlg.DoModal();

    //
    // Edit control no longer exists.
    //
    log->setHandle(0);

    //
    // Clean up.
    //
    try
    {
        communicator->destroy();
    }
    catch(const Ice::Exception&)
    {
    }

    // Since the dialog has been closed, return FALSE so that we exit the
    // application, rather than start the application's message pump.
    return FALSE;
}

#else

BOOL CChatClientApp::InitInstance()
{
    InitCommonControls();
    CWinApp::InitInstance();
    AfxMessageBox(CString("This demo requires Ice-E built with router support."), MB_OK|MB_ICONEXCLAMATION);
    return FALSE;
}

#endif
