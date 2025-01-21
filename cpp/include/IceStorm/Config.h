// Copyright (c) ZeroC, Inc.

#ifndef ICESTORM_CONFIG_H
#define ICESTORM_CONFIG_H

#include "Ice/Config.h"

//
// Automatically link with IceStorm[D].lib
//

#if !defined(ICE_BUILDING_ICESTORM_LIB) && defined(ICESTORM_API_EXPORTS)
#    define ICE_BUILDING_ICESTORM_LIB
#endif

#if defined(_MSC_VER) && !defined(ICE_BUILDING_ICESTORM_LIB)
#    pragma comment(lib, ICE_LIBNAME("IceStorm"))
#endif

#endif
