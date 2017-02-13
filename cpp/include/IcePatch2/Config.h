// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_PATCH2_CONFIG_H
#define ICE_PATCH2_CONFIG_H

//
// Automatically link with IcePatch2[D].lib
//

#if !defined(ICE_BUILDING_ICE_PATCH2) && defined(ICE_PATCH2_API_EXPORTS)
#   define ICE_BUILDING_ICE_PATCH2
#endif

#ifdef _MSC_VER
#   if !defined(ICE_BUILDING_ICE_PATCH2)
#      if defined(_DEBUG) && !defined(ICE_OS_WINRT)
#          pragma comment(lib, "IcePatch2D.lib")
#      else
#          pragma comment(lib, "IcePatch2.lib")
#      endif
#   endif
#endif

#endif

