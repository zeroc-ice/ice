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

#ifndef ICE_PHP_COMMUNICATOR_H
#define ICE_PHP_COMMUNICATOR_H

extern "C"
{
#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
}

#include <Ice/Ice.h>

bool Ice_Communicator_init(TSRMLS_DC);
bool Ice_Communicator_create(zval* TSRMLS_DC);
bool Ice_Communicator_shutdown(TSRMLS_DC);

//
// Ice_Communicator class methods.
//
ZEND_FUNCTION(Ice_Communicator___construct);
ZEND_FUNCTION(Ice_Communicator_stringToProxy);
ZEND_FUNCTION(Ice_Communicator_proxyToString);

#define ICE_PHP_COMMUNICATOR_FUNCTIONS \
    ZEND_FE(Ice_Communicator___construct,   NULL) \
    ZEND_FE(Ice_Communicator_stringToProxy, NULL) \
    ZEND_FE(Ice_Communicator_proxyToString, NULL)

#endif
