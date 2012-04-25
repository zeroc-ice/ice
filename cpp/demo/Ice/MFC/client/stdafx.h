// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

//
// For VC6 and STLport
//
#if !defined(NDEBUG) && !defined(_STLP_DEBUG)
#   define _STLP_DEBUG
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

// Change this to the appropriate value to target Windows 98 and
// Windows 2000 or later if not using VC6.
#ifndef _WIN32_WINNT
#  if defined(_MSC_VER) && _MSC_VER < 1500
#    define _WIN32_WINNT 0x0400
#  endif
#endif

#ifndef WINVER
#  if defined(_MSC_VER) && _MSC_VER > 1300 && _MSC_VER < 1600
#    define WINVER 0x0400
#  elif defined(_MSC_VER) && _MSC_VER >=1600
 #   define WINVER 0x0501
#  endif
#endif

#ifndef _WIN32_IE                       // Allow use of features specific to IE 4.0 or later.
#   if defined(_MSC_VER) && _MSC_VER >=1600
#      define _WIN32_IE 0x0500        // Change this to the appropriate value to target IE 5.0 or later.
#   else
#      define _WIN32_IE 0x0400        
#   endif
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions

#include <afxdtctl.h>           // MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>                     // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <Ice/Ice.h>
