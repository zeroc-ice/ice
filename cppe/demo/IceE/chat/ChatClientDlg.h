// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef CHAT_CLIENT_DLG_H
#define CHAT_CLIENT_DLG_H

#pragma once

#include <LogI.h>
#include <Chat.h>
#include <PingThread.h>

class CChatClientDlg : public CDialog
{
public:

    CChatClientDlg(const Ice::CommunicatorPtr&, const LogIPtr&, CWnd* = NULL);
    ~CChatClientDlg();

    enum { IDD = IDD_CHATCLIENT_DIALOG };

protected:

    virtual void DoDataExchange(CDataExchange*);    // DDX/DDV support
#ifndef _WIN32_WCE
    virtual void OnOK();
#endif

protected:

    const Ice::CommunicatorPtr _communicator;
    Ice::ObjectAdapterPtr _adapter;
    Demo::ChatCallbackPrx _callback;
    Demo::ChatSessionPrx _chat;
    Glacier2::RouterPrx _router;
    const LogIPtr _log;

    SessionPingThreadPtr _ping;
    CEdit* _edit;
    CEdit* _display;
    HICON _hIcon;

    CString _user;
    CString _password;
    CString _host;
    CString _port;

    void setDialogState();

    // Generated message map functions
    virtual BOOL OnInitDialog();
    virtual void OnCancel();
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnLogin();
    afx_msg void OnSend();
    afx_msg LRESULT OnLog(WPARAM, LPARAM);
    DECLARE_MESSAGE_MAP()
};

#endif
