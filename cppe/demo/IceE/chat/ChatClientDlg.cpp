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
#include <ChatConfigDlg.h>
#include <Router.h>
#include <IceE/IceE.h>

#ifdef ICEE_HAS_ROUTER

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace std;
using namespace Demo;

class ChatCallbackI : public ChatCallback
{
public:

    ChatCallbackI(const LogIPtr& log)
        : _log(log)
    {
    }

    virtual void
    message(const string& data, const Ice::Current&)
    {
	_log->message(data);
    }

private:

    const LogIPtr _log;

};

CChatClientDlg::CChatClientDlg(const Ice::CommunicatorPtr& communicator, const LogIPtr& log,
			       CWnd* pParent /*=NULL*/) :
    CDialog(CChatClientDlg::IDD, pParent),
    _communicator(communicator), 
    _chat(0), 
    _log(log),
    //_user(""), // For ease of testing these can be filled in.
    //_password(""),
    //_host(""),
    _port("10005")
{
    _hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CChatClientDlg::~CChatClientDlg()
{
    //
    // If the ping thread is still active, destroy it and wait for it
    // to terminate.
    //
    if(_ping)
    {
	_ping->destroy();
	_ping->getThreadControl().join();
	_ping = 0;
    }
}

void
CChatClientDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

#ifndef _WIN32_WCE
//
// Under Windows pressing enter in the _edit CEdit sends IDOK to the
// dialog. Under CE pressing enter on the keyboard causes the default
// button to be pressed.
//
void
CChatClientDlg::OnOK()
{
    OnSend();
}
#endif

BEGIN_MESSAGE_MAP(CChatClientDlg, CDialog)
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_CONFIG, OnLogin)
    ON_BN_CLICKED(IDC_SEND, OnSend)
    ON_MESSAGE(WM_USER, OnLog)
END_MESSAGE_MAP()

void
CChatClientDlg::setDialogState()
{
    CWnd* sendWnd = GetDlgItem(IDC_SEND);
    CWnd* configWnd = GetDlgItem(IDC_CONFIG);
    CWnd* loginWnd = GetDlgItem(IDC_LOGIN);

    if(_chat == 0)
    {
    	//
	// Logged out: Disable all except Login.
	//
        _edit->EnableWindow(FALSE);
        _display->EnableWindow(FALSE);
        sendWnd->EnableWindow(FALSE);
#ifdef _WIN32_WCE
        configWnd->SetWindowText(L"Login");
#else
        configWnd->SetWindowText("Login");
#endif

	//
	// Set the focus to the login button
	//
	loginWnd->SetFocus();

	//
	// Set the default button.
	//
	sendWnd->SendMessage(BM_SETSTYLE, (WPARAM)BS_PUSHBUTTON, (LPARAM)TRUE);
	SendMessage(DM_SETDEFID, (WPARAM)IDC_CONFIG, 0);
	configWnd->SendMessage(BM_SETSTYLE, (WPARAM)BS_DEFPUSHBUTTON, (LPARAM)TRUE);
    }
    else
    {
        //
	// Logged in: Enable all and change Login to Logout
	//
        _edit->EnableWindow(TRUE);
        _display->EnableWindow(TRUE);
        sendWnd->EnableWindow(TRUE);
#ifdef _WIN32_WCE
        configWnd->SetWindowText(L"Logout");
#else
        configWnd->SetWindowText("Logout");
#endif
        _edit->SetFocus();

	//
	// Set the default button.
	//
	configWnd->SendMessage(BM_SETSTYLE, (WPARAM)BS_PUSHBUTTON, (LPARAM)TRUE);
	SendMessage(DM_SETDEFID, (WPARAM)IDC_SEND, 0);
	sendWnd->SendMessage(BM_SETSTYLE, (WPARAM)BS_DEFPUSHBUTTON, (LPARAM)TRUE);
    }
}

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
    // Retrieve the chat display edit control for log output.
    //
    _display = (CEdit*)GetDlgItem(IDC_LOG2);

    //
    // Set the window handle on the logger.
    //
    _log->setHandle(m_hWnd);

    //
    // Disable the input, output and send as we are
    // not logged in yet.
    //
    setDialogState();
 
    return FALSE; // return FALSE because we explicitly set the focus
}

void
CChatClientDlg::OnCancel()
{
    if(_chat)
    {
	//
	// Clear the router.
	//
	assert(_router);
    	try
	{
	    _router->destroySession();
	}
        catch(const Ice::ConnectionLostException&)
        {
            //
            // Expected: the router closed the connection.
            //
        }
	catch(const Ice::Exception& ex)
	{
	    AfxMessageBox(CString(ex.toString().c_str()), MB_OK|MB_ICONEXCLAMATION);
	}
    }

    _log->setHandle(0);
    CDialog::OnCancel();
}

//
// If you add a minimize button to your dialog, you will need the code below
// to draw the icon.  For MFC applications using the document/view model,
// this is automatically done for you by the framework.
//

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

//
// The system calls this function to obtain the cursor to display while the user drags
// the minimized window.
//
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

    //
    // Trim the leading and trailing whitespace. If the text is empty,
    // then we're done.
    //
    text.TrimLeft();
    text.TrimRight();
    if(text.IsEmpty())
    {
	return;
    }

    try
    {
#ifdef _WIN32_WCE
	char buffer[256];
	wcstombs(buffer, text, 256);
	_chat->say(buffer);
#else
        _chat->say(string(text));
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
    _edit->SetFocus();
}

void
CChatClientDlg::OnLogin()
{
    if(_chat == 0)
    {
        //
	// Login: Create and display login dialog.
	//
        CChatConfigDlg dlg(_user, _password, _host, _port);
        if(dlg.DoModal() == IDOK)
	{
	    _user = dlg.getUser();
	    _password = dlg.getPassword();
	    _host = dlg.getHost();
	    _port = dlg.getPort();

	    string user;
	    string password;
	    string host;
	    string port;
#ifdef _WIN32_WCE
	    char buffer[64];
	    wcstombs(buffer, _user, 64);
	    user = buffer;

	    wcstombs(buffer, _password, 64);
	    password = buffer;

	    wcstombs(buffer, _host, 64);
	    host = buffer;

	    wcstombs(buffer, _port, 64);
	    port = buffer;
#else
	    user = _user;
	    password = _password;
	    host = _host;
	    port = _port;
#endif

	    try
	    {
		string routerStr = 
		    Ice::printfToString("DemoGlacier2/router:tcp -p %s -h %s", port.c_str(), host.c_str());
		_router = Glacier2::RouterPrx::checkedCast(_communicator->stringToProxy(routerStr));
		assert(_router);

		//
		// Now setup the new router.
		//
		_chat = ChatSessionPrx::uncheckedCast(_router->createSession(user, password)->ice_router(_router));


    		//
    		// Create the OA.
    		//
    		_adapter = _communicator->createObjectAdapterWithRouter("Chat.Client", _router);
    		_adapter->activate();

		//
		// Create the callback object. This must have the
		// category as defined by the Glacier2 session.
		//
		string category = _router->getServerProxy()->ice_getIdentity().category;
		Ice::Identity callbackReceiverIdent;
		callbackReceiverIdent.name = "callbackReceiver";
		callbackReceiverIdent.category = category;
		_callback = ChatCallbackPrx::uncheckedCast(
		    _adapter->add(new ChatCallbackI(_log), callbackReceiverIdent));

		_chat->setCallback(_callback);

		//
		// Create a ping thread to keep the session alive.
		//
		_ping = new SessionPingThread(_chat, (long)_router->getSessionTimeout() / 2);
		_ping->start();
	    }
	    catch(const Glacier2::CannotCreateSessionException& ex)
	    {
		AfxMessageBox(CString(ex.reason.c_str()), MB_OK|MB_ICONEXCLAMATION);
	    }
	    catch(const Ice::Exception& ex)
	    {
		AfxMessageBox(CString(ex.toString().c_str()), MB_OK|MB_ICONEXCLAMATION);
		_chat = 0;
	    }
	}
    }
    else
    {
        //
	// Logout: Destroy session and stop ping thread.
	//
	assert(_callback);
	_adapter->remove(_callback->ice_getIdentity());
	_callback = 0;

	assert(_chat);
	_chat = 0;

	//
	// Destroy the ping thread.
	//
	_ping->destroy();
	_ping->getThreadControl().join();
	_ping = 0;

	//
	// Clear the router.
	//
	assert(_router);
    	try
	{
	    _router->destroySession();
	}
        catch(const Ice::ConnectionLostException&)
        {
            //
            // Expected: the router closed the connection.
            //
        }
	catch(const Ice::Exception& ex)
	{
	    AfxMessageBox(CString(ex.toString().c_str()), MB_OK|MB_ICONEXCLAMATION);
	}

	_adapter->destroy();
	_router = 0;
    }

    //
    // Reset window state appropriate to logged in state.
    //
    setDialogState();
}

LRESULT
CChatClientDlg::OnLog(WPARAM wParam, LPARAM lParam)
{
    char* text = (char*)lParam;

    _display->SetSel(-1, -1);
    _display->ReplaceSel(CString(text));

    delete[] text;

    return 0;
}

#endif // ICEE_HAS_ROUTER
