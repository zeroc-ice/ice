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

#ifndef ICE_PHP_EXCEPTION_H
#define ICE_PHP_EXCEPTION_H

extern "C"
{
#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
}

#include <Ice/Ice.h>

bool Ice_LocalException_init(TSRMLS_DC);

void ice_throw_exception(const IceUtil::Exception& TSRMLS_DC);

//
// Ice_LocalException class methods.
//
ZEND_FUNCTION(Ice_LocalException___construct);
ZEND_FUNCTION(Ice_LocalException_message);

#define ICE_PHP_LOCAL_EXCEPTION_FUNCTIONS \
    ZEND_FE(Ice_LocalException___construct, NULL) \
    ZEND_FE(Ice_LocalException_message,     NULL)

#endif
