// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************


#ifndef HELLO_CLIENT_DLG_H
#define HELLO_CLIENT_DLG_H

#include "Hello.h"
#include <IceE/IceE.h>

#pragma once

class CHelloClientDlg : public CDialog
{
public:
    CHelloClientDlg(const Ice::CommunicatorPtr&, CWnd* = NULL);

    enum { IDD = IDD_HELLOCLIENT_DIALOG };

protected:
    virtual void DoDataExchange(CDataExchange*);    // DDX/DDV support

protected:
    Ice::CommunicatorPtr _communicator;
    CComboBox* _mode;
    CButton* _secure;
    CButton* _timeout;
    CStatic* _status;
    Demo::HelloPrx _proxy;
    Demo::HelloPrx _currentProxy;
    int _currentMode;
    bool _useSecure;
    bool _useTimeout;
    HICON _hIcon;

    // Generated message map functions
    virtual BOOL OnInitDialog();
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnSayHello();
    afx_msg void OnFlush();
    afx_msg void OnShutdown();
    DECLARE_MESSAGE_MAP()

    void updateProxy();
    void handleException(const IceUtil::Exception&);
};

#endif
