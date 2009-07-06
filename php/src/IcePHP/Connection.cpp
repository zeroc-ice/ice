// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Connection.h>
#include <Util.h>

using namespace std;
using namespace IcePHP;

ZEND_EXTERN_MODULE_GLOBALS(ice)

//
// Class entries represent the PHP class implementations we have registered.
//
namespace IcePHP
{
zend_class_entry* connectionClassEntry = 0;
}

//
// Ice::Connection support.
//
static zend_object_handlers _handlers;

extern "C"
{
static zend_object_value handleAlloc(zend_class_entry* TSRMLS_DC);
static void handleFreeStorage(void* TSRMLS_DC);
static int handleCompare(zval*, zval* TSRMLS_DC);
}

ZEND_METHOD(Ice_Connection, __construct)
{
    runtimeError("Connection cannot be instantiated" TSRMLS_CC);
}

ZEND_METHOD(Ice_Connection, __toString)
{
    if(ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    Ice::ConnectionPtr _this = Wrapper<Ice::ConnectionPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    try
    {
        string str = _this->toString();
        RETURN_STRINGL(STRCAST(str.c_str()), str.length(), 1);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Connection, close)
{
    Ice::ConnectionPtr _this = Wrapper<Ice::ConnectionPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    zend_bool b;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("b"), &b TSRMLS_CC) != SUCCESS)
    {
        RETURN_NULL();
    }

    try
    {
        _this->close(b ? true : false);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Connection, flushBatchRequests)
{
    if(ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    Ice::ConnectionPtr _this = Wrapper<Ice::ConnectionPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    try
    {
        _this->flushBatchRequests();
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Connection, type)
{
    if(ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    Ice::ConnectionPtr _this = Wrapper<Ice::ConnectionPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    try
    {
        string str = _this->type();
        RETURN_STRINGL(STRCAST(str.c_str()), str.length(), 1);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Connection, timeout)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    Ice::ConnectionPtr _this = Wrapper<Ice::ConnectionPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    try
    {
        Ice::Int timeout = _this->timeout();
        ZVAL_LONG(return_value, static_cast<long>(timeout));
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Connection, toString)
{
    ZEND_MN(Ice_Connection___toString)(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

#ifdef _WIN32
extern "C"
#endif
static zend_object_value
handleAlloc(zend_class_entry* ce TSRMLS_DC)
{
    zend_object_value result;

    Wrapper<Ice::ConnectionPtr>* obj = Wrapper<Ice::ConnectionPtr>::create(ce TSRMLS_CC);
    assert(obj);

    result.handle = zend_objects_store_put(obj, 0, (zend_objects_free_object_storage_t)handleFreeStorage, 0 TSRMLS_CC);
    result.handlers = &_handlers;

    return result;
}

#ifdef _WIN32
extern "C"
#endif
static void
handleFreeStorage(void* p TSRMLS_DC)
{
    Wrapper<Ice::ConnectionPtr>* obj = static_cast<Wrapper<Ice::ConnectionPtr>*>(p);
    delete obj->ptr;
    zend_objects_free_object_storage(static_cast<zend_object*>(p) TSRMLS_CC);
}

#ifdef _WIN32
extern "C"
#endif
static int
handleCompare(zval* zobj1, zval* zobj2 TSRMLS_DC)
{
    //
    // PHP guarantees that the objects have the same class.
    //

    Ice::ConnectionPtr con1 = Wrapper<Ice::ConnectionPtr>::value(zobj1 TSRMLS_CC);
    assert(con1);
    Ice::ConnectionPtr con2 = Wrapper<Ice::ConnectionPtr>::value(zobj2 TSRMLS_CC);
    assert(con2);

    if(con1 == con2)
    {
        return 0;
    }
    else if(con1 < con2)
    {
        return -1;
    }
    else
    {
        return 1;
    }
}

//
// Predefined methods for Connection.
//
static function_entry _interfaceMethods[] =
{
    {0, 0, 0}
};
static function_entry _classMethods[] =
{
    ZEND_ME(Ice_Connection, __construct, NULL, ZEND_ACC_PRIVATE|ZEND_ACC_CTOR)
    ZEND_ME(Ice_Connection, __toString, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Connection, close, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Connection, flushBatchRequests, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Connection, type, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Connection, timeout, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Connection, toString, NULL, ZEND_ACC_PUBLIC)
    {0, 0, 0}
};

bool
IcePHP::connectionInit(TSRMLS_D)
{
    //
    // We register an interface and a class that implements the interface. This allows
    // applications to safely include the Slice-generated code for the type.
    //

    //
    // Register the Connection interface.
    //
    zend_class_entry ce;
#ifdef ICEPHP_USE_NAMESPACES
    INIT_NS_CLASS_ENTRY(ce, STRCAST("Ice"), STRCAST("Connection"), _interfaceMethods);
#else
    INIT_CLASS_ENTRY(ce, "Ice_Connection", _interfaceMethods);
#endif
    zend_class_entry* interface = zend_register_internal_interface(&ce TSRMLS_CC);

    //
    // Register the Connection class.
    //
    INIT_CLASS_ENTRY(ce, "IcePHP_Connection", _classMethods);
    ce.create_object = handleAlloc;
    connectionClassEntry = zend_register_internal_class(&ce TSRMLS_CC);
    memcpy(&_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    _handlers.compare_objects = handleCompare;
    zend_class_implements(connectionClassEntry TSRMLS_CC, 1, interface);

    return true;
}

bool
IcePHP::createConnection(zval* zv, const Ice::ConnectionPtr& p TSRMLS_DC)
{
    if(object_init_ex(zv, connectionClassEntry) != SUCCESS)
    {
        runtimeError("unable to initialize connection" TSRMLS_CC);
        return false;
    }

    Wrapper<Ice::ConnectionPtr>* obj = Wrapper<Ice::ConnectionPtr>::extract(zv TSRMLS_CC);
    assert(obj);
    assert(!obj->ptr);
    obj->ptr = new Ice::ConnectionPtr(p);

    return true;
}

bool
IcePHP::fetchConnection(zval* zv, Ice::ConnectionPtr& connection TSRMLS_DC)
{
    if(ZVAL_IS_NULL(zv))
    {
        connection = 0;
    }
    else
    {
        if(Z_TYPE_P(zv) != IS_OBJECT || Z_OBJCE_P(zv) != connectionClassEntry)
        {
            invalidArgument("value is not a connection" TSRMLS_CC);
            return false;
        }
        Wrapper<Ice::ConnectionPtr>* obj = Wrapper<Ice::ConnectionPtr>::extract(zv TSRMLS_CC);
        if(!obj)
        {
            return false;
        }
        connection = *obj->ptr;
    }
    return true;
}
