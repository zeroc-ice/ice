// Copyright (c) ZeroC, Inc.

#include "Logger.h"
#include "Util.h"

using namespace std;
using namespace IcePHP;

ZEND_EXTERN_MODULE_GLOBALS(ice)

// Class entries represent the PHP class implementations we have registered.
namespace IcePHP
{
    zend_class_entry* loggerClassEntry = 0;
}

// Logger support.
static zend_object_handlers _loggerHandlers;

extern "C"
{
    static zend_object* handleAlloc(zend_class_entry*);
    static void handleFreeStorage(zend_object*);
    static zend_object* handleClone(zend_object*);
}

ZEND_METHOD(Ice_Logger, __construct) { runtimeError("logger objects cannot be instantiated"); }

ZEND_METHOD(Ice_Logger, __toString)
{
    if (ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    RETURN_NULL();
}

ZEND_BEGIN_ARG_INFO_EX(Ice_Logger_print_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(1))
ZEND_ARG_INFO(0, message)
ZEND_END_ARG_INFO()

ZEND_METHOD(Ice_Logger, print)
{
    char* m;
    size_t mLen;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("s"), &m, &mLen) == FAILURE)
    {
        RETURN_NULL();
    }

    Ice::LoggerPtr _this = Wrapper<Ice::LoggerPtr>::value(getThis());
    assert(_this);

    string msg(m, mLen);
    try
    {
        _this->print(msg);
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

ZEND_BEGIN_ARG_INFO_EX(Ice_Logger_trace_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(2))
ZEND_ARG_INFO(0, category)
ZEND_ARG_INFO(0, message)
ZEND_END_ARG_INFO()

ZEND_METHOD(Ice_Logger, trace)
{
    char* c;
    size_t cLen;
    char* m;
    size_t mLen;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("ss"), &c, &cLen, &m, &mLen) == FAILURE)
    {
        RETURN_NULL();
    }

    Ice::LoggerPtr _this = Wrapper<Ice::LoggerPtr>::value(getThis());
    assert(_this);

    string category(c, cLen);
    string msg(m, mLen);
    try
    {
        _this->trace(category, msg);
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

ZEND_BEGIN_ARG_INFO_EX(Ice_Logger_warning_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(1))
ZEND_ARG_INFO(0, message)
ZEND_END_ARG_INFO()

ZEND_METHOD(Ice_Logger, warning)
{
    char* m;
    size_t mLen;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("s"), &m, &mLen) == FAILURE)
    {
        RETURN_NULL();
    }

    Ice::LoggerPtr _this = Wrapper<Ice::LoggerPtr>::value(getThis());
    assert(_this);

    string msg(m, mLen);
    try
    {
        _this->warning(msg);
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

ZEND_BEGIN_ARG_INFO_EX(Ice_Logger_error_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(1))
ZEND_ARG_INFO(0, message)
ZEND_END_ARG_INFO()

ZEND_METHOD(Ice_Logger, error)
{
    char* m;
    size_t mLen;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("s"), &m, &mLen) == FAILURE)
    {
        RETURN_NULL();
    }

    Ice::LoggerPtr _this = Wrapper<Ice::LoggerPtr>::value(getThis());
    assert(_this);

    string msg(m, mLen);
    try
    {
        _this->error(msg);
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

ZEND_BEGIN_ARG_INFO_EX(Ice_Logger_cloneWithPrefix_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(1))
ZEND_ARG_INFO(0, prefix)
ZEND_END_ARG_INFO()

ZEND_METHOD(Ice_Logger, cloneWithPrefix)
{
    char* p;
    size_t pLen;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("s"), &p, &pLen) == FAILURE)
    {
        RETURN_NULL();
    }

    Ice::LoggerPtr _this = Wrapper<Ice::LoggerPtr>::value(getThis());
    assert(_this);

    Ice::LoggerPtr clone;

    string prefix(p, pLen);
    try
    {
        clone = _this->cloneWithPrefix(prefix);
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }

    if (!createLogger(return_value, clone))
    {
        RETURN_NULL();
    }
}

static zend_object*
handleAlloc(zend_class_entry* ce)
{
    Wrapper<Ice::LoggerPtr>* obj = Wrapper<Ice::LoggerPtr>::create(ce);
    assert(obj);
    obj->zobj.handlers = &_loggerHandlers;
    return &obj->zobj;
}

static void
handleFreeStorage(zend_object* object)
{
    Wrapper<Ice::LoggerPtr>* obj = Wrapper<Ice::LoggerPtr>::fetch(object);
    delete obj->ptr;
    zend_object_std_dtor(object);
}

static zend_object*
handleClone(zend_object*)
{
    php_error_docref(0, E_ERROR, "loggers cannot be cloned");
    return 0;
}

// Predefined methods for Logger.
static zend_function_entry _interfaceMethods[] = {{0, 0, 0}};
static zend_function_entry _classMethods[] = {
    // _construct
    ZEND_ME(Ice_Logger, __construct, ice_void_arginfo, ZEND_ACC_PRIVATE | ZEND_ACC_CTOR)
    // __toString
    ZEND_ME(Ice_Logger, __toString, ice_to_string_arginfo, ZEND_ACC_PUBLIC)
    // print
    ZEND_ME(Ice_Logger, print, Ice_Logger_print_arginfo, ZEND_ACC_PUBLIC)
    // trace
    ZEND_ME(Ice_Logger, trace, Ice_Logger_trace_arginfo, ZEND_ACC_PUBLIC)
    // warning
    ZEND_ME(Ice_Logger, warning, Ice_Logger_warning_arginfo, ZEND_ACC_PUBLIC)
    // error
    ZEND_ME(Ice_Logger, error, Ice_Logger_error_arginfo, ZEND_ACC_PUBLIC)
    // cloneWithPrefix
    ZEND_ME(Ice_Logger, cloneWithPrefix, Ice_Logger_cloneWithPrefix_arginfo, ZEND_ACC_PUBLIC){0, 0, 0}};

bool
IcePHP::loggerInit(void)
{
    // TODO review we no longer have generated code for local Slice
    // We register an interface and a class that implements the interface. This allows
    // applications to safely include the Slice-generated code for the type.

    // Register the Logger interface.
    zend_class_entry ce;
    INIT_NS_CLASS_ENTRY(ce, "Ice", "Logger", _interfaceMethods);
    zend_class_entry* interface = zend_register_internal_interface(&ce);

    // Register the Logger class.
    INIT_CLASS_ENTRY(ce, "IcePHP_Logger", _classMethods);
    ce.create_object = handleAlloc;
    loggerClassEntry = zend_register_internal_class(&ce);
    memcpy(&_loggerHandlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    _loggerHandlers.clone_obj = handleClone;
    _loggerHandlers.free_obj = handleFreeStorage;
    _loggerHandlers.offset = XtOffsetOf(Wrapper<Ice::LoggerPtr>, zobj);
    zend_class_implements(loggerClassEntry, 1, interface);

    return true;
}

bool
IcePHP::createLogger(zval* zv, const Ice::LoggerPtr& p)
{
    if (object_init_ex(zv, loggerClassEntry) != SUCCESS)
    {
        runtimeError("unable to initialize logger object");
        return false;
    }

    Wrapper<Ice::LoggerPtr>* obj = Wrapper<Ice::LoggerPtr>::extract(zv);
    assert(!obj->ptr);
    obj->ptr = new Ice::LoggerPtr(p);
    return true;
}

bool
IcePHP::fetchLogger(zval* zv, Ice::LoggerPtr& p)
{
    if (!ZVAL_IS_NULL(zv))
    {
        if (Z_TYPE_P(zv) != IS_OBJECT || Z_OBJCE_P(zv) != loggerClassEntry)
        {
            invalidArgument("value is not a logger object");
            return false;
        }
        p = Wrapper<Ice::LoggerPtr>::value(zv);
        if (!p)
        {
            runtimeError("unable to retrieve logger object from object store");
            return false;
        }
    }
    return true;
}
