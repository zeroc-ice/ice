// Copyright (c) ZeroC, Inc.

#ifndef ICEGRID_CONFIG_H
#define ICEGRID_CONFIG_H

#include "Ice/Config.h"

#ifndef ICEGRID_API
#    if defined(ICEGRID_API_EXPORTS)
#        define ICEGRID_API ICE_DECLSPEC_EXPORT
#    else
#        define ICEGRID_API ICE_DECLSPEC_IMPORT
#    endif
#endif

#if !defined(ICE_BUILDING_ICEGRID) && defined(ICEGRID_API_EXPORTS)
#    define ICE_BUILDING_ICEGRID
#endif

#if defined(_MSC_VER) && !defined(ICE_BUILDING_ICEGRID)
#    pragma comment(lib, ICE_LIBNAME("IceGrid")) // Automatically link with IceGrid[D].lib
#endif

#endif
