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

#ifndef PHP_ICE_H
#define PHP_ICE_H

/*
 * This file may be included by PHP's build system, therefore we have
 * to guard the use of 'extern "C"'.
 */
#ifdef __cplusplus
extern "C"
{
#endif

extern zend_module_entry ice_module_entry;
#define phpext_ice_ptr &ice_module_entry

#ifdef PHP_WIN32
#define PHP_ICE_API __declspec(dllexport)
#else
#define PHP_ICE_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

ZEND_MINIT_FUNCTION(ice);
ZEND_MSHUTDOWN_FUNCTION(ice);
ZEND_RINIT_FUNCTION(ice);
ZEND_RSHUTDOWN_FUNCTION(ice);
ZEND_MINFO_FUNCTION(ice);

ZEND_BEGIN_MODULE_GLOBALS(ice)
    zval* communicator;
    void* typeMap;
    void* objectMap;
    void* patchList;
ZEND_END_MODULE_GLOBALS(ice)

#ifdef ZTS
#define ICE_G(v) TSRMG(ice_globals_id, zend_ice_globals*, v)
#else
#define ICE_G(v) (ice_globals.v)
#endif

#ifdef __cplusplus
}
#endif

#endif	/* PHP_ICE_H */
