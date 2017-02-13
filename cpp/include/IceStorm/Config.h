// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_STORM_CONFIG_H
#define ICE_STORM_CONFIG_H

//
// Automatically link with IceStorm[D].lib
//

#if !defined(ICE_BUILDING_ICE_STORM_LIB) && defined(ICE_STORM_LIB_API_EXPORTS)
#   define ICE_BUILDING_ICE_STORM_LIB
#endif

#ifdef _MSC_VER
#   if !defined(ICE_BUILDING_ICE_STORM_LIB)
#      if defined(_DEBUG) && !defined(ICE_OS_WINRT)
#          pragma comment(lib, "IceStormD.lib")
#      else
#          pragma comment(lib, "IceStorm.lib")
#      endif
#   endif
#endif

#endif
