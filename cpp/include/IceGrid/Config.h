//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
