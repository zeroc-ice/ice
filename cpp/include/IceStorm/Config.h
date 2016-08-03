// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_STORM_CONFIG_H
#define ICE_STORM_CONFIG_H

#include <Ice/Config.h>

//
// Automatically link with IceStorm[D|++11|++11D].lib
//

#if !defined(ICE_BUILDING_ICE_STORM_LIB) && defined(ICE_STORM_LIB_API_EXPORTS)
#   define ICE_BUILDING_ICE_STORM_LIB
#endif

#if defined(_MSC_VER) && !defined(ICE_BUILDING_ICE_STORM_LIB)
#   pragma comment(lib, ICE_LIBNAME("IceStorm"))
#endif

#endif
