// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include "stdafx.h"
#include "HelloServer.h"
#include "HelloServerDlg.h"
#include "HelloI.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CHelloServerApp, CWinApp)
    ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

using namespace std;

CHelloServerApp::CHelloServerApp()
{
    // Place all significant initialization in InitInstance
}

// The one and only CHelloServerApp object

CHelloServerApp theApp;

BOOL CHelloServerApp::InitInstance()
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
        Ice::InitializationData initData;
        initData.properties = Ice::createProperties();
        initData.properties->load("config");
        log = new LogI;
        initData.logger = log;
        communicator = Ice::initialize(argc, 0, initData);
        adapter = communicator->createObjectAdapter("Hello");
    }
    catch(const IceUtil::Exception& ex)
    {
        ostringstream ostr;
        ostr << ex;
        string s = ostr.str();
        AfxMessageBox(CString(s.c_str()), MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    //
    // Create the dialog.
    //
    CHelloServerDlg dlg(communicator, log);

    //
    // Instantiate the servant.
    //
    Ice::ObjectPtr servant = new HelloI(log, &dlg);
    adapter->add(servant, communicator->stringToIdentity("hello"));
    adapter->activate();
    log->message("Ready to receive requests.");

    //
    // Show dialog and wait until it is closed, or until the servant receives
    // a shutdown request.
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
    catch(const IceUtil::Exception&)
    {
    }

    // Since the dialog has been closed, return FALSE so that we exit the
    // application, rather than start the application's message pump.
    return FALSE;
}
