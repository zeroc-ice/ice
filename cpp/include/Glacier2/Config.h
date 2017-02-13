// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef GLACIER2_CONFIG_H
#define GLACIER2_CONFIG_H

//
// Automatically link with Glacier2[D].lib
//

#if !defined(ICE_BUILDING_GLACIER2) && defined(GLACIER2_API_EXPORTS)
#   define ICE_BUILDING_GLACIER2
#endif

#ifdef _MSC_VER
#   if !defined(ICE_BUILDING_GLACIER2)
#      if defined(_DEBUG) && !defined(ICE_OS_WINRT)
#          pragma comment(lib, "Glacier2D.lib")
#      else
#          pragma comment(lib, "Glacier2.lib")
#      endif
#   endif
#endif

#ifndef GLACIER2_API
#   ifdef GLACIER2_API_EXPORTS
#       define GLACIER2_API ICE_DECLSPEC_EXPORT
#   elif defined(ICE_STATIC_LIBS)
#       define GLACIER2_API /**/
#   else
#       define GLACIER2_API ICE_DECLSPEC_IMPORT
#   endif
#endif

#endif
