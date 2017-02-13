// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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
static zend_object* handleAlloc(zend_class_entry*);
static void handleFreeStorage(zend_object*);
static zend_object* handleClone(zval*);
}

ZEND_METHOD(Ice_Properties, __construct)
{
    runtimeError("properties objects cannot be instantiated, use createProperties()");
}

ZEND_METHOD(Ice_Properties, __toString)
{
    if(ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    Ice::PropertiesPtr _this = Wrapper<Ice::PropertiesPtr>::value(getThis());
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
        RETURN_STRINGL(STRCAST(str.c_str()), static_cast<int>(str.length()));
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Properties, getProperty)
{
    char* name;
    size_t nameLen;

    if(zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("s"), &name, &nameLen) == FAILURE)
    {
        RETURN_NULL();
    }

    Ice::PropertiesPtr _this = Wrapper<Ice::PropertiesPtr>::value(getThis());
    assert(_this);

    string propName(name, nameLen);
    try
    {
        string val = _this->getProperty(propName);
        RETURN_STRINGL(STRCAST(val.c_str()), static_cast<int>(val.length()));
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Properties, getPropertyWithDefault)
{
    char* name;
    size_t nameLen;
    char* def;
    size_t defLen;

    if(zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("ss!"), &name, &nameLen, &def, &defLen) ==
        FAILURE)
    {
        RETURN_NULL();
    }

    Ice::PropertiesPtr _this = Wrapper<Ice::PropertiesPtr>::value(getThis());
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
        RETURN_STRINGL(STRCAST(val.c_str()), static_cast<int>(val.length()));
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Properties, getPropertyAsInt)
{
    char* name;
    size_t nameLen;

    if(zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("s"), &name, &nameLen) == FAILURE)
    {
        RETURN_NULL();
    }

    Ice::PropertiesPtr _this = Wrapper<Ice::PropertiesPtr>::value(getThis());
    assert(_this);

    string propName(name, nameLen);
    try
    {
        Ice::Int val = _this->getPropertyAsInt(propName);
        RETURN_LONG(static_cast<long>(val));
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Properties, getPropertyAsIntWithDefault)
{
    char* name;
    size_t nameLen;
    long def;

    if(zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("sl"), &name, &nameLen, &def) == FAILURE)
    {
        RETURN_NULL();
    }

    Ice::PropertiesPtr _this = Wrapper<Ice::PropertiesPtr>::value(getThis());
    assert(_this);

    string propName(name, nameLen);
    try
    {
        Ice::Int val = _this->getPropertyAsIntWithDefault(propName, def);
        RETURN_LONG(static_cast<long>(val));
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Properties, getPropertyAsList)
{
    char* name;
    size_t nameLen;

    if(zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("s"), &name, &nameLen) == FAILURE)
    {
        RETURN_NULL();
    }

    Ice::PropertiesPtr _this = Wrapper<Ice::PropertiesPtr>::value(getThis());
    assert(_this);

    string propName(name, nameLen);
    try
    {
        Ice::StringSeq val = _this->getPropertyAsList(propName);
        if(!createStringArray(return_value, val))
        {
            RETURN_NULL();
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Properties, getPropertyAsListWithDefault)
{
    char* name;
    size_t nameLen;
    zval* def;

    if(zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("sa!"), &name, &nameLen, &def) == FAILURE)
    {
        RETURN_NULL();
    }

    Ice::PropertiesPtr _this = Wrapper<Ice::PropertiesPtr>::value(getThis());
    assert(_this);

    string propName(name, nameLen);
    Ice::StringSeq defaultValue;
    if(def && !extractStringArray(def, defaultValue))
    {
        RETURN_NULL();
    }

    try
    {
        Ice::StringSeq val = _this->getPropertyAsListWithDefault(propName, defaultValue);
        if(!createStringArray(return_value, val))
        {
            RETURN_NULL();
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Properties, getPropertiesForPrefix)
{
    char* p;
    size_t pLen;

    if(zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("s!"), &p, &pLen) == FAILURE)
    {
        RETURN_NULL();
    }

    Ice::PropertiesPtr _this = Wrapper<Ice::PropertiesPtr>::value(getThis());
    assert(_this);

    string prefix;
    if(p)
    {
        prefix = string(p, pLen);
    }

    try
    {
        Ice::PropertyDict val = _this->getPropertiesForPrefix(prefix);
        if(!createStringMap(return_value, val))
        {
            RETURN_NULL();
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Properties, setProperty)
{
    char* name;
    size_t nameLen;
    char* val;
    size_t valLen;

    if(zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("ss!"), &name, &nameLen, &val, &valLen) ==
        FAILURE)
    {
        RETURN_NULL();
    }

    Ice::PropertiesPtr _this = Wrapper<Ice::PropertiesPtr>::value(getThis());
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
        throwException(ex);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Properties, getCommandLineOptions)
{
    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }

    Ice::PropertiesPtr _this = Wrapper<Ice::PropertiesPtr>::value(getThis());
    assert(_this);

    try
    {
        Ice::StringSeq val = _this->getCommandLineOptions();
        if(!createStringArray(return_value, val))
        {
            RETURN_NULL();
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Properties, parseCommandLineOptions)
{
    char* p;
    size_t pLen;
    zval* opts;

    if(zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("s!a!"), &p, &pLen, &opts) == FAILURE)
    {
        RETURN_NULL();
    }

    Ice::PropertiesPtr _this = Wrapper<Ice::PropertiesPtr>::value(getThis());
    assert(_this);

    string prefix;
    if(p)
    {
        prefix = string(p, pLen);
    }
    Ice::StringSeq options;
    if(opts && !extractStringArray(opts, options))
    {
        RETURN_NULL();
    }

    try
    {
        Ice::StringSeq val = _this->parseCommandLineOptions(prefix, options);
        if(!createStringArray(return_value, val))
        {
            RETURN_NULL();
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Properties, parseIceCommandLineOptions)
{
    zval* opts;

    if(zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("a!"), &opts) == FAILURE)
    {
        RETURN_NULL();
    }

    Ice::PropertiesPtr _this = Wrapper<Ice::PropertiesPtr>::value(getThis());
    assert(_this);

    Ice::StringSeq options;
    if(opts && !extractStringArray(opts, options))
    {
        RETURN_NULL();
    }

    try
    {
        Ice::StringSeq val = _this->parseIceCommandLineOptions(options);
        if(!createStringArray(return_value, val))
        {
            RETURN_NULL();
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Properties, load)
{
    char* f;
    size_t fLen;

    if(zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("s"), &f, &fLen) == FAILURE)
    {
        RETURN_NULL();
    }

    Ice::PropertiesPtr _this = Wrapper<Ice::PropertiesPtr>::value(getThis());
    assert(_this);

    string file(f, fLen);

    try
    {
        _this->load(file);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Properties, clone)
{
    if(ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    Ice::PropertiesPtr _this = Wrapper<Ice::PropertiesPtr>::value(getThis());
    assert(_this);

    try
    {
        Ice::PropertiesPtr pclone = _this->clone();

        if(!createProperties(return_value, pclone))
        {
            RETURN_NULL();
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex);
        RETURN_NULL();
    }
}

#ifdef _WIN32
extern "C"
#endif
static zend_object*
handleAlloc(zend_class_entry* ce)
{
    Wrapper<Ice::PropertiesPtr>* obj = Wrapper<Ice::PropertiesPtr>::create(ce);
    assert(obj);

    obj->zobj.handlers = &_handlers;

    return &obj->zobj;
}

#ifdef _WIN32
extern "C"
#endif
static void
handleFreeStorage(zend_object* object)
{
    Wrapper<Ice::PropertiesPtr>* obj = Wrapper<Ice::PropertiesPtr>::fetch(object);
    delete obj->ptr;
    zend_object_std_dtor(object);
}

#ifdef _WIN32
extern "C"
#endif
static zend_object*
handleClone(zval* zv)
{
    Ice::PropertiesPtr p = Wrapper<Ice::PropertiesPtr>::value(zv);
    assert(p);

    Ice::PropertiesPtr pclone = p->clone();

    zval clone;
    if(!createProperties(&clone, pclone))
    {
        return 0;
    }

    return Z_OBJ(clone);
}

ZEND_FUNCTION(Ice_createProperties)
{
    zval* arglist = 0;
    zval* defaultsObj = 0;

    if(zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("|a!O!"), &arglist, &defaultsObj,
                             propertiesClassEntry) == FAILURE)
    {
        RETURN_NULL();
    }

    Ice::StringSeq seq;
    if(arglist && !extractStringArray(arglist, seq))
    {
        RETURN_NULL();
    }

    Ice::PropertiesPtr defaults;
    if(defaultsObj && !fetchProperties(defaultsObj, defaults))
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

        if(!createProperties(return_value, props))
        {
            RETURN_NULL();
        }

        if(arglist && Z_ISREF_P(arglist))
        {
            zval_dtor(arglist);
            if(!createStringArray(arglist, seq))
            {
                RETURN_NULL();
            }
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex);
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
    ZEND_ME(Ice_Properties, __construct, ICE_NULLPTR, ZEND_ACC_PRIVATE|ZEND_ACC_CTOR)
    ZEND_ME(Ice_Properties, __toString, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Properties, getProperty, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Properties, getPropertyWithDefault, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Properties, getPropertyAsInt, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Properties, getPropertyAsIntWithDefault, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Properties, getPropertyAsList, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Properties, getPropertyAsListWithDefault, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Properties, getPropertiesForPrefix, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Properties, setProperty, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Properties, getCommandLineOptions, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Properties, parseCommandLineOptions, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Properties, parseIceCommandLineOptions, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Properties, load, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Properties, clone, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    {0, 0, 0}
};

//
// enable warning again
//
#if defined(__GNUC__)
#  pragma GCC diagnostic error "-Wwrite-strings"
#endif

bool
IcePHP::propertiesInit(void)
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
    zend_class_entry* interface = zend_register_internal_interface(&ce);

    //
    // Register the Properties class.
    //
    INIT_CLASS_ENTRY(ce, "IcePHP_Properties", _classMethods);
    ce.create_object = handleAlloc;
    propertiesClassEntry = zend_register_internal_class(&ce);
    memcpy(&_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    _handlers.clone_obj = handleClone;
    _handlers.free_obj = handleFreeStorage;
    _handlers.offset = XtOffsetOf(Wrapper<Ice::PropertiesPtr>, zobj);
    zend_class_implements(propertiesClassEntry, 1, interface);

    return true;
}

bool
IcePHP::createProperties(zval* zv, const Ice::PropertiesPtr& p)
{
    if(object_init_ex(zv, propertiesClassEntry) != SUCCESS)
    {
        runtimeError("unable to initialize properties object");
        return false;
    }

    Wrapper<Ice::PropertiesPtr>* obj = Wrapper<Ice::PropertiesPtr>::extract(zv);
    assert(!obj->ptr);
    obj->ptr = new Ice::PropertiesPtr(p);

    return true;
}

bool
IcePHP::fetchProperties(zval* zv, Ice::PropertiesPtr& p)
{
    if(!ZVAL_IS_NULL(zv))
    {
        if(Z_TYPE_P(zv) != IS_OBJECT || Z_OBJCE_P(zv) != propertiesClassEntry)
        {
            invalidArgument("value is not a properties object");
            return false;
        }
        p = Wrapper<Ice::PropertiesPtr>::value(zv);
        if(!p)
        {
            runtimeError("unable to retrieve properties object from object store");
            return false;
        }
    }
    return true;
}
