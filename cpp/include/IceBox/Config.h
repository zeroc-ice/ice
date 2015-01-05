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

#ifdef _MSC_VER
//
// Automatically link with IceBox[D].lib
//
#   if defined(ICE_STATIC_LIBS)
#      pragma comment(lib, "IceBox.lib")
#   elif !defined(ICE_BOX_API_EXPORTS)
#      if defined(_DEBUG)
#          pragma comment(lib, "IceBoxD.lib")
#      else
#          pragma comment(lib, "IceBox.lib")
#      endif
#   endif
#endif

#endif
