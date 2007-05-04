// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef CHAT_CONFIG_DLG_H
#define CHAT_CONFIG_DLG_H

#pragma once

#include <resource.h>

class CChatConfigDlg : public CDialog
{
public:

    CChatConfigDlg(const CString&, const CString&, const CString&, const CString&, CWnd* = NULL);

    enum { IDD = IDD_CHATCONFIG_DIALOG };

    CString getUser() const;
    CString getPassword() const;
    CString getHost() const;
    CString getPort() const;

protected:

    virtual void DoDataExchange(CDataExchange*);    // DDX/DDV support

protected:

    CEdit* _useredit;
    CEdit* _passedit;
    CEdit* _hostedit;
    CEdit* _portedit;

    CString _user;
    CString _password;
    CString _host;
    CString _port;

    HICON _hIcon;

    // Generated message map functions
    virtual BOOL OnInitDialog();
    virtual void OnCancel();
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnLogin();
    DECLARE_MESSAGE_MAP()
};

#endif
