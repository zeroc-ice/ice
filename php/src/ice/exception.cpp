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

#include "exception.h"
#include "util.h"

using namespace std;

//
// Class entries represent the PHP class implementations we have registered.
//
static zend_class_entry* Ice_LocalException_entry_ptr;

//
// Ice::LocalException support.
//
static zend_object_handlers Ice_LocalException_handlers;

extern "C"
{
static zend_object_value handleAlloc(zend_class_entry* TSRMLS_DC);
static void handleDestroy(void*, zend_object_handle TSRMLS_DC);
}

//
// Function entries for Ice::LocalException methods.
//
static function_entry Ice_LocalException_methods[] =
{
    {"__construct", PHP_FN(Ice_LocalException___construct), NULL},
    {"message",     PHP_FN(Ice_LocalException_message),     NULL},
    {NULL, NULL, NULL}
};

bool
Ice_LocalException_init(TSRMLS_D)
{
    //
    // Register the Ice_LocalException class.
    //
    zend_class_entry ce_LocalException;
    INIT_CLASS_ENTRY(ce_LocalException, "Ice_LocalException", Ice_LocalException_methods);
    ce_LocalException.create_object = handleAlloc;
    Ice_LocalException_entry_ptr = zend_register_internal_class(&ce_LocalException TSRMLS_CC);
    memcpy(&Ice_LocalException_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    return true;
}

void
ice_throw_exception(const IceUtil::Exception& ex TSRMLS_DC)
{
    try
    {
        ex.ice_throw();
    }
    catch(const Ice::LocalException& e)
    {
        zval* zex;
        MAKE_STD_ZVAL(zex);
        if(object_init_ex(zex, Ice_LocalException_entry_ptr) != SUCCESS)
        {
            zend_error(E_ERROR, "unable to create LocalException in %s()", get_active_function_name(TSRMLS_C));
            return;
        }

        ice_object* obj = ice_getObject(zex TSRMLS_CC);
        if(!obj)
        {
            return;
        }
        assert(!obj->ptr);

        ostringstream ostr;
        e.ice_print(ostr);
        obj->ptr = estrdup(ostr.str().c_str());
        EG(exception) = zex;
    }
    catch(const Ice::UserException& e)
    {
        assert(false);
    }
    catch(const IceUtil::Exception& e)
    {
        ostringstream ostr;
        e.ice_print(ostr);
        zend_error(E_ERROR, "exception: %s", ostr.str().c_str());
    }
}

ZEND_FUNCTION(Ice_LocalException___construct)
{
    if(ZEND_NUM_ARGS() > 1)
    {
        WRONG_PARAM_COUNT;
    }

    char *msg = "";
    int len;

    if(ZEND_NUM_ARGS() > 0)
    {
        if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &msg, &len) == FAILURE)
        {
            RETURN_NULL();
        }
    }

    ice_object* obj = ice_getObject(getThis() TSRMLS_CC);
    if(!obj)
    {
        RETURN_NULL();
    }
    assert(!obj->ptr);
    obj->ptr = estrdup(msg);
}

ZEND_FUNCTION(Ice_LocalException_message)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = ice_getObject(getThis() TSRMLS_CC);
    if(!obj)
    {
        RETURN_EMPTY_STRING();
    }
    assert(obj->ptr);
    RETURN_STRING(static_cast<char*>(obj->ptr), 1);
}

static zend_object_value
handleAlloc(zend_class_entry* ce TSRMLS_DC)
{
    zend_object_value result;

    ice_object* obj = ice_newObject(ce TSRMLS_CC);
    assert(obj);

    result.handle = zend_objects_store_put(obj, handleDestroy, NULL TSRMLS_CC);
    result.handlers = &Ice_LocalException_handlers;

    return result;
}

static void
handleDestroy(void* p, zend_object_handle handle TSRMLS_DC)
{
    ice_object* obj = static_cast<ice_object*>(p);

    if(obj->ptr)
    {
        efree(obj->ptr);
        obj->ptr = 0;
    }

    zend_objects_destroy_object(static_cast<zend_object*>(p), handle TSRMLS_CC);
}
