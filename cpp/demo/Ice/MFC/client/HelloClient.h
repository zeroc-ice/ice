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
