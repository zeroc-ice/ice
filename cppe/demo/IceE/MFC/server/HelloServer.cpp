// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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
    IceE::CommunicatorPtr communicator;
    IceE::ObjectAdapterPtr adapter;
    LogIPtr log;
    try
    {
        int argc = 0;
        IceE::PropertiesPtr properties = IceE::createProperties();
	properties->setProperty("Hello.Endpoints", "tcp -p 10000");
	//properties->load("config");
        communicator = IceE::initializeWithProperties(argc, 0, properties);
        log = new LogI;
        communicator->setLogger(log);
        adapter = communicator->createObjectAdapter("Hello");
    }
    catch(const IceE::Exception& ex)
    {
        AfxMessageBox(CString(ex.toString().c_str()), MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    //
    // Create the dialog.
    //
    CHelloServerDlg dlg(communicator, log);
        
    //
    // Instantiate the servant.
    //
    IceE::ObjectPtr servant = new HelloI(log, &dlg);
    adapter->add(servant, IceE::stringToIdentity("hello"));
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
    log->setControl(0);

    //
    // Clean up.
    //
    try
    {
        communicator->destroy();
    }
    catch(const IceE::Exception&)
    {
    }

    // Since the dialog has been closed, return FALSE so that we exit the
    // application, rather than start the application's message pump.
    return FALSE;
}
