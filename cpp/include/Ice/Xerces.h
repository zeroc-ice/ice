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

#ifndef ICE_XERCES_H
#define ICE_XERCES_H

#include <xercesc/util/XercesDefs.hpp>

#if _XERCES_VERSION >= 20200
#   define ICE_XERCES_NS XERCES_CPP_NAMESPACE_QUALIFIER
#else
#   define ICE_XERCES_NS
#endif

#endif
