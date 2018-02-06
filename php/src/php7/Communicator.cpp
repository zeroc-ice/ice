// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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

typedef std::map<std::string, zval> ObjectFactoryMap;

class CommunicatorInfoI : public CommunicatorInfo
{
public:

    CommunicatorInfoI(const ActiveCommunicatorPtr&, zval*);

    virtual void getZval(zval*);
    virtual void addRef(void);
    virtual void decRef(void);

    virtual Ice::CommunicatorPtr getCommunicator() const;

    bool addObjectFactory(const std::string&, zval*);
    bool findObjectFactory(const std::string&, zval*);
    void destroyObjectFactories(void);

    const ActiveCommunicatorPtr ac;
    zval zv;
    ObjectFactoryMap objectFactories;
};
typedef IceUtil::Handle<CommunicatorInfoI> CommunicatorInfoIPtr;

//
// Each PHP request has its own set of object factories. More precisely, there is
// an object factory map for each communicator that is created by a PHP request.
// The factory class defined below delegates the create/destroy methods to PHP
// objects supplied by the application. An instance of this class is installed
// as the communicator's default object factory, and the class holds a reference
// to its communicator. When create is invoked, the class resolves the appropriate
// PHP object as follows:
//
//  * Using its communicator reference as the key, look up the corresponding
//    CommunicatorInfoI object in the request-specific communicator map.
//
//  * In the object factory map held by the CommunicatorInfoI object, look for a
//    PHP factory object using the same algorithm as the Ice core.
//
class ObjectFactoryI : public Ice::ObjectFactory
{
public:

    ObjectFactoryI(const Ice::CommunicatorPtr&);

    virtual Ice::ObjectPtr create(const std::string&);
    virtual void destroy();

private:

    Ice::CommunicatorPtr _communicator;
};

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
static zend_object* handleAlloc(zend_class_entry*);
static void handleFreeStorage(zend_object*);
static zend_object* handleClone(zval*);
}

ZEND_METHOD(Ice_Communicator, __construct)
{
    runtimeError("communicators cannot be instantiated directly");
}

ZEND_METHOD(Ice_Communicator, destroy)
{
    CommunicatorInfoIPtr _this = Wrapper<CommunicatorInfoIPtr>::value(getThis());
    assert(_this);

    Ice::CommunicatorPtr c = _this->getCommunicator();
    assert(c);
    CommunicatorMap* m = reinterpret_cast<CommunicatorMap*>(ICE_G(communicatorMap));
    assert(m);
    if(m->find(c) != m->end()) // If not already destroyed
    {
        m->erase(c);
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
        // We need to destroy any object factories installed by this request.
        //
        _this->destroyObjectFactories();

        try
        {
            c->destroy();
        }
        catch(const IceUtil::Exception& ex)
        {
            throwException(ex);
            RETURN_NULL();
        }
    }
}

ZEND_METHOD(Ice_Communicator, stringToProxy)
{
    CommunicatorInfoIPtr _this = Wrapper<CommunicatorInfoIPtr>::value(getThis());
    assert(_this);

    char* str;
    size_t strLen;
    if(zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("s"), &str, &strLen) != SUCCESS)
    {
        RETURN_NULL();
    }
    string s(str, strLen);

    try
    {
        Ice::ObjectPrx prx = _this->getCommunicator()->stringToProxy(s);
        if(!createProxy(return_value, prx, _this))
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

ZEND_METHOD(Ice_Communicator, proxyToString)
{
    CommunicatorInfoIPtr _this = Wrapper<CommunicatorInfoIPtr>::value(getThis());
    assert(_this);

    zval* zv;
    if(zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("O!"), &zv, proxyClassEntry) != SUCCESS)
    {
        RETURN_NULL();
    }

    try
    {
        string str;
        if(zv)
        {
            Ice::ObjectPrx prx;
            ClassInfoPtr info;
            if(!fetchProxy(zv, prx, info))
            {
                RETURN_NULL();
            }
            assert(prx);
            str = prx->ice_toString();
        }
        RETURN_STRINGL(STRCAST(str.c_str()), static_cast<int>(str.length()));
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Communicator, propertyToProxy)
{
    CommunicatorInfoIPtr _this = Wrapper<CommunicatorInfoIPtr>::value(getThis());
    assert(_this);

    char* str;
    size_t strLen;
    if(zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("s"), &str, &strLen) != SUCCESS)
    {
        RETURN_NULL();
    }
    string s(str, strLen);

    try
    {
        Ice::ObjectPrx prx = _this->getCommunicator()->propertyToProxy(s);
        if(!createProxy(return_value, prx, _this))
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

ZEND_METHOD(Ice_Communicator, proxyToProperty)
{
    CommunicatorInfoIPtr _this = Wrapper<CommunicatorInfoIPtr>::value(getThis());
    assert(_this);

    zval* zv;
    char* str;
    size_t strLen;
    if(zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("O!s"), &zv, proxyClassEntry, &str, &strLen)
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
            ClassInfoPtr info;
            if(!fetchProxy(zv, prx, info))
            {
                RETURN_NULL();
            }
            assert(prx);

            Ice::PropertyDict val = _this->getCommunicator()->proxyToProperty(prx, prefix);
            if(!createStringMap(return_value, val))
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
        throwException(ex);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Communicator, stringToIdentity)
{
    CommunicatorInfoIPtr _this = Wrapper<CommunicatorInfoIPtr>::value(getThis());
    assert(_this);

    char* str;
    size_t strLen;
    if(zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("s"), &str, &strLen) != SUCCESS)
    {
        RETURN_NULL();
    }
    string s(str, strLen);

    try
    {
        Ice::Identity id = _this->getCommunicator()->stringToIdentity(s);
        if(!createIdentity(return_value, id))
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

ZEND_METHOD(Ice_Communicator, identityToString)
{
    CommunicatorInfoIPtr _this = Wrapper<CommunicatorInfoIPtr>::value(getThis());
    assert(_this);

    zend_class_entry* identityClass = idToClass("::Ice::Identity");
    assert(identityClass);

    zval* zv;
    if(zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("O"), &zv, identityClass) != SUCCESS)
    {
        RETURN_NULL();
    }
    Ice::Identity id;
    if(!extractIdentity(zv, id))
    {
        RETURN_NULL();
    }

    try
    {
        string str = _this->getCommunicator()->identityToString(id);
        RETURN_STRINGL(STRCAST(str.c_str()), static_cast<int>(str.length()));
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Communicator, addObjectFactory)
{
    CommunicatorInfoIPtr _this = Wrapper<CommunicatorInfoIPtr>::value(getThis());
    assert(_this);

    zend_class_entry* factoryClass = idToClass("Ice::ObjectFactory");
    assert(factoryClass);

    zval* factory;
    char* id;
    size_t idLen;
    if(zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("Os!"), &factory, factoryClass, &id,
                             &idLen) != SUCCESS)
    {
        RETURN_NULL();
    }

    string type;
    if(id)
    {
        type = string(id, idLen);
    }

    if(!_this->addObjectFactory(type, factory))
    {
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Communicator, findObjectFactory)
{
    CommunicatorInfoIPtr _this = Wrapper<CommunicatorInfoIPtr>::value(getThis());
    assert(_this);

    char* id;
    size_t idLen;
    if(zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("s!"), &id, &idLen) != SUCCESS)
    {
        RETURN_NULL();
    }

    string type;
    if(id)
    {
        type = string(id, idLen);
    }

    if(!_this->findObjectFactory(type, return_value))
    {
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Communicator, getImplicitContext)
{
    runtimeError("not implemented");
}

ZEND_METHOD(Ice_Communicator, getProperties)
{
    if(ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    CommunicatorInfoIPtr _this = Wrapper<CommunicatorInfoIPtr>::value(getThis());
    assert(_this);

    try
    {
        Ice::PropertiesPtr props = _this->getCommunicator()->getProperties();
        if(!createProperties(return_value, props))
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

ZEND_METHOD(Ice_Communicator, getLogger)
{
    if(ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    CommunicatorInfoIPtr _this = Wrapper<CommunicatorInfoIPtr>::value(getThis());
    assert(_this);

    try
    {
        Ice::LoggerPtr logger = _this->getCommunicator()->getLogger();
        if(!createLogger(return_value, logger))
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


ZEND_METHOD(Ice_Communicator, getDefaultRouter)
{
    if(ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    CommunicatorInfoIPtr _this = Wrapper<CommunicatorInfoIPtr>::value(getThis());
    assert(_this);

    try
    {
        Ice::RouterPrx router = _this->getCommunicator()->getDefaultRouter();
        if(router)
        {
            ClassInfoPtr info = getClassInfoById("::Ice::Router");
            if(!info)
            {
                runtimeError("no definition for Ice::Router");
                RETURN_NULL();
            }
            if(!createProxy(return_value, router, info, _this))
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
        throwException(ex);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Communicator, setDefaultRouter)
{
    CommunicatorInfoIPtr _this = Wrapper<CommunicatorInfoIPtr>::value(getThis());
    assert(_this);

    zval* zv;
    if(zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("O!"), &zv, proxyClassEntry) !=
        SUCCESS)
    {
        RETURN_NULL();
    }

    Ice::ObjectPrx proxy;
    ClassInfoPtr info;
    if(zv && !fetchProxy(zv, proxy, info))
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
                invalidArgument("setDefaultRouter requires a proxy narrowed to Ice::Router");
                RETURN_NULL();
            }
            router = Ice::RouterPrx::uncheckedCast(proxy);
        }
        _this->getCommunicator()->setDefaultRouter(router);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Communicator, getDefaultLocator)
{
    if(ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    CommunicatorInfoIPtr _this = Wrapper<CommunicatorInfoIPtr>::value(getThis());
    assert(_this);

    try
    {
        Ice::LocatorPrx locator = _this->getCommunicator()->getDefaultLocator();
        if(locator)
        {
            ClassInfoPtr info = getClassInfoById("::Ice::Locator");
            if(!info)
            {
                runtimeError("no definition for Ice::Locator");
                RETURN_NULL();
            }
            if(!createProxy(return_value, locator, info, _this))
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
        throwException(ex);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Communicator, setDefaultLocator)
{
    CommunicatorInfoIPtr _this = Wrapper<CommunicatorInfoIPtr>::value(getThis());
    assert(_this);

    zval* zv;
    if(zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("O!"), &zv, proxyClassEntry) !=
        SUCCESS)
    {
        RETURN_NULL();
    }

    Ice::ObjectPrx proxy;
    ClassInfoPtr info;
    if(zv && !fetchProxy(zv, proxy, info))
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
                invalidArgument("setDefaultLocator requires a proxy narrowed to Ice::Locator");
                RETURN_NULL();
            }
            locator = Ice::LocatorPrx::uncheckedCast(proxy);
        }
        _this->getCommunicator()->setDefaultLocator(locator);
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex);
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Communicator, flushBatchRequests)
{
    CommunicatorInfoIPtr _this = Wrapper<CommunicatorInfoIPtr>::value(getThis());
    assert(_this);

    if(ZEND_NUM_ARGS() != 8)
    {
        WRONG_PARAM_COUNT;
    }

    try
    {
        _this->getCommunicator()->flushBatchRequests();
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
    Wrapper<CommunicatorInfoIPtr>* obj = Wrapper<CommunicatorInfoIPtr>::create(ce);
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
    Wrapper<CommunicatorInfoIPtr>* obj = Wrapper<CommunicatorInfoIPtr>::fetch(object);
    assert(obj);

    delete obj->ptr;
    zend_object_std_dtor(object);
}

#ifdef _WIN32
extern "C"
#endif
static zend_object*
handleClone(zval* zv)
{
    php_error_docref(0, E_ERROR, "communicators cannot be cloned");
    return 0;
}

static CommunicatorInfoIPtr
createCommunicator(zval* zv, const ActiveCommunicatorPtr& ac)
{
    try
    {
        if(object_init_ex(zv, communicatorClassEntry) != SUCCESS)
        {
            runtimeError("unable to initialize communicator object");
            return 0;
        }

        Wrapper<CommunicatorInfoIPtr>* obj = Wrapper<CommunicatorInfoIPtr>::extract(zv);
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
        throwException(ex);
        return 0;
    }
}

static CommunicatorInfoIPtr
initializeCommunicator(zval* zv, Ice::StringSeq& args, bool hasArgs, const Ice::InitializationData& initData)
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

        //
        // Install a default object factory that delegates to PHP factories.
        //
        c->addObjectFactory(new ObjectFactoryI(c), "");

        CommunicatorInfoIPtr info = createCommunicator(zv, ac);
        if(!info)
        {
            try
            {
                c->destroy();
            }
            catch(...)
            {
            }
        }

        return info;
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex);
        return 0;
    }
}

ZEND_FUNCTION(Ice_initialize)
{
    if(ZEND_NUM_ARGS() > 2)
    {
        runtimeError("too many arguments");
        RETURN_NULL();
    }

    zend_class_entry* initClass = idToClass("::Ice::InitializationData");
    assert(initClass);

    //
    // Retrieve the arguments.
    //
    zval* args = static_cast<zval*>(emalloc(ZEND_NUM_ARGS() * sizeof(zval)));
    AutoEfree autoArgs(args); // Call efree on return
    if(zend_get_parameters_array_ex(ZEND_NUM_ARGS(), args) == FAILURE)
    {
        runtimeError("unable to get arguments");
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
        if(Z_TYPE(args[0]) == IS_ARRAY)
        {
            if(!extractStringArray(&args[0], seq))
            {
                RETURN_NULL();
            }
            zvargs = &args[0];
            hasArgs = true;
            if(ZEND_NUM_ARGS() > 1)
            {
                if(Z_TYPE(args[1]) != IS_OBJECT || Z_OBJCE(args[1]) != initClass)
                {
                    string s = zendTypeToString(Z_TYPE(args[1]));
                    invalidArgument("expected InitializationData object but received %s", s.c_str());
                    RETURN_NULL();
                }
                zvinit = &args[1];
            }
        }
        else if(Z_TYPE(args[0]) == IS_OBJECT && Z_OBJCE(args[0]) == initClass)
        {
            if(ZEND_NUM_ARGS() > 1)
            {
                runtimeError("too many arguments");
                RETURN_NULL();
            }
            zvinit = &args[0];
        }
        else
        {
            string s = zendTypeToString(Z_TYPE(args[0]));
            invalidArgument("unexpected argument type %s", s.c_str());
            RETURN_NULL();
        }
    }

    if(zvinit)
    {
        zval* data;
        string member;

        member = "properties";
        {
            if((data = zend_hash_str_find(Z_OBJPROP_P(zvinit), STRCAST(member.c_str()), member.size())))
            {
                assert(Z_TYPE_P(data) == IS_INDIRECT);
                if(!fetchProperties(Z_INDIRECT_P(data), initData.properties))
                {
                    RETURN_NULL();
                }
            }
        }

        member = "logger";
        {
            if((data = zend_hash_str_find(Z_OBJPROP_P(zvinit), STRCAST(member.c_str()), member.size())))
            {
                assert(Z_TYPE_P(data) == IS_INDIRECT);
                if(!fetchLogger(Z_INDIRECT_P(data), initData.logger))
                {
                    RETURN_NULL();
                }
            }
        }
    }

    initData.compactIdResolver = new IdResolver();

    CommunicatorInfoIPtr info = initializeCommunicator(return_value, seq, hasArgs, initData);
    if(!info)
    {
        RETURN_NULL();
    }

    if(zvargs && Z_ISREF_P(zvargs))
    {
        zval_dtor(zvargs);
        if(!createStringArray(zvargs, seq))
        {
            RETURN_NULL();
        }
    }
}

ZEND_FUNCTION(Ice_register)
{
    zval* comm;
    char* s;
    size_t sLen;
    long expires = 0;
    if(zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("Os|l"), &comm, communicatorClassEntry, &s,
                             &sLen, &expires) != SUCCESS)
    {
        RETURN_NULL();
    }

    string id(s, sLen);
    if(id.empty())
    {
        invalidArgument("communicator id cannot be empty");
        RETURN_NULL();
    }

    CommunicatorInfoIPtr info = Wrapper<CommunicatorInfoIPtr>::value(comm);
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
    size_t sLen;
    if(zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("s"), &s, &sLen) != SUCCESS)
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
    size_t sLen;
    if(zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("s"), &s, &sLen) != SUCCESS)
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
            q->second->getZval(return_value);
            return;
        }
    }

    if(!createCommunicator(return_value, p->second))
    {
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_getProperties)
{
    char* s = 0;
    size_t sLen;
    if(zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("|s"), &s, &sLen) != SUCCESS)
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
    if(!createProperties(return_value, clone))
    {
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_identityToString)
{
    zend_class_entry* identityClass = idToClass("::Ice::Identity");
    assert(identityClass);

    zval* zv;
    if(zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("O"), &zv, identityClass) != SUCCESS)
    {
        RETURN_NULL();
    }
    Ice::Identity id;
    if(!extractIdentity(zv, id))
    {
        RETURN_NULL();
    }

    try
    {
        string str = Ice::identityToString(id);
        RETURN_STRINGL(STRCAST(str.c_str()), static_cast<int>(str.length()));
    }
    catch(const IceUtil::Exception& ex)
    {
        throwException(ex);
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_stringToIdentity)
{
    char* str;
    size_t strLen;
    if(zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("s"), &str, &strLen) != SUCCESS)
    {
        RETURN_NULL();
    }
    string s(str, strLen);

    try
    {
        Ice::Identity id = Ice::stringToIdentity(s);
        if(!createIdentity(return_value, id))
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
    ZEND_ME(Ice_Communicator, __construct, NULL, ZEND_ACC_PRIVATE|ZEND_ACC_CTOR)
    ZEND_ME(Ice_Communicator, destroy, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Communicator, stringToProxy, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Communicator, proxyToString, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Communicator, propertyToProxy, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Communicator, proxyToProperty, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Communicator, stringToIdentity, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Communicator, identityToString, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Communicator, addObjectFactory, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Communicator, findObjectFactory, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Communicator, getImplicitContext, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Communicator, getProperties, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Communicator, getLogger, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Communicator, getDefaultRouter, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Communicator, setDefaultRouter, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Communicator, getDefaultLocator, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Communicator, setDefaultLocator, NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(Ice_Communicator, flushBatchRequests, NULL, ZEND_ACC_PUBLIC)
    {0, 0, 0}
};

static bool
createProfile(const string& name, const string& config, const string& options)
{
    ProfileMap::iterator p = _profiles.find(name);
    if(p != _profiles.end())
    {
        php_error_docref(0, E_WARNING, "duplicate Ice profile `%s'", name.c_str());
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
            php_error_docref(0, E_WARNING, "unable to load Ice configuration file %s:\n%s", config.c_str(),
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
            php_error_docref(0, E_WARNING, "error occurred while parsing the options `%s':\n%s",
                             options.c_str(), msg.c_str());
            return false;
        }

        properties->parseCommandLineOptions("", args);
    }

    _profiles[name] = properties;
    return true;
}

static bool
parseProfiles(const string& file)
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
        php_error_docref(0, E_WARNING, "unable to open Ice profiles in %s", file.c_str());
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
                php_error_docref(0, E_WARNING, "invalid profile section in file %s:\n%s\n", file.c_str(),
                                 line);
                return false;
            }

            if(!name.empty())
            {
                createProfile(name, config, options);
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
                php_error_docref(0, E_WARNING, "invalid profile entry in file %s:\n%s\n", file.c_str(), line);
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
                php_error_docref(0, E_WARNING, "unknown profile entry in file %s:\n%s\n", file.c_str(), line);
            }

            if(name.empty())
            {
                php_error_docref(0, E_WARNING, "no section for profile entry in file %s:\n%s\n", file.c_str(),
                                 line);
                return false;
            }
        }
    }

    if(!name.empty())
    {
        if(!createProfile(name, config, options))
        {
            return false;
        }
    }

    return true;
}

bool
IcePHP::communicatorInit(void)
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
    communicatorClassEntry = zend_register_internal_class(&ce);
    memcpy(&_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    _handlers.clone_obj = handleClone;
    _handlers.free_obj = handleFreeStorage;
    _handlers.offset = XtOffsetOf(Wrapper<CommunicatorInfoIPtr>, zobj);
    zend_class_implements(communicatorClassEntry, 1, interface);

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
    if(!createProfile(_defaultProfileName, config, options))
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
        if(!parseProfiles(profiles))
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
IcePHP::communicatorShutdown(void)
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
IcePHP::communicatorRequestInit(void)
{
    ICE_G(communicatorMap) = 0;

    return true;
}

bool
IcePHP::communicatorRequestShutdown(void)
{
    if(ICE_G(communicatorMap))
    {
        CommunicatorMap* m = static_cast<CommunicatorMap*>(ICE_G(communicatorMap));
        for(CommunicatorMap::iterator p = m->begin(); p != m->end(); ++p)
        {
            CommunicatorInfoIPtr info = p->second;

            //
            // We need to destroy any object factories installed during this request.
            //
            info->destroyObjectFactories();
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

IcePHP::CommunicatorInfoI::CommunicatorInfoI(const ActiveCommunicatorPtr& c, zval* z) :
    ac(c)
{
    ZVAL_COPY_VALUE(&zv, z);
}

void
IcePHP::CommunicatorInfoI::getZval(zval* z)
{
    ZVAL_COPY_VALUE(z, &zv);
    addRef();
}

void
IcePHP::CommunicatorInfoI::addRef(void)
{
    Z_ADDREF_P(&zv);
}

void
IcePHP::CommunicatorInfoI::decRef(void)
{
    Z_DELREF_P(&zv);
}

Ice::CommunicatorPtr
IcePHP::CommunicatorInfoI::getCommunicator() const
{
    return ac->communicator;
}

bool
IcePHP::CommunicatorInfoI::addObjectFactory(const string& id, zval* factory)
{
    ObjectFactoryMap::iterator p = objectFactories.find(id);
    if(p != objectFactories.end())
    {
        Ice::AlreadyRegisteredException ex(__FILE__, __LINE__);
        ex.kindOfObject = "object factory";
        ex.id = id;
        throwException(ex TSRMLS_CC);
        return false;
    }

    zval zv;
    ZVAL_COPY_VALUE(&zv, factory);
    objectFactories.insert(ObjectFactoryMap::value_type(id, zv));

    return true;
}

bool
IcePHP::CommunicatorInfoI::findObjectFactory(const string& id, zval* zv)
{
    ObjectFactoryMap::iterator p = objectFactories.find(id);
    if(p != objectFactories.end())
    {
        ZVAL_COPY(zv, &p->second);
        return true;
    }

    return false;
}

void
IcePHP::CommunicatorInfoI::destroyObjectFactories()
{
    for(ObjectFactoryMap::iterator p = objectFactories.begin(); p != objectFactories.end(); ++p)
    {
        //
        // Invoke the destroy method on each registered PHP factory.
        //
        invokeMethod(&p->second, "destroy");
        zend_clear_exception();
        zval_ptr_dtor(&p->second);
    }
}

IcePHP::ObjectFactoryI::ObjectFactoryI(const Ice::CommunicatorPtr& communicator) :
    _communicator(communicator)
{
}

Ice::ObjectPtr
IcePHP::ObjectFactoryI::create(const string& id)
{
    CommunicatorMap* m = static_cast<CommunicatorMap*>(ICE_G(communicatorMap));
    assert(m);
    CommunicatorMap::iterator p = m->find(_communicator);
    assert(p != m->end());

    CommunicatorInfoIPtr info = p->second;

    zval factory;
    ZVAL_UNDEF(&factory);

    //
    // Check if the application has registered a factory for this id.
    //
    ObjectFactoryMap::iterator q = info->objectFactories.find(id);
    if(q == info->objectFactories.end())
    {
        q = info->objectFactories.find(""); // Look for a default factory.
    }
    if(q != info->objectFactories.end())
    {
        ZVAL_COPY(&factory, &q->second);
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
        cls = getClassInfoById("::Ice::UnknownSlicedObject");
    }
    else
    {
        cls = getClassInfoById(id);
    }

    if(!cls)
    {
        return 0;
    }

    if(!Z_ISUNDEF(factory))
    {
        zval arg;
        ZVAL_STRINGL(&arg, STRCAST(id.c_str()), static_cast<int>(id.length()));

        zval obj;
        ZVAL_UNDEF(&obj);

        zend_try
        {
            zend_call_method(&factory, 0, 0, const_cast<char*>("create"), sizeof("create") - 1, &obj, 1, &arg, 0);

        }
        zend_catch
        {
        }
        zend_end_try();

        zval_ptr_dtor(&arg);

        //
        // Bail out if an exception has already been thrown.
        //
        if(Z_ISUNDEF(obj) || EG(exception))
        {
            throw AbortMarshaling();
        }

        AutoDestroy destroy(&obj);

        if(Z_TYPE(obj) == IS_NULL)
        {
            return 0;
        }

        return new ObjectReader(&obj, cls, info);
    }

    //
    // If the requested type is an abstract class, then we give up.
    //
    if(cls->isAbstract)
    {
        return 0;
    }

    //
    // Instantiate the object.
    //
    zval obj;
    ZVAL_UNDEF(&obj);

    if(object_init_ex(&obj, const_cast<zend_class_entry*>(cls->zce)) != SUCCESS)
    {
        throw AbortMarshaling();
    }

    if(!invokeMethod(&obj, ZEND_CONSTRUCTOR_FUNC_NAME))
    {
        throw AbortMarshaling();
    }


    return new ObjectReader(&obj, cls, info);
}

void
IcePHP::ObjectFactoryI::destroy()
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
