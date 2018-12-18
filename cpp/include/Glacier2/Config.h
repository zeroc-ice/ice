// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef GLACIER2_CONFIG_H
#define GLACIER2_CONFIG_H

//
// Automatically link with Glacier2[D|++11|++11D].lib
//

#if !defined(ICE_BUILDING_GLACIER2) && defined(GLACIER2_API_EXPORTS)
#   define ICE_BUILDING_GLACIER2
#endif

#if defined(_MSC_VER) && !defined(ICE_BUILDING_GLACIER2)
#   pragma comment(lib, ICE_LIBNAME("Glacier2"))
#endif

#ifndef GLACIER2_API
#   if defined(ICE_STATIC_LIBS)
#       define GLACIER2_API /**/
#   elif defined(GLACIER2_API_EXPORTS)
#       define GLACIER2_API ICE_DECLSPEC_EXPORT
#   else
#       define GLACIER2_API ICE_DECLSPEC_IMPORT
#   endif
#endif

#endif
