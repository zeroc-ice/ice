// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_CONFIG_H
#define ICE_SSL_CONFIG_H

#include <Ice/Config.h>

#if defined(__APPLE__)
#  define ICE_USE_SECURE_TRANSPORT 1
#elif defined(_WIN32)
#  define ICE_USE_SCHANNEL 1
#else
#  define ICE_USE_OPENSSL 1
#endif

#endif
