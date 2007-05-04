// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************


#include "stdafx.h"
#include "HelloClient.h"
#include "HelloClientDlg.h"

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#endif

using namespace std;

BEGIN_MESSAGE_MAP(CHelloClientApp, CWinApp)
END_MESSAGE_MAP()

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

    //
    // Create a communicator.
    //
    Ice::CommunicatorPtr communicator;
    try
    {
        int argc = 0;
	Ice::InitializationData initData;
	initData.properties = Ice::createProperties();
	//
	// Set a default value for Hello.Proxy so that the demo will
	// run without a configuration file.
	//
	initData.properties->setProperty("Hello.Proxy", "hello:tcp -p 10000");

	//
	// Now, load the configuration file if present. Under WinCE we
	// use "config.txt" since it can be edited with pocket word.
	//
#ifdef _WIN32_WCE
	string config = "config.txt";
#else
	string config = "config";
#endif
	try
	{
	    initData.properties->load(config);
	}
	catch(const Ice::FileException&)
	{
	}

        communicator = Ice::initialize(argc, 0, initData);
    }
    catch(const Ice::Exception& ex)
    {
        AfxMessageBox(CString(ex.toString().c_str()), MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    CHelloClientDlg dlg(communicator);
    m_pMainWnd = &dlg;
    dlg.DoModal();

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
