// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//


#if !defined(AFX_STDAFX_H__CCAD4B79_862C_4DFB_A13B_E56154F5C0B8__INCLUDED_)
#define AFX_STDAFX_H__CCAD4B79_862C_4DFB_A13B_E56154F5C0B8__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef _WIN32_WCE

#if !defined(NDEBUG) && !defined(_STLP_DEBUG)
#   define _STLP_DEBUG
#endif

#ifndef WINVER
#define WINVER 0x0400
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif

#else

#if _MSC_VER >= 1400
#  define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA
#  ifndef WINVER
#    define WINVER 0x0400
#  endif
#endif

#if (_WIN32_WCE <= 200)
#error : This project does not support MFCCE 2.00 or earlier, because it requires CControlBar, available only in MFCCE 2.01 or later
#endif

//
// _STLP_USE_MFC must be defined. We don't use iostreams so lets turn
// that off too. This must defined here since some of the AFX header
// files include STLport headers.
//
#define _STLP_USE_MFC
#define _STLP_NO_NEW_IOSTREAMS
#define _STLP_NO_IOSTREAMS

#endif // _WIN32_WCE

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions

#if defined(_WIN32_WCE) && (_WIN32_WCE >= 211) && (_AFXDLL)
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#endif


#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxsock.h>		// MFC socket extensions

//{{AFX_INSERT_LOCATION}}
// Microsoft eMbedded Visual C++ will insert additional declarations immediately before the previous line.

#include <IceE/IceE.h>

#endif // !defined(AFX_STDAFX_H__CCAD4B79_862C_4DFB_A13B_E56154F5C0B8__INCLUDED_)
