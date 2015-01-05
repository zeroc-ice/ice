// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef GLACIER2_CONFIG_H
#define GLACIER2_CONFIG_H

#ifdef _MSC_VER
//
// Automatically link with Glacier2[D].lib
//
#   if defined(ICE_STATIC_LIBS)
#      pragma comment(lib, "Glacier2.lib")
#   elif !defined(GLACIER2_API_EXPORTS)
#      if defined(_DEBUG)
#          pragma comment(lib, "Glacier2D.lib")
#      else
#          pragma comment(lib, "Glacier2.lib")
#      endif
#   endif
#endif

#endif
