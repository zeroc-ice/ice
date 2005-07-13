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
#include "Router.h"
#include "Chat.h"
#include "LogI.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CChatClientApp, CWinApp)
    ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

class ChatCallbackI : public Demo::ChatCallback
{
public:

    ChatCallbackI(LogIPtr log)
        : _log(log)
    {
    }

    virtual void
    message(const std::string& data, const Ice::Current&)
    {
	_log->message(data);
    }

private:

    LogIPtr _log;
};


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
    Demo::ChatSessionPrx session;
    try
    {
        int argc = 0;
        Ice::PropertiesPtr properties = Ice::createProperties();
	//properties->setProperty("Hello.Endpoints", "tcp -p 10000");
	properties->load("config");

        communicator = Ice::initializeWithProperties(argc, 0, properties);
        log = new LogI;
        communicator->setLogger(log);

	Ice::RouterPrx defaultRouter = communicator->getDefaultRouter();
	if(!defaultRouter)
	{
	    AfxMessageBox(CString("No default router set"), MB_OK|MB_ICONEXCLAMATION);
	    return FALSE;
	}

	Glacier2::RouterPrx router = Glacier2::RouterPrx::checkedCast(defaultRouter);
	if(!router)
	{
	    AfxMessageBox(CString("Configured router is not a Glacier2 router"), MB_OK|MB_ICONEXCLAMATION);
	    return FALSE;
	}

        try
        {
            session = Demo::ChatSessionPrx::uncheckedCast(router->createSession("username", "password"));
        }
        catch(const Glacier2::PermissionDeniedException& ex)
        {
            AfxMessageBox(CString(ex.toString().c_str()), MB_OK|MB_ICONEXCLAMATION);
            return FALSE;
        }

        std::string category = router->getServerProxy()->ice_getIdentity().category;
        Ice::Identity callbackReceiverIdent;
        callbackReceiverIdent.name = "callbackReceiver";
        callbackReceiverIdent.category = category;

        Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("Chat.Client");
        Demo::ChatCallbackPrx callback = Demo::ChatCallbackPrx::uncheckedCast(
            adapter->add(new ChatCallbackI(log), callbackReceiverIdent));
        adapter->activate();

        session->setCallback(callback); 
    }
    catch(const Ice::Exception& ex)
    {
        AfxMessageBox(CString(ex.toString().c_str()), MB_OK|MB_ICONEXCLAMATION);
        return FALSE;
    }

    //
    // Create the dialog.
    //
    CChatClientDlg dlg(communicator, session, log);
        
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
    catch(const Ice::Exception&)
    {
    }

    // Since the dialog has been closed, return FALSE so that we exit the
    // application, rather than start the application's message pump.
    return FALSE;
}
