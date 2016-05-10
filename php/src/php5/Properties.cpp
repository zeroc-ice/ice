// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Properties.h>
#include <Util.h>

using namespace std;
using namespace IcePHP;

ZEND_EXTERN_MODULE_GLOBALS(ice)

//
// Class entries represent the PHP class implementations we have registered.
//
namespace IcePHP
{
zend_class_entry* propertiesClassEntry = 0;
}

//
// Properties support.
//
static zend_object_handlers _handlers;

extern "C"
{
static zend_object_value handleAlloc(zend_class_entry* TSRMLS_DC);
static void handleFreeStorage(void* TSRMLS_DC);
static zend_object_value handleClone(zval* TSRMLS_DC);
}

ZEND_METHOD(Ice_Properties, __construct)
{
    runtimeError("properties objects cannot be instantiated, use createProperties()" TSRMLS_CC);
}

ZEND_METHOD(Ice_Properties, __toString)
{
    if(ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    Ice::PropertiesPtr _this = Wrapper<Ice::PropertiesPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    try
    {
        Ice::PropertyDict val = _this->getPropertiesForPrefix("");
        string str;
        for(Ice::PropertyDict::const_iterator p = val.begin(); p != val.end(); ++p)
        {
            if(p != val.begin())
            {
                str.append("\n");
            }
            str.append(p->first + "=" + p->second);
        }
        RETURN_STRINGL(STRCAST(str.c_str()), static_cast<int>(str.length()), 1);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Properties, getProperty)
{
    char* name;
    int nameLen;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("s"), &name, &nameLen) == FAILURE)
    {
        RETURN_NULL();
    }

    Ice::PropertiesPtr _this = Wrapper<Ice::PropertiesPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    string propName(name, nameLen);
    try
    {
        string val = _this->getProperty(propName);
        RETURN_STRINGL(STRCAST(val.c_str()), static_cast<int>(val.length()), 1);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Properties, getPropertyWithDefault)
{
    char* name;
    int nameLen;
    char* def;
    int defLen;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("ss!"), &name, &nameLen, &def, &defLen) ==
        FAILURE)
    {
        RETURN_NULL();
    }

    Ice::PropertiesPtr _this = Wrapper<Ice::PropertiesPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    string propName(name, nameLen);
    string defaultValue;
    if(def)
    {
        defaultValue = string(def, defLen);
    }

    try
    {
        string val = _this->getPropertyWithDefault(propName, defaultValue);
        RETURN_STRINGL(STRCAST(val.c_str()), static_cast<int>(val.length()), 1);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Properties, getPropertyAsInt)
{
    char* name;
    int nameLen;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("s"), &name, &nameLen) == FAILURE)
    {
        RETURN_NULL();
    }

    Ice::PropertiesPtr _this = Wrapper<Ice::PropertiesPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    string propName(name, nameLen);
    try
    {
        Ice::Int val = _this->getPropertyAsInt(propName);
        RETURN_LONG(static_cast<long>(val));
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Properties, getPropertyAsIntWithDefault)
{
    char* name;
    int nameLen;
    long def;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("sl"), &name, &nameLen, &def) == FAILURE)
    {
        RETURN_NULL();
    }

    Ice::PropertiesPtr _this = Wrapper<Ice::PropertiesPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    string propName(name, nameLen);
    try
    {
        Ice::Int val = _this->getPropertyAsIntWithDefault(propName, def);
        RETURN_LONG(static_cast<long>(val));
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Properties, getPropertyAsList)
{
    char* name;
    int nameLen;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("s"), &name, &nameLen) == FAILURE)
    {
        RETURN_NULL();
    }

    Ice::PropertiesPtr _this = Wrapper<Ice::PropertiesPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    string propName(name, nameLen);
    try
    {
        Ice::StringSeq val = _this->getPropertyAsList(propName);
        if(!createStringArray(return_value, val TSRMLS_CC))
        {
            RETURN_NULL();
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Properties, getPropertyAsListWithDefault)
{
    char* name;
    int nameLen;
    zval* def;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("sa!"), &name, &nameLen, &def) == FAILURE)
    {
        RETURN_NULL();
    }

    Ice::PropertiesPtr _this = Wrapper<Ice::PropertiesPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    string propName(name, nameLen);
    Ice::StringSeq defaultValue;
    if(def && !extractStringArray(def, defaultValue TSRMLS_CC))
    {
        RETURN_NULL();
    }

    try
    {
        Ice::StringSeq val = _this->getPropertyAsListWithDefault(propName, defaultValue);
        if(!createStringArray(return_value, val TSRMLS_CC))
        {
            RETURN_NULL();
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Properties, getPropertiesForPrefix)
{
    char* p;
    int pLen;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("s!"), &p, &pLen) == FAILURE)
    {
        RETURN_NULL();
    }

    Ice::PropertiesPtr _this = Wrapper<Ice::PropertiesPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    string prefix;
    if(p)
    {
        prefix = string(p, pLen);
    }

    try
    {
        Ice::PropertyDict val = _this->getPropertiesForPrefix(prefix);
        if(!createStringMap(return_value, val TSRMLS_CC))
        {
            RETURN_NULL();
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Properties, setProperty)
{
    char* name;
    int nameLen;
    char* val;
    int valLen;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("ss!"), &name, &nameLen, &val, &valLen) ==
        FAILURE)
    {
        RETURN_NULL();
    }

    Ice::PropertiesPtr _this = Wrapper<Ice::PropertiesPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    string propName(name, nameLen);
    string propValue;
    if(val)
    {
        propValue = string(val, valLen);
    }

    try
    {
        _this->setProperty(propName, propValue);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Properties, getCommandLineOptions)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    Ice::PropertiesPtr _this = Wrapper<Ice::PropertiesPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    try
    {
        Ice::StringSeq val = _this->getCommandLineOptions();
        if(!createStringArray(return_value, val TSRMLS_CC))
        {
            RETURN_NULL();
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Properties, parseCommandLineOptions)
{
    char* p;
    int pLen;
    zval* opts;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("s!a!"), &p, &pLen, &opts) == FAILURE)
    {
        RETURN_NULL();
    }

    Ice::PropertiesPtr _this = Wrapper<Ice::PropertiesPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    string prefix;
    if(p)
    {
        prefix = string(p, pLen);
    }
    Ice::StringSeq options;
    if(opts && !extractStringArray(opts, options TSRMLS_CC))
    {
        RETURN_NULL();
    }

    try
    {
        Ice::StringSeq val = _this->parseCommandLineOptions(prefix, options);
        if(!createStringArray(return_value, val TSRMLS_CC))
        {
            RETURN_NULL();
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Properties, parseIceCommandLineOptions)
{
    zval* opts;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("a!"), &opts) == FAILURE)
    {
        RETURN_NULL();
    }

    Ice::PropertiesPtr _this = Wrapper<Ice::PropertiesPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    Ice::StringSeq options;
    if(opts && !extractStringArray(opts, options TSRMLS_CC))
    {
        RETURN_NULL();
    }

    try
    {
        Ice::StringSeq val = _this->parseIceCommandLineOptions(options);
        if(!createStringArray(return_value, val TSRMLS_CC))
        {
            RETURN_NULL();
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Properties, load)
{
    char* f;
    int fLen;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("s"), &f, &fLen) == FAILURE)
    {
        RETURN_NULL();
    }

    Ice::PropertiesPtr _this = Wrapper<Ice::PropertiesPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    string file(f, fLen);

    try
    {
        _this->load(file);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Properties, clone)
{
    if(ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    Ice::PropertiesPtr _this = Wrapper<Ice::PropertiesPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    try
    {
        Ice::PropertiesPtr pclone = _this->clone();

        if(!createProperties(return_value, pclone TSRMLS_CC))
        {
            RETURN_NULL();
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

#ifdef _WIN32
extern "C"
#endif
static zend_object_value
handleAlloc(zend_class_entry* ce TSRMLS_DC)
{
    zend_object_value result;

    Wrapper<Ice::PropertiesPtr>* obj = Wrapper<Ice::PropertiesPtr>::create(ce TSRMLS_CC);
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
    Wrapper<Ice::PropertiesPtr>* obj = static_cast<Wrapper<Ice::PropertiesPtr>*>(p);
    delete obj->ptr;
    zend_object_std_dtor(static_cast<zend_object*>(p) TSRMLS_CC);
    efree(p);
}

#ifdef _WIN32
extern "C"
#endif
static zend_object_value
handleClone(zval* zv TSRMLS_DC)
{
    zend_object_value result;
    memset(&result, 0, sizeof(zend_object_value));

    Ice::PropertiesPtr p = Wrapper<Ice::PropertiesPtr>::value(zv TSRMLS_CC);
    assert(p);

    Ice::PropertiesPtr pclone = p->clone();

    zval* clone;
    MAKE_STD_ZVAL(clone);
    if(!createProperties(clone, pclone TSRMLS_CC))
    {
        return result;
    }

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

ZEND_FUNCTION(Ice_createProperties)
{
    zval* arglist = 0;
    zval* defaultsObj = 0;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("|a!O!"), &arglist, &defaultsObj,
                             propertiesClassEntry) == FAILURE)
    {
        RETURN_NULL();
    }

    Ice::StringSeq seq;
    if(arglist && !extractStringArray(arglist, seq TSRMLS_CC))
    {
        RETURN_NULL();
    }

    Ice::PropertiesPtr defaults;
    if(defaultsObj && !fetchProperties(defaultsObj, defaults TSRMLS_CC))
    {
        RETURN_NULL();
    }

    try
    {
        Ice::PropertiesPtr props;
        if(arglist || defaults)
        {
            props = Ice::createProperties(seq, defaults);
        }
        else
        {
            props = Ice::createProperties();
        }

        if(!createProperties(return_value, props TSRMLS_CC))
        {
            RETURN_NULL();
        }

        if(arglist && PZVAL_IS_REF(arglist))
        {
            zval_dtor(arglist);
            if(!createStringArray(arglist, seq TSRMLS_CC))
            {
                RETURN_NULL();
            }
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

//
// Necessary to suppress warnings from zend_function_entry in php-5.2.
//
#if defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wwrite-strings"
#endif

//
// Predefined methods for Properties.
//
static zend_function_entry _interfaceMethods[] =
{
    {0, 0, 0}
};
static zend_function_entry _classMethods[] =
{
    ZEND_ME(Ice_Properties, __construct, NULL, ZEND_ACC_PRIVATE|ZEND_ACC_CTOR)
    ZEND_ME(Ice_Properties, __toString, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Properties, getProperty, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Properties, getPropertyWithDefault, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Properties, getPropertyAsInt, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Properties, getPropertyAsIntWithDefault, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Properties, getPropertyAsList, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Properties, getPropertyAsListWithDefault, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Properties, getPropertiesForPrefix, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Properties, setProperty, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Properties, getCommandLineOptions, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Properties, parseCommandLineOptions, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Properties, parseIceCommandLineOptions, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Properties, load, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Properties, clone, NULL, ZEND_ACC_PUBLIC)
    {0, 0, 0}
};

//
// enable warning again
//
#if defined(__GNUC__)
#  pragma GCC diagnostic error "-Wwrite-strings"
#endif

bool
IcePHP::propertiesInit(TSRMLS_D)
{
    //
    // We register an interface and a class that implements the interface. This allows
    // applications to safely include the Slice-generated code for the type.
    //

    //
    // Register the Properties interface.
    //
    zend_class_entry ce;
#ifdef ICEPHP_USE_NAMESPACES
    INIT_NS_CLASS_ENTRY(ce, "Ice", "Properties", _interfaceMethods);
#else
    INIT_CLASS_ENTRY(ce, "Ice_Properties", _interfaceMethods);
#endif
    zend_class_entry* interface = zend_register_internal_interface(&ce TSRMLS_CC);

    //
    // Register the Properties class.
    //
    INIT_CLASS_ENTRY(ce, "IcePHP_Properties", _classMethods);
    ce.create_object = handleAlloc;
    propertiesClassEntry = zend_register_internal_class(&ce TSRMLS_CC);
    memcpy(&_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    _handlers.clone_obj = handleClone;
    zend_class_implements(propertiesClassEntry TSRMLS_CC, 1, interface);

    return true;
}

bool
IcePHP::createProperties(zval* zv, const Ice::PropertiesPtr& p TSRMLS_DC)
{
    if(object_init_ex(zv, propertiesClassEntry) != SUCCESS)
    {
        runtimeError("unable to initialize properties object" TSRMLS_CC);
        return false;
    }

    Wrapper<Ice::PropertiesPtr>* obj = Wrapper<Ice::PropertiesPtr>::extract(zv TSRMLS_CC);
    assert(!obj->ptr);
    obj->ptr = new Ice::PropertiesPtr(p);

    return true;
}

bool
IcePHP::fetchProperties(zval* zv, Ice::PropertiesPtr& p TSRMLS_DC)
{
    if(!ZVAL_IS_NULL(zv))
    {
        if(Z_TYPE_P(zv) != IS_OBJECT || Z_OBJCE_P(zv) != propertiesClassEntry)
        {
            invalidArgument("value is not a properties object" TSRMLS_CC);
            return false;
        }
        p = Wrapper<Ice::PropertiesPtr>::value(zv TSRMLS_CC);
        if(!p)
        {
            runtimeError("unable to retrieve properties object from object store" TSRMLS_CC);
            return false;
        }
    }
    return true;
}
