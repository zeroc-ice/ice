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

#ifndef ICE_PHP_ICE_PROXY_H
#define ICE_PHP_ICE_PROXY_H

#include "ice_common.h"
#include <Slice/Parser.h>

bool Ice_ObjectPrx_init(TSRMLS_D);
bool Ice_ObjectPrx_create(zval*, const Ice::ObjectPrx& TSRMLS_DC);
bool Ice_ObjectPrx_create(zval*, const Ice::ObjectPrx&, const Slice::ClassDefPtr& TSRMLS_DC);
bool Ice_ObjectPrx_fetch(zval*, Ice::ObjectPrx&, Slice::ClassDefPtr& TSRMLS_DC);

//
// Class entry.
//
extern zend_class_entry* Ice_ObjectPrx_entry_ptr;

//
// Ice_ObjectPrx class methods.
//
extern "C"
{
ZEND_FUNCTION(Ice_ObjectPrx___construct);
ZEND_FUNCTION(Ice_ObjectPrx_ice_isA);
ZEND_FUNCTION(Ice_ObjectPrx_ice_ping);
ZEND_FUNCTION(Ice_ObjectPrx_ice_id);
ZEND_FUNCTION(Ice_ObjectPrx_ice_ids);
ZEND_FUNCTION(Ice_ObjectPrx_ice_facets);
ZEND_FUNCTION(Ice_ObjectPrx_ice_getIdentity);
ZEND_FUNCTION(Ice_ObjectPrx_ice_newIdentity);
ZEND_FUNCTION(Ice_ObjectPrx_ice_getFacet);
ZEND_FUNCTION(Ice_ObjectPrx_ice_newFacet);
ZEND_FUNCTION(Ice_ObjectPrx_ice_appendFacet);
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
//ZEND_FUNCTION(Ice_ObjectPrx_ice_router);
//ZEND_FUNCTION(Ice_ObjectPrx_ice_locator);
//ZEND_FUNCTION(Ice_ObjectPrx_ice_collocationOptimization);
ZEND_FUNCTION(Ice_ObjectPrx_ice_default);
ZEND_FUNCTION(Ice_ObjectPrx_ice_flush);
ZEND_FUNCTION(Ice_ObjectPrx_ice_uncheckedCast);
ZEND_FUNCTION(Ice_ObjectPrx_ice_checkedCast);
}

#define ICE_PHP_PROXY_FUNCTIONS \
    ZEND_FE(Ice_ObjectPrx___construct,           NULL) \
    ZEND_FE(Ice_ObjectPrx_ice_isA,               NULL) \
    ZEND_FE(Ice_ObjectPrx_ice_ping,              NULL) \
    ZEND_FE(Ice_ObjectPrx_ice_id,                NULL) \
    ZEND_FE(Ice_ObjectPrx_ice_ids,               NULL) \
    ZEND_FE(Ice_ObjectPrx_ice_facets,            NULL) \
    ZEND_FE(Ice_ObjectPrx_ice_getIdentity,       NULL) \
    ZEND_FE(Ice_ObjectPrx_ice_newIdentity,       NULL) \
    ZEND_FE(Ice_ObjectPrx_ice_getFacet,          NULL) \
    ZEND_FE(Ice_ObjectPrx_ice_newFacet,          NULL) \
    ZEND_FE(Ice_ObjectPrx_ice_appendFacet,       NULL) \
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
    ZEND_FE(Ice_ObjectPrx_ice_default,           NULL) \
    ZEND_FE(Ice_ObjectPrx_ice_flush,             NULL) \
    ZEND_FE(Ice_ObjectPrx_ice_uncheckedCast,     NULL) \
    ZEND_FE(Ice_ObjectPrx_ice_checkedCast,       NULL)

#endif
