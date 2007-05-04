// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************


#ifndef HELLO_SERVER_H
#define HELLO_SERVER_H

#pragma once

#ifndef __AFXWIN_H__
    #error include 'stdafx.h' before including this file for PCH
#endif

#include "Resource.h"

class CHelloServerApp : public CWinApp
{
public:
    CHelloServerApp();

    virtual BOOL InitInstance();

    DECLARE_MESSAGE_MAP()
};

extern CHelloServerApp theApp;

#endif
