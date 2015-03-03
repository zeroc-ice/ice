// **********************************************************************
//
// Copyright (c) 2003-2011 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Communicator.h>
#include <Connection.h>
#include <Endpoint.h>
#include <Logger.h>
#include <Operation.h>
#include <Properties.h>
#include <Proxy.h>
#include <Types.h>
#include <Util.h>

using namespace std;
using namespace IcePHP;

ZEND_DECLARE_MODULE_GLOBALS(ice)

//
// Entries for all global functions.
//
function_entry ice_functions[] =
{
    ICEPHP_COMMUNICATOR_FUNCTIONS
    ICEPHP_COMMUNICATOR_NS_FUNCTIONS
    ICEPHP_OPERATION_FUNCTIONS
    ICEPHP_PROPERTIES_FUNCTIONS
    ICEPHP_PROPERTIES_NS_FUNCTIONS
    ICEPHP_TYPE_FUNCTIONS
    ICEPHP_UTIL_FUNCTIONS
    ICEPHP_UTIL_NS_FUNCTIONS
    {0, 0, 0}
};

zend_module_entry ice_module_entry =
{
    STANDARD_MODULE_HEADER,
    "ice",
    ice_functions,
    ZEND_MINIT(ice),
    ZEND_MSHUTDOWN(ice),
    ZEND_RINIT(ice),
    ZEND_RSHUTDOWN(ice),
    ZEND_MINFO(ice),
    NO_VERSION_YET,
    STANDARD_MODULE_PROPERTIES
};

ZEND_GET_MODULE(ice)

//
// Declare initialization file entries.
//
PHP_INI_BEGIN()
  PHP_INI_ENTRY("ice.config", "", PHP_INI_SYSTEM, 0)
  PHP_INI_ENTRY("ice.options", "", PHP_INI_SYSTEM, 0)
  PHP_INI_ENTRY("ice.profiles", "", PHP_INI_SYSTEM, 0)
  PHP_INI_ENTRY("ice.hide_profiles", "1", PHP_INI_SYSTEM, 0)
PHP_INI_END()

extern "C"
int initIceGlobals(zend_ice_globals* g)
{
    g->communicatorMap = 0;
    g->idToClassInfoMap = 0;
    g->nameToClassInfoMap = 0;
    g->proxyInfoMap = 0;
    g->exceptionInfoMap = 0;
    return SUCCESS;
}

ZEND_MINIT_FUNCTION(ice)
{
    REGISTER_INI_ENTRIES();
    ZEND_INIT_MODULE_GLOBALS(ice, initIceGlobals, 0);

    if(!communicatorInit(TSRMLS_C))
    {
        return FAILURE;
    }

    if(!propertiesInit(TSRMLS_C))
    {
        return FAILURE;
    }

    if(!proxyInit(TSRMLS_C))
    {
        return FAILURE;
    }

    if(!typesInit(TSRMLS_C))
    {
        return FAILURE;
    }

    if(!loggerInit(TSRMLS_C))
    {
        return FAILURE;
    }

    if(!endpointInit(TSRMLS_C))
    {
        return FAILURE;
    }

    if(!connectionInit(TSRMLS_C))
    {
        return FAILURE;
    }

    return SUCCESS;
}

ZEND_MSHUTDOWN_FUNCTION(ice)
{
    UNREGISTER_INI_ENTRIES();

    int status = SUCCESS;

    if(!communicatorShutdown(TSRMLS_C))
    {
        status = FAILURE;
    }

    return status;
}

ZEND_RINIT_FUNCTION(ice)
{
    ICE_G(communicatorMap) = 0;

    if(!communicatorRequestInit(TSRMLS_C))
    {
        return FAILURE;
    }

    if(!typesRequestInit(TSRMLS_C))
    {
        return FAILURE;
    }

    return SUCCESS;
}

ZEND_RSHUTDOWN_FUNCTION(ice)
{
    if(!communicatorRequestShutdown(TSRMLS_C))
    {
        return FAILURE;
    }

    if(!typesRequestShutdown(TSRMLS_C))
    {
        return FAILURE;
    }

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
