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
// Automatically link with IceGrid[D|++11|++11D].lib
//

#if !defined(ICE_BUILDING_ICE_GRID) && defined(ICEGRID_API_EXPORTS)
#   define ICE_BUILDING_ICE_GRID
#endif

#if defined(_MSC_VER) && !defined(ICE_BUILDING_ICE_GRID)
#   pragma comment(lib, ICE_LIBNAME("IceGrid"))
#endif

#endif
