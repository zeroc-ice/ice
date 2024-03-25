//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_PATCH2_CONFIG_H
#define ICE_PATCH2_CONFIG_H

#include "Ice/Config.h"

#ifndef ICEPATCH2_API
#    if defined(ICE_STATIC_LIBS)
#        define ICEPATCH2_API /**/
#    elif defined(ICEPATCH2_API_EXPORTS)
#        define ICEPATCH2_API ICE_DECLSPEC_EXPORT
#    else
#        define ICEPATCH2_API ICE_DECLSPEC_IMPORT
#    endif
#endif

//
// Automatically link with IcePatch2[D].lib
//

#if !defined(ICE_BUILDING_ICE_PATCH2) && defined(ICEPATCH2_API_EXPORTS)
#    define ICE_BUILDING_ICE_PATCH2
#endif

#if defined(_MSC_VER) && !defined(ICE_BUILDING_ICE_PATCH2)
#    pragma comment(lib, ICE_LIBNAME("IcePatch2"))
#endif

#endif
