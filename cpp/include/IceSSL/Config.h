// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************


#ifndef ICE_SSL_CONFIG_H
#define ICE_SSL_CONFIG_H

#include <IceUtil/Config.h>

#ifndef ICE_SSL_API
#   ifdef ICE_SSL_API_EXPORTS
#       define ICE_SSL_API ICE_DECLSPEC_EXPORT
#   else
#       define ICE_SSL_API ICE_DECLSPEC_IMPORT
#   endif
#endif

#endif
