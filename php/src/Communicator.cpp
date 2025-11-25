// Copyright (c) ZeroC, Inc.

#include "Communicator.h"
#include "DefaultSliceLoader.h"
#include "Ice/Options.h"
#include "Ice/StringUtil.h"
#include "Ice/Timer.h"
#include "IceDiscovery/IceDiscovery.h"
#include "IceLocatorDiscovery/IceLocatorDiscovery.h"
#include "Logger.h"
#include "Properties.h"
#include "Proxy.h"
#include "Types.h"
#include "Util.h"

#include <chrono>
#include <condition_variable>
#include <fstream>
#include <mutex>
#include <thread>
#include <valarray>

#ifdef getcwd
#    undef getcwd
#endif

#include "Ice/FileUtil.h"

using namespace std;
using namespace IcePHP;

ZEND_EXTERN_MODULE_GLOBALS(ice)

// Class entries represent the PHP class implementations we have registered.
namespace IcePHP
{
    zend_class_entry* communicatorClassEntry = 0;

    // An ActiveCommunicator represents a communicator that is still in use—either because a request is currently
    // using it, or because it has been registered and its expiration time has not yet elapsed.
    class ActiveCommunicator
    {
    public:
        ActiveCommunicator(const Ice::CommunicatorPtr& c);
        ~ActiveCommunicator();

        const Ice::CommunicatorPtr communicator;
        // The list of IDs used to register this communicator or empty.
        vector<string> ids;
        // The timer task used to reap this communicator if it expires.
        IceInternal::TimerTaskPtr reapTask;
        // The number of milliseconds in which this communicator will expire if not used. Expired communicators are
        // destroyed by the reap task.
        std::chrono::milliseconds expires;
        // The last time this communicator was accessed.
        std::chrono::steady_clock::time_point lastAccess;
    };
    using ActiveCommunicatorPtr = shared_ptr<ActiveCommunicator>;

    // CommunicatorInfoI encapsulates communicator-related information that is specific to a PHP "request". In other
    // words, multiple PHP requests might share the same communicator instance but still need separate workspaces.
    class CommunicatorInfoI final : public CommunicatorInfo, public enable_shared_from_this<CommunicatorInfoI>
    {
    public:
        CommunicatorInfoI(const ActiveCommunicatorPtr&, zval*);

        void getZval(zval*) final;
        void addRef(void) final;
        void decRef(void) final;

        Ice::CommunicatorPtr getCommunicator() const final;
        Ice::SliceLoaderPtr getSliceLoader() const final;

        const ActiveCommunicatorPtr ac;
        zval zv;

    private:
        mutable Ice::SliceLoaderPtr _sliceLoader; // lazily initialized DefaultSliceLoader
    };
    using CommunicatorInfoIPtr = std::shared_ptr<CommunicatorInfoI>;
}

namespace
{
    // Communicator support.
    zend_object_handlers _handlers;

    // The profile map holds Properties objects corresponding to the "default" profile
    // (defined via the ice.config & ice.options settings in php.ini) as well as named
    // profiles defined in an external file.
    using ProfileMap = map<string, Ice::PropertiesPtr>;
    ProfileMap _profiles;
    const string _defaultProfileName = "";

    // This map represents communicators that have been registered so that they can be reused for multiple requests.
    using RegisteredCommunicatorMap = map<string, ActiveCommunicatorPtr>;
    RegisteredCommunicatorMap _registeredCommunicators;

    // Protects _registeredCommunicators
    std::mutex _registeredCommunicatorsMutex;
    IceInternal::TimerPtr _timer{nullptr};

    // This map is stored in the "global" variables for each PHP request and holds the communicators that have been
    // created (or registered communicators that have been used) by the request.
    using CommunicatorMap = map<Ice::CommunicatorPtr, CommunicatorInfoIPtr>;

    class ReapCommunicatorTimerTask : public IceInternal::TimerTask,
                                      public enable_shared_from_this<ReapCommunicatorTimerTask>
    {
    public:
        ReapCommunicatorTimerTask(ActiveCommunicatorPtr activeCommunicator)
            : _activeCommunicator(std::move(activeCommunicator))
        {
        }

        void runTimerTask() override
        {
            ActiveCommunicatorPtr activeCommunicator;
            {
                lock_guard lock(_registeredCommunicatorsMutex);
                auto now = std::chrono::steady_clock::now();
                if (_activeCommunicator->lastAccess + _activeCommunicator->expires <= now)
                {
                    // Cancel the task to avoid schedule it again after the communicator has been destroyed.
                    _timer->cancel(shared_from_this());

                    // Remove all the registrations for this communicator.
                    for (const auto& id : _activeCommunicator->ids)
                    {
                        _registeredCommunicators.erase(id);
                    }
                }
                activeCommunicator = _activeCommunicator;
            }

            // Destroy the communicator outside the lock.
            if (activeCommunicator)
            {
                activeCommunicator->communicator->destroy();
            }
        }

    private:
        const ActiveCommunicatorPtr _activeCommunicator;
    };
}

extern "C"
{
    static zend_object* handleAlloc(zend_class_entry*);
    static void handleFreeStorage(zend_object*);
    static zend_object* handleClone(zend_object*);
}

ZEND_METHOD(Ice_Communicator, __construct) { runtimeError("communicators cannot be instantiated directly"); }

ZEND_METHOD(Ice_Communicator, shutdown)
{
    CommunicatorInfoIPtr _this = Wrapper<CommunicatorInfoIPtr>::value(getThis());
    assert(_this);

    try
    {
        _this->getCommunicator()->shutdown();
    }
    catch (...)
    {
        throwException(current_exception());
    }
}

ZEND_METHOD(Ice_Communicator, isShutdown)
{
    CommunicatorInfoIPtr _this = Wrapper<CommunicatorInfoIPtr>::value(getThis());
    assert(_this);

    try
    {
        RETURN_BOOL(_this->getCommunicator()->isShutdown() ? 1 : 0);
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_FALSE;
    }
}

ZEND_METHOD(Ice_Communicator, waitForShutdown)
{
    CommunicatorInfoIPtr _this = Wrapper<CommunicatorInfoIPtr>::value(getThis());
    assert(_this);

    try
    {
        _this->getCommunicator()->waitForShutdown();
    }
    catch (...)
    {
        throwException(current_exception());
    }
}

ZEND_METHOD(Ice_Communicator, destroy)
{
    CommunicatorInfoIPtr _this = Wrapper<CommunicatorInfoIPtr>::value(getThis());
    assert(_this);

    Ice::CommunicatorPtr c = _this->getCommunicator();
    assert(c);
    CommunicatorMap* m = reinterpret_cast<CommunicatorMap*>(ICE_G(communicatorMap));
    assert(m);

    m->erase(c);

    // Remove all registrations.
    {
        lock_guard lock(_registeredCommunicatorsMutex);
        for (const auto& id : _this->ac->ids)
        {
            _registeredCommunicators.erase(id);
        }
        _this->ac->ids.clear();

        // Cancel the reap task if it is still scheduled.
        if (_this->ac->reapTask)
        {
            _timer->cancel(_this->ac->reapTask);
            _this->ac->reapTask = nullptr;
        }
    }
    c->destroy();
}

ZEND_BEGIN_ARG_INFO_EX(Ice_Communicator_stringToProxy_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(1))
ZEND_ARG_INFO(0, str)
ZEND_ARG_INFO(0, id)
ZEND_END_ARG_INFO()

ZEND_METHOD(Ice_Communicator, stringToProxy)
{
    CommunicatorInfoIPtr _this = Wrapper<CommunicatorInfoIPtr>::value(getThis());
    assert(_this);

    // The second argument (the Slice type ID) is optional.
    if (ZEND_NUM_ARGS() < 1 || ZEND_NUM_ARGS() > 2)
    {
        WRONG_PARAM_COUNT;
    }

    char* str;
    size_t strLen;
    char* id = nullptr; // the Slice type ID
    size_t idLen = 0;

    if (ZEND_NUM_ARGS() == 2)
    {
        if (zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("s|s"), &str, &strLen, &id, &idLen) != SUCCESS)
        {
            RETURN_NULL();
        }
    }
    else if (zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("s"), &str, &strLen) != SUCCESS)
    {
        RETURN_NULL();
    }

    string s{str, strLen};
    ProxyInfoPtr proxyInfo;
    if (id)
    {
        proxyInfo = getProxyInfo(id);
        if (!proxyInfo)
        {
            invalidArgument(("unknown Slice interface type: " + string{id, idLen}).c_str());
            RETURN_NULL();
        }
    }

    try
    {
        auto prx = _this->getCommunicator()->stringToProxy(s);
        if (prx)
        {
            if (!createProxy(return_value, prx.value(), std::move(proxyInfo), _this))
            {
                RETURN_NULL();
            }
        }
        else
        {
            RETURN_NULL();
        }
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

ZEND_BEGIN_ARG_INFO_EX(Ice_Communicator_proxyToString_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(1))
ZEND_ARG_INFO(0, proxy)
ZEND_END_ARG_INFO()

ZEND_METHOD(Ice_Communicator, proxyToString)
{
    CommunicatorInfoIPtr _this = Wrapper<CommunicatorInfoIPtr>::value(getThis());
    assert(_this);

    zval* zv;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("O!"), &zv, proxyClassEntry) != SUCCESS)
    {
        RETURN_NULL();
    }

    try
    {
        string str;
        if (zv)
        {
            optional<Ice::ObjectPrx> prx;
            ProxyInfoPtr info;
            if (!fetchProxy(zv, prx, info))
            {
                RETURN_NULL();
            }
            assert(prx);
            str = prx->ice_toString();
        }
        RETURN_STRINGL(str.c_str(), static_cast<int>(str.length()));
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

ZEND_BEGIN_ARG_INFO_EX(Ice_Communicator_propertyToProxy_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(1))
ZEND_ARG_INFO(0, property)
ZEND_END_ARG_INFO()

ZEND_METHOD(Ice_Communicator, propertyToProxy)
{
    CommunicatorInfoIPtr _this = Wrapper<CommunicatorInfoIPtr>::value(getThis());
    assert(_this);

    char* str;
    size_t strLen;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("s"), &str, &strLen) != SUCCESS)
    {
        RETURN_NULL();
    }
    string s(str, strLen);

    try
    {
        auto prx = _this->getCommunicator()->propertyToProxy(s);
        if (prx)
        {
            if (!createProxy(return_value, prx.value(), _this))
            {
                RETURN_NULL();
            }
        }
        else
        {
            RETURN_NULL();
        }
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

ZEND_BEGIN_ARG_INFO_EX(Ice_Communicator_proxyToProperty_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(2))
ZEND_ARG_INFO(0, proxy)
ZEND_ARG_INFO(0, property)
ZEND_END_ARG_INFO()

ZEND_METHOD(Ice_Communicator, proxyToProperty)
{
    CommunicatorInfoIPtr _this = Wrapper<CommunicatorInfoIPtr>::value(getThis());
    assert(_this);

    zval* zv;
    char* str;
    size_t strLen;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("O!s"), &zv, proxyClassEntry, &str, &strLen) !=
        SUCCESS)
    {
        RETURN_NULL();
    }

    string prefix(str, strLen);

    try
    {
        if (zv)
        {
            optional<Ice::ObjectPrx> prx;
            ProxyInfoPtr info;
            if (!fetchProxy(zv, prx, info))
            {
                RETURN_NULL();
            }
            assert(prx);

            Ice::PropertyDict val = _this->getCommunicator()->proxyToProperty(prx, prefix);
            if (!createStringMap(return_value, val))
            {
                RETURN_NULL();
            }
        }
        else
        {
            array_init(return_value);
        }
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

ZEND_BEGIN_ARG_INFO_EX(Ice_Communicator_identityToString_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(1))
ZEND_ARG_INFO(0, id)
ZEND_END_ARG_INFO()

ZEND_METHOD(Ice_Communicator, identityToString)
{
    CommunicatorInfoIPtr _this = Wrapper<CommunicatorInfoIPtr>::value(getThis());
    assert(_this);

    zend_class_entry* identityClass = nameToClass("\\Ice\\Identity");
    assert(identityClass);

    zval* zv;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("O"), &zv, identityClass) != SUCCESS)
    {
        RETURN_NULL();
    }
    Ice::Identity id;
    if (!extractIdentity(zv, id))
    {
        RETURN_NULL();
    }

    try
    {
        string str = _this->getCommunicator()->identityToString(id);
        RETURN_STRINGL(str.c_str(), static_cast<int>(str.length()));
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Communicator, getImplicitContext) { runtimeError("not implemented"); }

ZEND_METHOD(Ice_Communicator, getProperties)
{
    if (ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    CommunicatorInfoIPtr _this = Wrapper<CommunicatorInfoIPtr>::value(getThis());
    assert(_this);

    try
    {
        Ice::PropertiesPtr props = _this->getCommunicator()->getProperties();
        if (!IcePHP::createProperties(return_value, props))
        {
            RETURN_NULL();
        }
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Communicator, getLogger)
{
    if (ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    CommunicatorInfoIPtr _this = Wrapper<CommunicatorInfoIPtr>::value(getThis());
    assert(_this);

    try
    {
        Ice::LoggerPtr logger = _this->getCommunicator()->getLogger();
        if (!createLogger(return_value, logger))
        {
            RETURN_NULL();
        }
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Communicator, getDefaultRouter)
{
    if (ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    CommunicatorInfoIPtr _this = Wrapper<CommunicatorInfoIPtr>::value(getThis());
    assert(_this);

    try
    {
        auto router = _this->getCommunicator()->getDefaultRouter();
        if (router)
        {
            ProxyInfoPtr info = getProxyInfo("::Ice::Router");
            if (!info)
            {
                runtimeError("no definition for Ice::Router");
                RETURN_NULL();
            }
            if (!createProxy(return_value, router.value(), info, _this))
            {
                RETURN_NULL();
            }
        }
        else
        {
            RETURN_NULL();
        }
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

ZEND_BEGIN_ARG_INFO_EX(Ice_Communicator_setDefaultRouter_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(1))
ZEND_ARG_INFO(0, router)
ZEND_END_ARG_INFO()

ZEND_METHOD(Ice_Communicator, setDefaultRouter)
{
    CommunicatorInfoIPtr _this = Wrapper<CommunicatorInfoIPtr>::value(getThis());
    assert(_this);

    zval* zv;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("O!"), &zv, proxyClassEntry) != SUCCESS)
    {
        RETURN_NULL();
    }

    optional<Ice::ObjectPrx> proxy;
    ProxyInfoPtr info;
    if (zv && !fetchProxy(zv, proxy, info))
    {
        RETURN_NULL();
    }

    try
    {
        optional<Ice::RouterPrx> router;
        if (proxy)
        {
            if (!info || !info->isA("::Ice::Router"))
            {
                invalidArgument("setDefaultRouter requires a proxy narrowed to Ice::Router");
                RETURN_NULL();
            }
            router = Ice::uncheckedCast<Ice::RouterPrx>(proxy);
        }
        _this->getCommunicator()->setDefaultRouter(router);
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

ZEND_METHOD(Ice_Communicator, getDefaultLocator)
{
    if (ZEND_NUM_ARGS() > 0)
    {
        WRONG_PARAM_COUNT;
    }

    CommunicatorInfoIPtr _this = Wrapper<CommunicatorInfoIPtr>::value(getThis());
    assert(_this);

    try
    {
        auto locator = _this->getCommunicator()->getDefaultLocator();
        if (locator)
        {
            ProxyInfoPtr info = getProxyInfo("::Ice::Locator");
            if (!info)
            {
                runtimeError("no definition for Ice::Locator");
                RETURN_NULL();
            }
            if (!createProxy(return_value, locator.value(), info, _this))
            {
                RETURN_NULL();
            }
        }
        else
        {
            RETURN_NULL();
        }
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

ZEND_BEGIN_ARG_INFO_EX(Ice_Communicator_setDefaultLocator_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(1))
ZEND_ARG_INFO(0, locator)
ZEND_END_ARG_INFO()

ZEND_METHOD(Ice_Communicator, setDefaultLocator)
{
    CommunicatorInfoIPtr _this = Wrapper<CommunicatorInfoIPtr>::value(getThis());
    assert(_this);

    zval* zv;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("O!"), &zv, proxyClassEntry) != SUCCESS)
    {
        RETURN_NULL();
    }

    optional<Ice::ObjectPrx> proxy;
    ProxyInfoPtr info;
    if (zv && !fetchProxy(zv, proxy, info))
    {
        RETURN_NULL();
    }

    try
    {
        optional<Ice::LocatorPrx> locator;
        if (proxy)
        {
            if (!info || !info->isA("::Ice::Locator"))
            {
                invalidArgument("setDefaultLocator requires a proxy narrowed to Ice::Locator");
                RETURN_NULL();
            }
            locator = Ice::uncheckedCast<Ice::LocatorPrx>(proxy);
        }
        _this->getCommunicator()->setDefaultLocator(locator);
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

ZEND_BEGIN_ARG_INFO_EX(Ice_Communicator_flushBatchRequests_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(1))
ZEND_ARG_INFO(0, compress)
ZEND_END_ARG_INFO()

ZEND_METHOD(Ice_Communicator, flushBatchRequests)
{
    zval* compress;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("z"), &compress) != SUCCESS)
    {
        RETURN_NULL();
    }

    if (Z_TYPE_P(compress) != IS_LONG)
    {
        invalidArgument("value for 'compress' argument must be an enumerator of CompressBatch");
        RETURN_NULL();
    }
    Ice::CompressBatch cb = static_cast<Ice::CompressBatch>(Z_LVAL_P(compress));

    CommunicatorInfoIPtr _this = Wrapper<CommunicatorInfoIPtr>::value(getThis());
    assert(_this);

    try
    {
        _this->getCommunicator()->flushBatchRequests(cb);
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

static zend_object*
handleAlloc(zend_class_entry* ce)
{
    Wrapper<CommunicatorInfoIPtr>* obj = Wrapper<CommunicatorInfoIPtr>::create(ce);
    assert(obj);

    obj->zobj.handlers = &_handlers;

    return &obj->zobj;
}

static void
handleFreeStorage(zend_object* object)
{
    Wrapper<CommunicatorInfoIPtr>* obj = Wrapper<CommunicatorInfoIPtr>::fetch(object);
    assert(obj);

    delete obj->ptr;
    zend_object_std_dtor(object);
}

static zend_object*
handleClone(zend_object* zobj)
{
    php_error_docref(0, E_ERROR, "communicators cannot be cloned");
    return nullptr;
}

static CommunicatorInfoIPtr
createCommunicator(zval* zv, const ActiveCommunicatorPtr& ac)
{
    try
    {
        if (object_init_ex(zv, communicatorClassEntry) != SUCCESS)
        {
            runtimeError("unable to initialize communicator object");
            return nullptr;
        }

        Wrapper<CommunicatorInfoIPtr>* obj = Wrapper<CommunicatorInfoIPtr>::extract(zv);
        assert(!obj->ptr);
        obj->ptr = new shared_ptr<CommunicatorInfoI>(make_shared<CommunicatorInfoI>(ac, zv));
        shared_ptr<CommunicatorInfoI> info = *obj->ptr;

        CommunicatorMap* m;
        if (ICE_G(communicatorMap))
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
    catch (...)
    {
        throwException(current_exception());
        return 0;
    }
}

static CommunicatorInfoIPtr
initializeCommunicator(zval* zv, Ice::InitializationData initData)
{
    try
    {
        Ice::CommunicatorPtr c = Ice::initialize(std::move(initData));
        ActiveCommunicatorPtr ac = make_shared<ActiveCommunicator>(c);

        CommunicatorInfoIPtr info = createCommunicator(zv, ac);
        if (!info)
        {
            try
            {
                c->destroy();
            }
            catch (...)
            {
            }
        }

        return info;
    }
    catch (...)
    {
        throwException(current_exception());
        return 0;
    }
}

ZEND_FUNCTION(Ice_initialize)
{
    // The argument options are:
    //
    // initialize()
    // initialize(args)
    // initialize(initData)

    if (ZEND_NUM_ARGS() > 1)
    {
        runtimeError("too many arguments");
        RETURN_NULL();
    }

    zend_class_entry* initClass = nameToClass("\\Ice\\InitializationData");
    assert(initClass);

    //
    // Retrieve the arguments.
    //

    zval* args = static_cast<zval*>(ecalloc(1, ZEND_NUM_ARGS() * sizeof(zval)));
    AutoEfree autoArgs(args); // Call efree on return
    if (zend_get_parameters_array_ex(ZEND_NUM_ARGS(), args) == FAILURE)
    {
        runtimeError("unable to get arguments");
        RETURN_NULL();
    }

    zval* zvargs = nullptr; // args
    zval* zvinit = nullptr; // initData

    if (ZEND_NUM_ARGS() == 1)
    {
        zval* arg = &args[0];
        while (Z_TYPE_P(arg) == IS_REFERENCE)
        {
            arg = Z_REFVAL_P(arg);
        }

        if (Z_TYPE_P(arg) == IS_ARRAY)
        {
            zvargs = arg;
        }
        else if (Z_TYPE_P(arg) == IS_OBJECT && Z_OBJCE_P(arg) == initClass)
        {
            zvinit = arg;
        }
        else
        {
            invalidArgument("initialize expects an argument list or an InitializationData object");
            RETURN_NULL();
        }
    }

    Ice::InitializationData initData;

    if (zvargs)
    {
        Ice::StringSeq seq;
        if (!extractStringArray(zvargs, seq))
        {
            RETURN_NULL();
        }

        initData.properties = Ice::createProperties(seq);

        zval_dtor(zvargs);
        if (!createStringArray(zvargs, seq))
        {
            RETURN_NULL();
        }
    }
    else if (zvinit)
    {
        zval* data;
        string member;

        member = "properties";
        {
            if ((data = zend_hash_str_find(Z_OBJPROP_P(zvinit), member.c_str(), member.size())) != 0)
            {
                assert(Z_TYPE_P(data) == IS_INDIRECT);
                if (!fetchProperties(Z_INDIRECT_P(data), initData.properties))
                {
                    RETURN_NULL();
                }
            }
        }

        member = "logger";
        {
            if ((data = zend_hash_str_find(Z_OBJPROP_P(zvinit), member.c_str(), member.size())) != 0)
            {
                assert(Z_TYPE_P(data) == IS_INDIRECT);
                if (!fetchLogger(Z_INDIRECT_P(data), initData.logger))
                {
                    RETURN_NULL();
                }
            }
        }
    }

    // Make sure we have a Properties object.
    if (!initData.properties)
    {
        initData.properties = std::make_shared<Ice::Properties>();
    }

    if (initData.properties->getIceProperty("Ice.ProgramName").empty())
    {
        // Try to get the script filename from PHP's global variables
        zval* scriptInfo = zend_hash_str_find(&EG(symbol_table), "_SERVER", sizeof("_SERVER") - 1);
        string programName{"IcePHP"};
        if (scriptInfo && Z_TYPE_P(scriptInfo) == IS_ARRAY)
        {
            zval* phpScriptName =
                zend_hash_str_find(Z_ARRVAL_P(scriptInfo), "SCRIPT_FILENAME", sizeof("SCRIPT_FILENAME") - 1);
            if (phpScriptName && Z_TYPE_P(phpScriptName) == IS_STRING)
            {
                string name{Z_STRVAL_P(phpScriptName), Z_STRLEN_P(phpScriptName)};
                if (!name.empty())
                {
                    programName = name;
                    size_t pos = programName.find_last_of("/\\");
                    if (pos != string::npos)
                    {
                        programName = programName.substr(pos + 1);
                    }
                }
            }
        }

        initData.properties->setProperty("Ice.ProgramName", programName);
    }

    // Always accept class cycles during the unmarshaling of PHP objects by the C++ code.
    initData.properties->setProperty("Ice.AcceptClassCycles", "1");

    // Add IceDiscovery/IceLocatorDiscovery if these plug-ins are configured via Ice.Plugin.name.
    if (!initData.properties->getIceProperty("Ice.Plugin.IceDiscovery").empty())
    {
        initData.pluginFactories.push_back(IceDiscovery::discoveryPluginFactory());
    }

    if (!initData.properties->getIceProperty("Ice.Plugin.IceLocatorDiscovery").empty())
    {
        initData.pluginFactories.push_back(IceLocatorDiscovery::locatorDiscoveryPluginFactory());
    }

    CommunicatorInfoIPtr info = initializeCommunicator(return_value, std::move(initData));
    if (!info)
    {
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_register)
{
    zval* comm;
    char* s;
    size_t sLen;
    double expires = 0;
    if (zend_parse_parameters(
            ZEND_NUM_ARGS(),
            const_cast<char*>("Os|d"),
            &comm,
            communicatorClassEntry,
            &s,
            &sLen,
            &expires) != SUCCESS)
    {
        RETURN_NULL();
    }

    string id(s, sLen);
    if (id.empty())
    {
        invalidArgument("communicator id cannot be empty");
        RETURN_NULL();
    }

    CommunicatorInfoIPtr info = Wrapper<CommunicatorInfoIPtr>::value(comm);
    assert(info);

    lock_guard lock(_registeredCommunicatorsMutex);

    RegisteredCommunicatorMap::iterator p = _registeredCommunicators.find(id);
    if (p != _registeredCommunicators.end())
    {
        if (p->second->communicator != info->getCommunicator())
        {
            // A different communicator is already registered with that ID.
            RETURN_FALSE;
        }
    }
    else
    {
        info->ac->ids.push_back(id);
        _registeredCommunicators[id] = info->ac;
    }

    if (info->ac->reapTask)
    {
        // Cancel existing reap task, we schedule a new reap task below according to the expiration time.
        _timer->cancel(info->ac->reapTask);
    }

    if (expires > 0)
    {
        // Update the expiration time. If a communicator is registered with multiple IDs, we always use the most
        // recent expiration setting. The expires parameter is number of minutes as a double number, internally we
        // convert it to milliseconds.
        info->ac->expires = std::chrono::milliseconds(static_cast<int>(expires * 60 * 1000));
        info->ac->lastAccess = std::chrono::steady_clock::now();
        info->ac->reapTask = make_shared<ReapCommunicatorTimerTask>(info->ac);
        _timer->scheduleRepeated(info->ac->reapTask, info->ac->expires / 2);
    }

    RETURN_TRUE;
}

ZEND_FUNCTION(Ice_unregister)
{
    char* s;
    size_t sLen;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("s"), &s, &sLen) != SUCCESS)
    {
        RETURN_NULL();
    }

    string id(s, sLen);

    lock_guard lock(_registeredCommunicatorsMutex);

    RegisteredCommunicatorMap::iterator p = _registeredCommunicators.find(id);
    if (p == _registeredCommunicators.end())
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
    if (zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("s"), &s, &sLen) != SUCCESS)
    {
        RETURN_NULL();
    }

    string id(s, sLen);

    lock_guard lock(_registeredCommunicatorsMutex);

    RegisteredCommunicatorMap::iterator p = _registeredCommunicators.find(id);
    if (p == _registeredCommunicators.end())
    {
        //
        // No communicator registered with that ID.
        //
        RETURN_NULL();
    }

    if (p->second->reapTask)
    {
        p->second->lastAccess = std::chrono::steady_clock::now();
    }

    //
    // Check if this communicator has already been obtained by the current request.
    // If so, we can return the existing PHP object that corresponds to the communicator.
    //
    CommunicatorMap* m = reinterpret_cast<CommunicatorMap*>(ICE_G(communicatorMap));
    if (m)
    {
        CommunicatorMap::iterator q = m->find(p->second->communicator);
        if (q != m->end())
        {
            q->second->getZval(return_value);
            return;
        }
    }

    if (!createCommunicator(return_value, p->second))
    {
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_getProperties)
{
    char* s = 0;
    size_t sLen;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("|s"), &s, &sLen) != SUCCESS)
    {
        RETURN_NULL();
    }

    string name;
    if (s)
    {
        name = string(s, sLen);
    }

    ProfileMap::iterator p = _profiles.find(name);
    if (p == _profiles.end())
    {
        RETURN_NULL();
    }

    Ice::PropertiesPtr clone = p->second->clone();
    if (!IcePHP::createProperties(return_value, clone))
    {
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_identityToString)
{
    zend_class_entry* identityClass = nameToClass("\\Ice\\Identity");
    assert(identityClass);

    zval* zv;
    zend_long mode = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("O|l"), &zv, identityClass, &mode) != SUCCESS)
    {
        RETURN_NULL();
    }
    Ice::Identity id;
    if (!extractIdentity(zv, id))
    {
        RETURN_NULL();
    }

    try
    {
        string str = Ice::identityToString(id, static_cast<Ice::ToStringMode>(mode));
        RETURN_STRINGL(str.c_str(), static_cast<int>(str.length()));
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

ZEND_FUNCTION(Ice_stringToIdentity)
{
    char* str;
    size_t strLen;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), const_cast<char*>("s"), &str, &strLen) != SUCCESS)
    {
        RETURN_NULL();
    }
    string s(str, strLen);

    try
    {
        Ice::Identity id = Ice::stringToIdentity(s);
        if (!createIdentity(return_value, id))
        {
            RETURN_NULL();
        }
    }
    catch (...)
    {
        throwException(current_exception());
        RETURN_NULL();
    }
}

// Predefined methods for Communicator.
static zend_function_entry _interfaceMethods[] = {{0, 0, 0}};

static zend_function_entry _classMethods[] = {
    // __construct
    ZEND_ME(Ice_Communicator, __construct, ice_void_arginfo, ZEND_ACC_PRIVATE | ZEND_ACC_CTOR)
    // shutdown
    ZEND_ME(Ice_Communicator, shutdown, ice_void_arginfo, ZEND_ACC_PUBLIC)
    // isShutdown
    ZEND_ME(Ice_Communicator, isShutdown, ice_void_arginfo, ZEND_ACC_PUBLIC)
    // waitForShutdown
    ZEND_ME(Ice_Communicator, waitForShutdown, ice_void_arginfo, ZEND_ACC_PUBLIC)
    // destroy
    ZEND_ME(Ice_Communicator, destroy, ice_void_arginfo, ZEND_ACC_PUBLIC)
    // stringToProxy
    ZEND_ME(Ice_Communicator, stringToProxy, Ice_Communicator_stringToProxy_arginfo, ZEND_ACC_PUBLIC)
    // proxyToString
    ZEND_ME(Ice_Communicator, proxyToString, Ice_Communicator_proxyToString_arginfo, ZEND_ACC_PUBLIC)
    // propertyToProxy
    ZEND_ME(Ice_Communicator, propertyToProxy, Ice_Communicator_propertyToProxy_arginfo, ZEND_ACC_PUBLIC)
    // proxyToProperty
    ZEND_ME(Ice_Communicator, proxyToProperty, Ice_Communicator_proxyToProperty_arginfo, ZEND_ACC_PUBLIC)
    // identityToString
    ZEND_ME(Ice_Communicator, identityToString, Ice_Communicator_identityToString_arginfo, ZEND_ACC_PUBLIC)
    // getImplicitContext
    ZEND_ME(Ice_Communicator, getImplicitContext, ice_void_arginfo, ZEND_ACC_PUBLIC)
    // getProperties
    ZEND_ME(Ice_Communicator, getProperties, ice_void_arginfo, ZEND_ACC_PUBLIC)
    // getLogger
    ZEND_ME(Ice_Communicator, getLogger, ice_void_arginfo, ZEND_ACC_PUBLIC)
    // getDefaultRouter
    ZEND_ME(Ice_Communicator, getDefaultRouter, ice_void_arginfo, ZEND_ACC_PUBLIC)
    // setDefaultRouter
    ZEND_ME(Ice_Communicator, setDefaultRouter, Ice_Communicator_setDefaultRouter_arginfo, ZEND_ACC_PUBLIC)
    // getDefaultLocator
    ZEND_ME(Ice_Communicator, getDefaultLocator, ice_void_arginfo, ZEND_ACC_PUBLIC)
    // setDefaultLocator
    ZEND_ME(Ice_Communicator, setDefaultLocator, Ice_Communicator_setDefaultLocator_arginfo, ZEND_ACC_PUBLIC)
    // flushBatchRequests
    ZEND_ME(Ice_Communicator, flushBatchRequests, Ice_Communicator_flushBatchRequests_arginfo, ZEND_ACC_PUBLIC){
        0,
        0,
        0}};

static bool
createProfile(const string& name, const string& config, const string& options)
{
    ProfileMap::iterator p = _profiles.find(name);
    if (p != _profiles.end())
    {
        php_error_docref(0, E_WARNING, "duplicate Ice profile `%s'", name.c_str());
        return false;
    }

    Ice::PropertiesPtr properties = Ice::createProperties();

    if (!config.empty())
    {
        try
        {
            properties->load(config);
        }
        catch (const Ice::Exception& ex)
        {
            ostringstream ostr;
            ex.ice_print(ostr);
            php_error_docref(
                0,
                E_WARNING,
                "unable to load Ice configuration file %s:\n%s",
                config.c_str(),
                ostr.str().c_str());
            return false;
        }
    }

    if (!options.empty())
    {
        vector<string> args;
        try
        {
            args = IceInternal::Options::split(options);
        }
        catch (const Ice::Exception& ex)
        {
            ostringstream ostr;
            ex.ice_print(ostr);
            string msg = ostr.str();
            php_error_docref(
                0,
                E_WARNING,
                "error occurred while parsing the options `%s':\n%s",
                options.c_str(),
                msg.c_str());
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
    // The Zend engine doesn't export a function for loading an INI file, so we have to do it ourselves. The format is:
    //
    // [profile-name]
    // ice.config = config-file
    // ice.options = args
    ifstream in(IceInternal::streamFilename(file).c_str());
    if (!in)
    {
        php_error_docref(0, E_WARNING, "unable to open Ice profiles in %s", file.c_str());
        return false;
    }

    string name, config, options;
    char line[1024];
    while (in.getline(line, 1024))
    {
        const string delim = " \t\r\n";
        string s = line;

        string::size_type idx = s.find(';');
        if (idx != string::npos)
        {
            s.erase(idx);
        }

        idx = s.find_last_not_of(delim);
        if (idx != string::npos && idx + 1 < s.length())
        {
            s.erase(idx + 1);
        }

        string::size_type beg = s.find_first_not_of(delim);
        if (beg == string::npos)
        {
            continue;
        }

        if (s[beg] == '[')
        {
            beg++;
            string::size_type end = s.find_first_of(" \t]", beg);
            if (end == string::npos || s[s.length() - 1] != ']')
            {
                php_error_docref(0, E_WARNING, "invalid profile section in file %s:\n%s\n", file.c_str(), line);
                return false;
            }

            if (!name.empty())
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
            if (end == string::npos)
            {
                php_error_docref(0, E_WARNING, "invalid profile entry in file %s:\n%s\n", file.c_str(), line);
                return false;
            }
            ++end;

            string value;
            beg = s.find_first_not_of(delim, end);
            if (beg != string::npos)
            {
                end = s.length();
                value = s.substr(beg, end - beg);

                // Check for quotes and remove them if present
                string::size_type qpos = IceInternal::checkQuote(value);
                if (qpos != string::npos)
                {
                    value = value.substr(1, qpos - 1);
                }
            }

            if (key == "config" || key == "ice.config")
            {
                config = value;
            }
            else if (key == "options" || key == "ice.options")
            {
                options = value;
            }
            else
            {
                php_error_docref(0, E_WARNING, "unknown profile entry in file %s:\n%s\n", file.c_str(), line);
            }

            if (name.empty())
            {
                php_error_docref(0, E_WARNING, "no section for profile entry in file %s:\n%s\n", file.c_str(), line);
                return false;
            }
        }
    }

    if (!name.empty())
    {
        if (!createProfile(name, config, options))
        {
            return false;
        }
    }

    return true;
}

bool
IcePHP::communicatorInit(void)
{
    // Register the Communicator interface.
    zend_class_entry ce;
    INIT_NS_CLASS_ENTRY(ce, "Ice", "Communicator", _interfaceMethods);
    zend_class_entry* interface = zend_register_internal_interface(&ce);

    // Register the Communicator class.
    INIT_CLASS_ENTRY(ce, "IcePHP_Communicator", _classMethods);
    ce.create_object = handleAlloc;
    communicatorClassEntry = zend_register_internal_class(&ce);
    memcpy(&_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    _handlers.clone_obj = handleClone;
    _handlers.free_obj = handleFreeStorage;
    _handlers.offset = XtOffsetOf(Wrapper<CommunicatorInfoIPtr>, zobj);
    zend_class_implements(communicatorClassEntry, 1, interface);

    // Create the profiles from configuration settings.
    const char* empty = "";
    const char* config = INI_STR("ice.config"); // Needs to be a string literal!
    if (!config)
    {
        config = empty;
    }
    const char* options = INI_STR("ice.options"); // Needs to be a string literal!
    if (!options)
    {
        options = empty;
    }
    if (!createProfile(_defaultProfileName, config, options))
    {
        return false;
    }

    const char* profiles = INI_STR("ice.profiles"); // Needs to be a string literal!
    if (!profiles)
    {
        profiles = empty;
    }

    if (strlen(profiles) > 0)
    {
        if (!parseProfiles(profiles))
        {
            return false;
        }
    }

    _timer = make_shared<IceInternal::Timer>();

    return true;
}

bool
IcePHP::communicatorShutdown(void)
{
    _profiles.clear();

    RegisteredCommunicatorMap registeredCommunicators;
    {
        lock_guard lock(_registeredCommunicatorsMutex);
        _registeredCommunicators.swap(registeredCommunicators);
    }

    // Clearing the map releases the last remaining reference counts of the ActiveCommunicator objects. The
    // ActiveCommunicator destructor destroys its communicator.
    registeredCommunicators.clear();

    if (_timer)
    {
        _timer->destroy();
    }
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
    if (ICE_G(communicatorMap))
    {
        CommunicatorMap* m = static_cast<CommunicatorMap*>(ICE_G(communicatorMap));

        // Deleting the map decrements the reference count of its ActiveCommunicator values. If there are no other
        // references to an ActiveCommunicator, its destructor destroys the communicator.
        delete m;
    }

    return true;
}

IcePHP::ActiveCommunicator::ActiveCommunicator(const Ice::CommunicatorPtr& c) : communicator(c) {}

IcePHP::ActiveCommunicator::~ActiveCommunicator()
{
    if (reapTask)
    {
        // Cancel the reap task if it is still scheduled.
        _timer->cancel(reapTask);
    }

    // There are no more references to this communicator, so we can safely destroy it now.
    try
    {
        communicator->destroy();
    }
    catch (...)
    {
    }
}

IcePHP::CommunicatorInfoI::CommunicatorInfoI(const ActiveCommunicatorPtr& c, zval* z) : ac(c)
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

Ice::SliceLoaderPtr
IcePHP::CommunicatorInfoI::getSliceLoader() const
{
    if (!_sliceLoader)
    {
        auto self = const_cast<CommunicatorInfoI*>(this)->shared_from_this();
        _sliceLoader = make_shared<DefaultSliceLoader>(self);
    }
    return _sliceLoader;
}
