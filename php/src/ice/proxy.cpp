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

#include "proxy.h"
#include "communicator.h"
#include "exception.h"
#include "marshal.h"
#include "slice.h"
#include "util.h"

using namespace std;

//
// Class entries represent the PHP class implementations we have registered.
//
zend_class_entry* Ice_ObjectPrx_entry_ptr;

//
// Encapsulates an operation description.
//
class Operation : public IceUtil::SimpleShared
{
public:
    Operation(const Ice::ObjectPrx&, const Slice::OperationPtr&, const IceInternal::InstancePtr&);
    virtual ~Operation();

    zend_uchar* getArgTypes() const;
    void invoke(INTERNAL_FUNCTION_PARAMETERS);

private:
    Ice::ObjectPrx _proxy;
    Slice::OperationPtr _op;
    IceInternal::InstancePtr _instance;
    string _name;
    vector<string> _paramNames;
    MarshalerPtr _result;
    vector<MarshalerPtr> _inParams;
    vector<MarshalerPtr> _outParams;
    zend_uchar* _argTypes;
};
typedef IceUtil::Handle<Operation> OperationPtr;

//
// Encapsulates proxy and type information.
//
class Proxy
{
public:
    Proxy(const Ice::ObjectPrx&, const Slice::ClassDefPtr& = Slice::ClassDefPtr());

    const Ice::ObjectPrx& getProxy() const;
    const Slice::ClassDefPtr& getClass() const;

    OperationPtr getOperation(const string&);

private:
    Ice::ObjectPrx _proxy;
    Slice::ClassDefPtr _class;
    IceInternal::InstancePtr _instance;
    Slice::OperationList _classOps;
    map<string, OperationPtr> _ops;
};

//
// Ice::ObjectPrx support.
//
static zend_object_handlers Ice_ObjectPrx_handlers;
static zend_object_value Ice_ObjectPrx_alloc(zend_class_entry* TSRMLS_DC);
static void Ice_ObjectPrx_dtor(void*, zend_object_handle TSRMLS_DC);
static union _zend_function* Ice_ObjectPrx_get_method(zval*, char*, int TSRMLS_DC);
ZEND_FUNCTION(Ice_ObjectPrx_call);

//
// Function entries for Ice::ObjectPrx methods.
//
static function_entry Ice_ObjectPrx_methods[] =
{
    {"__construct",         PHP_FN(Ice_ObjectPrx___construct),         NULL},
    {"ice_isA",             PHP_FN(Ice_ObjectPrx_ice_isA),             NULL},
    {"ice_ping",            PHP_FN(Ice_ObjectPrx_ice_ping),            NULL},
    {"ice_id",              PHP_FN(Ice_ObjectPrx_ice_id),              NULL},
    {"ice_ids",             PHP_FN(Ice_ObjectPrx_ice_ids),             NULL},
    {"ice_facets",          PHP_FN(Ice_ObjectPrx_ice_facets),          NULL},
    {"ice_getFacet",        PHP_FN(Ice_ObjectPrx_ice_getFacet),        NULL},
    {"ice_newFacet",        PHP_FN(Ice_ObjectPrx_ice_newFacet),        NULL},
    {"ice_appendFacet",     PHP_FN(Ice_ObjectPrx_ice_appendFacet),     NULL},
    {"ice_twoway",          PHP_FN(Ice_ObjectPrx_ice_twoway),          NULL},
    {"ice_isTwoway",        PHP_FN(Ice_ObjectPrx_ice_isTwoway),        NULL},
    {"ice_oneway",          PHP_FN(Ice_ObjectPrx_ice_oneway),          NULL},
    {"ice_isOneway",        PHP_FN(Ice_ObjectPrx_ice_isOneway),        NULL},
    {"ice_batchOneway",     PHP_FN(Ice_ObjectPrx_ice_batchOneway),     NULL},
    {"ice_isBatchOneway",   PHP_FN(Ice_ObjectPrx_ice_isBatchOneway),   NULL},
    {"ice_datagram",        PHP_FN(Ice_ObjectPrx_ice_datagram),        NULL},
    {"ice_isDatagram",      PHP_FN(Ice_ObjectPrx_ice_isDatagram),      NULL},
    {"ice_batchDatagram",   PHP_FN(Ice_ObjectPrx_ice_batchDatagram),   NULL},
    {"ice_isBatchDatagram", PHP_FN(Ice_ObjectPrx_ice_isBatchDatagram), NULL},
    {"ice_secure",          PHP_FN(Ice_ObjectPrx_ice_secure),          NULL},
    {"ice_compress",        PHP_FN(Ice_ObjectPrx_ice_compress),        NULL},
    {"ice_timeout",         PHP_FN(Ice_ObjectPrx_ice_timeout),         NULL},
    {"ice_default",         PHP_FN(Ice_ObjectPrx_ice_default),         NULL},
    {"ice_flush",           PHP_FN(Ice_ObjectPrx_ice_flush),           NULL},
    {"ice_uncheckedCast",   PHP_FN(Ice_ObjectPrx_ice_uncheckedCast),   NULL},
    {"ice_checkedCast",     PHP_FN(Ice_ObjectPrx_ice_checkedCast),     NULL},
    {NULL, NULL, NULL}
};

bool
Ice_ObjectPrx_init(TSRMLS_DC)
{
    //
    // Register the Ice_ObjectPrx class.
    //
    zend_class_entry ce_ObjectPrx;
    INIT_CLASS_ENTRY(ce_ObjectPrx, "Ice_ObjectPrx", Ice_ObjectPrx_methods);
    ce_ObjectPrx.create_object = Ice_ObjectPrx_alloc;
    Ice_ObjectPrx_entry_ptr = zend_register_internal_class(&ce_ObjectPrx TSRMLS_CC);
    memcpy(&Ice_ObjectPrx_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    Ice_ObjectPrx_handlers.get_method = Ice_ObjectPrx_get_method;

    return true;
}

bool
Ice_ObjectPrx_create(zval* zv, const Ice::ObjectPrx& p TSRMLS_DC)
{
    return Ice_ObjectPrx_create(zv, p, 0 TSRMLS_CC);
}

bool
Ice_ObjectPrx_create(zval* zv, const Ice::ObjectPrx& p, const Slice::ClassDeclPtr& decl TSRMLS_DC)
{
    Slice::ClassDefPtr def;
    if(decl)
    {
        def = decl->definition();
        if(!def)
        {
            string scoped = decl->scoped();
            php_error(E_ERROR, "%s(): no definition for type %s", get_active_function_name(TSRMLS_C), scoped.c_str());
            return false;
        }

        if(decl->isLocal())
        {
            string scoped = decl->scoped();
            php_error(E_ERROR, "%s(): cannot use local type %s", get_active_function_name(TSRMLS_C), scoped.c_str());
            return false;
        }
    }

    if(object_init_ex(zv, Ice_ObjectPrx_entry_ptr) != SUCCESS)
    {
        php_error(E_ERROR, "unable to initialize proxy in %s()", get_active_function_name(TSRMLS_C));
        return false;
    }

    ice_object* zprx = static_cast<ice_object*>(zend_object_store_get_object(zv TSRMLS_CC));
    assert(!zprx->ptr);
    zprx->ptr = new Proxy(p, def);
    return true;
}

bool
Ice_ObjectPrx_fetch(zval* zv, Ice::ObjectPrx& prx TSRMLS_DC)
{
    if(!ZVAL_IS_NULL(zv))
    {
        void* p = zend_object_store_get_object(zv TSRMLS_CC);
        if(!p)
        {
            php_error(E_ERROR, "unable to retrieve proxy object from object store in %s()",
                      get_active_function_name(TSRMLS_C));
            return false;
        }
        zend_object* zo = static_cast<zend_object*>(p);
        if(zo->ce != Ice_ObjectPrx_entry_ptr)
        {
            php_error(E_ERROR, "value is not a proxy in %s()", get_active_function_name(TSRMLS_C));
            return false;
        }
        ice_object* obj = static_cast<ice_object*>(p);
        assert(obj->ptr);
        Proxy* proxy = static_cast<Proxy*>(obj->ptr);
        prx = proxy->getProxy();
    }
    return true;
}

ZEND_FUNCTION(Ice_ObjectPrx___construct)
{
    php_error(E_ERROR, "Ice_ObjectPrx cannot be instantiated, use $ICE->stringToProxy()");
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_isA)
{
    if(ZEND_NUM_ARGS() != 1)
    {
        WRONG_PARAM_COUNT;
    }

    char *id;
    int len;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &id, &len) == FAILURE)
    {
        RETURN_FALSE;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        if(_this->getProxy()->ice_isA(id))
        {
            RETVAL_TRUE;
        }
        else
        {
            RETVAL_FALSE;
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        ice_throw_exception(ex TSRMLS_CC);
        RETVAL_FALSE;
    }
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_ping)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        _this->getProxy()->ice_ping();
    }
    catch(const IceUtil::Exception& ex)
    {
        ice_throw_exception(ex TSRMLS_CC);
    }

    RETURN_NULL();
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_id)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        string id = _this->getProxy()->ice_id();
        RETURN_STRINGL(const_cast<char*>(id.c_str()), id.length(), 1);
    }
    catch(const IceUtil::Exception& ex)
    {
        ice_throw_exception(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_ids)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        vector<string> ids = _this->getProxy()->ice_ids();
        array_init(return_value);
        uint idx = 0;
        for(vector<string>::const_iterator p = ids.begin(); p != ids.end(); ++p, ++idx)
        {
            add_index_stringl(return_value, idx, const_cast<char*>((*p).c_str()), (*p).length(), 1);
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        ice_throw_exception(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_facets)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        Ice::FacetPath facets = _this->getProxy()->ice_facets();
        array_init(return_value);
        uint idx = 0;
        for(vector<string>::const_iterator p = facets.begin(); p != facets.end(); ++p, ++idx)
        {
            add_index_stringl(return_value, idx, const_cast<char*>((*p).c_str()), (*p).length(), 1);
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        ice_throw_exception(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_getFacet)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        Ice::FacetPath facet = _this->getProxy()->ice_getFacet();
        array_init(return_value);
        uint idx = 0;
        for(vector<string>::const_iterator p = facet.begin(); p != facet.end(); ++p, ++idx)
        {
            add_index_stringl(return_value, idx, const_cast<char*>((*p).c_str()), (*p).length(), 1);
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        ice_throw_exception(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_newFacet)
{
    if(ZEND_NUM_ARGS() != 1)
    {
        WRONG_PARAM_COUNT;
    }

    zval* zarr;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a", &zarr) == FAILURE)
    {
        RETURN_NULL();
    }

    Ice::FacetPath facet;
    HashTable* arr = Z_ARRVAL_P(zarr);
    HashPosition pos;
    zval** val;

    zend_hash_internal_pointer_reset_ex(arr, &pos);
    while(zend_hash_get_current_data_ex(arr, (void**)&val, &pos) != FAILURE)
    {
        if(Z_TYPE_PP(val) != IS_STRING)
        {
            php_error(E_ERROR, "facet must be a string array");
            RETURN_NULL();
        }
        facet.push_back(string(Z_STRVAL_PP(val), Z_STRLEN_PP(val)));
        zend_hash_move_forward_ex(arr, &pos);
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {

        Ice::ObjectPrx prx = _this->getProxy()->ice_newFacet(facet);
        if(!Ice_ObjectPrx_create(return_value, prx TSRMLS_CC))
        {
            RETURN_NULL();
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        ice_throw_exception(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_appendFacet)
{
    if(ZEND_NUM_ARGS() != 1)
    {
        WRONG_PARAM_COUNT;
    }

    char *name;
    int len;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &len) == FAILURE)
    {
        RETURN_NULL();
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        Ice::FacetPath facet;

        Ice::ObjectPrx prx = _this->getProxy()->ice_appendFacet(name);
        if(!Ice_ObjectPrx_create(return_value, prx TSRMLS_CC))
        {
            RETURN_NULL();
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        ice_throw_exception(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_twoway)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        Ice::ObjectPrx prx = _this->getProxy()->ice_twoway();
        if(!Ice_ObjectPrx_create(return_value, prx TSRMLS_CC))
        {
            RETURN_NULL();
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        ice_throw_exception(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_isTwoway)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        bool b = _this->getProxy()->ice_isTwoway();
        RETURN_BOOL(b ? 1 : 0);
    }
    catch(const IceUtil::Exception& ex)
    {
        ice_throw_exception(ex TSRMLS_CC);
        RETURN_FALSE;
    }
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_oneway)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        Ice::ObjectPrx prx = _this->getProxy()->ice_oneway();
        if(!Ice_ObjectPrx_create(return_value, prx TSRMLS_CC))
        {
            RETURN_NULL();
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        ice_throw_exception(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_isOneway)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        bool b = _this->getProxy()->ice_isOneway();
        RETURN_BOOL(b ? 1 : 0);
    }
    catch(const IceUtil::Exception& ex)
    {
        ice_throw_exception(ex TSRMLS_CC);
        RETURN_FALSE;
    }
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_batchOneway)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        Ice::ObjectPrx prx = _this->getProxy()->ice_batchOneway();
        if(!Ice_ObjectPrx_create(return_value, prx TSRMLS_CC))
        {
            RETURN_NULL();
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        ice_throw_exception(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_isBatchOneway)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        bool b = _this->getProxy()->ice_isBatchOneway();
        RETURN_BOOL(b ? 1 : 0);
    }
    catch(const IceUtil::Exception& ex)
    {
        ice_throw_exception(ex TSRMLS_CC);
        RETURN_FALSE;
    }
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_datagram)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        Ice::ObjectPrx prx = _this->getProxy()->ice_datagram();
        if(!Ice_ObjectPrx_create(return_value, prx TSRMLS_CC))
        {
            RETURN_NULL();
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        ice_throw_exception(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_isDatagram)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        bool b = _this->getProxy()->ice_isDatagram();
        RETURN_BOOL(b ? 1 : 0);
    }
    catch(const IceUtil::Exception& ex)
    {
        ice_throw_exception(ex TSRMLS_CC);
        RETURN_FALSE;
    }
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_batchDatagram)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        Ice::ObjectPrx prx = _this->getProxy()->ice_batchDatagram();
        if(!Ice_ObjectPrx_create(return_value, prx TSRMLS_CC))
        {
            RETURN_NULL();
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        ice_throw_exception(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_isBatchDatagram)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        bool b = _this->getProxy()->ice_isBatchDatagram();
        RETURN_BOOL(b ? 1 : 0);
    }
    catch(const IceUtil::Exception& ex)
    {
        ice_throw_exception(ex TSRMLS_CC);
        RETURN_FALSE;
    }
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_secure)
{
    if(ZEND_NUM_ARGS() != 1)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    zend_bool b;
    if(zend_parse_parameters(ZEND_NUM_ARGS(), "b", &b) != SUCCESS)
    {
        RETURN_NULL();
    }

    try
    {
        Ice::ObjectPrx prx = _this->getProxy()->ice_secure(b ? true : false);
        if(!Ice_ObjectPrx_create(return_value, prx TSRMLS_CC))
        {
            RETURN_NULL();
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        ice_throw_exception(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_compress)
{
    if(ZEND_NUM_ARGS() != 1)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        zend_bool b;
        if(zend_parse_parameters(ZEND_NUM_ARGS(), "b", &b) != SUCCESS)
        {
            RETURN_NULL();
        }
        Ice::ObjectPrx prx = _this->getProxy()->ice_compress(b ? true : false);
        if(!Ice_ObjectPrx_create(return_value, prx TSRMLS_CC))
        {
            RETURN_NULL();
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        ice_throw_exception(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_timeout)
{
    if(ZEND_NUM_ARGS() != 1)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        long l;
        if(zend_parse_parameters(ZEND_NUM_ARGS(), "l", &l) != SUCCESS)
        {
            RETURN_NULL();
        }
        // TODO: range check?
        Ice::ObjectPrx prx = _this->getProxy()->ice_timeout(l);
        if(!Ice_ObjectPrx_create(return_value, prx TSRMLS_CC))
        {
            RETURN_NULL();
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        ice_throw_exception(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_default)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        Ice::ObjectPrx prx = _this->getProxy()->ice_default();
        if(!Ice_ObjectPrx_create(return_value, prx TSRMLS_CC))
        {
            RETURN_NULL();
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        ice_throw_exception(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_flush)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        _this->getProxy()->ice_flush();
    }
    catch(const IceUtil::Exception& ex)
    {
        ice_throw_exception(ex TSRMLS_CC);
    }
}

static void
do_cast(INTERNAL_FUNCTION_PARAMETERS, bool check)
{
    if(ZEND_NUM_ARGS() != 1)
    {
        WRONG_PARAM_COUNT;
    }

    char *id;
    int len;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &id, &len) == FAILURE)
    {
        RETURN_NULL();
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        Slice::ClassDefPtr def = _this->getClass();
        if(def && def->scoped() == id)
        {
            // TODO: legal?
            *return_value = *(getThis());
            return;
        }
        else
        {
            Slice::UnitPtr unit = Slice_getUnit();
            Slice::TypeList l;

            if(unit)
            {
                l = unit->lookupTypeNoBuiltin(id, false);
            }

            if(l.empty())
            {
                php_error(E_ERROR, "%s(): no Slice definition found for type %s", get_active_function_name(TSRMLS_C),
                          id);
                RETURN_NULL();
            }

            //
            // Allow the use of "::Type" (ClassDecl) or "::Type*" (Proxy).
            //
            Slice::ClassDeclPtr decl;
            Slice::TypePtr type = l.front();
            Slice::ProxyPtr proxy = Slice::ProxyPtr::dynamicCast(type);
            if(proxy)
            {
                decl = proxy->_class();
            }
            else
            {
                decl = Slice::ClassDeclPtr::dynamicCast(type);
            }

            if(!decl)
            {
                php_error(E_ERROR, "%s(): type %s is not a class or interface", get_active_function_name(TSRMLS_C),
                          id);
                RETURN_NULL();
            }

            if(check)
            {
                //
                // Verify that the object supports the requested type. We don't use id here,
                // because it might contain a proxy type (e.g., "::MyClass*").
                //
                if(!_this->getProxy()->ice_isA(decl->scoped()))
                {
                    RETURN_NULL();
                }
            }

            if(!Ice_ObjectPrx_create(return_value, _this->getProxy(), decl TSRMLS_CC))
            {
                RETURN_NULL();
            }
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        ice_throw_exception(ex TSRMLS_CC);
        RETVAL_FALSE;
    }
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_uncheckedCast)
{
    do_cast(INTERNAL_FUNCTION_PARAM_PASSTHRU, false);
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_checkedCast)
{
    do_cast(INTERNAL_FUNCTION_PARAM_PASSTHRU, true);
}

Operation::Operation(const Ice::ObjectPrx& proxy, const Slice::OperationPtr& op,
                     const IceInternal::InstancePtr& instance) :
    _proxy(proxy), _op(op), _instance(instance), _name(op->name())
{
    //
    // Create Marshaler objects for return type and parameters.
    //
    Slice::TypePtr ret = op->returnType();
    if(ret)
    {
        _result = Marshaler::createMarshaler(ret);
        if(!_result)
        {
            return;
        }
    }

    Slice::ParamDeclList params = op->parameters();

    //
    // Create an array that indicates how arguments are passed to the operation.
    // The first element in the array determines how many follow it.
    //
    _argTypes = new zend_uchar[params.size() + 1];
    _argTypes[0] = static_cast<zend_uchar>(params.size());

    int i;
    Slice::ParamDeclList::const_iterator p;
    for(p = params.begin(), i = 1; p != params.end(); ++p, ++i)
    {
        MarshalerPtr m = Marshaler::createMarshaler((*p)->type());
        if(!m)
        {
            break;
        }
        _paramNames.push_back((*p)->name());
        if((*p)->isOutParam())
        {
            _argTypes[i] = BYREF_FORCE;
            _outParams.push_back(m);
        }
        else
        {
            _argTypes[i] = BYREF_NONE;
            _inParams.push_back(m);
        }
    }
}

Operation::~Operation()
{
    delete []_argTypes;
}

zend_uchar*
Operation::getArgTypes() const
{
    return _argTypes;
}

void
Operation::invoke(INTERNAL_FUNCTION_PARAMETERS)
{
    Ice::OperationMode mode = (Ice::OperationMode)_op->mode();
    int i;

    //
    // Verify that the expected number of arguments are supplied.
    //
    vector<MarshalerPtr>::size_type numParams = _inParams.size() + _outParams.size();
    if(ZEND_NUM_ARGS() != numParams)
    {
        php_error(E_ERROR, "operation %s expects %d parameter%s", _name.c_str(), numParams, numParams == 1 ? "" : "s");
        return;
    }

    //
    // Retrieve the arguments.
    //
    zval** args[ZEND_NUM_ARGS()];
    if(zend_get_parameters_array_ex(ZEND_NUM_ARGS(), args) == FAILURE)
    {
        php_error(E_ERROR, "unable to get arguments");
        return;
    }

    //
    // Verify that the zvals for out parameters are passed by reference.
    //
    for(i = static_cast<int>(_inParams.size()); i < ZEND_NUM_ARGS(); ++i)
    {
        if(!PZVAL_IS_REF(*args[i]))
        {
            php_error(E_ERROR, "argument for out parameter %s must be passed by reference", _paramNames[i].c_str());
            return;
        }
    }

    try
    {
        //
        // Marshal the arguments.
        //
        // TODO: Check for class usage.
        //
        IceInternal::BasicStream os(_instance.get());
        vector<MarshalerPtr>::iterator p;
        for(i = 0, p = _inParams.begin(); p != _inParams.end(); ++i, ++p)
        {
            if(!(*p)->marshal(*args[i], os TSRMLS_CC))
            {
                return;
            }
        }

        //
        // Invoke the operation.
        //
        IceInternal::BasicStream is(_instance.get());
        if(!_proxy->ice_invoke(_name, mode, os.b, is.b))
        {
            // TODO
            php_error(E_ERROR, "user exception occurred");
            return;
        }

        //
        // Unmarshal the results.
        //
        // TODO: Check for oneway/datagram errors
        //
        // TODO: Check for class usage.
        //
        is.i = is.b.begin();
        for(i = _inParams.size(), p = _outParams.begin(); p != _outParams.end(); ++i, ++p)
        {
            //
            // It appears we must explicitly destroy the contents of all zvals passed
            // as out parameters, otherwise leaks occur.
            //
            zval_dtor(*args[i]);
            if(!(*p)->unmarshal(*args[i], is TSRMLS_CC))
            {
                return;
            }
        }
        if(_result)
        {
            if(!_result->unmarshal(return_value, is TSRMLS_CC))
            {
                return;
            }
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        ice_throw_exception(ex TSRMLS_CC);
    }
}

Proxy::Proxy(const Ice::ObjectPrx& proxy, const Slice::ClassDefPtr& cls) :
    _proxy(proxy), _class(cls)
{
    Ice::CommunicatorPtr communicator = Ice_Communicator_instance();
    _instance = IceInternal::getInstance(communicator);

    if(cls)
    {
        _classOps = _class->allOperations();
    }
}

const Ice::ObjectPrx&
Proxy::getProxy() const
{
    return _proxy;
}

const Slice::ClassDefPtr&
Proxy::getClass() const
{
    return _class;
}

OperationPtr
Proxy::getOperation(const string& name)
{
    OperationPtr result;

    string n = ice_lowercase(name);
    map<string, OperationPtr>::const_iterator p = _ops.find(n);
    if(p == _ops.end())
    {
        for(Slice::OperationList::const_iterator q = _classOps.begin(); q != _classOps.end(); ++q)
        {
            if(n == ice_lowercase((*q)->name()))
            {
                result = new Operation(_proxy, *q, _instance);
                _ops[n] = result;
                break;
            }
        }
    }
    else
    {
        result = p->second;
    }

    return result;
}

static zend_object_value
Ice_ObjectPrx_alloc(zend_class_entry* ce TSRMLS_DC)
{
    zend_object_value result;

    ice_object* obj = ice_object_new(ce TSRMLS_CC);
    assert(obj);

    result.handle = zend_objects_store_put(obj, Ice_ObjectPrx_dtor, NULL TSRMLS_CC);
    result.handlers = &Ice_ObjectPrx_handlers;

    return result;
}

static void
Ice_ObjectPrx_dtor(void* p, zend_object_handle handle TSRMLS_DC)
{
    ice_object* obj = static_cast<ice_object*>(p);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    delete _this;

    zend_objects_destroy_object(static_cast<zend_object*>(p), handle TSRMLS_CC);
}

static union _zend_function*
Ice_ObjectPrx_get_method(zval* zv, char* method, int len TSRMLS_DC)
{
    zend_function* result;

    //
    // First delegate to the standard implementation of get_method. This will find
    // any of our predefined proxy methods. If it returns NULL, then we return a
    // function that will check the class definition.
    //
    result = zend_get_std_object_handlers()->get_method(zv, method, len TSRMLS_CC);
    if(result == NULL)
    {
        ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(zv TSRMLS_CC));
        assert(obj->ptr);
        Proxy* _this = static_cast<Proxy*>(obj->ptr);

        if(!_this->getClass())
        {
            php_error(E_ERROR, "proxy has not been narrowed");
            return NULL;
        }

        OperationPtr op = _this->getOperation(method);
        if(!op)
        {
            php_error(E_ERROR, "unknown operation %s", method);
            return NULL;
        }

        zend_internal_function* zif = static_cast<zend_internal_function*>(emalloc(sizeof(zend_internal_function)));
        zif->type = ZEND_INTERNAL_FUNCTION;
        zif->arg_types = op->getArgTypes();
        zif->function_name = estrndup(method, len);
        zif->scope = Ice_ObjectPrx_entry_ptr;
        zif->fn_flags = ZEND_ACC_PUBLIC;
        zif->handler = ZEND_FN(Ice_ObjectPrx_call);
        result = (zend_function*)zif;
    }

    return result;
}

ZEND_FUNCTION(Ice_ObjectPrx_call)
{
    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    OperationPtr op = _this->getOperation(get_active_function_name(TSRMLS_C));
    assert(op); // get_method should have already verified the operation's existence.

    op->invoke(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
