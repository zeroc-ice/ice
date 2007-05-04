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

CHelloClientDlg::CHelloClientDlg(const Ice::CommunicatorPtr& communicator, CWnd* pParent /*=NULL*/) :
    CDialog(CHelloClientDlg::IDD, pParent), _communicator(communicator), _currentMode(0),
    _useSecure(false), _useTimeout(false)
{
    _hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void
CHelloClientDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CHelloClientDlg, CDialog)
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    //}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_INVOKE, OnSayHello)
    ON_BN_CLICKED(IDC_FLUSH, OnFlush)
    ON_BN_CLICKED(IDC_SHUTDOWN, OnShutdown)
END_MESSAGE_MAP()

BOOL
CHelloClientDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    // Set the icon for this dialog.  The framework does this automatically
    // when the application's main window is not a dialog
    SetIcon(_hIcon, TRUE);            // Set big icon
    SetIcon(_hIcon, FALSE);        // Set small icon

    //
    // Retrieve the controls.
    //
    _mode = (CComboBox*)GetDlgItem(IDC_MODE);
    _timeout = (CButton*)GetDlgItem(IDC_TIMEOUT);
    _status = (CStatic*)GetDlgItem(IDC_STATUSBAR);

    //
    // Use twoway mode as the initial default.
    //
    _mode->SetCurSel(_currentMode);


    //
    // Disable flush button if built without batch support.
    //
#ifndef ICEE_HAS_BATCH
    (CButton*)GetDlgItem(IDC_FLUSH)->EnableWindow(FALSE);
#endif


    //
    // Create the proxy.
    //
    Ice::PropertiesPtr properties = _communicator->getProperties();
    const char* proxyProperty = "Hello.Proxy";
    std::string proxy = properties->getProperty(proxyProperty);
    Ice::ObjectPrx obj = _communicator->stringToProxy(proxy);
    _proxy = Demo::HelloPrx::uncheckedCast(obj);
    _currentProxy = _proxy;
    _status->SetWindowText(CString(" Ready"));

    return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
// to draw the icon.  For MFC applications using the document/view model,
// this is automatically done for you by the framework.

void
CHelloClientDlg::OnPaint() 
{
#ifdef _WIN32_WCE
    CDialog::OnPaint();
#else
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
#endif
}

// The system calls this function to obtain the cursor to display while the user drags
// the minimized window.
HCURSOR
CHelloClientDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(_hIcon);
}

void
CHelloClientDlg::OnSayHello()
{
    try
    {
        updateProxy();
        _currentProxy->sayHello();
#ifdef ICEE_HAS_BATCH
        if(_currentProxy->ice_isBatchOneway())
        {
            _status->SetWindowText(CString(" Queued batch request"));
        }
        else
#endif
        {
            _status->SetWindowText(CString(" Sent request"));
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
#ifdef ICEE_HAS_BATCH
    try
    {
        _communicator->flushBatchRequests();
        _status->SetWindowText(CString(" Flushed batch requests"));
    }
    catch(const IceUtil::Exception& ex)
    {
        handleException(ex);
    }
#endif
}

void
CHelloClientDlg::OnShutdown()
{
    try
    {
        updateProxy();
        _currentProxy->shutdown();
#ifdef ICEE_HAS_BATCH
        if(_currentProxy->ice_isBatchOneway())
        {
            _status->SetWindowText(CString(" Queued shutdown request"));
        }
        else
#endif
        {
            _status->SetWindowText(CString(" Sent shutdown request"));
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        handleException(ex);
    }
}

void
CHelloClientDlg::updateProxy()
{
    int mode = _mode->GetCurSel();
    bool timeout = _timeout->GetCheck() == BST_CHECKED;
    if(mode == _currentMode && timeout == _useTimeout)
    {
        return;
    }

    Ice::ObjectPrx proxy;
    switch(mode)
    {
    case 0:
        proxy = _proxy->ice_twoway();
        break;
    case 1:
        proxy = _proxy->ice_oneway();
        break;
    case 2:
#ifdef ICEE_HAS_BATCH
        proxy = _proxy->ice_batchOneway();
        break;
#else
        AfxMessageBox(CString("Batch mode is currently not enabled."),
                      MB_OK|MB_ICONEXCLAMATION);
	return;
#endif
    default:
        assert(false);
    }
    if(timeout)
    {
        proxy = proxy->ice_timeout(2000);
    }
    else
    {
        proxy = proxy->ice_timeout(-1);
    }

    if(proxy->ice_isTwoway())
    {
        _currentProxy = Demo::HelloPrx::checkedCast(proxy);
    }
    else
    {
        _currentProxy = Demo::HelloPrx::uncheckedCast(proxy);
    }
    _currentMode = mode;
    _useTimeout = timeout;
}

void
CHelloClientDlg::handleException(const IceUtil::Exception& e)
{
    try
    {
        e.ice_throw();
    }
    catch(const Ice::NoEndpointException&)
    {
        AfxMessageBox(CString("The proxy does not support the current configuration"),
                      MB_OK|MB_ICONEXCLAMATION);
    }
    catch(const IceUtil::Exception& ex)
    {
        AfxMessageBox(CString(ex.toString().c_str()), MB_OK|MB_ICONEXCLAMATION);
    }
    _status->SetWindowText(CString(" Ready"));
}
