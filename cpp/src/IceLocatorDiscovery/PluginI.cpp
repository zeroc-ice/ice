//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
#include <Ice/Network.h> // For getInterfacesForMulticast
#include <Ice/LoggerUtil.h>

#include <IceLocatorDiscovery/Plugin.h>
#include <IceLocatorDiscovery/IceLocatorDiscovery.h>

#include <thread>

using namespace std;
using namespace IceLocatorDiscovery;

namespace
{

class LocatorI; // Forward declaration
typedef std::pair<function<void(bool, const pair<const uint8_t*, const uint8_t*>&)>,
                  function<void(exception_ptr)>> AMDCallback;

class Request : public std::enable_shared_from_this<Request>
{
public:

    Request(LocatorI* locator,
            const string& operation,
            Ice::OperationMode mode,
            const pair<const uint8_t*, const uint8_t*>& inParams,
            const Ice::Context& ctx,
            const AMDCallback& amdCB) :
        _locator(locator),
        _operation(operation),
        _mode(mode),
        _context(ctx),
        _inParams(inParams.first, inParams.second),
        _amdCB(amdCB)
    {
    }

    void invoke(const Ice::LocatorPrxPtr&);
    void response(bool, const pair<const uint8_t*, const uint8_t*>&);
    void exception(std::exception_ptr);

protected:

    LocatorI* _locator;
    const string _operation;
    const Ice::OperationMode _mode;
    const Ice::Context _context;
    const Ice::ByteSeq _inParams;
    AMDCallback _amdCB;
    exception_ptr _exception;

    Ice::LocatorPrxPtr _locatorPrx;
};
using RequestPtr = std::shared_ptr<Request>;

class LocatorI : public Ice::BlobjectArrayAsync,
                 public IceUtil::TimerTask,
                 public std::enable_shared_from_this<LocatorI>
{
public:

    LocatorI(const string&, const LookupPrxPtr&, const Ice::PropertiesPtr&, const string&, const Ice::LocatorPrxPtr&);
    void setLookupReply(const LookupReplyPrxPtr&);

    virtual void ice_invokeAsync(pair<const uint8_t*, const uint8_t*>,
                                 function<void(bool, const pair<const uint8_t*, const uint8_t*>&)>,
                                 function<void(exception_ptr)>,
                                 const Ice::Current&);

    void foundLocator(const Ice::LocatorPrxPtr&);
    void invoke(const Ice::LocatorPrxPtr&, const RequestPtr&);

    vector<Ice::LocatorPrxPtr> getLocators(const string&, const chrono::milliseconds&);

    void exception(std::exception_ptr);

private:

    virtual void runTimerTask();

    LookupPrxPtr _lookup;
    vector<pair<LookupPrxPtr, LookupReplyPrxPtr> > _lookups;
    chrono::milliseconds _timeout;
    int _retryCount;
    chrono::milliseconds _retryDelay;
    const IceUtil::TimerPtr _timer;
    const int _traceLevel;

    string _instanceName;
    bool _warned;
    Ice::LocatorPrxPtr _locator;
    map<string, Ice::LocatorPrxPtr> _locators;
    Ice::LocatorPrxPtr _voidLocator;

    chrono::steady_clock::time_point _nextRetry;
    bool _pending;
    int _pendingRetryCount;
    size_t _failureCount;
    bool _warnOnce;
    vector<RequestPtr> _pendingRequests;
    std::mutex _mutex;
    std::condition_variable _conditionVariable;
};
using LocatorIPtr = std::shared_ptr<LocatorI>;

class LookupReplyI : public LookupReply
{
public:

    LookupReplyI(const LocatorIPtr& locator) : _locator(locator)
    {
    }

    virtual void foundLocator(Ice::LocatorPrxPtr, const Ice::Current&);

private:

    const LocatorIPtr _locator;
};

//
// The void locator implementation below is used when no locator is found.
//
class VoidLocatorI : public Ice::Locator
{
public:

    virtual void
    findObjectByIdAsync(::Ice::Identity,
                        function<void(const ::Ice::ObjectPrxPtr&)> response,
                        function<void(exception_ptr)>,
                        const Ice::Current&) const
    {
        response(nullopt);
    }

    virtual void
    findAdapterByIdAsync(string,
                         function<void(const ::Ice::ObjectPrxPtr&)> response,
                         function<void(exception_ptr)>,
                         const Ice::Current&) const
    {
        response(nullopt);
    }

    virtual Ice::LocatorRegistryPrxPtr
    getRegistry(const Ice::Current&) const
    {
        return nullopt;
    }
};

class PluginI : public Plugin
{
public:

    PluginI(const std::string&, const Ice::CommunicatorPtr&);

    virtual void initialize();
    virtual void destroy();
    virtual vector<Ice::LocatorPrxPtr> getLocators(const string&, const chrono::milliseconds&) const;

private:

    const string _name;
    const Ice::CommunicatorPtr _communicator;
    Ice::ObjectAdapterPtr _locatorAdapter;
    Ice::ObjectAdapterPtr _replyAdapter;
    LocatorIPtr _locator;
    Ice::LocatorPrxPtr _locatorPrx;
    Ice::LocatorPrxPtr _defaultLocator;
};

}

//
// Plugin factory function.
//
extern "C" ICE_LOCATOR_DISCOVERY_API Ice::Plugin*
createIceLocatorDiscovery(const Ice::CommunicatorPtr& communicator, const string& name, const Ice::StringSeq&)
{
    return new PluginI(name, communicator);
}

namespace Ice
{

ICE_LOCATOR_DISCOVERY_API void
registerIceLocatorDiscovery(bool loadOnInitialize)
{
    Ice::registerPluginFactory("IceLocatorDiscovery", createIceLocatorDiscovery, loadOnInitialize);

#ifdef ICE_STATIC_LIBS
    //
    // Also register the UDP plugin with static builds to ensure the UDP transport is loaded.
    //
    registerIceUDP(true);
#endif
}

}

PluginI::PluginI(const string& name, const Ice::CommunicatorPtr& communicator) :
    _name(name), _communicator(communicator)
{
}

void
PluginI::initialize()
{
    Ice::PropertiesPtr properties = _communicator->getProperties();

    bool ipv4 = properties->getPropertyAsIntWithDefault("Ice.IPv4", 1) > 0;
    bool preferIPv6 = properties->getPropertyAsInt("Ice.PreferIPv6Address") > 0;
    string address;
    if(ipv4 && !preferIPv6)
    {
        address = properties->getPropertyWithDefault(_name + ".Address", "239.255.0.1");
    }
    else
    {
        address = properties->getPropertyWithDefault(_name + ".Address", "ff15::1");
    }
    int port = properties->getPropertyAsIntWithDefault(_name + ".Port", 4061);
    string intf = properties->getProperty(_name + ".Interface");

    string lookupEndpoints = properties->getProperty(_name + ".Lookup");
    if(lookupEndpoints.empty())
    {
        //
        // If no lookup endpoints are specified, we get all the network interfaces and create
        // an endpoint for each of them. We'll send UDP multicast packages on each interface.
        //
        IceInternal::ProtocolSupport protocol = ipv4 && !preferIPv6 ? IceInternal::EnableIPv4 : IceInternal::EnableIPv6;
        vector<string> interfaces = IceInternal::getInterfacesForMulticast(intf, protocol);
        ostringstream lookup;
        for(vector<string>::const_iterator p = interfaces.begin(); p != interfaces.end(); ++p)
        {
            if(p != interfaces.begin())
            {
                lookup << ":";
            }
            lookup << "udp -h \"" << address << "\" -p " << port << " --interface \"" << *p << "\"";
        }
        lookupEndpoints = lookup.str();
    }

    if(properties->getProperty(_name + ".Reply.Endpoints").empty())
    {
        properties->setProperty(_name + ".Reply.Endpoints", "udp -h " + (intf.empty() ? "*" : "\"" + intf + "\""));
    }

    if(properties->getProperty(_name + ".Locator.Endpoints").empty())
    {
        properties->setProperty(_name + ".Locator.AdapterId", Ice::generateUUID()); // Collocated adapter
    }

    _replyAdapter = _communicator->createObjectAdapter(_name + ".Reply");
    _locatorAdapter = _communicator->createObjectAdapter(_name + ".Locator");

    // We don't want those adapters to be registered with the locator so clear their locator.
    _replyAdapter->setLocator(nullopt);
    _locatorAdapter->setLocator(nullopt);

    Ice::ObjectPrxPtr lookupPrx = _communicator->stringToProxy("IceLocatorDiscovery/Lookup -d:" + lookupEndpoints);
    // No collocation optimization for the multicast proxy!
    lookupPrx = lookupPrx->ice_collocationOptimized(false)->ice_router(nullopt);

    auto voidLocator = Ice::uncheckedCast<Ice::LocatorPrx>(_locatorAdapter->addWithUUID(make_shared<VoidLocatorI>()));

    string instanceName = properties->getProperty(_name + ".InstanceName");
    Ice::Identity id;
    id.name = "Locator";
    id.category = !instanceName.empty() ? instanceName : Ice::generateUUID();
    _locator = make_shared<LocatorI>(_name, Ice::uncheckedCast<LookupPrx>(lookupPrx), properties, instanceName,
                               voidLocator);
    _defaultLocator = _communicator->getDefaultLocator();
    _locatorPrx = Ice::uncheckedCast<Ice::LocatorPrx>(_locatorAdapter->add(_locator, id));
    _communicator->setDefaultLocator(_locatorPrx);

    Ice::ObjectPrxPtr lookupReply = _replyAdapter->addWithUUID(make_shared<LookupReplyI>(_locator))->ice_datagram();
    _locator->setLookupReply(Ice::uncheckedCast<LookupReplyPrx>(lookupReply));

    _replyAdapter->activate();
    _locatorAdapter->activate();
}

vector<Ice::LocatorPrxPtr>
PluginI::getLocators(const string& instanceName, const chrono::milliseconds& waitTime) const
{
    return _locator->getLocators(instanceName, waitTime);
}

void
PluginI::destroy()
{
    if(_replyAdapter)
    {
        _replyAdapter->destroy();
    }
    if(_locatorAdapter)
    {
        _locatorAdapter->destroy();
    }
    if(_communicator->getDefaultLocator() == _locatorPrx)
    {
        // Restore original default locator proxy, if the user didn't change it in the meantime
        _communicator->setDefaultLocator(_defaultLocator);
    }
}

void
Request::invoke(const Ice::LocatorPrxPtr& l)
{
    if(l != _locatorPrx)
    {
        _locatorPrx = l;
        try
        {
            auto self = shared_from_this();
            l->ice_invokeAsync(_operation, _mode, _inParams,
                               [self](bool ok, vector<uint8_t> outParams)
                               {
                                   pair<const uint8_t*, const uint8_t*> outPair;
                                   if(outParams.empty())
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
                               [self](exception_ptr e)
                               {
                                   self->exception(e);
                               },
                               nullptr,
                               _context);
        }
        catch(const Ice::LocalException&)
        {
            exception(current_exception());
        }
    }
    else
    {
        assert(_exception); // Don't retry if the proxy didn't change
        _amdCB.second(_exception);
    }
}

void
Request::response(bool ok, const pair<const uint8_t*, const uint8_t*>& outParams)
{
    _amdCB.first(ok, outParams);
}

void
Request::exception(std::exception_ptr ex)
{
    try
    {
        rethrow_exception(ex);
    }
    catch(const Ice::RequestFailedException&)
    {
        _amdCB.second(ex);
    }
    catch(const Ice::UnknownException&)
    {
        _amdCB.second(ex);
    }
    catch(const Ice::NoEndpointException&)
    {
        _amdCB.second(make_exception_ptr(Ice::ObjectNotExistException(__FILE__, __LINE__)));
    }
    catch(const Ice::CommunicatorDestroyedException&)
    {
        _amdCB.second(make_exception_ptr(Ice::ObjectNotExistException(__FILE__, __LINE__)));
    }
    catch(const Ice::ObjectAdapterDeactivatedException&)
    {
        _amdCB.second(make_exception_ptr(Ice::ObjectNotExistException(__FILE__, __LINE__)));
    }
    catch (...)
    {
        _exception = ex;
        _locator->invoke(_locatorPrx, shared_from_this()); // Retry with new locator proxy
    }
}

LocatorI::LocatorI(const string& name,
                   const LookupPrxPtr& lookup,
                   const Ice::PropertiesPtr& p,
                   const string& instanceName,
                   const Ice::LocatorPrxPtr& voidLocator) :
    _lookup(lookup),
    _timeout(chrono::milliseconds(p->getPropertyAsIntWithDefault(name + ".Timeout", 300))),
    _retryCount(p->getPropertyAsIntWithDefault(name + ".RetryCount", 3)),
    _retryDelay(chrono::milliseconds(p->getPropertyAsIntWithDefault(name + ".RetryDelay", 2000))),
    _timer(IceInternal::getInstanceTimer(lookup->ice_getCommunicator())),
    _traceLevel(p->getPropertyAsInt(name + ".Trace.Lookup")),
    _instanceName(instanceName),
    _warned(false),
    _locator(lookup->ice_getCommunicator()->getDefaultLocator()),
    _voidLocator(voidLocator),
    _pending(false),
    _pendingRetryCount(0),
    _failureCount(0),
    _warnOnce(true)
{
    if(_timeout < chrono::milliseconds::zero())
    {
        _timeout = chrono::milliseconds(300);
    }
    if(_retryCount < 0)
    {
        _retryCount = 0;
    }
    if(_retryDelay < chrono::milliseconds::zero())
    {
        _retryDelay = chrono::milliseconds::zero();
    }

    //
    // Create one lookup proxy per endpoint from the given proxy. We want to send a multicast
    // datagram on each endpoint.
    //
    Ice::EndpointSeq endpoints = lookup->ice_getEndpoints();
    for(vector<Ice::EndpointPtr>::const_iterator q = endpoints.begin(); q != endpoints.end(); ++q)
    {
        Ice::EndpointSeq single;
        single.push_back(*q);
        _lookups.push_back(make_pair(lookup->ice_endpoints(single), LookupReplyPrxPtr()));
    }
    assert(!_lookups.empty());
}

void
LocatorI::setLookupReply(const LookupReplyPrxPtr& lookupReply)
{
    //
    // Use a lookup reply proxy whose adress matches the interface used to send multicast datagrams.
    //
    for(vector<pair<LookupPrxPtr, LookupReplyPrxPtr> >::iterator p = _lookups.begin(); p != _lookups.end(); ++p)
    {
        Ice::UDPEndpointInfoPtr info = dynamic_pointer_cast<Ice::UDPEndpointInfo>(p->first->ice_getEndpoints()[0]->getInfo());
        if(info && !info->mcastInterface.empty())
        {
            Ice::EndpointSeq endpts = lookupReply->ice_getEndpoints();
            for(Ice::EndpointSeq::const_iterator q = endpts.begin(); q != endpts.end(); ++q)
            {
                Ice::IPEndpointInfoPtr r = dynamic_pointer_cast<Ice::IPEndpointInfo>((*q)->getInfo());
                if(r && r->host == info->mcastInterface)
                {
                    Ice::EndpointSeq single;
                    single.push_back(*q);
                    p->second = lookupReply->ice_endpoints(single);
                }
            }
        }

        if(!p->second)
        {
            p->second = lookupReply; // Fallback: just use the given lookup reply proxy if no matching endpoint found.
        }
    }
}

void
LocatorI::ice_invokeAsync(pair<const uint8_t*, const uint8_t*> inParams,
                          function<void(bool, const pair<const uint8_t*, const uint8_t*>&)> responseCB,
                          function<void(exception_ptr)> exceptionCB,
                          const Ice::Current& current)
{
    invoke(nullopt, make_shared<Request>(this, current.operation, current.mode, inParams, current.ctx,
                                         make_pair(std::move(responseCB), std::move(exceptionCB))));
}

vector<Ice::LocatorPrxPtr>
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
    if(instanceName.empty())
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
    vector<Ice::LocatorPrxPtr> locators;
    for(map<string, Ice::LocatorPrxPtr>::const_iterator p = _locators.begin(); p != _locators.end(); ++p)
    {
        locators.push_back(p->second);
    }
    return locators;
}

void
LocatorI::foundLocator(const Ice::LocatorPrxPtr& locator)
{
    lock_guard lock(_mutex);

    if(!locator)
    {
        if(_traceLevel > 2)
        {
            Ice::Trace out(_lookup->ice_getCommunicator()->getLogger(), "Lookup");
            out << "ignoring locator reply: (null locator)";
        }
        return;
    }

    if(!_instanceName.empty() && locator->ice_getIdentity().category != _instanceName)
    {
        if(_traceLevel > 2)
        {
            Ice::Trace out(_lookup->ice_getCommunicator()->getLogger(), "Lookup");
            out << "ignoring locator reply: instance name doesn't match\n";
            out << "expected = " << _instanceName;
            out << "received = " << locator->ice_getIdentity().category;
        }
        return;
    }

    //
    // If we already have a locator assigned, ensure the given locator
    // has the same identity, otherwise ignore it.
    //
    if(!_pendingRequests.empty() &&
       _locator && locator->ice_getIdentity().category != _locator->ice_getIdentity().category)
    {
        if(!_warned)
        {
            _warned = true; // Only warn once.

            Ice::Warning out(locator->ice_getCommunicator()->getLogger());
            out << "received Ice locator with different instance name:\n";
            out << "using = `" << _locator->ice_getIdentity().category << "'\n";
            out << "received = `" << locator->ice_getIdentity().category << "'\n";
            out << "This is typically the case if multiple Ice locators with different";
            out << "instance names are deployed and the property `IceLocatorDiscovery.InstanceName' ";
            out << "is not set.";
        }
        return;
    }

    if(_pending) // No need to continue, we found a locator.
    {
        _timer->cancel(shared_from_this());
        _pendingRetryCount = 0;
        _pending = false;
    }

    if(_traceLevel > 0)
    {
        Ice::Trace out(_lookup->ice_getCommunicator()->getLogger(), "Lookup");
        out << "locator lookup succeeded:\nlocator = " << locator;
        if(!_instanceName.empty())
        {
            out << "\ninstance name = " << _instanceName;
        }
    }

    Ice::LocatorPrxPtr l = _pendingRequests.empty() ? _locators[locator->ice_getIdentity().category] : _locator;
    if(l)
    {
        //
        // We found another locator replica, append its endpoints to the
        // current locator proxy endpoints.
        //
        Ice::EndpointSeq newEndpoints = l->ice_getEndpoints();
        Ice::EndpointSeq endpts = locator->ice_getEndpoints();
        for(Ice::EndpointSeq::const_iterator p = endpts.begin(); p != endpts.end(); ++p)
        {
            //
            // Only add endpoints if not already in the locator proxy endpoints
            //
            bool found = false;
            for(Ice::EndpointSeq::const_iterator q = newEndpoints.begin(); q != newEndpoints.end(); ++q)
            {
                if(*p == *q)
                {
                    found = true;
                    break;
                }
            }
            if(!found)
            {
                newEndpoints.push_back(*p);
            }
        }
        l = l->ice_endpoints(newEndpoints);
    }
    else
    {
        l = locator;
    }

    if(_pendingRequests.empty())
    {
        _locators[locator->ice_getIdentity().category] = l;
        _conditionVariable.notify_one();
    }
    else
    {
        _locator = l;
        if(_instanceName.empty())
        {
            _instanceName = _locator->ice_getIdentity().category; // Stick to the first discovered locator.
        }

        //
        // Send pending requests if any.
        //
        for(vector<RequestPtr>::const_iterator p = _pendingRequests.begin(); p != _pendingRequests.end(); ++p)
        {
            (*p)->invoke(_locator);
        }
        _pendingRequests.clear();
    }
}

void
LocatorI::invoke(const Ice::LocatorPrxPtr& locator, const RequestPtr& request)
{
    lock_guard lock(_mutex);
    if(request && _locator && _locator != locator)
    {
        request->invoke(_locator);
    }
    else if(request && chrono::steady_clock::now() < _nextRetry)
    {
        request->invoke(_voidLocator); // Don't retry to find a locator before the retry delay expires
    }
    else
    {
        _locator = nullopt;

        if(request)
        {
            _pendingRequests.push_back(request);
        }

        if(!_pending) // No request in progress
        {
            _pending = true;
            _failureCount = 0;
            _pendingRetryCount = _retryCount;
            try
            {
                if(_traceLevel > 1)
                {
                    Ice::Trace out(_lookup->ice_getCommunicator()->getLogger(), "Lookup");
                    out << "looking up locator:\nlookup = " << _lookup;
                    if(!_instanceName.empty())
                    {
                        out << "\ninstance name = " << _instanceName;
                    }
                }
                for(vector<pair<LookupPrxPtr, LookupReplyPrxPtr> >::const_iterator l = _lookups.begin();
                    l != _lookups.end(); ++l)
                {
                    auto self = shared_from_this();
                    l->first->findLocatorAsync(_instanceName, l->second, nullptr, [self](exception_ptr ex)
                    {
                        self->exception(ex);
                    });
                }
                _timer->schedule(shared_from_this(), _timeout);
            }
            catch(const Ice::LocalException& ex)
            {
                if(_traceLevel > 0)
                {
                    Ice::Trace out(_lookup->ice_getCommunicator()->getLogger(), "Lookup");
                    out << "locator lookup failed:\nlookup = " << _lookup;
                    if(!_instanceName.empty())
                    {
                        out << "\ninstance name = " << _instanceName;
                    }
                    out << "\n" << ex;
                }

                for(vector<RequestPtr>::const_iterator p = _pendingRequests.begin(); p != _pendingRequests.end(); ++p)
                {
                    (*p)->invoke(_voidLocator);
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
    if(++_failureCount == _lookups.size() && _pending)
    {
        //
        // All the lookup calls failed, cancel the timer and propagate the error to the requests.
        //
        _timer->cancel(shared_from_this());
        _pendingRetryCount = 0;
        _pending = false;

        if(_warnOnce)
        {
            try
            {
                rethrow_exception(ex);
            }
            catch (const std::exception& e)
            {
                Ice::Warning warn(_lookup->ice_getCommunicator()->getLogger());
                warn << "failed to lookup locator with lookup proxy `" << _lookup << "':\n" << e;
                _warnOnce = false;
            }
        }

        if(_traceLevel > 0)
        {
            Ice::Trace out(_lookup->ice_getCommunicator()->getLogger(), "Lookup");
            out << "locator lookup failed:\nlookup = " << _lookup;
            if(!_instanceName.empty())
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

        if(_pendingRequests.empty())
        {
            _conditionVariable.notify_one();
        }
        else
        {
            for(vector<RequestPtr>::const_iterator p = _pendingRequests.begin(); p != _pendingRequests.end(); ++p)
            {
                (*p)->invoke(_voidLocator);
            }
           _pendingRequests.clear();
        }
    }
}

void
LocatorI::runTimerTask()
{
    lock_guard lock(_mutex);
    if(!_pending)
    {
        assert(_pendingRequests.empty());
        return; // The request failed
    }

    if(_pendingRetryCount > 0)
    {
        --_pendingRetryCount;
        try
        {
            if(_traceLevel > 1)
            {
                Ice::Trace out(_lookup->ice_getCommunicator()->getLogger(), "Lookup");
                out << "retrying locator lookup:\nlookup = " << _lookup << "\nretry count = " << _pendingRetryCount;
                if(!_instanceName.empty())
                {
                    out << "\ninstance name = " << _instanceName;
                }
            }
            _failureCount = 0;
            for(vector<pair<LookupPrxPtr, LookupReplyPrxPtr> >::const_iterator l = _lookups.begin();
                l != _lookups.end(); ++l)
            {
                auto self = shared_from_this();
                l->first->findLocatorAsync(_instanceName, l->second, nullptr, [self](exception_ptr ex)
                {
                    self->exception(ex);
                });
            }
            _timer->schedule(shared_from_this(), _timeout);
            return;
        }
        catch(const Ice::LocalException&)
        {
        }
        _pendingRetryCount = 0;
    }

    assert(_pendingRetryCount == 0);
    _pending = false;

    if(_traceLevel > 0)
    {
        Ice::Trace out(_lookup->ice_getCommunicator()->getLogger(), "Lookup");
        out << "locator lookup timed out:\nlookup = " << _lookup;
        if(!_instanceName.empty())
        {
            out << "\ninstance name = " << _instanceName;
        }
    }

    if(_pendingRequests.empty())
    {
        _conditionVariable.notify_one();
    }
    else
    {
        for(vector<RequestPtr>::const_iterator p = _pendingRequests.begin(); p != _pendingRequests.end(); ++p)
        {
            (*p)->invoke(_voidLocator); // Send pending requests on void locator.
        }
       _pendingRequests.clear();
    }
    _nextRetry = chrono::steady_clock::now() + _retryDelay; // Only retry when the retry delay expires
}

void
LookupReplyI::foundLocator(Ice::LocatorPrxPtr locator, const Ice::Current&)
{
    _locator->foundLocator(locator);
}
