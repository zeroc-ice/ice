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

#ifndef ICE_PHP_IDENTITY_H
#define ICE_PHP_IDENTITY_H

#include "common.h"

bool Ice_Identity_init(TSRMLS_D);
bool Ice_Identity_create(zval*, const Ice::Identity& TSRMLS_DC);
bool Ice_Identity_extract(zval*, Ice::Identity& TSRMLS_DC);

//
// Class entry.
//
extern zend_class_entry* Ice_Identity_entry_ptr;

//
// Ice_Identity global functions.
//
extern "C"
{
ZEND_FUNCTION(Ice_stringToIdentity);
ZEND_FUNCTION(Ice_identityToString);
}

#define ICE_PHP_IDENTITY_FUNCTIONS \
    ZEND_FE(Ice_stringToIdentity,   NULL) \
    ZEND_FE(Ice_identityToString,   NULL)

#endif
