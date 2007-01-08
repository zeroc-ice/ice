// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef PATCH_DLG_H
#define PATCH_DLG_H

#pragma once

const int kilobyte = 1024;
const int megabyte = 1024 * kilobyte;
const int gigabyte = 1024 * megabyte;

class CPatchDlg : public CDialog
{
public:
    CPatchDlg(const Ice::CommunicatorPtr&, CWnd* = NULL);

    enum { IDD = IDD_PATCHCLIENT_DIALOG };

    bool checksumStart();
    bool checksumProgress(const std::string&);
    bool checksumEnd();

    bool fileListStart();
    bool fileListProgress(Ice::Int);
    bool fileListEnd();

    bool patchStart(const std::string&, Ice::Long, Ice::Long, Ice::Long);
    bool patchProgress(Ice::Long, Ice::Long, Ice::Long, Ice::Long);
    bool patchEnd();

protected:
    virtual void DoDataExchange(CDataExchange*);    // DDX/DDV support

protected:
    const Ice::CommunicatorPtr _communicator;
    IceUtil::Time _startTime;
    CEdit* _path;
    CButton* _thorough;
    CButton* _remove;
    CButton* _select;
    CButton* _start;
    CButton* _cancel;
    CStatic* _file;
    CStatic* _total;
    CStatic* _speed;
    CStatic* _status;
    CStatic* _percent;
    CProgressCtrl* _progress;
    bool _isPatch;
    bool _isCancel;
    bool _isComplete;
    HICON _hIcon;

    // Generated message map functions
    virtual BOOL OnInitDialog();
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnSelectDir();
    afx_msg void OnStartPatch();
    afx_msg void OnCancel();
    DECLARE_MESSAGE_MAP()

    void reset(const CString&);
    void processMessages();
    void handleException(const IceUtil::Exception&);
    CString convertSize(Ice::Long) const;
};

#endif
