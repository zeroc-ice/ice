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

#include "ice_proxy.h"
#include "ice_communicator.h"
#include "ice_identity.h"
#include "ice_marshal.h"
#include "ice_slice.h"
#include "ice_util.h"

using namespace std;

ZEND_EXTERN_MODULE_GLOBALS(ice)

//
// Here's a brief description of how proxies are handled by this extension.
//
// A single PHP class, Ice_ObjectPrx, is registered. This is an "internal" class,
// i.e., implemented by this extension, and it is used to represent all proxies
// regardless of interface type.
//
// Like in C++, a proxy is only capable of invoking the Ice::ObjectPrx operations
// until it is narrowed with a checked or unchecked cast. Unlike C++, no PHP classes
// are created for proxies, because all marshaling activity is driven by the Slice
// definitions, not by statically-generated code.
//
// In order to perform a checked or unchecked cast, the user invokes ice_checkedCast
// or ice_uncheckedCast on the proxy to be narrowed, supplying a scoped name for the
// desired type. Internally, the proxy validates the scoped name and returns a new
// proxy containing the Slice class or interface definition. This proxy is considered
// to be narrowed to that interface and therefore supports user-defined operations.
//
// Naturally, there are many predefined proxy methods (e.g., ice_isA, etc.), but
// the proxy also needs to support user-defined operations (if it has type information).
// We use a Zend API hook that allows us to intercept the invocation of unknown methods
// on the proxy object. At this point, the proxy checks the interface definition for
// an operation with the given name, and then creates an Operation object (see below)
// that is responsible for invoking the operation. The proxy caches the Operation objects
// for future reuse.
//

//
// Class entries represent the PHP class implementations we have registered.
//
zend_class_entry* Ice_ObjectPrx_entry_ptr;

//
// Ice::ObjectPrx support.
//
static zend_object_handlers Ice_ObjectPrx_handlers;

extern "C"
{
static zend_object_value handleAlloc(zend_class_entry* TSRMLS_DC);
static void handleDestroy(void*, zend_object_handle TSRMLS_DC);
static zend_object_value handleClone(zval* TSRMLS_DC);
static union _zend_function* handleGetMethod(zval*, char*, int TSRMLS_DC);
static int handleCompare(zval*, zval* TSRMLS_DC);
ZEND_FUNCTION(Ice_ObjectPrx_call);
}

//
// Encapsulates an operation description.
//
class Operation : public IceUtil::SimpleShared
{
public:
    Operation(const Ice::ObjectPrx&, const string&, const Slice::OperationPtr&, const IceInternal::InstancePtr&
              TSRMLS_DC);
    virtual ~Operation();

    zend_function* getZendFunction() const;
    void invoke(INTERNAL_FUNCTION_PARAMETERS);

private:
    void throwException(IceInternal::BasicStream& TSRMLS_DC);

    Ice::ObjectPrx _proxy;
    string _name; // Local name, not the on-the-wire name
    Slice::OperationPtr _op;
    IceInternal::InstancePtr _instance;
#ifdef ZTS
    TSRMLS_D;
#endif
    vector<string> _paramNames;
    MarshalerPtr _result;
    vector<MarshalerPtr> _inParams;
    vector<MarshalerPtr> _outParams;
    zend_internal_function* _zendFunction;
};
typedef IceUtil::Handle<Operation> OperationPtr;

//
// Encapsulates proxy and type information.
//
class Proxy
{
public:
    Proxy(const Ice::ObjectPrx&, const Slice::ClassDefPtr& TSRMLS_DC);
    ~Proxy();

    const Ice::ObjectPrx& getProxy() const;
    const Slice::ClassDefPtr& getClass() const;

    OperationPtr getOperation(const string&);

private:
    Ice::ObjectPrx _proxy;
    Slice::ClassDefPtr _class;
#ifdef ZTS
    TSRMLS_D;
#endif
    zval _communicator;
    IceInternal::InstancePtr _instance;
    Slice::OperationList _classOps;
    map<string, OperationPtr> _ops;
};

//
// Predefined methods for Ice_ObjectPrx.
//
static function_entry Ice_ObjectPrx_methods[] =
{
    {"__construct",         PHP_FN(Ice_ObjectPrx___construct),         NULL},
    {"ice_isA",             PHP_FN(Ice_ObjectPrx_ice_isA),             NULL},
    {"ice_ping",            PHP_FN(Ice_ObjectPrx_ice_ping),            NULL},
    {"ice_id",              PHP_FN(Ice_ObjectPrx_ice_id),              NULL},
    {"ice_ids",             PHP_FN(Ice_ObjectPrx_ice_ids),             NULL},
    {"ice_facets",          PHP_FN(Ice_ObjectPrx_ice_facets),          NULL},
    {"ice_getIdentity",     PHP_FN(Ice_ObjectPrx_ice_getIdentity),     NULL},
    {"ice_newIdentity",     PHP_FN(Ice_ObjectPrx_ice_newIdentity),     NULL},
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
Ice_ObjectPrx_init(TSRMLS_D)
{
    //
    // Register the Ice_ObjectPrx class.
    //
    zend_class_entry ce_ObjectPrx;
    INIT_CLASS_ENTRY(ce_ObjectPrx, "Ice_ObjectPrx", Ice_ObjectPrx_methods);
    ce_ObjectPrx.create_object = handleAlloc;
    Ice_ObjectPrx_entry_ptr = zend_register_internal_class(&ce_ObjectPrx TSRMLS_CC);
    memcpy(&Ice_ObjectPrx_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    Ice_ObjectPrx_handlers.clone_obj = handleClone;
    Ice_ObjectPrx_handlers.get_method = handleGetMethod;
    Ice_ObjectPrx_handlers.compare_objects = handleCompare;

    return true;
}

bool
Ice_ObjectPrx_create(zval* zv, const Ice::ObjectPrx& p TSRMLS_DC)
{
    return Ice_ObjectPrx_create(zv, p, 0 TSRMLS_CC);
}

bool
Ice_ObjectPrx_create(zval* zv, const Ice::ObjectPrx& p, const Slice::ClassDefPtr& def TSRMLS_DC)
{
    if(object_init_ex(zv, Ice_ObjectPrx_entry_ptr) != SUCCESS)
    {
        zend_error(E_ERROR, "unable to initialize proxy");
        return false;
    }

    ice_object* zprx = static_cast<ice_object*>(zend_object_store_get_object(zv TSRMLS_CC));
    assert(!zprx->ptr);
    zprx->ptr = new Proxy(p, def TSRMLS_CC);

    return true;
}

bool
Ice_ObjectPrx_fetch(zval* zv, Ice::ObjectPrx& prx, Slice::ClassDefPtr& def TSRMLS_DC)
{
    if(!ZVAL_IS_NULL(zv))
    {
        void* p = zend_object_store_get_object(zv TSRMLS_CC);
        if(!p)
        {
            zend_error(E_ERROR, "unable to retrieve proxy object from object store");
            return false;
        }
        if(Z_OBJCE_P(zv) != Ice_ObjectPrx_entry_ptr)
        {
            zend_error(E_ERROR, "%s(): value is not a proxy", get_active_function_name(TSRMLS_C));
            return false;
        }
        ice_object* obj = static_cast<ice_object*>(p);
        assert(obj->ptr);
        Proxy* proxy = static_cast<Proxy*>(obj->ptr);
        prx = proxy->getProxy();
        def = proxy->getClass();
    }
    return true;
}

ZEND_FUNCTION(Ice_ObjectPrx___construct)
{
    zend_error(E_ERROR, "Ice_ObjectPrx cannot be instantiated, use $ICE->stringToProxy()");
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_isA)
{
    if(ZEND_NUM_ARGS() != 1)
    {
        WRONG_PARAM_COUNT;
    }

    char* id;
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
        ice_throwException(ex TSRMLS_CC);
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
        ice_throwException(ex TSRMLS_CC);
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
        ice_throwException(ex TSRMLS_CC);
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
        ice_throwException(ex TSRMLS_CC);
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
        ice_throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_getIdentity)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    Ice_Identity_create(return_value, _this->getProxy()->ice_getIdentity() TSRMLS_CC);
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_newIdentity)
{
    if(ZEND_NUM_ARGS() != 1)
    {
        WRONG_PARAM_COUNT;
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    zend_class_entry* cls = ice_findClass("Ice_Identity" TSRMLS_CC);
    assert(cls);

    zval *zid;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zid, cls) == FAILURE)
    {
        RETURN_NULL();
    }

    Ice::Identity id;
    if(Ice_Identity_extract(zid, id TSRMLS_CC))
    {
        try
        {
            Ice::ObjectPrx prx = _this->getProxy()->ice_newIdentity(id);
            if(!Ice_ObjectPrx_create(return_value, prx TSRMLS_CC))
            {
                RETURN_NULL();
            }
        }
        catch(const IceUtil::Exception& ex)
        {
            ice_throwException(ex TSRMLS_CC);
            RETURN_NULL();
        }
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
        ice_throwException(ex TSRMLS_CC);
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
            zend_error(E_ERROR, "%s(): facet must be a string array", get_active_function_name(TSRMLS_C));
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
        ice_throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_ObjectPrx_ice_appendFacet)
{
    if(ZEND_NUM_ARGS() != 1)
    {
        WRONG_PARAM_COUNT;
    }

    char* name;
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
        ice_throwException(ex TSRMLS_CC);
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
        ice_throwException(ex TSRMLS_CC);
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
        ice_throwException(ex TSRMLS_CC);
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
        ice_throwException(ex TSRMLS_CC);
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
        ice_throwException(ex TSRMLS_CC);
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
        ice_throwException(ex TSRMLS_CC);
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
        ice_throwException(ex TSRMLS_CC);
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
        ice_throwException(ex TSRMLS_CC);
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
        ice_throwException(ex TSRMLS_CC);
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
        ice_throwException(ex TSRMLS_CC);
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
        ice_throwException(ex TSRMLS_CC);
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
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &b TSRMLS_CC) != SUCCESS)
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
        ice_throwException(ex TSRMLS_CC);
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
        if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &b) != SUCCESS)
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
        ice_throwException(ex TSRMLS_CC);
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
        if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &l) != SUCCESS)
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
        ice_throwException(ex TSRMLS_CC);
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
        ice_throwException(ex TSRMLS_CC);
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
        ice_throwException(ex TSRMLS_CC);
    }
}

static void
do_cast(INTERNAL_FUNCTION_PARAMETERS, bool check)
{
    //
    // First argument is required and should be a scoped name. The second argument
    // is optional and represents a facet name.
    //
    if(ZEND_NUM_ARGS() < 1 || ZEND_NUM_ARGS() > 2)
    {
        WRONG_PARAM_COUNT;
    }

    char* id;
    int idLen;
    char* facet = NULL;
    int facetLen;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|s", &id, &idLen, &facet, &facetLen) == FAILURE)
    {
        RETURN_NULL();
    }

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    try
    {
        Slice::UnitPtr unit = Slice_getUnit();
        Slice::TypeList l = unit->lookupTypeNoBuiltin(id, false);
        if(l.empty())
        {
            zend_error(E_ERROR, "%s(): no Slice definition found for type %s", get_active_function_name(TSRMLS_C), id);
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

        string scoped = decl->scoped();

        if(!decl)
        {
            zend_error(E_ERROR, "%s(): type %s is not a class or interface", get_active_function_name(TSRMLS_C),
                       scoped.c_str());
            RETURN_NULL();
        }

        if(decl->isLocal())
        {
            zend_error(E_ERROR, "%s(): %s is a local type", get_active_function_name(TSRMLS_C), scoped.c_str());
            RETURN_NULL();
        }

        Slice::ClassDefPtr def = decl->definition();
        if(!def)
        {
            zend_error(E_ERROR, "%s(): %s is declared but not defined", get_active_function_name(TSRMLS_C),
                       scoped.c_str());
            RETURN_NULL();
        }

        //
        // Verify that the script has compiled the Slice definition for this type.
        //
        if(ice_findClassScoped(scoped TSRMLS_CC) == 0)
        {
            zend_error(E_ERROR, "%s(): the Slice definition for type %s has not been compiled",
                       get_active_function_name(TSRMLS_C), scoped.c_str());
            RETURN_NULL();
        }

        Ice::ObjectPrx prx = _this->getProxy();
        if(facet)
        {
            prx = prx->ice_appendFacet(facet);
        }

        if(check)
        {
            //
            // Verify that the object supports the requested type. We don't use id here,
            // because it might contain a proxy type (e.g., "::MyClass*").
            //
            if(!prx->ice_isA(scoped))
            {
                RETURN_NULL();
            }
        }

        if(!Ice_ObjectPrx_create(return_value, prx, def TSRMLS_CC))
        {
            RETURN_NULL();
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        ice_throwException(ex TSRMLS_CC);
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

Operation::Operation(const Ice::ObjectPrx& proxy, const string& name, const Slice::OperationPtr& op,
                     const IceInternal::InstancePtr& instance TSRMLS_DC) :
    _proxy(proxy), _name(name), _op(op), _instance(instance), _zendFunction(0)
{
#ifdef ZTS
    this->TSRMLS_C = TSRMLS_C;
#endif
    //
    // Create Marshaler objects for return type and parameters.
    //
    Slice::TypePtr ret = op->returnType();
    if(ret)
    {
        _result = Marshaler::createMarshaler(ret TSRMLS_CC);
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
    zend_uchar* argTypes = new zend_uchar[params.size() + 1];
    argTypes[0] = static_cast<zend_uchar>(params.size());

    int i;
    Slice::ParamDeclList::const_iterator p;
    for(p = params.begin(), i = 1; p != params.end(); ++p, ++i)
    {
        MarshalerPtr m = Marshaler::createMarshaler((*p)->type() TSRMLS_CC);
        if(!m)
        {
            break;
        }
        _paramNames.push_back((*p)->name());
        if((*p)->isOutParam())
        {
            argTypes[i] = BYREF_FORCE;
            _outParams.push_back(m);
        }
        else
        {
            argTypes[i] = BYREF_NONE;
            _inParams.push_back(m);
        }
    }

    _zendFunction = static_cast<zend_internal_function*>(emalloc(sizeof(zend_internal_function)));
    _zendFunction->type = ZEND_INTERNAL_FUNCTION;
    _zendFunction->arg_types = argTypes;
    _zendFunction->function_name = estrndup(const_cast<char*>(name.c_str()), name.length());
    _zendFunction->scope = Ice_ObjectPrx_entry_ptr;
    _zendFunction->fn_flags = ZEND_ACC_PUBLIC;
    _zendFunction->handler = ZEND_FN(Ice_ObjectPrx_call);
}

Operation::~Operation()
{
    if(_zendFunction)
    {
        delete []_zendFunction->arg_types;
        efree(_zendFunction->function_name);
        efree(_zendFunction);
    }
}

zend_function*
Operation::getZendFunction() const
{
    return (zend_function*)_zendFunction;
}

void
Operation::invoke(INTERNAL_FUNCTION_PARAMETERS)
{
    Ice::OperationMode mode = (Ice::OperationMode)_op->mode();
    int i;

    //
    // Verify that the expected number of arguments are supplied. The context argument is optional.
    //
    int numParams = static_cast<int>(_inParams.size() + _outParams.size());
    if(ZEND_NUM_ARGS() != numParams && ZEND_NUM_ARGS() != numParams + 1)
    {
        zend_error(E_ERROR, "%s(): incorrect number of parameters (%d)", get_active_function_name(TSRMLS_C),
                   numParams);
        return;
    }

    //
    // Retrieve the arguments.
    //
    zval*** args = static_cast<zval***>(emalloc(ZEND_NUM_ARGS() * sizeof(zval**)));
    AutoEfree autoArgs(args); // Call efree on return
    if(zend_get_parameters_array_ex(ZEND_NUM_ARGS(), args) == FAILURE)
    {
        zend_error(E_ERROR, "unable to get arguments");
        return;
    }

    //
    // Verify that the zvals for out parameters are passed by reference.
    //
    for(i = static_cast<int>(_inParams.size()); i < numParams; ++i)
    {
        if(!PZVAL_IS_REF(*args[i]))
        {
            zend_error(E_ERROR, "%s(): argument for out parameter %s must be passed by reference",
                       get_active_function_name(TSRMLS_C), _paramNames[i].c_str());
            return;
        }
    }

    try
    {
        //
        // Marshal the arguments.
        //
        PHPStream os(_instance.get());
        vector<MarshalerPtr>::iterator p;
        for(i = 0, p = _inParams.begin(); p != _inParams.end(); ++i, ++p)
        {
            if(!(*p)->marshal(*args[i], os TSRMLS_CC))
            {
                return;
            }
        }

        if(_op->sendsClasses())
        {
            os.writePendingObjects();
        }

        //
        // Populate the context (if necessary).
        //
        Ice::Context ctx;
        if(ZEND_NUM_ARGS() == numParams + 1)
        {
            if(Z_TYPE_PP(args[numParams]) != IS_ARRAY)
            {
                string s = ice_zendTypeToString(Z_TYPE_PP(args[i]));
                zend_error(E_ERROR, "%s(): expected an array for the context argument but received %s",
                           get_active_function_name(TSRMLS_C), s.c_str());
                return;
            }

            HashTable* arr = Z_ARRVAL_PP(args[i]);
            HashPosition pos;
            zval** val;

            zend_hash_internal_pointer_reset_ex(arr, &pos);
            while(zend_hash_get_current_data_ex(arr, (void**)&val, &pos) != FAILURE)
            {
                //
                // Get the key (which can be a long or a string).
                //
                char* keyStr;
                uint keyLen;
                ulong keyNum;
                int keyType = zend_hash_get_current_key_ex(arr, &keyStr, &keyLen, &keyNum, 0, &pos);

                //
                // Store the key in a zval, so that we can reuse the PrimitiveMarshaler logic.
                //
                zval zkey;
                if(keyType != HASH_KEY_IS_STRING)
                {
                    zend_error(E_ERROR, "%s(): context key must be a string", get_active_function_name(TSRMLS_C));
                    return;
                }

                zend_hash_get_current_data_ex(arr, (void**)&val, &pos);
                if(Z_TYPE_PP(val) != IS_STRING)
                {
                    zend_error(E_ERROR, "%s(): context value must be a string", get_active_function_name(TSRMLS_C));
                    return;
                }

                ctx[keyStr] = Z_STRVAL_PP(val);

                zend_hash_move_forward_ex(arr, &pos);
            }
        }

        //
        // Invoke the operation. Don't use _name here.
        //
        PHPStream is(_instance.get());
        bool status = _proxy->ice_invoke(_op->name(), mode, os.b, is.b, ctx);

        //
        // Reset the input stream's iterator.
        //
        is.i = is.b.begin();

        if(status)
        {
            //
            // Unmarshal the results.
            //
            // TODO: Check for oneway/datagram errors
            //
            for(i = _inParams.size(), p = _outParams.begin(); p != _outParams.end(); ++i, ++p)
            {
                //
                // We must explicitly destroy the existing contents of all zvals passed
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
            if(_op->returnsClasses())
            {
                is.readPendingObjects();
            }
        }
        else
        {
            //
            // Unmarshal and "throw" a user exception.
            //
            throwException(is TSRMLS_CC);
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        ice_throwException(ex TSRMLS_CC);
    }
}

void
Operation::throwException(IceInternal::BasicStream& is TSRMLS_DC)
{
    Slice::UnitPtr unit = _op->unit();

    bool usesClasses;
    is.read(usesClasses);

    string id;
    is.read(id);
    while(!id.empty())
    {
        //
        // Look for a definition of this type.
        //
        Slice::ExceptionPtr ex = unit->lookupException(id, false);
        if(ex)
        {
            if(ex->isLocal())
            {
                zend_error(E_ERROR, "%s(): cannot unmarshal local exception %s", get_active_function_name(TSRMLS_C),
                           id.c_str());
                return;
            }

            MarshalerPtr m = Marshaler::createExceptionMarshaler(ex TSRMLS_CC);
            assert(m);

            zval* zex;
            MAKE_STD_ZVAL(zex);
            if(m->unmarshal(zex, is TSRMLS_CC))
            {
                if(usesClasses)
                {
                    is.readPendingObjects();
                }
                EG(exception) = zex;
            }
            else
            {
                zval_dtor(zex);
            }

            return;
        }
        else
        {
            is.skipSlice();
            is.read(id);
        }
    }
    //
    // Getting here should be impossible: we can get here only if the
    // sender has marshaled a sequence of type IDs, none of which we
    // have factory for. This means that sender and receiver disagree
    // about the Slice definitions they use.
    //
    throw Ice::UnknownUserException(__FILE__, __LINE__);
}

Proxy::Proxy(const Ice::ObjectPrx& proxy, const Slice::ClassDefPtr& cls TSRMLS_DC) :
    _proxy(proxy), _class(cls)
{
#ifdef ZTS
    this->TSRMLS_C = TSRMLS_C;
#endif

    //
    // We want to ensure that the PHP object corresponding to the communicator is
    // not destroyed until after this proxy is destroyed. We keep a copy of the
    // communicator's zval because the symbol table holding the communicator's zval
    // may be destroyed before this proxy, therefore our destructor cannot rely on
    // symbol table lookup when it needs to decrement the reference count.
    //
    zval* zc = Ice_Communicator_getZval(TSRMLS_C);
    _communicator = *zc; // This is legal - it simply copies the object's handle
    Z_OBJ_HT(_communicator)->add_ref(&_communicator TSRMLS_CC);

    Ice::CommunicatorPtr communicator = Ice_Communicator_getInstance(TSRMLS_C);
    _instance = IceInternal::getInstance(communicator);

    if(cls)
    {
        _classOps = _class->allOperations();
    }
}

Proxy::~Proxy()
{
    //
    // In order to avoid the communicator's "leak warning", we have to ensure that we
    // remove any references to the communicator or its supporting objects. This must
    // be done prior to invoking del_ref(), because the C++ communicator object may
    // be destroyed during this call.
    // 
    _instance = 0;
    _ops.clear();
    _proxy = 0;
    Z_OBJ_HT(_communicator)->del_ref(&_communicator TSRMLS_CC);
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

    string n = ice_lowerCase(name);
    map<string, OperationPtr>::const_iterator p = _ops.find(n);
    if(p == _ops.end())
    {
        for(Slice::OperationList::const_iterator q = _classOps.begin(); q != _classOps.end(); ++q)
        {
            string opName = ice_lowerCase(ice_fixIdent((*q)->name()));
            if(n == opName)
            {
                result = new Operation(_proxy, opName, *q, _instance TSRMLS_CC);
                _ops[opName] = result;
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

#ifdef WIN32
extern "C"
#endif
static zend_object_value
handleAlloc(zend_class_entry* ce TSRMLS_DC)
{
    zend_object_value result;

    ice_object* obj = ice_newObject(ce TSRMLS_CC);
    assert(obj);

    result.handle = zend_objects_store_put(obj, handleDestroy, NULL TSRMLS_CC);
    result.handlers = &Ice_ObjectPrx_handlers;

    return result;
}

#ifdef WIN32
extern "C"
#endif
static void
handleDestroy(void* p, zend_object_handle handle TSRMLS_DC)
{
    ice_object* obj = static_cast<ice_object*>(p);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    delete _this;

    zend_objects_destroy_object(static_cast<zend_object*>(p), handle TSRMLS_CC);
}

#ifdef WIN32
extern "C"
#endif
static zend_object_value
handleClone(zval* zv TSRMLS_DC)
{
    //
    // Create a new object that shares a C++ proxy instance with this object.
    //

    zend_object_value result;
    memset(&result, 0, sizeof(zend_object_value));

    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(zv TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    zval* clone;
    MAKE_STD_ZVAL(clone);
    if(object_init_ex(clone, Ice_ObjectPrx_entry_ptr) != SUCCESS)
    {
        zend_error(E_ERROR, "unable to initialize proxy");
        return result;
    }

    ice_object* cobj = static_cast<ice_object*>(zend_object_store_get_object(clone TSRMLS_CC));
    assert(!cobj->ptr);
    cobj->ptr = new Proxy(_this->getProxy(), _this->getClass() TSRMLS_CC);

    //
    // We only need to return the new object's handle, so we must destroy the zval containing
    // a reference to the new object. We increment the object's reference count to ensure it
    // does not get destroyed.
    //
    result = clone->value.obj;
    Z_OBJ_HT_P(clone)->add_ref(clone TSRMLS_CC);
    zval_dtor(clone);
    efree(clone);

    return result;
}

#ifdef WIN32
extern "C"
#endif
static union _zend_function*
handleGetMethod(zval* zv, char* method, int len TSRMLS_DC)
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
            zend_error(E_ERROR, "%s(): proxy has not been narrowed", get_active_function_name(TSRMLS_C));
            return NULL;
        }

        OperationPtr op = _this->getOperation(method);
        if(!op)
        {
            zend_error(E_ERROR, "%s(): unknown operation", get_active_function_name(TSRMLS_C));
            return NULL;
        }

        result = op->getZendFunction();
    }

    return result;
}

#ifdef WIN32
extern "C"
#endif
static int
handleCompare(zval* zobj1, zval* zobj2 TSRMLS_DC)
{
    //
    // PHP guarantees that the objects have the same class.
    //

    ice_object* obj1 = static_cast<ice_object*>(zend_object_store_get_object(zobj1 TSRMLS_CC));
    assert(obj1->ptr);
    Proxy* _this1 = static_cast<Proxy*>(obj1->ptr);
    Ice::ObjectPrx prx1 = _this1->getProxy();

    ice_object* obj2 = static_cast<ice_object*>(zend_object_store_get_object(zobj2 TSRMLS_CC));
    assert(obj2->ptr);
    Proxy* _this2 = static_cast<Proxy*>(obj2->ptr);
    Ice::ObjectPrx prx2 = _this2->getProxy();

    if(prx1 == prx2)
    {
        return 0;
    }
    else if(prx1 < prx2)
    {
        return -1;
    }
    else
    {
        return 1;
    }
}

ZEND_FUNCTION(Ice_ObjectPrx_call)
{
    ice_object* obj = static_cast<ice_object*>(zend_object_store_get_object(getThis() TSRMLS_CC));
    assert(obj->ptr);
    Proxy* _this = static_cast<Proxy*>(obj->ptr);

    OperationPtr op = _this->getOperation(get_active_function_name(TSRMLS_C));
    assert(op); // handleGetethod should have already verified the operation's existence.

    op->invoke(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
