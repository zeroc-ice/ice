// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
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

    CHelloClientDlg(const Ice::CommunicatorPtr&, CWnd* = NULL);

    enum { IDD = IDD_HELLOCLIENT_DIALOG };

    afx_msg void OnCbnSelchangeMode();

protected:

    virtual void DoDataExchange(CDataExchange*);    // DDX/DDV support

    Ice::CommunicatorPtr _communicator;
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
    afx_msg LRESULT OnAMISent(WPARAM, LPARAM);
    afx_msg LRESULT OnAMIResponse(WPARAM, LPARAM);
    afx_msg LRESULT OnAMIException(WPARAM, LPARAM);
    afx_msg LRESULT OnAMIFlush(WPARAM, LPARAM);
    DECLARE_MESSAGE_MAP()

private:

    Demo::HelloPrx createProxy();
    BOOL deliveryModeIsBatch();
    void handleException(const IceUtil::Exception&);
};

#endif
