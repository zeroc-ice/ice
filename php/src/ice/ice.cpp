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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ice_communicator.h"
#include "ice_identity.h"
#include "ice_proxy.h"
#include "ice_slice.h"
#include "ice_marshal.h"
#include "ice_util.h"

using namespace std;

ZEND_DECLARE_MODULE_GLOBALS(ice)

//
// Entries for all object methods and global functions.
//
function_entry ice_functions[] =
{
    ICE_PHP_COMMUNICATOR_FUNCTIONS
    ICE_PHP_IDENTITY_FUNCTIONS
    ICE_PHP_PROXY_FUNCTIONS
    ICE_PHP_SLICE_FUNCTIONS
    {NULL, NULL, NULL}
};

zend_module_entry ice_module_entry =
{
    STANDARD_MODULE_HEADER,
    "Ice",
    ice_functions,
    ZEND_MINIT(ice),
    ZEND_MSHUTDOWN(ice),
    ZEND_RINIT(ice),
    ZEND_RSHUTDOWN(ice),
    ZEND_MINFO(ice),
    NO_VERSION_YET,
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_ICE
ZEND_GET_MODULE(ice)
#endif

//
// Declare initialization file entries.
//
PHP_INI_BEGIN()
  PHP_INI_ENTRY("ice.config", "", PHP_INI_SYSTEM, NULL)
  PHP_INI_ENTRY("ice.parse", "", PHP_INI_SYSTEM, NULL)
PHP_INI_END()

ZEND_MINIT_FUNCTION(ice)
{
    REGISTER_INI_ENTRIES();
    ZEND_INIT_MODULE_GLOBALS(ice, NULL, NULL);

    if(!Ice_Communicator_init(TSRMLS_C))
    {
        return FAILURE;
    }

    if(!Ice_ObjectPrx_init(TSRMLS_C))
    {
        return FAILURE;
    }

    if(!Slice_init(TSRMLS_C))
    {
        return FAILURE;
    }

    return SUCCESS;
}

ZEND_MSHUTDOWN_FUNCTION(ice)
{
    UNREGISTER_INI_ENTRIES();

    int status = SUCCESS;

    if(!Slice_shutdown(TSRMLS_C))
    {
        status = FAILURE;
    }

    return status;
}

ZEND_RINIT_FUNCTION(ice)
{
    ICE_G(communicator) = NULL;
    ICE_G(marshalerMap) = new MarshalerMap;
    ICE_G(coreTypesLoaded) = 0;

    //
    // Create the global variable "ICE" to hold the communicator for this request. The
    // communicator won't actually be created until the script uses this global variable
    // for the first time.
    //
    if(!Ice_Communicator_create(TSRMLS_C))
    {
        return FAILURE;
    }

    return SUCCESS;
}

ZEND_RSHUTDOWN_FUNCTION(ice)
{
    delete static_cast<MarshalerMap*>(ICE_G(marshalerMap));

    return SUCCESS;
}

ZEND_MINFO_FUNCTION(ice)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "Ice support", "enabled");
    php_info_print_table_row(2, "Ice version", ICE_STRING_VERSION);
    php_info_print_table_end();

    DISPLAY_INI_ENTRIES();
}
