//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_UTIL_RESOURCE_CONFIG_H
#define ICE_UTIL_RESOURCE_CONFIG_H

#include "winver.h"

#define ICE_VERSION 4,0,0,0
#define ICE_STRING_VERSION "4.0.0-alpha.0\0"
#define ICE_SO_VERSION "40a0\0"
#define ICE_ALPHA_VERSION 0
#define ICE_COMPANY_NAME "ZeroC, Inc.\0"
#define ICE_COPYRIGHT "\251 ZeroC, Inc.\0"
#define ICE_PRODUCT_NAME "Ice\0"

#  if defined(_DEBUG)
#      define ICE_LIBNAME(NAME) NAME ICE_SO_VERSION "D"
#  else
#      define ICE_LIBNAME(NAME) NAME ICE_SO_VERSION
#  endif

#ifndef DEBUG
#  define VER_DEBUG 0
#else
#  define VER_DEBUG VS_FF_DEBUG
#endif

#endif
