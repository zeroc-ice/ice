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

#include "common.h"

bool Ice_Communicator_init(TSRMLS_D);
bool Ice_Communicator_create(TSRMLS_D);
Ice::CommunicatorPtr Ice_Communicator_getInstance(TSRMLS_D);
zval* Ice_Communicator_getZval(TSRMLS_D);

//
// Ice_Communicator class methods.
//
extern "C"
{
ZEND_FUNCTION(Ice_Communicator___construct);
ZEND_FUNCTION(Ice_Communicator_stringToProxy);
ZEND_FUNCTION(Ice_Communicator_proxyToString);
}

#define ICE_PHP_COMMUNICATOR_FUNCTIONS \
    ZEND_FE(Ice_Communicator___construct,   NULL) \
    ZEND_FE(Ice_Communicator_stringToProxy, NULL) \
    ZEND_FE(Ice_Communicator_proxyToString, NULL)

#endif
