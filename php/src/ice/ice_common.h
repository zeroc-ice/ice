// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
#include <Slice/Parser.h>

#ifdef WIN32
#include <crtdbg.h>
#include <math.h>
#endif

#ifdef WIN32
extern "C"
{
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "zend_interfaces.h"
#include "zend_exceptions.h"

#ifdef WIN32
}
#endif

#include "php_ice.h"

#endif
