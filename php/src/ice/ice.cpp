// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ice_communicator.h"
#include "ice_marshal.h"
#include "ice_profile.h"
#include "ice_proxy.h"
#include "ice_util.h"

using namespace std;
using namespace IcePHP;

ZEND_DECLARE_MODULE_GLOBALS(ice)

//
// Entries for all object methods and global functions.
//
function_entry ice_functions[] =
{
    ICE_PHP_COMMUNICATOR_FUNCTIONS
    ICE_PHP_IDENTITY_FUNCTIONS
    ICE_PHP_PROXY_FUNCTIONS
    ICE_PHP_PROFILE_FUNCTIONS
    {NULL, NULL, NULL}
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

#ifdef COMPILE_DL_ICE
ZEND_GET_MODULE(ice)
#endif

//
// Declare initialization file entries.
//
PHP_INI_BEGIN()
  PHP_INI_ENTRY("ice.config", "", PHP_INI_SYSTEM, NULL)
  PHP_INI_ENTRY("ice.options", "", PHP_INI_SYSTEM, NULL)
  PHP_INI_ENTRY("ice.profiles", "", PHP_INI_SYSTEM, NULL)
  PHP_INI_ENTRY("ice.slice", "", PHP_INI_SYSTEM, NULL)
PHP_INI_END()

extern "C"
int initIceGlobals(zend_ice_globals* g)
{
    g->communicator = NULL;
    g->marshalerMap = 0;
    g->profile = 0;
    g->properties = 0;
    return SUCCESS;
}

ZEND_MINIT_FUNCTION(ice)
{
    REGISTER_INI_ENTRIES();
    ZEND_INIT_MODULE_GLOBALS(ice, initIceGlobals, NULL);

    if(!profileInit(TSRMLS_C))
    {
        return FAILURE;
    }

    if(!communicatorInit(TSRMLS_C))
    {
        return FAILURE;
    }

    if(!proxyInit(TSRMLS_C))
    {
        return FAILURE;
    }

    return SUCCESS;
}

ZEND_MSHUTDOWN_FUNCTION(ice)
{
    UNREGISTER_INI_ENTRIES();

    int status = SUCCESS;

    if(!profileShutdown(TSRMLS_C))
    {
        status = FAILURE;
    }

    return status;
}

ZEND_RINIT_FUNCTION(ice)
{
    ICE_G(communicator) = NULL;
    ICE_G(marshalerMap) = new MarshalerMap;
    ICE_G(profile) = 0;
    ICE_G(properties) = 0;
    ICE_G(objectFactoryMap) = new ObjectFactoryMap;

    //
    // Create the global variable "ICE" to hold the communicator for this request. The
    // communicator won't actually be created until the script uses this global variable
    // for the first time.
    //
    if(!createCommunicator(TSRMLS_C))
    {
        return FAILURE;
    }

    return SUCCESS;
}

ZEND_RSHUTDOWN_FUNCTION(ice)
{
    //
    // Invoke destroy() on each registered factory.
    //
    ObjectFactoryMap* ofm = static_cast<ObjectFactoryMap*>(ICE_G(objectFactoryMap));
    for(ObjectFactoryMap::iterator p = ofm->begin(); p != ofm->end(); ++p)
    {
        zval* factory = p->second;
        zend_call_method_with_0_params(&p->second, NULL, NULL, "destroy", NULL);
        zval_ptr_dtor(&p->second);
    }
    delete ofm;

    delete static_cast<MarshalerMap*>(ICE_G(marshalerMap));
    delete static_cast<Ice::PropertiesPtr*>(ICE_G(properties));

    return SUCCESS;
}

ZEND_MINFO_FUNCTION(ice)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "Ice support", "enabled");
    php_info_print_table_row(2, "Ice version", ICE_STRING_VERSION);
    php_info_print_table_row(2, "IcePHP version", ICEPHP_STRING_VERSION);
    php_info_print_table_end();

    DISPLAY_INI_ENTRIES();
}
