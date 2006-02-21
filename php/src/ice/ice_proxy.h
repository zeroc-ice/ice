// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_PHP_ICE_PROXY_H
#define ICE_PHP_ICE_PROXY_H

#include "ice_common.h"

//
// Ice_ObjectPrx class methods.
//
extern "C"
{
ZEND_FUNCTION(Ice_ObjectPrx___construct);
ZEND_FUNCTION(Ice_ObjectPrx___tostring);
ZEND_FUNCTION(Ice_ObjectPrx_ice_communicator);
ZEND_FUNCTION(Ice_ObjectPrx_ice_toString);
ZEND_FUNCTION(Ice_ObjectPrx_ice_isA);
ZEND_FUNCTION(Ice_ObjectPrx_ice_ping);
ZEND_FUNCTION(Ice_ObjectPrx_ice_id);
ZEND_FUNCTION(Ice_ObjectPrx_ice_ids);
ZEND_FUNCTION(Ice_ObjectPrx_ice_getIdentity);
ZEND_FUNCTION(Ice_ObjectPrx_ice_newIdentity);
ZEND_FUNCTION(Ice_ObjectPrx_ice_getContext);
ZEND_FUNCTION(Ice_ObjectPrx_ice_newContext);
ZEND_FUNCTION(Ice_ObjectPrx_ice_defaultContext);
ZEND_FUNCTION(Ice_ObjectPrx_ice_getFacet);
ZEND_FUNCTION(Ice_ObjectPrx_ice_newFacet);
ZEND_FUNCTION(Ice_ObjectPrx_ice_getAdapterId);
ZEND_FUNCTION(Ice_ObjectPrx_ice_newAdapterId);
ZEND_FUNCTION(Ice_ObjectPrx_ice_getEndpoints);
ZEND_FUNCTION(Ice_ObjectPrx_ice_newEndpoints);
ZEND_FUNCTION(Ice_ObjectPrx_ice_getLocatorCacheTimeout);
ZEND_FUNCTION(Ice_ObjectPrx_ice_locatorCacheTimeout);
ZEND_FUNCTION(Ice_ObjectPrx_ice_getCacheConnection);
ZEND_FUNCTION(Ice_ObjectPrx_ice_cacheConnection);
ZEND_FUNCTION(Ice_ObjectPrx_ice_getEndpointSelection);
ZEND_FUNCTION(Ice_ObjectPrx_ice_endpointSelection);
ZEND_FUNCTION(Ice_ObjectPrx_ice_twoway);
ZEND_FUNCTION(Ice_ObjectPrx_ice_isTwoway);
ZEND_FUNCTION(Ice_ObjectPrx_ice_oneway);
ZEND_FUNCTION(Ice_ObjectPrx_ice_isOneway);
ZEND_FUNCTION(Ice_ObjectPrx_ice_batchOneway);
ZEND_FUNCTION(Ice_ObjectPrx_ice_isBatchOneway);
ZEND_FUNCTION(Ice_ObjectPrx_ice_datagram);
ZEND_FUNCTION(Ice_ObjectPrx_ice_isDatagram);
ZEND_FUNCTION(Ice_ObjectPrx_ice_batchDatagram);
ZEND_FUNCTION(Ice_ObjectPrx_ice_isBatchDatagram);
ZEND_FUNCTION(Ice_ObjectPrx_ice_secure);
ZEND_FUNCTION(Ice_ObjectPrx_ice_compress);
ZEND_FUNCTION(Ice_ObjectPrx_ice_timeout);
ZEND_FUNCTION(Ice_ObjectPrx_ice_connectionId);
ZEND_FUNCTION(Ice_ObjectPrx_ice_connection);
ZEND_FUNCTION(Ice_ObjectPrx_ice_uncheckedCast);
ZEND_FUNCTION(Ice_ObjectPrx_ice_checkedCast);

ZEND_FUNCTION(Ice_Endpoint___construct);
ZEND_FUNCTION(Ice_Endpoint___tostring);
ZEND_FUNCTION(Ice_Endpoint_toString);

ZEND_FUNCTION(Ice_Connection___construct);
ZEND_FUNCTION(Ice_Connection___tostring);
ZEND_FUNCTION(Ice_Connection_close);
ZEND_FUNCTION(Ice_Connection_flushBatchRequests);
ZEND_FUNCTION(Ice_Connection_type);
ZEND_FUNCTION(Ice_Connection_timeout);
ZEND_FUNCTION(Ice_Connection_toString);
}

#define ICE_PHP_PROXY_FUNCTIONS \
    ZEND_FE(Ice_ObjectPrx___construct,           NULL) \
    ZEND_FE(Ice_ObjectPrx___tostring,            NULL) \
    ZEND_FE(Ice_ObjectPrx_ice_communicator,      NULL) \
    ZEND_FE(Ice_ObjectPrx_ice_toString,          NULL) \
    ZEND_FE(Ice_ObjectPrx_ice_isA,               NULL) \
    ZEND_FE(Ice_ObjectPrx_ice_ping,              NULL) \
    ZEND_FE(Ice_ObjectPrx_ice_id,                NULL) \
    ZEND_FE(Ice_ObjectPrx_ice_ids,               NULL) \
    ZEND_FE(Ice_ObjectPrx_ice_getIdentity,       NULL) \
    ZEND_FE(Ice_ObjectPrx_ice_newIdentity,       NULL) \
    ZEND_FE(Ice_ObjectPrx_ice_getAdapterId,      NULL) \
    ZEND_FE(Ice_ObjectPrx_ice_newAdapterId,      NULL) \
    ZEND_FE(Ice_ObjectPrx_ice_getEndpoints,      NULL) \
    ZEND_FE(Ice_ObjectPrx_ice_newEndpoints,      NULL) \
    ZEND_FE(Ice_ObjectPrx_ice_getContext,        NULL) \
    ZEND_FE(Ice_ObjectPrx_ice_newContext,        NULL) \
    ZEND_FE(Ice_ObjectPrx_ice_defaultContext,    NULL) \
    ZEND_FE(Ice_ObjectPrx_ice_getFacet,          NULL) \
    ZEND_FE(Ice_ObjectPrx_ice_newFacet,          NULL) \
    ZEND_FE(Ice_ObjectPrx_ice_twoway,            NULL) \
    ZEND_FE(Ice_ObjectPrx_ice_isTwoway,          NULL) \
    ZEND_FE(Ice_ObjectPrx_ice_oneway,            NULL) \
    ZEND_FE(Ice_ObjectPrx_ice_isOneway,          NULL) \
    ZEND_FE(Ice_ObjectPrx_ice_batchOneway,       NULL) \
    ZEND_FE(Ice_ObjectPrx_ice_isBatchOneway,     NULL) \
    ZEND_FE(Ice_ObjectPrx_ice_datagram,          NULL) \
    ZEND_FE(Ice_ObjectPrx_ice_isDatagram,        NULL) \
    ZEND_FE(Ice_ObjectPrx_ice_batchDatagram,     NULL) \
    ZEND_FE(Ice_ObjectPrx_ice_isBatchDatagram,   NULL) \
    ZEND_FE(Ice_ObjectPrx_ice_secure,            NULL) \
    ZEND_FE(Ice_ObjectPrx_ice_compress,          NULL) \
    ZEND_FE(Ice_ObjectPrx_ice_timeout,           NULL) \
    ZEND_FE(Ice_ObjectPrx_ice_connectionId,      NULL) \
    ZEND_FE(Ice_ObjectPrx_ice_connection,        NULL) \
    ZEND_FE(Ice_ObjectPrx_ice_uncheckedCast,     NULL) \
    ZEND_FE(Ice_ObjectPrx_ice_checkedCast,       NULL)

#define ICE_PHP_ENDPOINT_FUNCTIONS \
    ZEND_FE(Ice_Endpoint___construct,           NULL) \
    ZEND_FE(Ice_Endpoint___tostring,            NULL) \
    ZEND_FE(Ice_Endpoint_toString,              NULL)

#define ICE_PHP_CONNECTION_FUNCTIONS \
    ZEND_FE(Ice_Connection___construct,        NULL) \
    ZEND_FE(Ice_Connection___tostring,         NULL) \
    ZEND_FE(Ice_Connection_close,              NULL) \
    ZEND_FE(Ice_Connection_flushBatchRequests, NULL) \
    ZEND_FE(Ice_Connection_type,               NULL) \
    ZEND_FE(Ice_Connection_timeout,            NULL) \
    ZEND_FE(Ice_Connection_toString,           NULL)

namespace IcePHP
{

bool proxyInit(TSRMLS_D);

bool createProxy(zval*, const Ice::ObjectPrx& TSRMLS_DC);
bool createProxy(zval*, const Ice::ObjectPrx&, const Slice::ClassDefPtr& TSRMLS_DC);
bool fetchProxy(zval*, Ice::ObjectPrx&, Slice::ClassDefPtr& TSRMLS_DC);

//
// Class entry.
//
extern zend_class_entry* proxyClassEntry;

} // End of namespace IcePHP

#endif
