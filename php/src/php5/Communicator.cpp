// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Communicator.h>
#include <Logger.h>
#include <Properties.h>
#include <Proxy.h>
#include <Types.h>
#include <Util.h>
#include <IceUtil/DisableWarnings.h>
#include <IceUtil/Options.h>
#include <IceUtil/MutexPtrLock.h>
#include <IceUtil/StringUtil.h>
#include <IceUtil/Timer.h>

using namespace std;
using namespace IcePHP;

ZEND_EXTERN_MODULE_GLOBALS(ice)

//
// Class entries represent the PHP class implementations we have registered.
//
namespace IcePHP
{

zend_class_entry* communicatorClassEntry = 0;
zend_class_entry* valueFactoryManagerClassEntry = 0;

//
// An active communicator is in use by at least one request and may have
// registered so that it remains active after a request completes. The
// communicator is destroyed when there are no more references to this
// object.
//
class ActiveCommunicator : public IceUtil::Shared
{
public:

    ActiveCommunicator(const Ice::CommunicatorPtr& c);
    ~ActiveCommunicator();

    const Ice::CommunicatorPtr communicator;
    vector<string> ids;
    int expires;
    IceUtil::Time lastAccess;
};
typedef IceUtil::Handle<ActiveCommunicator> ActiveCommunicatorPtr;

class FactoryWrapper;
typedef IceUtil::Handle<FactoryWrapper> FactoryWrapperPtr;

class DefaultValueFactory;
typedef IceUtil::Handle<DefaultValueFactory> DefaultValueFactoryPtr;

//
// CommunicatorInfoI encapsulates communicator-related information that
// is specific to a PHP "request". In other words, multiple PHP requests
// might share the same communicator instance but still need separate
// workspaces. For example, we don't want the value factories installed
// by one request to influence the behavior of another request.
//
class CommunicatorInfoI : public CommunicatorInfo
{
public:

    CommunicatorInfoI(const ActiveCommunicatorPtr&, zval*);

    virtual void getZval(zval* TSRMLS_DC);
    virtual void addRef(TSRMLS_D);
    virtual void decRef(TSRMLS_D);

    virtual Ice::CommunicatorPtr getCommunicator() const;

    bool addFactory(zval*, const string&, bool TSRMLS_DC);
    FactoryWrapperPtr findFactory(const string&) const;
    Ice::ValueFactoryPtr defaultFactory() const { return _defaultFactory; }
    void destroyFactories(TSRMLS_D);

    const ActiveCommunicatorPtr ac;
    const zval zv;

private:

    typedef map<string, FactoryWrapperPtr> FactoryMap;

    FactoryMap _factories;
    DefaultValueFactoryPtr _defaultFactory;
};
typedef IceUtil::Handle<CommunicatorInfoI> CommunicatorInfoIPtr;

//
// Wraps a PHP object/value factory.
//
class FactoryWrapper : public Ice::ValueFactory
{
public:

    FactoryWrapper(zval*, bool, const CommunicatorInfoIPtr&);

    virtual Ice::ValuePtr create(const string&);

    void getZval(zval* TSRMLS_DC);

    bool isObjectFactory() const;

    void destroy(TSRMLS_D);

protected:

    zval* _factory;
    bool _isObjectFactory;
    CommunicatorInfoIPtr _info;
};

//
// Implements the default value factory behavior.
//
class DefaultValueFactory : public Ice::ValueFactory
{
public:

    DefaultValueFactory(const CommunicatorInfoIPtr&);

    virtual Ice::ValuePtr create(const string&);

    void setDelegate(const FactoryWrapperPtr& d) { _delegate = d; }
    FactoryWrapperPtr getDelegate() const { return _delegate; }

    void destroy(TSRMLS_D);

private:

    FactoryWrapperPtr _delegate;
    CommunicatorInfoIPtr _info;
};

//
// Each PHP request has its own set of value factories. More precisely, there is
// a value factory map for each communicator that is created by a PHP request.
// (see CommunicatorInfoI).
//
// We define a custom value factory manager implementation that delegates to
// to PHP objects supplied by the application.
//
// An instance of this class is installed as the communicator's value factory
// manager, and the class holds a reference to its communicator. When find() is
// invoked, the class resolves the appropriate factory as follows:
//
//  * Using its communicator reference as the key, look up the corresponding
//    CommunicatorInfoI object in the request-specific communicator map.
//
//  * If the type-id is empty, return the default factory. This factory will
//    either delegate to an application-supplied default factory (if present) or
//    default-construct an instance of a concrete Slice class type.
//
//  * For non-empty type-ids, return a wrapper around the application-supplied
//    factory, if any.
//
class ValueFactoryManager : public Ice::ValueFactoryManager
{
public:

    virtual void add(const Ice::ValueFactoryPtr&, const string&);
    virtual Ice::ValueFactoryPtr find(const string&) const;

    void setCommunicator(const Ice::CommunicatorPtr& c) { _communicator = c; }
    Ice::CommunicatorPtr getCommunicator() const { return _communicator; }

    void getZval(zval* TSRMLS_DC);

    void destroy();

private:

    Ice::CommunicatorPtr _communicator;
};
typedef IceUtil::Handle<ValueFactoryManager> ValueFactoryManagerPtr;

class ReaperTask : public IceUtil::TimerTask
{
public:

    virtual void runTimerTask();
};

}

namespace
{
//
// Communicator support.
//
zend_object_handlers _handlers;

//
// ValueFactoryManager support.
//
zend_object_handlers _vfmHandlers;

//
// The profile map holds Properties objects corresponding to the "default" profile
// (defined via the ice.config & ice.options settings in php.ini) as well as named
// profiles defined in an external file.
//
typedef map<string, Ice::PropertiesPtr> ProfileMap;
ProfileMap _profiles;
const string _defaultProfileName = "";

//
// This map represents communicators that have been registered so that they can be used
// by multiple PHP requests.
//
typedef map<string, ActiveCommunicatorPtr> RegisteredCommunicatorMap;
RegisteredCommunicatorMap _registeredCommunicators;
IceUtil::Mutex* _registeredCommunicatorsMutex = 0;

IceUtil::TimerPtr _timer;

//
// This map is stored in the "global" variables for each PHP request and holds
// the communicators that have been created (or registered communicators that have
// been used) by the request.
//
typedef map<Ice::CommunicatorPtr, CommunicatorInfoIPtr> CommunicatorMap;

class Init
{
public:

    Init()
    {
        _registeredCommunicatorsMutex = new IceUtil::Mutex();
    }

    ~Init()
    {
        delete _registeredCommunicatorsMutex;
        _registeredCommunicatorsMutex = 0;
    }
};

Init init;
}

extern "C"
{
static zend_object_value handleAlloc(zend_class_entry* TSRMLS_DC);
static void handleFreeStorage(void* TSRMLS_DC);
static zend_object_value handleClone(zval* TSRMLS_DC);

static zend_object_value handleVfmAlloc(zend_class_entry* TSRMLS_DC);
static void handleVfmFreeStorage(void* TSRMLS_DC);
static zend_object_value handleVfmClone(zval* TSRMLS_DC);
}

ZEND_METHOD(Ice_Communicator, __construct)
{
    runtimeError("communicators cannot be instantiated directly" TSRMLS_CC);
}

ZEND_METHOD(Ice_Communicator, destroy)
{
    CommunicatorInfoIPtr _this = Wrapper<CommunicatorInfoIPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    //
    // Remove all registrations.
    //
    {
        IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(_registeredCommunicatorsMutex);
        for(vector<string>::iterator p = _this->ac->ids.begin(); p != _this->ac->ids.end(); ++p)
        {
            _registeredCommunicators.erase(*p);
        }
        _this->ac->ids.clear();
    }

    //
    // We need to destroy any object|value factories installed by this request.
    //
    _this->destroyFactories(TSRMLS_C);

    Ice::CommunicatorPtr c = _this->getCommunicator();
    assert(c);
    CommunicatorMap* m = reinterpret_cast<CommunicatorMap*>(ICE_G(communicatorMap));
    assert(m);
    assert(m->find(c) != m->end());
    m->erase(c);

    ValueFactoryManagerPtr vfm = ValueFactoryManagerPtr::dynamicCast(c->getValueFactoryManager());
    assert(vfm);
    vfm->destroy();

    try
    {
        c->destroy();
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Communicator, stringToProxy)
{
    CommunicatorInfoIPtr _this = Wrapper<CommunicatorInfoIPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    char* str;
    int strLen;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("s"), &str, &strLen) != SUCCESS)
    {
        RETURN_NULL();
    }
    string s(str, strLen);

    try
    {
        Ice::ObjectPrx prx = _this->getCommunicator()->stringToProxy(s);
        if(!createProxy(return_value, prx, _this TSRMLS_CC))
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

ZEND_METHOD(Ice_Communicator, proxyToString)
{
    CommunicatorInfoIPtr _this = Wrapper<CommunicatorInfoIPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    zval* zv;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("O!"), &zv, proxyClassEntry) != SUCCESS)
    {
        RETURN_NULL();
    }

    try
    {
        string str;
        if(zv)
        {
            Ice::ObjectPrx prx;
            ProxyInfoPtr info;
            if(!fetchProxy(zv, prx, info TSRMLS_CC))
            {
                RETURN_NULL();
            }
            assert(prx);
            str = prx->ice_toString();
        }
        RETURN_STRINGL(STRCAST(str.c_str()), static_cast<int>(str.length()), 1);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Communicator, propertyToProxy)
{
    CommunicatorInfoIPtr _this = Wrapper<CommunicatorInfoIPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    char* str;
    int strLen;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("s"), &str, &strLen) != SUCCESS)
    {
        RETURN_NULL();
    }
    string s(str, strLen);

    try
    {
        Ice::ObjectPrx prx = _this->getCommunicator()->propertyToProxy(s);
        if(!createProxy(return_value, prx, _this TSRMLS_CC))
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

ZEND_METHOD(Ice_Communicator, proxyToProperty)
{
    CommunicatorInfoIPtr _this = Wrapper<CommunicatorInfoIPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    zval* zv;
    char* str;
    int strLen;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("O!s"), &zv, proxyClassEntry, &str, &strLen)
        != SUCCESS)
    {
        RETURN_NULL();
    }

    string prefix(str, strLen);

    try
    {
        if(zv)
        {
            Ice::ObjectPrx prx;
            ProxyInfoPtr info;
            if(!fetchProxy(zv, prx, info TSRMLS_CC))
            {
                RETURN_NULL();
            }
            assert(prx);

            Ice::PropertyDict val = _this->getCommunicator()->proxyToProperty(prx, prefix);
            if(!createStringMap(return_value, val TSRMLS_CC))
            {
                RETURN_NULL();
            }
        }
        else
        {
            array_init(return_value);
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Communicator, stringToIdentity)
{
    CommunicatorInfoIPtr _this = Wrapper<CommunicatorInfoIPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    char* str;
    int strLen;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("s"), &str, &strLen) != SUCCESS)
    {
        RETURN_NULL();
    }
    string s(str, strLen);

    try
    {
        Ice::Identity id = _this->getCommunicator()->stringToIdentity(s);
        if(!createIdentity(return_value, id TSRMLS_CC))
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

ZEND_METHOD(Ice_Communicator, identityToString)
{
    CommunicatorInfoIPtr _this = Wrapper<CommunicatorInfoIPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    zend_class_entry* identityClass = idToClass("::Ice::Identity" TSRMLS_CC);
    assert(identityClass);

    zval* zv;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("O"), &zv, identityClass) != SUCCESS)
    {
        RETURN_NULL();
    }
    Ice::Identity id;
    if(!extractIdentity(zv, id TSRMLS_CC))
    {
        RETURN_NULL();
    }

    try
    {
        string str = _this->getCommunicator()->identityToString(id);
        RETURN_STRINGL(STRCAST(str.c_str()), static_cast<int>(str.length()), 1);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Communicator, addObjectFactory)
{
    CommunicatorInfoIPtr _this = Wrapper<CommunicatorInfoIPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    zend_class_entry* factoryClass = idToClass("Ice::ObjectFactory" TSRMLS_CC);
    assert(factoryClass);

    zval* factory;
    char* id;
    int idLen;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("Os!"), &factory, factoryClass, &id,
                             &idLen TSRMLS_CC) != SUCCESS)
    {
        RETURN_NULL();
    }

    string type;
    if(id)
    {
        type = string(id, idLen);
    }

    if(!_this->addFactory(factory, type, true TSRMLS_CC))
    {
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Communicator, findObjectFactory)
{
    CommunicatorInfoIPtr _this = Wrapper<CommunicatorInfoIPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    char* id;
    int idLen;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("s!"), &id, &idLen TSRMLS_CC) != SUCCESS)
    {
        RETURN_NULL();
    }

    string type;
    if(id)
    {
        type = string(id, idLen);
    }

    FactoryWrapperPtr w = _this->findFactory(type);
    if(w && w->isObjectFactory())
    {
        w->getZval(return_value TSRMLS_CC);
    }
    else
    {
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Communicator, getValueFactoryManager)
{
    if(ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    CommunicatorInfoIPtr _this = Wrapper<CommunicatorInfoIPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    try
    {
        ValueFactoryManagerPtr vfm =
            ValueFactoryManagerPtr::dynamicCast(_this->getCommunicator()->getValueFactoryManager());
        assert(vfm);
        if(object_init_ex(return_value, valueFactoryManagerClassEntry) != SUCCESS)
        {
            runtimeError("unable to initialize properties object" TSRMLS_CC);
            RETURN_NULL();
        }

        Wrapper<ValueFactoryManagerPtr>* obj = Wrapper<ValueFactoryManagerPtr>::extract(return_value TSRMLS_CC);
        assert(!obj->ptr);
        obj->ptr = new ValueFactoryManagerPtr(vfm);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Communicator, getImplicitContext)
{
    runtimeError("not implemented" TSRMLS_CC);
}

ZEND_METHOD(Ice_Communicator, getProperties)
{
    if(ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    CommunicatorInfoIPtr _this = Wrapper<CommunicatorInfoIPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    try
    {
        Ice::PropertiesPtr props = _this->getCommunicator()->getProperties();
        if(!createProperties(return_value, props TSRMLS_CC))
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

ZEND_METHOD(Ice_Communicator, getLogger)
{
    if(ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    CommunicatorInfoIPtr _this = Wrapper<CommunicatorInfoIPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    try
    {
        Ice::LoggerPtr logger = _this->getCommunicator()->getLogger();
        if(!createLogger(return_value, logger TSRMLS_CC))
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

ZEND_METHOD(Ice_Communicator, getDefaultRouter)
{
    if(ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    CommunicatorInfoIPtr _this = Wrapper<CommunicatorInfoIPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    try
    {
        Ice::RouterPrx router = _this->getCommunicator()->getDefaultRouter();
        if(router)
        {
            ProxyInfoPtr info = getProxyInfo("::Ice::Router" TSRMLS_CC);
            if(!info)
            {
                runtimeError("no definition for Ice::Router" TSRMLS_CC);
                RETURN_NULL();
            }
            if(!createProxy(return_value, router, info, _this TSRMLS_CC))
            {
                RETURN_NULL();
            }
        }
        else
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

ZEND_METHOD(Ice_Communicator, setDefaultRouter)
{
    CommunicatorInfoIPtr _this = Wrapper<CommunicatorInfoIPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    zval* zv;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("O!"), &zv, proxyClassEntry TSRMLS_CC) !=
        SUCCESS)
    {
        RETURN_NULL();
    }

    Ice::ObjectPrx proxy;
    ProxyInfoPtr info;
    if(zv && !fetchProxy(zv, proxy, info TSRMLS_CC))
    {
        RETURN_NULL();
    }

    try
    {
        Ice::RouterPrx router;
        if(proxy)
        {
            if(!info || !info->isA("::Ice::Router"))
            {
                invalidArgument("setDefaultRouter requires a proxy narrowed to Ice::Router" TSRMLS_CC);
                RETURN_NULL();
            }
            router = Ice::RouterPrx::uncheckedCast(proxy);
        }
        _this->getCommunicator()->setDefaultRouter(router);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Communicator, getDefaultLocator)
{
    if(ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    CommunicatorInfoIPtr _this = Wrapper<CommunicatorInfoIPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    try
    {
        Ice::LocatorPrx locator = _this->getCommunicator()->getDefaultLocator();
        if(locator)
        {
            ProxyInfoPtr info = getProxyInfo("::Ice::Locator" TSRMLS_CC);
            if(!info)
            {
                runtimeError("no definition for Ice::Locator" TSRMLS_CC);
                RETURN_NULL();
            }
            if(!createProxy(return_value, locator, info, _this TSRMLS_CC))
            {
                RETURN_NULL();
            }
        }
        else
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

ZEND_METHOD(Ice_Communicator, setDefaultLocator)
{
    CommunicatorInfoIPtr _this = Wrapper<CommunicatorInfoIPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    zval* zv;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("O!"), &zv, proxyClassEntry TSRMLS_CC) !=
        SUCCESS)
    {
        RETURN_NULL();
    }

    Ice::ObjectPrx proxy;
    ProxyInfoPtr info;
    if(zv && !fetchProxy(zv, proxy, info TSRMLS_CC))
    {
        RETURN_NULL();
    }

    try
    {
        Ice::LocatorPrx locator;
        if(proxy)
        {
            if(!info || !info->isA("::Ice::Locator"))
            {
                invalidArgument("setDefaultLocator requires a proxy narrowed to Ice::Locator" TSRMLS_CC);
                RETURN_NULL();
            }
            locator = Ice::LocatorPrx::uncheckedCast(proxy);
        }
        _this->getCommunicator()->setDefaultLocator(locator);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Communicator, flushBatchRequests)
{
    zval* compress;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("z"), &compress TSRMLS_CC) != SUCCESS)
    {
        RETURN_NULL();
    }

    if(Z_TYPE_P(compress) != IS_LONG)
    {
        invalidArgument("value for 'compress' argument must be an enumerator of CompressBatch" TSRMLS_CC);
        RETURN_NULL();
    }
    Ice::CompressBatch cb = static_cast<Ice::CompressBatch>(Z_LVAL_P(compress));

    CommunicatorInfoIPtr _this = Wrapper<CommunicatorInfoIPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    try
    {
        _this->getCommunicator()->flushBatchRequests(cb);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_ValueFactoryManager, __construct)
{
    runtimeError("value factory managers cannot be instantiated directly" TSRMLS_CC);
}

ZEND_METHOD(Ice_ValueFactoryManager, add)
{
    ValueFactoryManagerPtr _this = Wrapper<ValueFactoryManagerPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    zend_class_entry* factoryClass = idToClass("Ice::ValueFactory" TSRMLS_CC);
    assert(factoryClass);

    zval* factory;
    char* id;
    int idLen;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("Os!"), &factory, factoryClass, &id,
                             &idLen TSRMLS_CC) != SUCCESS)
    {
        RETURN_NULL();
    }

    string type;
    if(id)
    {
        type = string(id, idLen);
    }

    CommunicatorMap* m = static_cast<CommunicatorMap*>(ICE_G(communicatorMap));
    assert(m);
    CommunicatorMap::iterator p = m->find(_this->getCommunicator());
    assert(p != m->end());

    CommunicatorInfoIPtr info = p->second;

    if(!info->addFactory(factory, type, false TSRMLS_CC))
    {
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_ValueFactoryManager, find)
{
    ValueFactoryManagerPtr _this = Wrapper<ValueFactoryManagerPtr>::value(getThis() TSRMLS_CC);
    assert(_this);

    char* id;
    int idLen;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("s!"), &id, &idLen TSRMLS_CC) != SUCCESS)
    {
        RETURN_NULL();
    }

    string type;
    if(id)
    {
        type = string(id, idLen);
    }

    CommunicatorMap* m = static_cast<CommunicatorMap*>(ICE_G(communicatorMap));
    assert(m);
    CommunicatorMap::iterator p = m->find(_this->getCommunicator());
    assert(p != m->end());

    CommunicatorInfoIPtr info = p->second;

    FactoryWrapperPtr w = info->findFactory(type);
    if(w)
    {
        w->getZval(return_value TSRMLS_CC);
    }
    else
    {
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

    Wrapper<CommunicatorInfoIPtr>* obj = Wrapper<CommunicatorInfoIPtr>::create(ce TSRMLS_CC);
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
    Wrapper<CommunicatorInfoIPtr>* obj = static_cast<Wrapper<CommunicatorInfoIPtr>*>(p);
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
    php_error_docref(0 TSRMLS_CC, E_ERROR, "communicators cannot be cloned");
    return zend_object_value();
}

#ifdef _WIN32
extern "C"
#endif
static zend_object_value
handleVfmAlloc(zend_class_entry* ce TSRMLS_DC)
{
    zend_object_value result;

    Wrapper<ValueFactoryManagerPtr>* obj = Wrapper<ValueFactoryManagerPtr>::create(ce TSRMLS_CC);
    assert(obj);

    result.handle =
        zend_objects_store_put(obj, 0, (zend_objects_free_object_storage_t)handleVfmFreeStorage, 0 TSRMLS_CC);
    result.handlers = &_vfmHandlers;

    return result;
}

#ifdef _WIN32
extern "C"
#endif
static void
handleVfmFreeStorage(void* p TSRMLS_DC)
{
    Wrapper<ValueFactoryManagerPtr>* obj = static_cast<Wrapper<ValueFactoryManagerPtr>*>(p);
    delete obj->ptr;
    zend_object_std_dtor(static_cast<zend_object*>(p) TSRMLS_CC);
    efree(p);
}

#ifdef _WIN32
extern "C"
#endif
static zend_object_value
handleVfmClone(zval* zv TSRMLS_DC)
{
    php_error_docref(0 TSRMLS_CC, E_ERROR, "value factory managers cannot be cloned");
    return zend_object_value();
}

static CommunicatorInfoIPtr
createCommunicator(zval* zv, const ActiveCommunicatorPtr& ac TSRMLS_DC)
{
    try
    {
        if(object_init_ex(zv, communicatorClassEntry) != SUCCESS)
        {
            runtimeError("unable to initialize communicator object" TSRMLS_CC);
            return 0;
        }

        Wrapper<CommunicatorInfoIPtr>* obj = Wrapper<CommunicatorInfoIPtr>::extract(zv TSRMLS_CC);
        assert(!obj->ptr);

        CommunicatorInfoIPtr info = new CommunicatorInfoI(ac, zv);
        obj->ptr = new CommunicatorInfoIPtr(info);

        CommunicatorMap* m;
        if(ICE_G(communicatorMap))
        {
            m = reinterpret_cast<CommunicatorMap*>(ICE_G(communicatorMap));
        }
        else
        {
            m = new CommunicatorMap;
            ICE_G(communicatorMap) = m;
        }
        m->insert(CommunicatorMap::value_type(ac->communicator, info));

        return info;
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        return 0;
    }
}

static CommunicatorInfoIPtr
initializeCommunicator(zval* zv, Ice::StringSeq& args, bool hasArgs, const Ice::InitializationData& initData TSRMLS_DC)
{
    try
    {
        Ice::CommunicatorPtr c;
        if(hasArgs)
        {
            c = Ice::initialize(args, initData);
        }
        else
        {
            c = Ice::initialize(initData);
        }

        ActiveCommunicatorPtr ac = new ActiveCommunicator(c);

        ValueFactoryManagerPtr vfm = ValueFactoryManagerPtr::dynamicCast(c->getValueFactoryManager());
        assert(vfm);
        vfm->setCommunicator(c);

        CommunicatorInfoIPtr info = createCommunicator(zv, ac TSRMLS_CC);
        if(!info)
        {
            try
            {
                c->destroy();
            }
            catch(...)
            {
            }

            vfm->destroy();
        }

        return info;
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        return 0;
    }
}

ZEND_FUNCTION(Ice_initialize)
{
    if(ZEND_NUM_ARGS() > 2)
    {
        runtimeError("too many arguments" TSRMLS_CC);
        RETURN_NULL();
    }

    zend_class_entry* initClass = idToClass("::Ice::InitializationData" TSRMLS_CC);
    assert(initClass);

    //
    // Retrieve the arguments.
    //
    zval*** args = static_cast<zval***>(emalloc(ZEND_NUM_ARGS() * sizeof(zval**)));
    AutoEfree autoArgs(args); // Call efree on return
    if(zend_get_parameters_array_ex(ZEND_NUM_ARGS(), args) == FAILURE)
    {
        runtimeError("unable to get arguments" TSRMLS_CC);
        RETURN_NULL();
    }

    Ice::StringSeq seq;
    Ice::InitializationData initData;
    zval* zvargs = 0;
    zval* zvinit = 0;

    //
    // Accept the following invocations:
    //
    // initialize(array, InitializationData)
    // initialize(array)
    // initialize(InitializationData)
    // initialize()
    //
    bool hasArgs = false;
    if(ZEND_NUM_ARGS())
    {
        if(Z_TYPE_PP(args[0]) == IS_ARRAY)
        {
            if(!extractStringArray(*args[0], seq TSRMLS_CC))
            {
                RETURN_NULL();
            }
            zvargs = *args[0];
            hasArgs = true;
            if(ZEND_NUM_ARGS() > 1)
            {
                if(Z_TYPE_PP(args[1]) != IS_OBJECT || Z_OBJCE_PP(args[1]) != initClass)
                {
                    string s = zendTypeToString(Z_TYPE_PP(args[1]));
                    invalidArgument("expected InitializationData object but received %s" TSRMLS_CC, s.c_str());
                    RETURN_NULL();
                }
                zvinit = *args[1];
            }
        }
        else if(Z_TYPE_PP(args[0]) == IS_OBJECT && Z_OBJCE_PP(args[0]) == initClass)
        {
            if(ZEND_NUM_ARGS() > 1)
            {
                runtimeError("too many arguments" TSRMLS_CC);
                RETURN_NULL();
            }
            zvinit = *args[0];
        }
        else
        {
            string s = zendTypeToString(Z_TYPE_PP(args[0]));
            invalidArgument("unexpected argument type %s" TSRMLS_CC, s.c_str());
            RETURN_NULL();
        }
    }

    if(zvinit)
    {
        void* data;
        string member;

        member = "properties";
        if(zend_hash_find(Z_OBJPROP_P(zvinit), STRCAST(member.c_str()), static_cast<uint>(member.size() + 1), &data)
            == SUCCESS)
        {
            zval** val = reinterpret_cast<zval**>(data);
            if(!fetchProperties(*val, initData.properties TSRMLS_CC))
            {
                RETURN_NULL();
            }
        }

        member = "logger";
        if(zend_hash_find(Z_OBJPROP_P(zvinit), STRCAST(member.c_str()), static_cast<uint>(member.size() + 1), &data)
            == SUCCESS)
        {
            zval** val = reinterpret_cast<zval**>(data);
            if(!fetchLogger(*val, initData.logger TSRMLS_CC))
            {
                RETURN_NULL();
            }
        }
    }

    initData.compactIdResolver = new IdResolver(TSRMLS_C);
    initData.valueFactoryManager = new ValueFactoryManager;

    CommunicatorInfoIPtr info = initializeCommunicator(return_value, seq, hasArgs, initData TSRMLS_CC);
    if(!info)
    {
        RETURN_NULL();
    }

    if(zvargs && PZVAL_IS_REF(zvargs))
    {
        zval_dtor(zvargs);
        if(!createStringArray(zvargs, seq TSRMLS_CC))
        {
            RETURN_NULL();
        }
    }
}

ZEND_FUNCTION(Ice_register)
{
    zval* comm;
    char* s;
    int sLen;
    long expires = 0;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("Os|l"), &comm, communicatorClassEntry, &s,
                             &sLen, &expires TSRMLS_CC) != SUCCESS)
    {
        RETURN_NULL();
    }

    string id(s, sLen);
    if(id.empty())
    {
        invalidArgument("communicator id cannot be empty" TSRMLS_CC);
        RETURN_NULL();
    }

    CommunicatorInfoIPtr info = Wrapper<CommunicatorInfoIPtr>::value(comm TSRMLS_CC);
    assert(info);

    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(_registeredCommunicatorsMutex);

    RegisteredCommunicatorMap::iterator p = _registeredCommunicators.find(id);
    if(p != _registeredCommunicators.end())
    {
        if(p->second->communicator != info->getCommunicator())
        {
            //
            // A different communicator is already registered with that ID.
            //
            RETURN_FALSE;
        }
    }
    else
    {
        info->ac->ids.push_back(id);
        _registeredCommunicators[id] = info->ac;
    }

    if(expires > 0)
    {
        //
        // Update the expiration time. If a communicator is registered with multiple IDs, we
        // always use the most recent expiration setting.
        //
        info->ac->expires = static_cast<int>(expires);
        info->ac->lastAccess = IceUtil::Time::now();

        //
        // Start the timer if necessary. Reap expired communicators every five minutes.
        //
        if(!_timer)
        {
            _timer = new IceUtil::Timer;
            _timer->scheduleRepeated(new ReaperTask, IceUtil::Time::seconds(5 * 60));
        }
    }

    RETURN_TRUE;
}

ZEND_FUNCTION(Ice_unregister)
{
    char* s;
    int sLen;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("s"), &s, &sLen TSRMLS_CC) != SUCCESS)
    {
        RETURN_NULL();
    }

    string id(s, sLen);

    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(_registeredCommunicatorsMutex);

    RegisteredCommunicatorMap::iterator p = _registeredCommunicators.find(id);
    if(p == _registeredCommunicators.end())
    {
        //
        // No communicator registered with that ID.
        //
        RETURN_FALSE;
    }

    //
    // Remove the ID from the ActiveCommunicator's list of registered IDs.
    //
    ActiveCommunicatorPtr ac = p->second;
    vector<string>::iterator q = find(ac->ids.begin(), ac->ids.end(), id);
    assert(q != ac->ids.end());
    ac->ids.erase(q);

    _registeredCommunicators.erase(p);

    RETURN_TRUE;
}

ZEND_FUNCTION(Ice_find)
{
    char* s;
    int sLen;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("s"), &s, &sLen TSRMLS_CC) != SUCCESS)
    {
        RETURN_NULL();
    }

    string id(s, sLen);

    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(_registeredCommunicatorsMutex);

    RegisteredCommunicatorMap::iterator p = _registeredCommunicators.find(id);
    if(p == _registeredCommunicators.end())
    {
        //
        // No communicator registered with that ID.
        //
        RETURN_NULL();
    }

    if(p->second->expires > 0)
    {
        p->second->lastAccess = IceUtil::Time::now();
    }

    //
    // Check if this communicator has already been obtained by the current request.
    // If so, we can return the existing PHP object that corresponds to the communicator.
    //
    CommunicatorMap* m = reinterpret_cast<CommunicatorMap*>(ICE_G(communicatorMap));
    if(m)
    {
        CommunicatorMap::iterator q = m->find(p->second->communicator);
        if(q != m->end())
        {
            q->second->getZval(return_value TSRMLS_CC);
            return;
        }
    }

    if(!createCommunicator(return_value, p->second TSRMLS_CC))
    {
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_getProperties)
{
    char* s = 0;
    int sLen;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("|s"), &s, &sLen TSRMLS_CC) != SUCCESS)
    {
        RETURN_NULL();
    }

    string name;
    if(s)
    {
        name = string(s, sLen);
    }

    ProfileMap::iterator p = _profiles.find(name);
    if(p == _profiles.end())
    {
        RETURN_NULL();
    }

    Ice::PropertiesPtr clone = p->second->clone();
    if(!createProperties(return_value, clone TSRMLS_CC))
    {
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_identityToString)
{
    zend_class_entry* identityClass = idToClass("::Ice::Identity" TSRMLS_CC);
    assert(identityClass);

    zval* zv;
    long mode = 0;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("O|l"), &zv, identityClass, &mode TSRMLS_CC) != SUCCESS)
    {
        RETURN_NULL();
    }
    Ice::Identity id;
    if(!extractIdentity(zv, id TSRMLS_CC))
    {
        RETURN_NULL();
    }

    try
    {
        string str = identityToString(id, static_cast<Ice::ToStringMode>(mode));
        RETURN_STRINGL(STRCAST(str.c_str()), static_cast<int>(str.length()), 1);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex TSRMLS_CC);
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_stringToIdentity)
{
    char* str;
    int strLen;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("s"), &str, &strLen) != SUCCESS)
    {
        RETURN_NULL();
    }
    string s(str, strLen);

    try
    {
        Ice::Identity id = Ice::stringToIdentity(s);
        if(!createIdentity(return_value, id TSRMLS_CC))
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

//
// Necessary to suppress warnings from zend_function_entry in php-5.2
// and INI_STR macro.
//
#ifdef __GNUC__
#   pragma GCC diagnostic ignored "-Wwrite-strings"
#endif

//
// Predefined methods for Communicator.
//
static zend_function_entry _interfaceMethods[] =
{
    {0, 0, 0}
};
static zend_function_entry _classMethods[] =
{
    ZEND_ME(Ice_Communicator, __construct, ICE_NULLPTR, ZEND_ACC_PRIVATE|ZEND_ACC_CTOR)
    ZEND_ME(Ice_Communicator, destroy, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Communicator, stringToProxy, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Communicator, proxyToString, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Communicator, propertyToProxy, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Communicator, proxyToProperty, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Communicator, stringToIdentity, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Communicator, identityToString, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Communicator, addObjectFactory, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Communicator, findObjectFactory, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Communicator, getValueFactoryManager, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Communicator, getImplicitContext, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Communicator, getProperties, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Communicator, getLogger, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Communicator, getDefaultRouter, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Communicator, setDefaultRouter, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Communicator, getDefaultLocator, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Communicator, setDefaultLocator, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Communicator, flushBatchRequests, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    {0, 0, 0}
};

//
// Predefined methods for ValueFactoryManager.
//
static zend_function_entry _vfmInterfaceMethods[] =
{
    {0, 0, 0}
};
static zend_function_entry _vfmClassMethods[] =
{
    ZEND_ME(Ice_ValueFactoryManager, __construct, ICE_NULLPTR, ZEND_ACC_PRIVATE|ZEND_ACC_CTOR)
    ZEND_ME(Ice_ValueFactoryManager, add, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_ValueFactoryManager, find, ICE_NULLPTR, ZEND_ACC_PUBLIC)
    {0, 0, 0}
};

static bool
createProfile(const string& name, const string& config, const string& options TSRMLS_DC)
{
    ProfileMap::iterator p = _profiles.find(name);
    if(p != _profiles.end())
    {
        php_error_docref(0 TSRMLS_CC, E_WARNING, "duplicate Ice profile `%s'", name.c_str());
        return false;
    }

    Ice::PropertiesPtr properties = Ice::createProperties();

    if(!config.empty())
    {
        try
        {
            properties->load(config);
        }
        catch(const IceUtil::Exception& ex)
        {
            ostringstream ostr;
            ex.ice_print(ostr);
            php_error_docref(0 TSRMLS_CC, E_WARNING, "unable to load Ice configuration file %s:\n%s", config.c_str(),
                             ostr.str().c_str());
            return false;
        }
    }

    if(!options.empty())
    {
        vector<string> args;
        try
        {
            args = IceUtilInternal::Options::split(options);
        }
        catch(const IceUtil::Exception& ex)
        {
            ostringstream ostr;
            ex.ice_print(ostr);
            string msg = ostr.str();
            php_error_docref(0 TSRMLS_CC, E_WARNING, "error occurred while parsing the options `%s':\n%s",
                             options.c_str(), msg.c_str());
            return false;
        }

        properties->parseCommandLineOptions("", args);
    }

    _profiles[name] = properties;
    return true;
}

static bool
parseProfiles(const string& file TSRMLS_DC)
{
    //
    // The Zend engine doesn't export a function for loading an INI file, so we
    // have to do it ourselves. The format is:
    //
    // [profile-name]
    // ice.config = config-file
    // ice.options = args
    //
    ifstream in(file.c_str());
    if(!in)
    {
        php_error_docref(0 TSRMLS_CC, E_WARNING, "unable to open Ice profiles in %s", file.c_str());
        return false;
    }

    string name, config, options;
    char line[1024];
    while(in.getline(line, 1024))
    {
        const string delim = " \t\r\n";
        string s = line;

        string::size_type idx = s.find(';');
        if(idx != string::npos)
        {
            s.erase(idx);
        }

        idx = s.find_last_not_of(delim);
        if(idx != string::npos && idx + 1 < s.length())
        {
            s.erase(idx + 1);
        }

        string::size_type beg = s.find_first_not_of(delim);
        if(beg == string::npos)
        {
            continue;
        }

        if(s[beg] == '[')
        {
            beg++;
            string::size_type end = s.find_first_of(" \t]", beg);
            if(end == string::npos || s[s.length() - 1] != ']')
            {
                php_error_docref(0 TSRMLS_CC, E_WARNING, "invalid profile section in file %s:\n%s\n", file.c_str(),
                                 line);
                return false;
            }

            if(!name.empty())
            {
                createProfile(name, config, options TSRMLS_CC);
                config.clear();
                options.clear();
            }

            name = s.substr(beg, end - beg);
        }
        else
        {
            string::size_type end = s.find_first_of(delim + "=", beg);
            assert(end != string::npos);

            string key = s.substr(beg, end - beg);

            end = s.find('=', end);
            if(end == string::npos)
            {
                php_error_docref(0 TSRMLS_CC, E_WARNING, "invalid profile entry in file %s:\n%s\n", file.c_str(), line);
                return false;
            }
            ++end;

            string value;
            beg = s.find_first_not_of(delim, end);
            if(beg != string::npos)
            {
                end = s.length();
                value = s.substr(beg, end - beg);

                //
                // Check for quotes and remove them if present
                //
                string::size_type qpos = IceUtilInternal::checkQuote(value);
                if(qpos != string::npos)
                {
                    value = value.substr(1, qpos - 1);
                }
            }

            if(key == "config" || key == "ice.config")
            {
                config = value;
            }
            else if(key == "options" || key == "ice.options")
            {
                options = value;
            }
            else
            {
                php_error_docref(0 TSRMLS_CC, E_WARNING, "unknown profile entry in file %s:\n%s\n", file.c_str(), line);
            }

            if(name.empty())
            {
                php_error_docref(0 TSRMLS_CC, E_WARNING, "no section for profile entry in file %s:\n%s\n", file.c_str(),
                                 line);
                return false;
            }
        }
    }

    if(!name.empty())
    {
        if(!createProfile(name, config, options TSRMLS_CC))
        {
            return false;
        }
    }

    return true;
}

bool
IcePHP::communicatorInit(TSRMLS_D)
{
    //
    // We register an interface and a class that implements the interface. This allows
    // applications to safely include the Slice-generated code for the type.
    //

    //
    // Register the Communicator interface.
    //
    zend_class_entry ce;
#ifdef ICEPHP_USE_NAMESPACES
    INIT_NS_CLASS_ENTRY(ce, "Ice", "Communicator", _interfaceMethods);
#else
    INIT_CLASS_ENTRY(ce, "Ice_Communicator", _interfaceMethods);
#endif
    zend_class_entry* interface = zend_register_internal_interface(&ce TSRMLS_CC);

    //
    // Register the Communicator class.
    //
    INIT_CLASS_ENTRY(ce, "IcePHP_Communicator", _classMethods);
    ce.create_object = handleAlloc;
    communicatorClassEntry = zend_register_internal_class(&ce TSRMLS_CC);
    memcpy(&_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    _handlers.clone_obj = handleClone;
    zend_class_implements(communicatorClassEntry TSRMLS_CC, 1, interface);

    //
    // Register the ValueFactoryManager interface.
    //
#ifdef ICEPHP_USE_NAMESPACES
    INIT_NS_CLASS_ENTRY(ce, "Ice", "ValueFactoryManager", _vfmInterfaceMethods);
#else
    INIT_CLASS_ENTRY(ce, "Ice_ValueFactoryManager", _vfmInterfaceMethods);
#endif
    zend_class_entry* vfmInterface = zend_register_internal_interface(&ce TSRMLS_CC);

    //
    // Register the ValueFactoryManager class.
    //
    INIT_CLASS_ENTRY(ce, "IcePHP_ValueFactoryManager", _vfmClassMethods);
    ce.create_object = handleVfmAlloc;
    valueFactoryManagerClassEntry = zend_register_internal_class(&ce TSRMLS_CC);
    memcpy(&_vfmHandlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    _vfmHandlers.clone_obj = handleVfmClone;
    zend_class_implements(valueFactoryManagerClassEntry TSRMLS_CC, 1, vfmInterface);

    //
    // Create the profiles from configuration settings.
    //
    const char* empty = "";
    const char* config = INI_STR("ice.config"); // Needs to be a string literal!
    if(!config)
    {
        config = empty;
    }
    const char* options = INI_STR("ice.options"); // Needs to be a string literal!
    if(!options)
    {
        options = empty;
    }
    if(!createProfile(_defaultProfileName, config, options TSRMLS_CC))
    {
        return false;
    }

    const char* profiles = INI_STR("ice.profiles"); // Needs to be a string literal!
    if(!profiles)
    {
        profiles = empty;
    }
    if(strlen(profiles) > 0)
    {
        if(!parseProfiles(profiles TSRMLS_CC))
        {
            return false;
        }

        if(INI_BOOL(const_cast<char*>("ice.hide_profiles")))
        {
            memset(const_cast<char*>(profiles), '*', strlen(profiles));
            //
            // For some reason the code below does not work as expected. It causes a call
            // to ini_get_all() to segfault.
            //
            /*
            if(zend_alter_ini_entry("ice.profiles", sizeof("ice.profiles"), "<hidden>", sizeof("<hidden>") - 1,
                                    PHP_INI_ALL, PHP_INI_STAGE_STARTUP) == FAILURE)
            {
                return false;
            }
            */
        }
    }

    return true;
}

bool
IcePHP::communicatorShutdown(TSRMLS_D)
{
    _profiles.clear();

    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(_registeredCommunicatorsMutex);

    if(_timer)
    {
        _timer->destroy();
        _timer = 0;
    }

    //
    // Clearing the map releases the last remaining reference counts of the ActiveCommunicator
    // objects. The ActiveCommunicator destructor destroys its communicator.
    //
    _registeredCommunicators.clear();

    return true;
}

bool
IcePHP::communicatorRequestInit(TSRMLS_D)
{
    ICE_G(communicatorMap) = 0;

    return true;
}

bool
IcePHP::communicatorRequestShutdown(TSRMLS_D)
{
    if(ICE_G(communicatorMap))
    {
        CommunicatorMap* m = static_cast<CommunicatorMap*>(ICE_G(communicatorMap));
        for(CommunicatorMap::iterator p = m->begin(); p != m->end(); ++p)
        {
            CommunicatorInfoIPtr info = p->second;

            //
            // We need to destroy any object|value factories installed during this request.
            //
            info->destroyFactories(TSRMLS_C);
        }

        //
        // Deleting the map decrements the reference count of its ActiveCommunicator
        // values. If there are no other references to an ActiveCommunicator, its
        // destructor destroys the communicator.
        //
        delete m;
    }

    return true;
}

IcePHP::ActiveCommunicator::ActiveCommunicator(const Ice::CommunicatorPtr& c) :
    communicator(c), expires(0)
{
}

IcePHP::ActiveCommunicator::~ActiveCommunicator()
{
    //
    // There are no more references to this communicator, so we can safely destroy it now.
    //
    try
    {
        communicator->destroy();
    }
    catch(...)
    {
    }
}

IcePHP::FactoryWrapper::FactoryWrapper(zval* factory, bool isObjectFactory, const CommunicatorInfoIPtr& info) :
    _factory(factory),
    _isObjectFactory(isObjectFactory),
    _info(info)
{
    Z_ADDREF_P(_factory);
}

Ice::ValuePtr
IcePHP::FactoryWrapper::create(const string& id)
{
    //
    // Get the TSRM id for the current request.
    //
    TSRMLS_FETCH();

    //
    // Get the type information.
    //
    ClassInfoPtr cls;
    if(id == Ice::Object::ice_staticId())
    {
        //
        // When the ID is that of Ice::Object, it indicates that the stream has not
        // found a factory and is providing us an opportunity to preserve the object.
        //
        cls = getClassInfoById("::Ice::UnknownSlicedValue" TSRMLS_CC);
    }
    else
    {
        cls = getClassInfoById(id TSRMLS_CC);
    }

    if(!cls)
    {
        return 0;
    }

    zval* arg;
    MAKE_STD_ZVAL(arg);
    ZVAL_STRINGL(arg, STRCAST(id.c_str()), static_cast<int>(id.length()), 1);

    zval* obj = 0;

    zend_try
    {
        const char* func = "create";
        zend_call_method(&_factory, 0, 0, const_cast<char*>(func), static_cast<int>(strlen(func)), &obj, 1, arg,
                         0 TSRMLS_CC);
    }
    zend_catch
    {
        obj = 0;
    }
    zend_end_try();

    zval_ptr_dtor(&arg);

    //
    // Bail out if an exception has already been thrown.
    //
    if(!obj || EG(exception))
    {
        throw AbortMarshaling();
    }

    AutoDestroy destroy(obj);

    if(Z_TYPE_P(obj) == IS_NULL)
    {
        return 0;
    }

    return new ObjectReader(obj, cls, _info TSRMLS_CC);
}

void
IcePHP::FactoryWrapper::getZval(zval* factory TSRMLS_DC)
{
    *factory = *_factory; // This is legal - it simply copies the object's handle.
    INIT_PZVAL(factory);
    zval_copy_ctor(factory);
}

bool
IcePHP::FactoryWrapper::isObjectFactory() const
{
    return _isObjectFactory;
}

void
IcePHP::FactoryWrapper::destroy(TSRMLS_D)
{
    if(_isObjectFactory)
    {
        //
        // Invoke the destroy method on the PHP factory.
        //
        invokeMethod(_factory, "destroy" TSRMLS_CC);
        zend_clear_exception(TSRMLS_C);
    }
    zval_ptr_dtor(&_factory);
    _info = 0;
}

IcePHP::DefaultValueFactory::DefaultValueFactory(const CommunicatorInfoIPtr& info) :
    _info(info)
{
}

Ice::ValuePtr
IcePHP::DefaultValueFactory::create(const string& id)
{
    //
    // Get the TSRM id for the current request.
    //
    TSRMLS_FETCH();

    if(_delegate)
    {
        Ice::ValuePtr v = _delegate->create(id);
        if(v)
        {
            return v;
        }
    }

    //
    // Get the type information.
    //
    ClassInfoPtr cls;
    if(id == Ice::Object::ice_staticId())
    {
        //
        // When the ID is that of Ice::Object, it indicates that the stream has not
        // found a factory and is providing us an opportunity to preserve the object.
        //
        cls = getClassInfoById("::Ice::UnknownSlicedValue" TSRMLS_CC);
    }
    else
    {
        cls = getClassInfoById(id TSRMLS_CC);
    }

    if(!cls)
    {
        return 0;
    }

    //
    // Instantiate the object.
    //
    zval* obj;
    MAKE_STD_ZVAL(obj);
    AutoDestroy destroy(obj);

    if(object_init_ex(obj, const_cast<zend_class_entry*>(cls->zce)) != SUCCESS)
    {
        throw AbortMarshaling();
    }

    if(!invokeMethod(obj, ZEND_CONSTRUCTOR_FUNC_NAME TSRMLS_CC))
    {
        throw AbortMarshaling();
    }

    return new ObjectReader(obj, cls, _info TSRMLS_CC);
}

void
IcePHP::DefaultValueFactory::destroy(TSRMLS_D)
{
    if(_delegate)
    {
        _delegate->destroy();
        _delegate = 0;
    }
    _info = 0;
}

IcePHP::CommunicatorInfoI::CommunicatorInfoI(const ActiveCommunicatorPtr& c, zval* z) :
    ac(c),
    zv(*z), // This is legal - it simply copies the object's handle.
    _defaultFactory(new DefaultValueFactory(this))
{
}

void
IcePHP::CommunicatorInfoI::getZval(zval* z TSRMLS_DC)
{
    Z_TYPE_P(z) = IS_OBJECT;
    z->value.obj = zv.value.obj;
    addRef(TSRMLS_C);
}

void
IcePHP::CommunicatorInfoI::addRef(TSRMLS_D)
{
    zval* p = const_cast<zval*>(&zv);
    Z_OBJ_HT_P(p)->add_ref(p TSRMLS_CC);
}

void
IcePHP::CommunicatorInfoI::decRef(TSRMLS_D)
{
    zval* p = const_cast<zval*>(&zv);
    Z_OBJ_HT(zv)->del_ref(p TSRMLS_CC);
}

Ice::CommunicatorPtr
IcePHP::CommunicatorInfoI::getCommunicator() const
{
    return ac->communicator;
}

bool
IcePHP::CommunicatorInfoI::addFactory(zval* factory, const string& id, bool isObjectFactory TSRMLS_DC)
{
    if(id.empty())
    {
        if(_defaultFactory->getDelegate())
        {
            Ice::AlreadyRegisteredException ex(__FILE__, __LINE__);
            ex.kindOfObject = "value factory";
            ex.id = id;
            throwException(ex TSRMLS_CC);
            return false;
        }

        _defaultFactory->setDelegate(new FactoryWrapper(factory, isObjectFactory, this));
    }
    else
    {
        FactoryMap::iterator p = _factories.find(id);
        if(p != _factories.end())
        {
            Ice::AlreadyRegisteredException ex(__FILE__, __LINE__);
            ex.kindOfObject = "value factory";
            ex.id = id;
            throwException(ex TSRMLS_CC);
            return false;
        }

        _factories.insert(FactoryMap::value_type(id, new FactoryWrapper(factory, isObjectFactory, this)));
    }

    return true;
}

FactoryWrapperPtr
IcePHP::CommunicatorInfoI::findFactory(const string& id) const
{
    if(id.empty())
    {
        return _defaultFactory->getDelegate();
    }
    else
    {
        FactoryMap::const_iterator p = _factories.find(id);
        if(p != _factories.end())
        {
            return p->second;
        }
    }

    return 0;
}

void
IcePHP::CommunicatorInfoI::destroyFactories(TSRMLS_D)
{
    for(FactoryMap::iterator p = _factories.begin(); p != _factories.end(); ++p)
    {
        p->second->destroy(TSRMLS_C);
    }
    _factories.clear();
    _defaultFactory->destroy(TSRMLS_C);
}

void
IcePHP::ValueFactoryManager::add(const Ice::ValueFactoryPtr&, const string&)
{
    //
    // We don't support factories registered in C++.
    //
    throw Ice::FeatureNotSupportedException(__FILE__, __LINE__, "C++ value factory");
}

Ice::ValueFactoryPtr
IcePHP::ValueFactoryManager::find(const string& id) const
{
    //
    // Get the TSRM id for the current request.
    //
    TSRMLS_FETCH();

    CommunicatorMap* m = static_cast<CommunicatorMap*>(ICE_G(communicatorMap));
    assert(m);
    CommunicatorMap::iterator p = m->find(_communicator);
    assert(p != m->end());

    CommunicatorInfoIPtr info = p->second;

    if(id.empty())
    {
        return info->defaultFactory();
    }
    else
    {
        return info->findFactory(id);
    }
}

void
IcePHP::ValueFactoryManager::destroy()
{
    _communicator = 0;
}

void
IcePHP::ReaperTask::runTimerTask()
{
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(_registeredCommunicatorsMutex);

    IceUtil::Time now = IceUtil::Time::now();
    RegisteredCommunicatorMap::iterator p = _registeredCommunicators.begin();
    while(p != _registeredCommunicators.end())
    {
        if(p->second->lastAccess + IceUtil::Time::seconds(p->second->expires * 60) <= now)
        {
            try
            {
                p->second->communicator->destroy();
            }
            catch(...)
            {
            }
            _registeredCommunicators.erase(p++);
        }
        else
        {
            ++p;
        }
    }
}
