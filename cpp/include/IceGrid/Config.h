// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_CONFIG_H
#define ICE_GRID_CONFIG_H

//
// Automatically link with IceGrid[D].lib
//

#if !defined(ICE_BUILDING_ICE_GRID) && defined(ICE_GRID_API_EXPORTS)
#   define ICE_BUILDING_ICE_GRID
#endif

#ifdef _MSC_VER
#   if !defined(ICE_BUILDING_ICE_GRID)
#      if defined(_DEBUG) && !defined(ICE_OS_WINRT)
#          pragma comment(lib, "IceGridD.lib")
#      else
#          pragma comment(lib, "IceGrid.lib")
#      endif
#   endif
#endif

#endif
