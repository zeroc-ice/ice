// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_STORM_CONFIG_H
#define ICE_STORM_CONFIG_H

#ifdef _MSC_VER
//
// Automatically link with IceStorm[D].lib
//
#   if defined(ICE_STATIC_LIBS)
#      pragma comment(lib, "IceStorm.lib")
#   elif !defined(ICE_STORM_LIB_API_EXPORTS)
#      if defined(_DEBUG)
#          pragma comment(lib, "IceStormD.lib")
#      else
#          pragma comment(lib, "IceStorm.lib")
#      endif
#   endif
#endif

#endif
