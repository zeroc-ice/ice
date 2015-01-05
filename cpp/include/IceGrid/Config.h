// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_CONFIG_H
#define ICE_GRID_CONFIG_H

#ifdef _MSC_VER
//
// Automatically link with IceGrid[D].lib
//
#   if defined(ICE_STATIC_LIBS)
#      pragma comment(lib, "IceGrid.lib")
#   elif !defined(ICE_GRID_API_EXPORTS)
#      if defined(_DEBUG)
#          pragma comment(lib, "IceGridD.lib")
#      else
#          pragma comment(lib, "IceGrid.lib")
#      endif
#   endif
#endif

#endif
