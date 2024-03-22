//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICEGRID_CONFIG_H
#define ICEGRID_CONFIG_H

#include "Ice/Config.h"

#ifndef ICEGRID_API
#    if defined(ICE_STATIC_LIBS)
#        define ICEGRID_API /**/
#    elif defined(ICEGRID_API_EXPORTS)
#        define ICEGRID_API ICE_DECLSPEC_EXPORT
#    else
#        define ICEGRID_API ICE_DECLSPEC_IMPORT
#    endif
#endif

//
// Automatically link with IceGrid[D].lib
//

#if !defined(ICE_BUILDING_ICE_GRID) && defined(ICEGRID_API_EXPORTS)
#    define ICE_BUILDING_ICE_GRID
#endif

#if defined(_MSC_VER) && !defined(ICE_BUILDING_ICE_GRID)
#    pragma comment(lib, ICE_LIBNAME("IceGrid"))
#endif

#endif
