// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef HELLO_CLIENT_DLG_H
#define HELLO_CLIENT_DLG_H

#include "Hello.h"

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
    HelloPrx _proxy;
    HelloPrx _currentProxy;
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
