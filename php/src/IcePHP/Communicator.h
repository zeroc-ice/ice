// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_PHP_COMMUNICATOR_H
#define ICE_PHP_COMMUNICATOR_H

#include <Config.h>

//
// Ice_Communicator class methods.
//
extern "C"
{
ZEND_FUNCTION(Ice_Communicator___construct);
ZEND_FUNCTION(Ice_Communicator_getProperty);
ZEND_FUNCTION(Ice_Communicator_setProperty);
ZEND_FUNCTION(Ice_Communicator_stringToProxy);
ZEND_FUNCTION(Ice_Communicator_proxyToString);
ZEND_FUNCTION(Ice_Communicator_propertyToProxy);
ZEND_FUNCTION(Ice_Communicator_stringToIdentity);
ZEND_FUNCTION(Ice_Communicator_identityToString);
ZEND_FUNCTION(Ice_Communicator_addObjectFactory);
ZEND_FUNCTION(Ice_Communicator_findObjectFactory);
ZEND_FUNCTION(Ice_Communicator_flushBatchRequests);
}

#define ICE_PHP_COMMUNICATOR_FUNCTIONS \
    ZEND_FE(Ice_Communicator___construct,            NULL) \
    ZEND_FE(Ice_Communicator_getProperty,            NULL) \
    ZEND_FE(Ice_Communicator_setProperty,            NULL) \
    ZEND_FE(Ice_Communicator_stringToProxy,          NULL) \
    ZEND_FE(Ice_Communicator_proxyToString,          NULL) \
    ZEND_FE(Ice_Communicator_propertyToProxy,        NULL) \
    ZEND_FE(Ice_Communicator_stringToIdentity,       NULL) \
    ZEND_FE(Ice_Communicator_identityToString,       NULL) \
    ZEND_FE(Ice_Communicator_addObjectFactory,       NULL) \
    ZEND_FE(Ice_Communicator_findObjectFactory,      NULL) \
    ZEND_FE(Ice_Communicator_flushBatchRequests,     NULL)

namespace IcePHP
{

bool communicatorInit(TSRMLS_D);

bool createCommunicator(TSRMLS_D);
Ice::CommunicatorPtr getCommunicator(TSRMLS_D);
zval* getCommunicatorZval(TSRMLS_D);

} // End of namespace IcePHP

#endif
