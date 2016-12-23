// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UTIL_RESOURCE_CONFIG_H
#define ICE_UTIL_RESOURCE_CONFIG_H

#include "winver.h"

#define ICE_VERSION 3,7,54,0
#define ICE_STRING_VERSION "3.7a4\0"
#define ICE_SO_VERSION "37a4"
#define ICE_COMPANY_NAME "ZeroC, Inc.\0"
#define ICE_COPYRIGHT "\251 2003-2016 ZeroC, Inc.\0"
#define ICE_PRODUCT_NAME "Ice\0"

#ifdef ICE_CPP11_MAPPING
#  if defined(_DEBUG)
#    if defined(ICE_OS_UWP)
#      define ICE_LIBNAME(NAME) NAME ICE_SO_VERSION "uwp++11D"
#    else
#      define ICE_LIBNAME(NAME) NAME ICE_SO_VERSION "++11D"
#    endif
#  else
#    if defined(ICE_OS_UWP)
#      define ICE_LIBNAME(NAME) NAME ICE_SO_VERSION "uwp++11"
#    else
#      define ICE_LIBNAME(NAME) NAME ICE_SO_VERSION "++11"
#    endif
#  endif
#else
#  if defined(_DEBUG)
#    if defined(ICE_OS_UWP)
#      define ICE_LIBNAME(NAME) NAME ICE_SO_VERSION "uwpD"
#    else
#      define ICE_LIBNAME(NAME) NAME ICE_SO_VERSION "D"
#    endif
#  else
#    if defined(ICE_OS_UWP)
#      define ICE_LIBNAME(NAME) NAME ICE_SO_VERSION "uwp"
#    else
#      define ICE_LIBNAME(NAME) NAME ICE_SO_VERSION ""
#    endif
#  endif
#endif

#ifndef DEBUG
#  define VER_DEBUG 0
#else
#  define VER_DEBUG VS_FF_DEBUG
#endif

#endif
