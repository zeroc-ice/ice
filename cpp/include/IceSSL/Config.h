// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef ICESSL_CONFIG_H
#define ICESSL_CONFIG_H

#include <Ice/Config.h>

//
// Automatically link IceSSL[D|++11|++11D].lib with Visual C++
//
#if !defined(ICE_BUILDING_ICESSL) && defined(ICESSL_API_EXPORTS)
#  define ICE_BUILDING_ICESSL
#endif

#if defined(_MSC_VER) && !defined(ICE_BUILDING_ICESSL)
#  pragma comment(lib, ICE_LIBNAME("IceSSL"))
#endif

#endif
