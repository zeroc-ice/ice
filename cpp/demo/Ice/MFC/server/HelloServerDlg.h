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

#ifndef HELLO_SERVER_DLG_H
#define HELLO_SERVER_DLG_H

#pragma once

#include "LogI.h"

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
    HICON _hIcon;

    // Generated message map functions
    virtual BOOL OnInitDialog();
    virtual void OnCancel();
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnShutdown();
    afx_msg void OnClear();
    DECLARE_MESSAGE_MAP()
};

#endif
