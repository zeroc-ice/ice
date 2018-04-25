// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
