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

#ifndef ICE_PHP_ICE_COMMON_H
#define ICE_PHP_ICE_COMMON_H

#if defined(_MSC_VER) && (_MSC_VER >= 1300)
    // Necessary for TryEnterCriticalSection. VC 7.x only.
#   define _WIN32_WINNT 0x0400
#   include <winsock2.h>
#endif

#include <Ice/Ice.h>

#ifdef WIN32
#include <crtdbg.h>
#include <math.h>
#endif

extern "C"
{
#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
}

#include "php_ice.h"

#endif
