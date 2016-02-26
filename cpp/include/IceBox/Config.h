// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_BOX_CONFIG_H
#define ICE_BOX_CONFIG_H

//
// Automatically link with IceBox[D|++11|++11D].lib
//

#if !defined(ICE_BUILDING_ICE_BOX) && defined(ICE_BOX_API_EXPORTS)
#   define ICE_BUILDING_ICE_BOX
#endif

#if defined(_MSC_VER) && !defined(ICE_BUILDING_ICE_BOX)
#   pragma comment(lib, ICE_LIBNAME("IceBox"))
#endif

#endif
