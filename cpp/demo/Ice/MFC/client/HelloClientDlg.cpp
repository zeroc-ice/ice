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

#include <iomanip>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_AMI_CALLBACK             (WM_USER + 1)

using namespace std;
using namespace Demo;

namespace
{

class Dispatcher : public Ice::Dispatcher
{
public:

    Dispatcher(CHelloClientDlg* dialog) : _dialog(dialog)
    {
    }

    virtual void 
    dispatch(const Ice::DispatcherCallPtr& call, const Ice::ConnectionPtr&)
    {
        //
        // Dispatch Ice AMI callbacks with the event loop thread.
        //
        _dialog->PostMessage(WM_AMI_CALLBACK, 0, reinterpret_cast<LPARAM>(new Ice::DispatcherCallPtr(call)));
    }

private:

    CHelloClientDlg* _dialog;
};

class Callback : public IceUtil::Shared
{
public:

    Callback(CHelloClientDlg* dialog) : _dialog(dialog)
    {
    }

    void
    sent(bool)
    {
        _dialog->sent();
    }

    void
    response()
    {
        _dialog->response();
    }

    void
    exception(const Ice::Exception& ex)
    {
        _dialog->exception(ex);
    }

    void
    flushSent(bool)
    {
        _dialog->flushed();
    }

private:

    CHelloClientDlg* _dialog;
};
typedef IceUtil::Handle<Callback> CallbackPtr;

enum DeliveryMode
{
    TWOWAY,
    TWOWAY_SECURE,
    ONEWAY,
    ONEWAY_SECURE,
    ONEWAY_BATCH,
    ONEWAY_SECURE_BATCH,
    DATAGRAM,
    DATAGRAM_BATCH
};

}

BEGIN_MESSAGE_MAP(CHelloClientDlg, CDialog)
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_WM_HSCROLL()
    ON_BN_CLICKED(IDC_INVOKE, OnSayHello)
    ON_BN_CLICKED(IDC_FLUSH, OnFlush)
    ON_BN_CLICKED(IDC_SHUTDOWN, OnShutdown)
    ON_MESSAGE(WM_AMI_CALLBACK, OnAMICallback)
END_MESSAGE_MAP()


CHelloClientDlg::CHelloClientDlg(CWnd* pParent /*=NULL*/) : CDialog(CHelloClientDlg::IDD, pParent)
{
    _hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

    //
    // Create the communicator.
    //
    Ice::InitializationData initData;
    initData.properties = Ice::createProperties();
    initData.properties->load("config");
    initData.dispatcher = new Dispatcher(this);
    _communicator = Ice::initialize(initData);

    //
    // Create AMI callbacks.
    // 
    CallbackPtr cb = new Callback(this);
    _sayHelloCallback = newCallback_Hello_sayHello(cb, &Callback::response, &Callback::exception, &Callback::sent);
    _shutdownCallback = newCallback_Hello_shutdown(cb, &Callback::response, &Callback::exception);
    _flushCallback = Ice::newCallback_Communicator_flushBatchRequests(cb, &Callback::exception, &Callback::flushSent);
}

void
CHelloClientDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}


BOOL
CHelloClientDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    // Set the icon for this dialog.  The framework does this automatically
    // when the application's main window is not a dialog
    SetIcon(_hIcon, TRUE);         // Set big icon
    SetIcon(_hIcon, FALSE);        // Set small icon

    //
    // Retrieve the controls.
    //
    _host = (CEdit*)GetDlgItem(IDC_HOST);
    _mode = (CComboBox*)GetDlgItem(IDC_MODE);
    _timeout = (CSliderCtrl*)GetDlgItem(IDC_TIMEOUT_SLIDER);
    _timeoutStatus = (CStatic*)GetDlgItem(IDC_TIMEOUT_STATUS);
    _delay = (CSliderCtrl*)GetDlgItem(IDC_DELAY_SLIDER);
    _delayStatus = (CStatic*)GetDlgItem(IDC_DELAY_STATUS);
    _status = (CStatic*)GetDlgItem(IDC_STATUSBAR);
    _flush = (CButton*)GetDlgItem(IDC_FLUSH);

    //
    // Use twoway mode as the initial default.
    //
    _mode->SetCurSel(TWOWAY);

    //
    // Disable flush
    //
    _flush->EnableWindow(FALSE);

    //
    // Set hostname
    //
    _host->SetWindowText(CString("127.0.0.1"));

    //
    // Initialize timeout slider
    //
    _timeout->SetRangeMin(0);
    _timeout->SetRangeMax(50);
    _timeoutStatus->SetWindowText(CString("0.0s"));

    //
    // Initialize delay slider
    //
    _delay->SetRangeMin(0);
    _delay->SetRangeMax(50);
    _delayStatus->SetWindowText(CString("0.0s"));

    _status->SetWindowText(CString(" Ready"));

    return TRUE;  // return TRUE  unless you set the focus to a control
}

void
CHelloClientDlg::OnClose()
{
    //
    // Destroy the communicator. If AMI calls are still in progress they will be
    // interrupted with an Ice::CommunicatorDestroyedException.
    //
    try
    {
        _communicator->destroy();
    }
    catch(const IceUtil::Exception&)
    {
    }
}

// If you add a minimize button to your dialog, you will need the code below
// to draw the icon.  For MFC applications using the document/view model,
// this is automatically done for you by the framework.

void
CHelloClientDlg::OnPaint() 
{
    if(IsIconic())
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
}

// The system calls this function to obtain the cursor to display while the user drags
// the minimized window.
HCURSOR
CHelloClientDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(_hIcon);
}

void
CHelloClientDlg::OnHScroll(UINT, UINT, CScrollBar* scroll)
{
    CSliderCtrl* slider = (CSliderCtrl*)scroll;
    ostringstream s;
    if(slider == _timeout)
    {
        s << setiosflags(ios::fixed) << setprecision(1) << (long)_timeout->GetPos()/10.0 << "s";
        _timeoutStatus->SetWindowText(CString(s.str().c_str()));
    }
    else
    {
        s << setiosflags(ios::fixed) << setprecision(1) << (long)_delay->GetPos()/10.0 << "s";
        _delayStatus->SetWindowText(CString(s.str().c_str()));
    }
}

void
CHelloClientDlg::OnSayHello()
{
    Demo::HelloPrx hello = createProxy();
    if(!hello)
    {
        return;
    }
    int delay = _delay->GetPos() * 100;
    try
    {
        if(!deliveryModeIsBatch())
        {
            _status->SetWindowText(CString(" Sending request"));
            hello->begin_sayHello(delay, _sayHelloCallback);
        }
        else
        {
            _flush->EnableWindow(TRUE);
            hello->sayHello(delay);
            _status->SetWindowText(CString(" Queued sayHello request"));
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        handleException(ex);
    }
}

void
CHelloClientDlg::OnShutdown()
{
    Demo::HelloPrx hello = createProxy();
    try
    {
        if(!deliveryModeIsBatch())
        {
            _status->SetWindowText(CString(" Sending request"));
            CallbackPtr cb = new Callback(this);
            hello->begin_shutdown(_shutdownCallback);
        }
        else
        {
            _flush->EnableWindow(TRUE);
            hello->shutdown();
            _status->SetWindowText(CString(" Queued shutdown request"));
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        handleException(ex);
    }
}

void
CHelloClientDlg::OnFlush()
{
    try
    {
        _communicator->begin_flushBatchRequests(_flushCallback);
    }
    catch(const IceUtil::Exception& ex)
    {
        handleException(ex);
    }
    _flush->EnableWindow(FALSE);
}

LRESULT
CHelloClientDlg::OnAMICallback(WPARAM, LPARAM lParam)
{
    try
    {
        Ice::DispatcherCallPtr* call = reinterpret_cast<Ice::DispatcherCallPtr*>(lParam);
        (*call)->run();
        delete call;
    }
    catch(const Ice::Exception& ex)
    {
        handleException(ex);
    }
    return 0;
}

void
CHelloClientDlg::sent()
{
    int mode = _mode->GetCurSel();
    if(mode == TWOWAY || mode == TWOWAY_SECURE)
    {
        _status->SetWindowText(CString(" Waiting for response"));
    }
    else
    {
        _status->SetWindowText(CString(" Ready"));
    }
}

void
CHelloClientDlg::response()
{
    _status->SetWindowText(CString(" Ready"));
}

void
CHelloClientDlg::exception(const Ice::Exception& ex)
{
    if(!dynamic_cast<const Ice::CommunicatorDestroyedException*>(&ex))
    {
        handleException(ex);
    }
}

void
CHelloClientDlg::flushed()
{
    _status->SetWindowText(CString(" Flushed batch requests"));
}

Demo::HelloPrx
CHelloClientDlg::createProxy()
{
    CString h;
    _host->GetWindowText(h);
    string host = IceUtil::wstringToString(wstring(h));
    if(host.size() == 0)
    {
        _status->SetWindowText(CString(" No hostname"));
    }

    string s = "hello:tcp -h " + host + " -p 10000:ssl -h " + host + " -p 10001:udp -h " + host + " -p 10000";
    Ice::ObjectPrx prx = _communicator->stringToProxy(s);

    int mode = _mode->GetCurSel();
    switch(mode)
    {
        case TWOWAY:
            prx = prx->ice_twoway();
            break;
        case TWOWAY_SECURE:
            prx = prx->ice_twoway()->ice_secure(true);;
            break;
        case ONEWAY:
            prx = prx->ice_oneway();
            break;
        case ONEWAY_SECURE:
            prx = prx->ice_oneway()->ice_secure(true);
            break;
        case ONEWAY_BATCH:
            prx = prx->ice_batchOneway();
            break;
        case ONEWAY_SECURE_BATCH:
            prx = prx->ice_batchOneway()->ice_secure(true);
            break;
        case DATAGRAM:
            prx = prx->ice_datagram();
            break;
        case DATAGRAM_BATCH:
            prx = prx->ice_batchDatagram();
            break;
    }

    int timeout = _timeout->GetPos() * 100;
    if(timeout != 0)
    {
        prx = prx->ice_timeout(timeout);
    }

    return Demo::HelloPrx::uncheckedCast(prx);
}

BOOL 
CHelloClientDlg::deliveryModeIsBatch()
{
    return _mode->GetCurSel() == ONEWAY_BATCH ||
           _mode->GetCurSel() == ONEWAY_SECURE_BATCH ||
           _mode->GetCurSel() == DATAGRAM_BATCH;
}

void
CHelloClientDlg::handleException(const IceUtil::Exception& ex)
{
    _status->SetWindowText(CString(ex.ice_name().c_str()));
}
