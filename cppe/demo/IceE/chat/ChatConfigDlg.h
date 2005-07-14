// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


#ifndef CHAT_CONFIG_DLG_H
#define CHAT_CONFIG_DLG_H

#pragma once

#include "ChatClientDlg.h"

class CChatConfigDlg : public CDialog
{
public:
    CChatConfigDlg(const Ice::CommunicatorPtr&, const LogIPtr&, CChatClientDlg*, CWnd* = NULL);

    enum { IDD = IDD_CHATCONFIG_DIALOG };

protected:
    virtual void DoDataExchange(CDataExchange*);    // DDX/DDV support

protected:
    Ice::CommunicatorPtr _communicator;
    LogIPtr _log;
    CChatClientDlg* _mainDiag;
    CEdit* _useredit;
    CEdit* _passedit;
    CEdit* _hostedit;
    CEdit* _portedit;
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
