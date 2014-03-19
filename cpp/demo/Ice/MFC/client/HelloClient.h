// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


#ifndef HELLO_CLIENT_H
#define HELLO_CLIENT_H

#pragma once

#ifndef __AFXWIN_H__
    #error include 'stdafx.h' before including this file for PCH
#endif

#include "Resource.h"

class CHelloClientApp : public CWinApp
{
public:
    CHelloClientApp();

    virtual BOOL InitInstance();

    DECLARE_MESSAGE_MAP()
};

extern CHelloClientApp theApp;

#endif
