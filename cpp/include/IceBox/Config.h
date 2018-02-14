// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_BOX_CONFIG_H
#define ICE_BOX_CONFIG_H

//
// Automatically link with IceBox[D].lib
//

#if !defined(ICE_BUILDING_ICE_BOX) && defined(ICE_BOX_API_EXPORTS)
#   define ICE_BUILDING_ICE_BOX
#endif

#ifdef _MSC_VER
#   if !defined(ICE_BUILDING_ICE_BOX)
#      if defined(_DEBUG) && !defined(ICE_OS_WINRT)
#          pragma comment(lib, "IceBoxD.lib")
#      else
#          pragma comment(lib, "IceBox.lib")
#      endif
#   endif
#endif

#endif
