// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
ZEND_FUNCTION(Ice_Communicator_getProperty);
ZEND_FUNCTION(Ice_Communicator_stringToProxy);
ZEND_FUNCTION(Ice_Communicator_proxyToString);
ZEND_FUNCTION(Ice_Communicator_addObjectFactory);
ZEND_FUNCTION(Ice_Communicator_removeObjectFactory);
ZEND_FUNCTION(Ice_Communicator_findObjectFactory);
ZEND_FUNCTION(Ice_Communicator_setDefaultContext);
ZEND_FUNCTION(Ice_Communicator_getDefaultContext);
ZEND_FUNCTION(Ice_Communicator_flushBatchRequests);
}

#define ICE_PHP_COMMUNICATOR_FUNCTIONS \
    ZEND_FE(Ice_Communicator___construct,            NULL) \
    ZEND_FE(Ice_Communicator_getProperty,            NULL) \
    ZEND_FE(Ice_Communicator_stringToProxy,          NULL) \
    ZEND_FE(Ice_Communicator_proxyToString,          NULL) \
    ZEND_FE(Ice_Communicator_addObjectFactory,       NULL) \
    ZEND_FE(Ice_Communicator_removeObjectFactory,    NULL) \
    ZEND_FE(Ice_Communicator_findObjectFactory,      NULL) \
    ZEND_FE(Ice_Communicator_setDefaultContext,      NULL) \
    ZEND_FE(Ice_Communicator_getDefaultContext,      NULL) \
    ZEND_FE(Ice_Communicator_flushBatchRequests,     NULL)

namespace IcePHP
{

bool communicatorInit(TSRMLS_D);

bool communicatorRegisterGlobal(TSRMLS_D);

Ice::CommunicatorPtr getCommunicator(TSRMLS_D);
zval* getCommunicatorZval(TSRMLS_D);

} // End of namespace IcePHP

#endif
