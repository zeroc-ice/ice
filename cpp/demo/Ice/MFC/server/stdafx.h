// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef VC_EXTRALEAN
#   define VC_EXTRALEAN                // Exclude rarely-used stuff from Windows headers
#endif

#ifndef _WIN32_WINNT
#    define _WIN32_WINNT 0x0501
#endif

#ifndef WINVER
#   define WINVER 0x0501
#endif

#ifndef _WIN32_IE                          // Allow use of features specific to IE 5.0 or later.
#   define _WIN32_IE 0x0500
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#include <afxwin.h>                        // MFC core and standard components
#include <afxext.h>                        // MFC extensions

#ifndef _AFX_NO_AFXCMN_SUPPORT
#   include <afxcmn.h>                    // MFC support for Windows Common Controls
#endif

#include <Ice/Ice.h>