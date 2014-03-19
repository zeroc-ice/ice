// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


#ifndef HELLO_CLIENT_DLG_H
#define HELLO_CLIENT_DLG_H

#include "Hello.h"

#pragma once

class CHelloClientDlg : public CDialog
{
public:

    CHelloClientDlg(CWnd* = NULL);

    enum { IDD = IDD_HELLOCLIENT_DIALOG };

    afx_msg void OnCbnSelchangeMode();

    void exception(const Ice::Exception&);
    void response();;
    void sent();
    void flushed();

protected:

    virtual void DoDataExchange(CDataExchange*);    // DDX/DDV support

    Ice::CommunicatorPtr _communicator;
    Demo::Callback_Hello_sayHelloPtr _sayHelloCallback;
    Demo::Callback_Hello_shutdownPtr _shutdownCallback;
    Ice::Callback_Communicator_flushBatchRequestsPtr _flushCallback;
    CEdit* _host;
    CComboBox* _mode;
    CSliderCtrl* _timeout;
    CStatic* _timeoutStatus;
    CSliderCtrl* _delay;
    CStatic* _delayStatus;
    CStatic* _status;
    CButton* _flush;
    HICON _hIcon;

    // Generated message map functions
    virtual BOOL OnInitDialog();
    afx_msg void OnClose();
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnHScroll(UINT, UINT, CScrollBar*);
    afx_msg void OnSayHello();
    afx_msg void OnFlush();
    afx_msg void OnShutdown();
    afx_msg LRESULT OnAMICallback(WPARAM, LPARAM);
    DECLARE_MESSAGE_MAP()

private:

    Demo::HelloPrx createProxy();
    BOOL deliveryModeIsBatch();
    void handleException(const IceUtil::Exception&);
};

#endif
