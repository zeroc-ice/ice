// Copyright (c) ZeroC, Inc.

#ifndef ICEBOX_CONFIG_H
#define ICEBOX_CONFIG_H

#include "Ice/Config.h"

#ifndef ICEBOX_API
#    if defined(ICEBOX_API_EXPORTS)
#        define ICEBOX_API ICE_DECLSPEC_EXPORT
#    else
#        define ICEBOX_API ICE_DECLSPEC_IMPORT
#    endif
#endif

//
// Automatically link with IceBox[D].lib
//

#if !defined(ICE_BUILDING_ICEBOX) && defined(ICEBOX_API_EXPORTS)
#    define ICE_BUILDING_ICEBOX
#endif

#if defined(_MSC_VER) && !defined(ICE_BUILDING_ICEBOX)
#    pragma comment(lib, ICE_LIBNAME("IceBox"))
#endif

#endif
