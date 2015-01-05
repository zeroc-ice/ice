// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_PATCH2_CONFIG_H
#define ICE_PATCH2_CONFIG_H

#ifdef _MSC_VER
//
// Automatically link with IcePatch2[D].lib
//
#   if defined(ICE_STATIC_LIBS)
#      pragma comment(lib, "IcePatch2.lib")
#   elif !defined(ICE_PATCH2_API_EXPORTS)
#      if defined(_DEBUG)
#          pragma comment(lib, "IcePatch2D.lib")
#      else
#          pragma comment(lib, "IcePatch2.lib")
#      endif
#   endif
#endif

#endif

