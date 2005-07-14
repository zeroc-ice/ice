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
#include "Chat.h"
#include "LogI.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CChatClientApp, CWinApp)
    ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

CChatClientApp::CChatClientApp()
{
    // Place all significant initialization in InitInstance
}

// The one and only CChatClientApp object

CChatClientApp theApp;

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
    // Create a communicator and object adapter.
    //
    Ice::CommunicatorPtr communicator;
    Ice::ObjectAdapterPtr adapter;
    LogIPtr log;
    try
    {
        int argc = 0;
        Ice::PropertiesPtr properties = Ice::createProperties();
	properties->setProperty("IceE.RetryIntervals", "-1");

        communicator = Ice::initializeWithProperties(argc, 0, properties);
        log = new LogI;
        communicator->setLogger(log);
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
    log->setControl(0);

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
