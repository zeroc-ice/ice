// Copyright (c) ZeroC, Inc.

#ifndef GLACIER2_CONFIG_H
#define GLACIER2_CONFIG_H

#include "Ice/Config.h"

#if !defined(ICE_BUILDING_GLACIER2) && defined(GLACIER2_API_EXPORTS)
#    define ICE_BUILDING_GLACIER2
#endif

#if defined(_MSC_VER) && !defined(ICE_BUILDING_GLACIER2)
#    pragma comment(lib, ICE_LIBNAME("Glacier2")) // Automatically link with Glacier2[D].lib
#endif

#ifndef GLACIER2_API
#    if defined(GLACIER2_API_EXPORTS)
#        define GLACIER2_API ICE_DECLSPEC_EXPORT
#    else
#        define GLACIER2_API ICE_DECLSPEC_IMPORT
#    endif
#endif

#endif
