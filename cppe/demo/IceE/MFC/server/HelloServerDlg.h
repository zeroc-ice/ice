// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************


#ifndef HELLO_SERVER_DLG_H
#define HELLO_SERVER_DLG_H

#pragma once

#include "LogI.h"
#include <IceE/IceE.h>

class CHelloServerDlg : public CDialog
{
public:
    CHelloServerDlg(const Ice::CommunicatorPtr&, const LogIPtr&, CWnd* = NULL);

    enum { IDD = IDD_HELLOSERVER_DIALOG };

protected:
    virtual void DoDataExchange(CDataExchange*);    // DDX/DDV support

protected:
    Ice::CommunicatorPtr _communicator;
    LogIPtr _log;
    CEdit* _edit;
    CEdit* _display;
    HICON _hIcon;

    // Generated message map functions
    virtual BOOL OnInitDialog();
    virtual void OnCancel();
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnShutdown();
    afx_msg void OnClear();
    afx_msg LRESULT OnLog(WPARAM, LPARAM);
    DECLARE_MESSAGE_MAP()
};

#endif
