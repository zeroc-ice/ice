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

#ifndef ICE_PHP_ICE_COMMUNICATOR_H
#define ICE_PHP_ICE_COMMUNICATOR_H

#include "ice_common.h"

//
// Ice_Communicator class methods.
//
extern "C"
{
ZEND_FUNCTION(Ice_Communicator___construct);
ZEND_FUNCTION(Ice_Communicator_stringToProxy);
ZEND_FUNCTION(Ice_Communicator_proxyToString);
ZEND_FUNCTION(Ice_Communicator_addObjectFactory);
ZEND_FUNCTION(Ice_Communicator_removeObjectFactory);
ZEND_FUNCTION(Ice_Communicator_findObjectFactory);
ZEND_FUNCTION(Ice_Communicator_flushBatchRequests);
}

#define ICE_PHP_COMMUNICATOR_FUNCTIONS \
    ZEND_FE(Ice_Communicator___construct,         NULL) \
    ZEND_FE(Ice_Communicator_stringToProxy,       NULL) \
    ZEND_FE(Ice_Communicator_proxyToString,       NULL) \
    ZEND_FE(Ice_Communicator_addObjectFactory,    NULL) \
    ZEND_FE(Ice_Communicator_removeObjectFactory, NULL) \
    ZEND_FE(Ice_Communicator_findObjectFactory,   NULL) \
    ZEND_FE(Ice_Communicator_flushBatchRequests,  NULL)

namespace IcePHP
{

bool communicatorInit(TSRMLS_D);

bool createCommunicator(TSRMLS_D);
Ice::CommunicatorPtr getCommunicator(TSRMLS_D);
zval* getCommunicatorZval(TSRMLS_D);

} // End of namespace IcePHP

#endif
