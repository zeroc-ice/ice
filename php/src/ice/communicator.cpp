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
#include "proxy.h"
#include "exception.h"
#include "util.h"
#include "php_ice.h"

using namespace std;

ZEND_EXTERN_MODULE_GLOBALS(ice)

//
// Class entries represent the PHP class implementations we have registered.
//
static zend_class_entry* Ice_Communicator_entry_ptr;

//
// Ice::Communicator support.
//
static zend_object_handlers Ice_Communicator_handlers;
static zend_object_value handleAlloc(zend_class_entry* TSRMLS_DC);
static void handleDestroy(void*, zend_object_handle TSRMLS_DC);
static union _zend_function* handleGetMethod(zval*, char*, int TSRMLS_DC);
static void initCommunicator(ice_object* TSRMLS_DC);

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
Ice_Communicator_init(TSRMLS_D)
{
    //
    // Register the Ice_Communicator class.
    //
    zend_class_entry ce_Communicator;
    INIT_CLASS_ENTRY(ce_Communicator, "Ice_Communicator", Ice_Communicator_methods);
    ce_Communicator.create_object = handleAlloc;
    Ice_Communicator_entry_ptr = zend_register_internal_class(&ce_Communicator TSRMLS_CC);
    memcpy(&Ice_Communicator_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    Ice_Communicator_handlers.get_method = handleGetMethod;

    return true;
}

bool
Ice_Communicator_create(TSRMLS_DC)
{
    zval* global;
    MAKE_STD_ZVAL(global);

    //
    // Create the global variable for the communicator, but delay creation of the communicator
    // itself until it is first used (see handleGetMethod).
    //

    if(object_init_ex(global, Ice_Communicator_entry_ptr) != SUCCESS)
    {
        zend_error(E_ERROR, "unable to create object for communicator");
        return false;
    }

    //
    // Register the global variable "ICE" to hold the communicator.
    //
    ICE_G(z_communicator) = global;
    ZEND_SET_GLOBAL_VAR("ICE", global);

    return true;
}

Ice::CommunicatorPtr
Ice_Communicator_instance(TSRMLS_D)
{
    Ice::CommunicatorPtr result;

    zval **zv;
    if(zend_hash_find(&EG(symbol_table), "ICE", sizeof("ICE"), (void **) &zv) == SUCCESS)
    {
        ice_object* obj = ice_getObject(*zv TSRMLS_CC);
        assert(obj);

        //
        // Initialize the communicator if necessary.
        //
        if(!obj->ptr)
        {
            try
            {
                initCommunicator(obj);
            }
            catch(const IceUtil::Exception& ex)
            {
                ostringstream ostr;
                ex.ice_print(ostr);
                zend_error(E_ERROR, "unable to initialize communicator:\n%s", ostr.str().c_str());
                return 0;
            }
        }

        Ice::CommunicatorPtr* _this = static_cast<Ice::CommunicatorPtr*>(obj->ptr);
        result = *_this;
    }

    return result;
}

ZEND_FUNCTION(Ice_Communicator___construct)
{
    zend_error(E_ERROR, "Ice_Communicator cannot be instantiated, use the global variable $ICE");
}

ZEND_FUNCTION(Ice_Communicator_stringToProxy)
{
    ice_object* obj = ice_getObject(getThis() TSRMLS_CC);
    if(!obj)
    {
        return;
    }
    assert(obj->ptr);
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
    ice_object* obj = ice_getObject(getThis() TSRMLS_CC);
    if(!obj)
    {
        return;
    }
    assert(obj->ptr);
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
handleAlloc(zend_class_entry* ce TSRMLS_DC)
{
    zend_object_value result;

    ice_object* obj = ice_newObject(ce TSRMLS_CC);
    assert(obj);

    result.handle = zend_objects_store_put(obj, handleDestroy, NULL TSRMLS_CC);
    result.handlers = &Ice_Communicator_handlers;

    return result;
}

static void
handleDestroy(void* p, zend_object_handle handle TSRMLS_DC)
{
    ice_object* obj = static_cast<ice_object*>(p);
    if(obj->ptr)
    {
        Ice::CommunicatorPtr* _this = static_cast<Ice::CommunicatorPtr*>(obj->ptr);
        try
        {
            (*_this)->destroy();
        }
        catch(const IceUtil::Exception& ex)
        {
            ostringstream ostr;
            ex.ice_print(ostr);
            zend_error(E_ERROR, "unable to destroy Ice::Communicator:\n%s", ostr.str().c_str());
        }
        delete _this;
    }

    zend_objects_destroy_object(static_cast<zend_object*>(p), handle TSRMLS_CC);
}

static union _zend_function*
handleGetMethod(zval* zv, char* method, int len TSRMLS_DC)
{
    //
    // Delegate to the standard implementation of get_method. We're simply using this hook
    // as a convenient way of implementing lazy initialization of the communicator.
    //
    zend_function* result = zend_get_std_object_handlers()->get_method(zv, method, len TSRMLS_CC);
    if(result)
    {
        ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(zv TSRMLS_CC));
        if(!obj->ptr)
        {
            try
            {
                initCommunicator(obj);
            }
            catch(const IceUtil::Exception& ex)
            {
                ice_throw_exception(ex TSRMLS_CC);
            }
        }
    }

    return result;
}

//
// Initialize a communicator instance and store it in the given object. Can raise exceptions.
//
static void
initCommunicator(ice_object* obj TSRMLS_DC)
{
    assert(!obj->ptr);

    Ice::PropertiesPtr props = Ice::createProperties();

    char* config = INI_STR("ice.config");
    if(config && strlen(config) > 0)
    {
        props->load(config);
    }

    int argc = 0;
    char** argv;
    Ice::CommunicatorPtr communicator = Ice::initializeWithProperties(argc, argv, props);
    obj->ptr = new Ice::CommunicatorPtr(communicator);
}
