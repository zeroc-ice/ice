// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef ICE_STORM_CONFIG_H
#define ICE_STORM_CONFIG_H

#include <Ice/Config.h>

//
// Automatically link with IceStorm[D|++11|++11D].lib
//

#if !defined(ICE_BUILDING_ICE_STORM_LIB) && defined(ICESTORM_API_EXPORTS)
#   define ICE_BUILDING_ICE_STORM_LIB
#endif

#if defined(_MSC_VER) && !defined(ICE_BUILDING_ICE_STORM_LIB)
#   pragma comment(lib, ICE_LIBNAME("IceStorm"))
#endif

#endif
