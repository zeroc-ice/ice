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

#include "communicator.h"
#include "objectprx.h"
#include "exception.h"
#include "util.h"

using namespace std;

//
// The global communicator.
//
static Ice::CommunicatorPtr _communicator;

//
// Class entries represent the PHP class implementations we have registered.
//
static zend_class_entry* Ice_Communicator_entry_ptr;

//
// Ice::Communicator support.
//
static zend_object_handlers Ice_Communicator_handlers;
static zend_object_value Ice_Communicator_alloc(zend_class_entry* TSRMLS_DC);
static void Ice_Communicator_dtor(void*, zend_object_handle TSRMLS_DC);

//
// Function entries for Ice::Communicator methods.
//
static function_entry Ice_Communicator_methods[] =
{
    {"__construct",   PHP_FN(Ice_Communicator___construct),   NULL},
    {"stringToProxy", PHP_FN(Ice_Communicator_stringToProxy), NULL},
    {"proxyToString", PHP_FN(Ice_Communicator_proxyToString), NULL},
    {NULL, NULL, NULL}
};

bool
Ice_Communicator_init(TSRMLS_DC)
{
    //
    // Register the Ice_Communicator class.
    //
    zend_class_entry ce_Communicator;
    INIT_CLASS_ENTRY(ce_Communicator, "Ice_Communicator", Ice_Communicator_methods);
    ce_Communicator.create_object = Ice_Communicator_alloc;
    Ice_Communicator_entry_ptr = zend_register_internal_class(&ce_Communicator TSRMLS_CC);
    memcpy(&Ice_Communicator_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

    //
    // Initialize the global communicator.
    //
    try
    {
        Ice::PropertiesPtr props = Ice::createProperties();

        char* config = INI_STR("ice.config");
        if(config && strlen(config) > 0)
        {
            props->load(config);
        }

        int argc = 0;
        char** argv;
        _communicator = Ice::initializeWithProperties(argc, argv, props);
    }
    catch(const IceUtil::Exception& ex)
    {
        ostringstream ostr;
        ex.ice_print(ostr);
        php_error(E_ERROR, "unable to create Ice::Communicator:\n%s", ostr.str().c_str());
        return false;
    }

    return true;
}

bool
Ice_Communicator_create(zval* zv TSRMLS_DC)
{
    assert(_communicator);

    if(object_init_ex(zv, Ice_Communicator_entry_ptr) != SUCCESS)
    {
        php_error(E_ERROR, "unable to initialize global variable in %s()", get_active_function_name(TSRMLS_C));
        return false;
    }

    ice_object* obj = ice_object_get(zv TSRMLS_CC);
    if(!obj)
    {
        return false;
    }
    assert(!obj->ptr);
    Ice::CommunicatorPtr* communicator = new Ice::CommunicatorPtr(_communicator);
    obj->ptr = communicator;

    return true;
}

Ice::CommunicatorPtr
Ice_Communicator_instance()
{
    return _communicator;
}

bool
Ice_Communicator_shutdown(TSRMLS_DC)
{
    if(_communicator)
    {
        try
        {
            _communicator->destroy();
            _communicator = 0;
        }
        catch(const IceUtil::Exception& ex)
        {
            ostringstream ostr;
            ex.ice_print(ostr);
            php_error(E_ERROR, "unable to destroy Ice::Communicator:\n%s", ostr.str().c_str());
            return false;
        }
    }

    return true;
}

ZEND_FUNCTION(Ice_Communicator___construct)
{
    php_error(E_ERROR, "Ice_Communicator cannot be instantiated, use the global variable $ICE");
}

ZEND_FUNCTION(Ice_Communicator_stringToProxy)
{
    ice_object* obj = ice_object_get(getThis() TSRMLS_CC);
    if(!obj)
    {
        return;
    }
    assert(!obj->ptr);
    Ice::CommunicatorPtr* _this = static_cast<Ice::CommunicatorPtr*>(obj->ptr);

    if(ZEND_NUM_ARGS() != 1)
    {
        WRONG_PARAM_COUNT;
    }

    char *str;
    int len;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &str, &len) == FAILURE)
    {
        RETURN_NULL();
    }

    Ice::ObjectPrx proxy;
    try
    {
        proxy = (*_this)->stringToProxy(str);
    }
    catch(const IceUtil::Exception& ex)
    {
        ice_throw_exception(ex TSRMLS_CC);
        RETURN_NULL();
    }

    if(!Ice_ObjectPrx_create(return_value, proxy TSRMLS_CC))
    {
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_Communicator_proxyToString)
{
    ice_object* obj = ice_object_get(getThis() TSRMLS_CC);
    if(!obj)
    {
        return;
    }
    assert(!obj->ptr);
    Ice::CommunicatorPtr* _this = static_cast<Ice::CommunicatorPtr*>(obj->ptr);

    if(ZEND_NUM_ARGS() != 1)
    {
        WRONG_PARAM_COUNT;
    }

    zval* zprx;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O!", &zprx, Ice_ObjectPrx_entry_ptr) == FAILURE)
    {
        RETURN_EMPTY_STRING();
    }

    Ice::ObjectPrx proxy;
    if(!Ice_ObjectPrx_fetch(zprx, proxy TSRMLS_CC))
    {
        RETURN_EMPTY_STRING();
    }

    try
    {
        string result = (*_this)->proxyToString(proxy);
        RETURN_STRING(const_cast<char*>(result.c_str()), 1);
    }
    catch(const IceUtil::Exception& ex)
    {
        ice_throw_exception(ex TSRMLS_CC);
        RETURN_EMPTY_STRING();
    }
}

static zend_object_value
Ice_Communicator_alloc(zend_class_entry* ce TSRMLS_DC)
{
    zend_object_value result;

    ice_object* obj = ice_object_new(ce TSRMLS_CC);
    assert(obj);

    result.handle = zend_objects_store_put(obj, Ice_Communicator_dtor, NULL TSRMLS_CC);
    result.handlers = &Ice_Communicator_handlers;

    return result;
}

static void
Ice_Communicator_dtor(void* p, zend_object_handle handle TSRMLS_DC)
{
    ice_object* obj = static_cast<ice_object*>(p);
    if(obj->ptr)
    {
        //
        // Delete the smart pointer - the communicator will be destroyed when
        // the module shuts down.
        //
        Ice::CommunicatorPtr* _this = static_cast<Ice::CommunicatorPtr*>(obj->ptr);
        delete _this;
    }

    zend_objects_destroy_object(static_cast<zend_object*>(p), handle TSRMLS_CC);
}
