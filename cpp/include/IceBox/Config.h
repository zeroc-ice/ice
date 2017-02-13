// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEBOX_CONFIG_H
#define ICEBOX_CONFIG_H

//
// Automatically link with IceBox[D|++11|++11D].lib
//

#if !defined(ICE_BUILDING_ICEBOX) && defined(ICEBOX_API_EXPORTS)
#   define ICE_BUILDING_ICEBOX
#endif

#if defined(_MSC_VER) && !defined(ICE_BUILDING_ICEBOX)
#   pragma comment(lib, ICE_LIBNAME("IceBox"))
#endif

#endif
