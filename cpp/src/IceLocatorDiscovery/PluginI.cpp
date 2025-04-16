// Copyright (c) ZeroC, Inc.

#include "../Ice/Network.h" // For getInterfacesForMulticast
#include "../Ice/Timer.h"
#include "Ice/LoggerUtil.h"
#include "IceLocatorDiscovery/IceLocatorDiscovery.h"
#include "IceLocatorDiscovery/Lookup.h"
#include "Plugin.h"

#include <algorithm>
#include <thread>

using namespace std;
using namespace IceLocatorDiscovery;

namespace
{
    const char* const locatorDiscoveryPluginName = "IceLocatorDiscovery";

    class LocatorI; // Forward declaration
    class Request : public std::enable_shared_from_this<Request>
    {
    public:
        Request(
            LocatorI* locator,
            string operation,
            Ice::OperationMode mode,
            pair<const byte*, const byte*> inParams,
            Ice::Context ctx,
            function<void(bool, pair<const byte*, const byte*>)> responseCallback,
            function<void(exception_ptr)> exceptionCallback)
            : _locator(locator),
              _operation(std::move(operation)),
              _mode(mode),
              _context(std::move(ctx)),
              _inParams(inParams.first, inParams.second),
              _responseCallback(std::move(responseCallback)),
              _exceptionCallback(std::move(exceptionCallback))
        {
        }

        void invoke(const Ice::LocatorPrx&);
        void response(bool, pair<const byte*, const byte*>);
        void exception(std::exception_ptr);

    protected:
        LocatorI* _locator;
        const string _operation;
        const Ice::OperationMode _mode;
        const Ice::Context _context;
        const Ice::ByteSeq _inParams;
        function<void(bool, pair<const byte*, const byte*>)> _responseCallback;
        function<void(exception_ptr)> _exceptionCallback;
        exception_ptr _exception;

        optional<Ice::LocatorPrx> _locatorPrx;
    };
    using RequestPtr = std::shared_ptr<Request>;

    class LocatorI final : public Ice::BlobjectArrayAsync,
                           public IceInternal::TimerTask,
                           public std::enable_shared_from_this<LocatorI>
    {
    public:
        LocatorI(const LookupPrx&, const Ice::PropertiesPtr&, string, Ice::LocatorPrx);
        void setLookupReply(const LookupReplyPrx&);

        void ice_invokeAsync(
            pair<const byte*, const byte*>,
            function<void(bool, pair<const byte*, const byte*>)>,
            function<void(exception_ptr)>,
            const Ice::Current&) final;

        void foundLocator(const optional<Ice::LocatorPrx>&);
        void invoke(const optional<Ice::LocatorPrx>&, const RequestPtr&);

        vector<Ice::LocatorPrx> getLocators(const string&, const chrono::milliseconds&);

        void exception(std::exception_ptr);

    private:
        void runTimerTask() final;

        LookupPrx _lookup;
        vector<pair<LookupPrx, LookupReplyPrx>> _lookups;
        chrono::milliseconds _timeout;
        int _retryCount;
        chrono::milliseconds _retryDelay;
        const IceInternal::TimerPtr _timer;
        const int _traceLevel;

        string _instanceName;
        bool _warned{false};
        optional<Ice::LocatorPrx> _locator;
        map<string, Ice::LocatorPrx> _locators;
        Ice::LocatorPrx _voidLocator;

        chrono::steady_clock::time_point _nextRetry;
        bool _pending{false};
        int _pendingRetryCount{0};
        size_t _failureCount{0};
        bool _warnOnce{true};
        vector<RequestPtr> _pendingRequests;
        std::mutex _mutex;
        std::condition_variable _conditionVariable;
    };
    using LocatorIPtr = std::shared_ptr<LocatorI>;

    class LookupReplyI final : public LookupReply
    {
    public:
        LookupReplyI(LocatorIPtr locator) : _locator(std::move(locator)) {}

        void foundLocator(optional<Ice::LocatorPrx>, const Ice::Current&) final;

    private:
        const LocatorIPtr _locator;
    };

    //
    // The void locator implementation below is used when no locator is found.
    //
    class VoidLocatorI final : public Ice::Locator
    {
    public:
        void findObjectByIdAsync(
            Ice::Identity,
            function<void(const optional<Ice::ObjectPrx>&)> response,
            function<void(exception_ptr)>,
            const Ice::Current&) const final
        {
            response(nullopt);
        }

        void findAdapterByIdAsync(
            string,
            function<void(const optional<Ice::ObjectPrx>&)> response,
            function<void(exception_ptr)>,
            const Ice::Current&) const final
        {
            response(nullopt);
        }

        [[nodiscard]] optional<Ice::LocatorRegistryPrx> getRegistry(const Ice::Current&) const final { return nullopt; }
    };

    class PluginI final : public Plugin
    {
    public:
        PluginI(Ice::CommunicatorPtr);

        void initialize() final;
        void destroy() final;
        [[nodiscard]] vector<Ice::LocatorPrx> getLocators(const string&, const chrono::milliseconds&) const final;

    private:
        const string _name;
        const Ice::CommunicatorPtr _communicator;
        Ice::ObjectAdapterPtr _locatorAdapter;
        Ice::ObjectAdapterPtr _replyAdapter;
        LocatorIPtr _locator;
        optional<Ice::LocatorPrx> _locatorPrx;
        optional<Ice::LocatorPrx> _defaultLocator;
    };
}

//
// Plugin factory function.
//
extern "C" ICE_LOCATOR_DISCOVERY_API Ice::Plugin*
createIceLocatorDiscovery(const Ice::CommunicatorPtr& communicator, const string& name, const Ice::StringSeq&)
{
    string pluginName{locatorDiscoveryPluginName};

    if (name != pluginName)
    {
#ifdef _MSC_VER
#    pragma warning(push)
#    pragma warning(disable : 4297) // function assumed not to throw an exception but does
#endif
        throw Ice::PluginInitializationException{
            __FILE__,
            __LINE__,
            "the Locator Discovery plug-in must be named '" + pluginName + "'"};
#ifdef _MSC_VER
#    pragma warning(pop)
#endif
    }

    return new PluginI(communicator);
}

Ice::PluginFactory
IceLocatorDiscovery::locatorDiscoveryPluginFactory()
{
    return {locatorDiscoveryPluginName, createIceLocatorDiscovery};
}

Plugin::~Plugin() = default;

PluginI::PluginI(Ice::CommunicatorPtr communicator) : _communicator(std::move(communicator)) {}

void
PluginI::initialize()
{
    Ice::PropertiesPtr properties = _communicator->getProperties();

    bool ipv4 = properties->getIcePropertyAsInt("Ice.IPv4") > 0;
    bool preferIPv6 = properties->getIcePropertyAsInt("Ice.PreferIPv6Address") > 0;
    string address = properties->getIceProperty("IceLocatorDiscovery.Address");
    if (address.empty())
    {
        address = ipv4 && !preferIPv6 ? "239.255.0.1" : "ff15::1";
    }
    int port = properties->getIcePropertyAsInt("IceLocatorDiscovery.Port");
    string intf = properties->getIceProperty("IceLocatorDiscovery.Interface");

    string lookupEndpoints = properties->getIceProperty("IceLocatorDiscovery.Lookup");
    if (lookupEndpoints.empty())
    {
        //
        // If no lookup endpoints are specified, we get all the network interfaces and create
        // an endpoint for each of them. We'll send UDP multicast packages on each interface.
        //
        IceInternal::ProtocolSupport protocol = ipv4 && !preferIPv6 ? IceInternal::EnableIPv4 : IceInternal::EnableIPv6;
        vector<string> interfaces = IceInternal::getInterfacesForMulticast(intf, protocol);
        ostringstream lookup;
        for (auto p = interfaces.begin(); p != interfaces.end(); ++p)
        {
            if (p != interfaces.begin())
            {
                lookup << ":";
            }
            lookup << "udp -h \"" << address << "\" -p " << port << " --interface \"" << *p << "\"";
        }
        lookupEndpoints = lookup.str();
    }

    if (properties->getIceProperty("IceLocatorDiscovery.Reply.Endpoints").empty())
    {
        properties->setProperty(
            "IceLocatorDiscovery.Reply.Endpoints",
            "udp -h " + (intf.empty() ? "*" : "\"" + intf + "\""));
    }

    if (properties->getIceProperty("IceLocatorDiscovery.Locator.Endpoints").empty())
    {
        properties->setProperty("IceLocatorDiscovery.Locator.AdapterId", Ice::generateUUID()); // Collocated adapter
    }

    _replyAdapter = _communicator->createObjectAdapter("IceLocatorDiscovery.Reply");
    _locatorAdapter = _communicator->createObjectAdapter("IceLocatorDiscovery.Locator");

    // We don't want those adapters to be registered with the locator so clear their locator.
    _replyAdapter->setLocator(nullopt);
    _locatorAdapter->setLocator(nullopt);

    LookupPrx lookupPrx(_communicator, "IceLocatorDiscovery/Lookup -d:" + lookupEndpoints);
    // No collocation optimization for the multicast proxy!
    lookupPrx = lookupPrx->ice_collocationOptimized(false)->ice_router(nullopt);

    auto voidLocator = _locatorAdapter->addWithUUID<Ice::LocatorPrx>(make_shared<VoidLocatorI>());

    string instanceName = properties->getIceProperty("IceLocatorDiscovery.InstanceName");
    Ice::Identity id;
    id.name = "Locator";
    id.category = !instanceName.empty() ? instanceName : Ice::generateUUID();
    _locator = make_shared<LocatorI>(lookupPrx, properties, instanceName, voidLocator);
    _defaultLocator = _communicator->getDefaultLocator();
    _locatorPrx = _locatorAdapter->add<Ice::LocatorPrx>(_locator, id);
    _communicator->setDefaultLocator(_locatorPrx);

    auto lookupReply = _replyAdapter->addWithUUID<LookupReplyPrx>(make_shared<LookupReplyI>(_locator))->ice_datagram();
    _locator->setLookupReply(lookupReply);

    _replyAdapter->activate();
    _locatorAdapter->activate();
}

vector<Ice::LocatorPrx>
PluginI::getLocators(const string& instanceName, const chrono::milliseconds& waitTime) const
{
    return _locator->getLocators(instanceName, waitTime);
}

void
PluginI::destroy()
{
    if (_replyAdapter)
    {
        _replyAdapter->destroy();
    }
    if (_locatorAdapter)
    {
        _locatorAdapter->destroy();
    }
    if (_communicator->getDefaultLocator() == _locatorPrx)
    {
        // Restore original default locator proxy, if the user didn't change it in the meantime
        _communicator->setDefaultLocator(_defaultLocator);
    }
}

void
Request::invoke(const Ice::LocatorPrx& l)
{
    if (l != _locatorPrx)
    {
        _locatorPrx = l;
        try
        {
            auto self = shared_from_this();
            l->ice_invokeAsync(
                _operation,
                _mode,
                _inParams,
                [self](bool ok, vector<byte> outParams)
                {
                    pair<const byte*, const byte*> outPair;
                    if (outParams.empty())
                    {
                        outPair.first = outPair.second = 0;
                    }
                    else
                    {
                        outPair.first = &outParams[0];
                        outPair.second = outPair.first + outParams.size();
                    }
                    self->response(ok, outPair);
                },
                [self](exception_ptr e) { self->exception(e); },
                nullptr,
                _context);
        }
        catch (const Ice::LocalException&)
        {
            exception(current_exception());
        }
    }
    else
    {
        assert(_exception); // Don't retry if the proxy didn't change
        _exceptionCallback(_exception);
    }
}

void
Request::response(bool ok, pair<const byte*, const byte*> outParams)
{
    _responseCallback(ok, outParams);
}

void
Request::exception(std::exception_ptr ex)
{
    try
    {
        rethrow_exception(ex);
    }
    catch (const Ice::RequestFailedException&)
    {
        _exceptionCallback(ex);
    }
    catch (const Ice::UnknownException&)
    {
        _exceptionCallback(ex);
    }
    catch (const Ice::NoEndpointException&)
    {
        _exceptionCallback(make_exception_ptr(Ice::ObjectNotExistException{__FILE__, __LINE__}));
    }
    catch (const Ice::CommunicatorDestroyedException&)
    {
        _exceptionCallback(make_exception_ptr(Ice::ObjectNotExistException{__FILE__, __LINE__}));
    }
    catch (const Ice::ObjectAdapterDeactivatedException&)
    {
        _exceptionCallback(make_exception_ptr(Ice::ObjectNotExistException{__FILE__, __LINE__}));
    }
    catch (const Ice::ObjectAdapterDestroyedException&)
    {
        _exceptionCallback(make_exception_ptr(Ice::ObjectNotExistException{__FILE__, __LINE__}));
    }
    catch (...)
    {
        _exception = ex;
        _locator->invoke(_locatorPrx, shared_from_this()); // Retry with new locator proxy
    }
}

LocatorI::LocatorI(
    const LookupPrx& lookup,
    const Ice::PropertiesPtr& p,
    string instanceName,
    Ice::LocatorPrx voidLocator)
    : _lookup(lookup),
      _timeout(chrono::milliseconds(p->getIcePropertyAsInt("IceLocatorDiscovery.Timeout"))),
      _retryCount(p->getIcePropertyAsInt("IceLocatorDiscovery.RetryCount")),
      _retryDelay(chrono::milliseconds(p->getIcePropertyAsInt("IceLocatorDiscovery.RetryDelay"))),
      _timer(IceInternal::getInstanceTimer(lookup->ice_getCommunicator())),
      _traceLevel(p->getIcePropertyAsInt("IceLocatorDiscovery.Trace.Lookup")),
      _instanceName(std::move(instanceName)),
      _locator(lookup->ice_getCommunicator()->getDefaultLocator()),
      _voidLocator(std::move(voidLocator))
{
    if (_timeout < chrono::milliseconds::zero())
    {
        _timeout = chrono::milliseconds(300);
    }
    if (_retryCount < 0)
    {
        _retryCount = 0;
    }
    if (_retryDelay < chrono::milliseconds::zero())
    {
        _retryDelay = chrono::milliseconds::zero();
    }
}

void
LocatorI::setLookupReply(const LookupReplyPrx& lookupReply)
{
    // Create one lookup proxy per endpoint from the given proxy. We want to send a multicast datagram on each
    // endpoint.
    for (const auto& lookupEndpoint : _lookup->ice_getEndpoints())
    {
        LookupReplyPrx reply = lookupReply;
        auto info = dynamic_pointer_cast<Ice::UDPEndpointInfo>(lookupEndpoint->getInfo());
        // Use a lookup reply proxy whose address matches the interface used to send multicast datagrams.
        if (info && !info->mcastInterface.empty())
        {
            for (const auto& replyEndpoint : reply->ice_getEndpoints())
            {
                Ice::IPEndpointInfoPtr r = dynamic_pointer_cast<Ice::IPEndpointInfo>(replyEndpoint->getInfo());
                if (r && r->host == info->mcastInterface)
                {
                    reply = reply->ice_endpoints(Ice::EndpointSeq{replyEndpoint});
                }
            }
        }

        _lookups.emplace_back(_lookup->ice_endpoints(Ice::EndpointSeq{lookupEndpoint}), reply);
    }
    assert(!_lookups.empty());
}

void
LocatorI::ice_invokeAsync(
    pair<const byte*, const byte*> inParams,
    function<void(bool, pair<const byte*, const byte*>)> responseCB,
    function<void(exception_ptr)> exceptionCB,
    const Ice::Current& current)
{
    invoke(
        nullopt,
        make_shared<Request>(
            this,
            current.operation,
            current.mode,
            inParams,
            current.ctx,
            std::move(responseCB),
            std::move(exceptionCB)));
}

vector<Ice::LocatorPrx>
LocatorI::getLocators(const string& instanceName, const chrono::milliseconds& waitTime)
{
    //
    // Clear locators from previous search
    //
    {
        lock_guard lock(_mutex);
        _locators.clear();
    }

    //
    // Find a locator
    //
    invoke(nullopt, nullptr);

    //
    // Wait for responses
    //
    if (instanceName.empty())
    {
        std::this_thread::sleep_for(waitTime);
    }
    else
    {
        unique_lock lock(_mutex);
        while (_locators.find(instanceName) == _locators.end() && _pending)
        {
            if (_conditionVariable.wait_for(lock, waitTime) == cv_status::timeout)
            {
                break;
            }
        }
    }

    //
    // Return found locators
    //
    lock_guard lock(_mutex);
    vector<Ice::LocatorPrx> locators;
    locators.reserve(_locators.size());
    for (const auto& [_, locator] : _locators)
    {
        locators.push_back(locator);
    }
    return locators;
}

void
LocatorI::foundLocator(const optional<Ice::LocatorPrx>& reply)
{
    lock_guard lock(_mutex);

    if (!reply)
    {
        if (_traceLevel > 2)
        {
            Ice::Trace out(_lookup->ice_getCommunicator()->getLogger(), "Lookup");
            out << "ignoring locator reply: (null locator)";
        }
        return;
    }

    Ice::LocatorPrx locator = *reply;
    if (!_instanceName.empty() && locator->ice_getIdentity().category != _instanceName)
    {
        if (_traceLevel > 2)
        {
            Ice::Trace out(_lookup->ice_getCommunicator()->getLogger(), "Lookup");
            out << "ignoring locator reply: instance name doesn't match\n";
            out << "expected = " << _instanceName;
            out << "received = " << locator->ice_getIdentity().category;
        }
        return;
    }

    // If we already have a locator assigned, ensure the given locator has the same identity, otherwise ignore it.
    if (!_pendingRequests.empty() && _locator &&
        locator->ice_getIdentity().category != _locator->ice_getIdentity().category)
    {
        if (!_warned)
        {
            _warned = true; // Only warn once.

            Ice::Warning out(locator->ice_getCommunicator()->getLogger());
            out << "received Ice locator with different instance name:\n";
            out << "using = '" << _locator->ice_getIdentity().category << "'\n";
            out << "received = '" << locator->ice_getIdentity().category << "'\n";
            out << "This is typically the case if multiple Ice locators with different";
            out << "instance names are deployed and the property `IceLocatorDiscovery.InstanceName' ";
            out << "is not set.";
        }
        return;
    }

    if (_pending) // No need to continue, we found a locator.
    {
        _timer->cancel(shared_from_this());
        _pendingRetryCount = 0;
        _pending = false;
    }

    if (_traceLevel > 0)
    {
        Ice::Trace out(_lookup->ice_getCommunicator()->getLogger(), "Lookup");
        out << "locator lookup succeeded:\nlocator = " << locator;
        if (!_instanceName.empty())
        {
            out << "\ninstance name = " << _instanceName;
        }
    }

    auto i = _locators.find(locator->ice_getIdentity().category);
    if (i != _locators.end())
    {
        // We found another locator replica, append its endpoints to the current locator proxy endpoints.
        Ice::EndpointSeq newEndpoints = i->second->ice_getEndpoints();
        Ice::EndpointSeq endpts = locator->ice_getEndpoints();
        for (const auto& endpoint : locator->ice_getEndpoints())
        {
            if (std::find(newEndpoints.begin(), newEndpoints.end(), endpoint) == newEndpoints.end())
            {
                newEndpoints.push_back(endpoint);
            }
        }
        i->second = i->second->ice_endpoints(newEndpoints);
    }
    else
    {
        _locators.insert(make_pair(locator->ice_getIdentity().category, locator));
    }

    Ice::LocatorPrx l = i == _locators.end() ? locator : i->second;

    if (_pendingRequests.empty())
    {
        _conditionVariable.notify_one();
    }
    else
    {
        _locator = l;
        if (_instanceName.empty())
        {
            _instanceName = _locator->ice_getIdentity().category; // Stick to the first discovered locator.
        }

        // Send pending requests if any.
        for (const auto& pendingRequest : _pendingRequests)
        {
            pendingRequest->invoke(l);
        }
        _pendingRequests.clear();
    }
}

void
LocatorI::invoke(const optional<Ice::LocatorPrx>& locator, const RequestPtr& request)
{
    lock_guard lock(_mutex);
    if (request && _locator && _locator != locator)
    {
        request->invoke(*_locator);
    }
    else if (request && chrono::steady_clock::now() < _nextRetry)
    {
        request->invoke(_voidLocator); // Don't retry to find a locator before the retry delay expires
    }
    else
    {
        _locator = nullopt;

        if (request)
        {
            _pendingRequests.push_back(request);
        }

        if (!_pending) // No request in progress
        {
            _pending = true;
            _failureCount = 0;
            _pendingRetryCount = _retryCount;
            try
            {
                if (_traceLevel > 1)
                {
                    Ice::Trace out(_lookup->ice_getCommunicator()->getLogger(), "Lookup");
                    out << "looking up locator:\nlookup = " << _lookup;
                    if (!_instanceName.empty())
                    {
                        out << "\ninstance name = " << _instanceName;
                    }
                }
                for (const auto& l : _lookups)
                {
                    l.first->findLocatorAsync(
                        _instanceName,
                        l.second,
                        nullptr,
                        [self = shared_from_this()](exception_ptr ex) { self->exception(ex); });
                }
                _timer->schedule(shared_from_this(), _timeout);
            }
            catch (const Ice::LocalException& ex)
            {
                if (_traceLevel > 0)
                {
                    Ice::Trace out(_lookup->ice_getCommunicator()->getLogger(), "Lookup");
                    out << "locator lookup failed:\nlookup = " << _lookup;
                    if (!_instanceName.empty())
                    {
                        out << "\ninstance name = " << _instanceName;
                    }
                    out << "\n" << ex;
                }

                for (const auto& pendingRequest : _pendingRequests)
                {
                    pendingRequest->invoke(_voidLocator);
                }
                _pendingRequests.clear();
                _pending = false;
                _pendingRetryCount = 0;
            }
        }
    }
}

void
LocatorI::exception(std::exception_ptr ex)
{
    lock_guard lock(_mutex);
    if (++_failureCount == _lookups.size() && _pending)
    {
        //
        // All the lookup calls failed, cancel the timer and propagate the error to the requests.
        //
        _timer->cancel(shared_from_this());
        _pendingRetryCount = 0;
        _pending = false;

        if (_warnOnce)
        {
            try
            {
                rethrow_exception(ex);
            }
            catch (const std::exception& e)
            {
                Ice::Warning warn(_lookup->ice_getCommunicator()->getLogger());
                warn << "failed to lookup locator with lookup proxy '" << _lookup << "':\n" << e;
                _warnOnce = false;
            }
        }

        if (_traceLevel > 0)
        {
            Ice::Trace out(_lookup->ice_getCommunicator()->getLogger(), "Lookup");
            out << "locator lookup failed:\nlookup = " << _lookup;
            if (!_instanceName.empty())
            {
                out << "\ninstance name = " << _instanceName;
            }

            try
            {
                rethrow_exception(ex);
            }
            catch (const std::exception& e)
            {
                out << "\n" << e;
            }
        }

        if (_pendingRequests.empty())
        {
            _conditionVariable.notify_one();
        }
        else
        {
            for (const auto& pendingRequest : _pendingRequests)
            {
                pendingRequest->invoke(_voidLocator);
            }
            _pendingRequests.clear();
        }
    }
}

void
LocatorI::runTimerTask()
{
    lock_guard lock(_mutex);
    if (!_pending)
    {
        assert(_pendingRequests.empty());
        return; // The request failed
    }

    if (_pendingRetryCount > 0)
    {
        --_pendingRetryCount;
        try
        {
            if (_traceLevel > 1)
            {
                Ice::Trace out(_lookup->ice_getCommunicator()->getLogger(), "Lookup");
                out << "retrying locator lookup:\nlookup = " << _lookup << "\nretry count = " << _pendingRetryCount;
                if (!_instanceName.empty())
                {
                    out << "\ninstance name = " << _instanceName;
                }
            }
            _failureCount = 0;
            for (const auto& l : _lookups)
            {
                l.first->findLocatorAsync(
                    _instanceName,
                    l.second,
                    nullptr,
                    [self = shared_from_this()](exception_ptr ex) { self->exception(ex); });
            }
            _timer->schedule(shared_from_this(), _timeout);
            return;
        }
        catch (const Ice::LocalException&)
        {
        }
        _pendingRetryCount = 0;
    }

    assert(_pendingRetryCount == 0);
    _pending = false;

    if (_traceLevel > 0)
    {
        Ice::Trace out(_lookup->ice_getCommunicator()->getLogger(), "Lookup");
        out << "locator lookup timed out:\nlookup = " << _lookup;
        if (!_instanceName.empty())
        {
            out << "\ninstance name = " << _instanceName;
        }
    }

    if (_pendingRequests.empty())
    {
        _conditionVariable.notify_one();
    }
    else
    {
        for (const auto& pendingRequest : _pendingRequests)
        {
            pendingRequest->invoke(_voidLocator); // Send pending requests on void locator.
        }
        _pendingRequests.clear();
    }
    _nextRetry = chrono::steady_clock::now() + _retryDelay; // Only retry when the retry delay expires
}

void
LookupReplyI::foundLocator(optional<Ice::LocatorPrx> locator, const Ice::Current&)
{
    _locator->foundLocator(locator);
}
