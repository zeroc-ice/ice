// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


#ifndef CHAT_CLIENT_DLG_H
#define CHAT_CLIENT_DLG_H

#pragma once

#include "LogI.h"
#include "Chat.h"

class CChatClientDlg : public CDialog
{
public:
    CChatClientDlg(const Ice::CommunicatorPtr&, const LogIPtr&, CWnd* = NULL);

    void setSession(const Demo::ChatSessionPrx&);

    enum { IDD = IDD_CHATCLIENT_DIALOG };

protected:
    virtual void DoDataExchange(CDataExchange*);    // DDX/DDV support

protected:
    Ice::CommunicatorPtr _communicator;
    Demo::ChatSessionPrx _chat;
    LogIPtr _log;
    CEdit* _edit;
    HICON _hIcon;

    // Generated message map functions
    virtual BOOL OnInitDialog();
    virtual void OnCancel();
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnLogin();
    afx_msg void OnSend();
    DECLARE_MESSAGE_MAP()
};

#endif
