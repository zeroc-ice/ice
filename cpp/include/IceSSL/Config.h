//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICESSL_CONFIG_H
#define ICESSL_CONFIG_H

#include "Ice/Config.h"

#ifndef ICESSL_API
#    if defined(ICE_STATIC_LIBS)
#        define ICESSL_API /**/
#    elif defined(ICESSL_API_EXPORTS)
#        define ICESSL_API ICE_DECLSPEC_EXPORT
#    else
#        define ICESSL_API ICE_DECLSPEC_IMPORT
#    endif
#endif

//
// Automatically link IceSSL[D].lib with Visual C++
//
#if !defined(ICE_BUILDING_ICESSL) && defined(ICESSL_API_EXPORTS)
#    define ICE_BUILDING_ICESSL
#endif

#if defined(_MSC_VER) && !defined(ICE_BUILDING_ICESSL)
#    pragma comment(lib, ICE_LIBNAME("IceSSL"))
#endif

#endif
