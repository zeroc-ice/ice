// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
#include <Ice/Network.h> // For getInterfacesForMulticast
#include <Ice/LoggerUtil.h>

#include <IceLocatorDiscovery/Plugin.h>
#include <IceLocatorDiscovery/IceLocatorDiscovery.h>

using namespace std;
using namespace IceLocatorDiscovery;

namespace
{

class LocatorI; // Forward declaration

#ifdef ICE_CPP11_MAPPING
typedef std::pair<function<void(bool, const pair<const Ice::Byte*, const Ice::Byte*>&)>,
                  function<void(exception_ptr)>> AMDCallback;
#else
typedef Ice::AMD_Object_ice_invokePtr AMDCallback;
#endif


class Request :
#ifdef ICE_CPP11_MAPPING
        public std::enable_shared_from_this<Request>
#else
        public virtual IceUtil::Shared
#endif
{
public:

    Request(LocatorI* locator,
            const string& operation,
            Ice::OperationMode mode,
            const pair<const Ice::Byte*, const Ice::Byte*>& inParams,
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
    void response(bool, const pair<const Ice::Byte*, const Ice::Byte*>&);
    void exception(const Ice::Exception&);

protected:

    LocatorI* _locator;
    const string _operation;
    const Ice::OperationMode _mode;
    const Ice::Context _context;
    const Ice::ByteSeq _inParams;
    AMDCallback _amdCB;
#ifdef ICE_CPP11_MAPPING
    exception_ptr _exception;
#else
    IceInternal::UniquePtr<Ice::Exception> _exception;
#endif

    Ice::LocatorPrxPtr _locatorPrx;
};
ICE_DEFINE_PTR(RequestPtr, Request);

class LocatorI : public Ice::BlobjectArrayAsync,
                 public IceUtil::TimerTask,
                 private IceUtil::Monitor<IceUtil::Mutex>
#ifdef ICE_CPP11_MAPPING
               , public std::enable_shared_from_this<LocatorI>
#endif
{
public:

    LocatorI(const string&, const LookupPrxPtr&, const Ice::PropertiesPtr&, const string&, const Ice::LocatorPrxPtr&);
    void setLookupReply(const LookupReplyPrxPtr&);

#ifdef ICE_CPP11_MAPPING
    virtual void ice_invokeAsync(pair<const Ice::Byte*, const Ice::Byte*>,
                                 function<void(bool, const pair<const Ice::Byte*, const Ice::Byte*>&)>,
                                 function<void(exception_ptr)>,
                                 const Ice::Current&);
#else
    virtual void ice_invoke_async(const Ice::AMD_Object_ice_invokePtr&, const pair<const Ice::Byte*, const Ice::Byte*>&,
                                  const Ice::Current&);
#endif

    void foundLocator(const Ice::LocatorPrxPtr&);
    void invoke(const Ice::LocatorPrxPtr&, const RequestPtr&);

    vector<Ice::LocatorPrxPtr> getLocators(const string&, const IceUtil::Time&);

    void exception(const Ice::LocalException&);

private:

    virtual void runTimerTask();

    LookupPrxPtr _lookup;
    vector<pair<LookupPrxPtr, LookupReplyPrxPtr> > _lookups;
    const IceUtil::Time _timeout;
    const int _retryCount;
    const IceUtil::Time _retryDelay;
    const IceUtil::TimerPtr _timer;

    string _instanceName;
    bool _warned;
    Ice::LocatorPrxPtr _locator;
    map<string, Ice::LocatorPrxPtr> _locators;
    Ice::LocatorPrxPtr _voidLocator;

    IceUtil::Time _nextRetry;
    int _pendingRetryCount;
    size_t _failureCount;
    bool _warnOnce;
    vector<RequestPtr> _pendingRequests;
};
ICE_DEFINE_PTR(LocatorIPtr, LocatorI);

class LookupReplyI : public LookupReply
{
public:

    LookupReplyI(const LocatorIPtr& locator) : _locator(locator)
    {
    }

    virtual void foundLocator(ICE_IN(Ice::LocatorPrxPtr), const Ice::Current&);

private:

    const LocatorIPtr _locator;
};

const ::std::string IceGrid_Locator_ids[3] =
{
    "::Ice::Locator",
    "::Ice::Object",
    "::IceGrid::Locator"
};

//
// The void locator implementation below is used when no locator is found.
//
class VoidLocatorI : public Ice::Locator
{
public:

#ifdef ICE_CPP11_MAPPING
    virtual void
    findObjectByIdAsync(::Ice::Identity,
                        function<void(const shared_ptr<::Ice::ObjectPrx>&)> response,
                        function<void(exception_ptr)>,
                        const Ice::Current&) const
    {
        response(nullptr);
    }

    virtual void
    findAdapterByIdAsync(string,
                         function<void(const shared_ptr<::Ice::ObjectPrx>&)> response,
                         function<void(exception_ptr)>,
                         const Ice::Current&) const
    {
        response(nullptr);
    }
#else
    virtual void
    findObjectById_async(const Ice::AMD_Locator_findObjectByIdPtr& amdCB,
                         const Ice::Identity&,
                         const Ice::Current&) const
    {
        amdCB->ice_response(0);
    }

    virtual void
    findAdapterById_async(const Ice::AMD_Locator_findAdapterByIdPtr& amdCB,
                          const string&,
                          const Ice::Current&) const
    {
        amdCB->ice_response(0);
    }
#endif

    virtual Ice::LocatorRegistryPrxPtr
    getRegistry(const Ice::Current&) const
    {
        return ICE_NULLPTR;
    }
};

class PluginI : public Plugin
{
public:

    PluginI(const std::string&, const Ice::CommunicatorPtr&);

    virtual void initialize();
    virtual void destroy();
    virtual vector<Ice::LocatorPrxPtr> getLocators(const string&, const IceUtil::Time&) const;

private:

    const string _name;
    const Ice::CommunicatorPtr _communicator;
    Ice::ObjectAdapterPtr _locatorAdapter;
    Ice::ObjectAdapterPtr _replyAdapter;
    LocatorIPtr _locator;
    Ice::LocatorPrxPtr _locatorPrx;
    Ice::LocatorPrxPtr _defaultLocator;
};

#ifndef ICE_CPP11_MAPPING

class CallbackI : public IceUtil::Shared
{
public:

    CallbackI(const LocatorIPtr& locator) : _locator(locator)
    {
    }

    void
    completed(const Ice::AsyncResultPtr& result)
    {
        try
        {
            result->throwLocalException();
        }
        catch(const Ice::LocalException& ex)
        {
            _locator->exception(ex);
        }
    }

private:

    LocatorIPtr _locator;
};

#endif

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

//
// Objective-C function to allow Objective-C programs to register plugin.
//
extern "C" ICE_LOCATOR_DISCOVERY_API void
ICEregisterIceLocatorDiscovery(bool loadOnInitialize)
{
    Ice::registerIceLocatorDiscovery(loadOnInitialize);
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
    _replyAdapter->setLocator(0);
    _locatorAdapter->setLocator(0);

    Ice::ObjectPrxPtr lookupPrx = _communicator->stringToProxy("IceLocatorDiscovery/Lookup -d:" + lookupEndpoints);
    // No collocation optimization for the multicast proxy!
    lookupPrx = lookupPrx->ice_collocationOptimized(false)->ice_router(ICE_NULLPTR);

    Ice::LocatorPrxPtr voidLocator = ICE_UNCHECKED_CAST(Ice::LocatorPrx,
                                                        _locatorAdapter->addWithUUID(ICE_MAKE_SHARED(VoidLocatorI)));

    string instanceName = properties->getProperty(_name + ".InstanceName");
    Ice::Identity id;
    id.name = "Locator";
    id.category = !instanceName.empty() ? instanceName : Ice::generateUUID();
    _locator = ICE_MAKE_SHARED(LocatorI, _name, ICE_UNCHECKED_CAST(LookupPrx, lookupPrx), properties, instanceName,
                               voidLocator);
    _defaultLocator = _communicator->getDefaultLocator();
    _locatorPrx = ICE_UNCHECKED_CAST(Ice::LocatorPrx, _locatorAdapter->add(_locator, id));
    _communicator->setDefaultLocator(_locatorPrx);

    Ice::ObjectPrxPtr lookupReply = _replyAdapter->addWithUUID(ICE_MAKE_SHARED(LookupReplyI, _locator))->ice_datagram();
    _locator->setLookupReply(ICE_UNCHECKED_CAST(LookupReplyPrx, lookupReply));

    _replyAdapter->activate();
    _locatorAdapter->activate();
}

vector<Ice::LocatorPrxPtr>
PluginI::getLocators(const string& instanceName, const IceUtil::Time& waitTime) const
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
#ifdef ICE_CPP11_MAPPING
    if(l != _locatorPrx)
    {
        _locatorPrx = l;
        try
        {
            auto self = shared_from_this();
            l->ice_invokeAsync(_operation, _mode, _inParams,
                               [self](bool ok, vector<Ice::Byte> outParams)
                               {
                                   pair<const Ice::Byte*, const Ice::Byte*> outPair;
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
                                   try
                                   {
                                       rethrow_exception(e);
                                   }
                                   catch(const Ice::Exception& ex)
                                   {
                                       self->exception(ex);
                                   }
                               },
                               nullptr,
                               _context);
        }
        catch(const Ice::LocalException& ex)
        {
            exception(ex);
        }
    }
    else
    {
        assert(_exception); // Don't retry if the proxy didn't change
        _amdCB.second(_exception);
    }

#else
    if(l != _locatorPrx)
    {
        _locatorPrx = l;
        try
        {
            l->begin_ice_invoke(_operation, _mode, _inParams, _context,
                                Ice::newCallback_Object_ice_invoke(this, &Request::response, &Request::exception));
        }
        catch(const Ice::LocalException& ex)
        {
            exception(ex);
        }
    }
    else
    {
        assert(_exception.get()); // Don't retry if the proxy didn't change
        _amdCB->ice_exception(*_exception.get());
    }
#endif
}


void
Request::response(bool ok, const pair<const Ice::Byte*, const Ice::Byte*>& outParams)
{
#ifdef ICE_CPP11_MAPPING
    _amdCB.first(ok, outParams);
#else
    _amdCB->ice_response(ok, outParams);
#endif
}

void
Request::exception(const Ice::Exception& ex)
{
#ifdef ICE_CPP11_MAPPING
    try
    {
        ex.ice_throw();
    }
    catch(const Ice::RequestFailedException&)
    {
        _amdCB.second(current_exception());
    }
    catch(const Ice::UnknownException&)
    {
        _amdCB.second(current_exception());
    }
    catch(const Ice::NoEndpointException&)
    {
        try
        {
            throw Ice::ObjectNotExistException(__FILE__, __LINE__);
        }
        catch(...)
        {
            _amdCB.second(current_exception());
        }
    }
    catch(const Ice::CommunicatorDestroyedException&)
    {
        try
        {
            throw Ice::ObjectNotExistException(__FILE__, __LINE__);
        }
        catch(...)
        {
            _amdCB.second(current_exception());
        }
    }
    catch(const Ice::ObjectAdapterDeactivatedException&)
    {
        try
        {
            throw Ice::ObjectNotExistException(__FILE__, __LINE__);
        }
        catch(...)
        {
            _amdCB.second(current_exception());
        }
    }
    catch(const Ice::Exception&)
    {
        _exception = current_exception();
        _locator->invoke(_locatorPrx, shared_from_this()); // Retry with new locator proxy
    }
#else
    try
    {
        ex.ice_throw();
    }
    catch(const Ice::RequestFailedException&)
    {
        _amdCB->ice_exception(ex);
    }
    catch(const Ice::UnknownException&)
    {
        _amdCB->ice_exception(ex);
    }
    catch(const Ice::NoEndpointException&)
    {
        _amdCB->ice_exception(Ice::ObjectNotExistException(__FILE__, __LINE__));
    }
    catch(const Ice::CommunicatorDestroyedException&)
    {
        _amdCB->ice_exception(Ice::ObjectNotExistException(__FILE__, __LINE__));
    }
    catch(const Ice::ObjectAdapterDeactivatedException&)
    {
        _amdCB->ice_exception(Ice::ObjectNotExistException(__FILE__, __LINE__));
    }
    catch(const Ice::Exception&)
    {
        _exception.reset(ex.ice_clone());
        _locator->invoke(_locatorPrx, this); // Retry with new locator proxy
    }
#endif
}

LocatorI::LocatorI(const string& name,
                   const LookupPrxPtr& lookup,
                   const Ice::PropertiesPtr& p,
                   const string& instanceName,
                   const Ice::LocatorPrxPtr& voidLocator) :
    _lookup(lookup),
    _timeout(IceUtil::Time::milliSeconds(p->getPropertyAsIntWithDefault(name + ".Timeout", 300))),
    _retryCount(p->getPropertyAsIntWithDefault(name + ".RetryCount", 3)),
    _retryDelay(IceUtil::Time::milliSeconds(p->getPropertyAsIntWithDefault(name + ".RetryDelay", 2000))),
    _timer(IceInternal::getInstanceTimer(lookup->ice_getCommunicator())),
    _instanceName(instanceName),
    _warned(false),
    _locator(lookup->ice_getCommunicator()->getDefaultLocator()),
    _voidLocator(voidLocator),
    _pendingRetryCount(0),
    _failureCount(0),
    _warnOnce(true)
{
    //
    // Create one lookup proxy per endpoint from the given proxy. We want to send a multicast
    // datagram on each endpoint.
    //
    Ice::EndpointSeq endpoints = lookup->ice_getEndpoints();
    for(vector<Ice::EndpointPtr>::const_iterator p = endpoints.begin(); p != endpoints.end(); ++p)
    {
        Ice::EndpointSeq single;
        single.push_back(*p);
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
        Ice::UDPEndpointInfoPtr info = ICE_DYNAMIC_CAST(Ice::UDPEndpointInfo, p->first->ice_getEndpoints()[0]->getInfo());
        if(info && !info->mcastInterface.empty())
        {
            Ice::EndpointSeq endpts = lookupReply->ice_getEndpoints();
            for(Ice::EndpointSeq::const_iterator q = endpts.begin(); q != endpts.end(); ++q)
            {
                Ice::IPEndpointInfoPtr r = ICE_DYNAMIC_CAST(Ice::IPEndpointInfo, (*q)->getInfo());
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

#ifdef ICE_CPP11_MAPPING
void
LocatorI::ice_invokeAsync(pair<const Ice::Byte*, const Ice::Byte*> inParams,
                          function<void(bool, const pair<const Ice::Byte*, const Ice::Byte*>&)> responseCB,
                          function<void(exception_ptr)> exceptionCB,
                          const Ice::Current& current)
{
    invoke(nullptr, make_shared<Request>(this, current.operation, current.mode, inParams, current.ctx,
                                         make_pair(move(responseCB), move(exceptionCB))));
}
#else
void
LocatorI::ice_invoke_async(const Ice::AMD_Object_ice_invokePtr& amdCB,
                           const pair<const Ice::Byte*, const Ice::Byte*>& inParams,
                           const Ice::Current& current)
{
    invoke(0, new Request(this, current.operation, current.mode, inParams, current.ctx, amdCB));
}
#endif

vector<Ice::LocatorPrxPtr>
LocatorI::getLocators(const string& instanceName, const IceUtil::Time& waitTime)
{
    //
    // Clear locators from previous search
    //
    {
        Lock sync(*this);
        _locators.clear();
    }

    //
    // Find a locator
    //
    invoke(ICE_NULLPTR, ICE_NULLPTR);

    //
    // Wait for responses
    //
    if(instanceName.empty())
    {
        IceUtil::ThreadControl::sleep(waitTime);
    }
    else
    {
        Lock sync(*this);
        while(_locators.find(instanceName) == _locators.end() && _pendingRetryCount > 0)
        {
            timedWait(waitTime);
        }
    }

    //
    // Return found locators
    //
    Lock sync(*this);
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
    Lock sync(*this);
    if(!locator || (!_instanceName.empty() && locator->ice_getIdentity().category != _instanceName))
    {
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

    if(_pendingRetryCount > 0) // No need to retry, we found a locator.
    {
        _timer->cancel(ICE_SHARED_FROM_THIS);
        _pendingRetryCount = 0;
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
        notify();
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
    Lock sync(*this);
    if(request && _locator && _locator != locator)
    {
        request->invoke(_locator);
    }
    else if(request && IceUtil::Time::now() < _nextRetry)
    {
        request->invoke(_voidLocator); // Don't retry to find a locator before the retry delay expires
    }
    else
    {
        _locator = 0;

        if(request)
        {
            _pendingRequests.push_back(request);
        }

        if(_pendingRetryCount == 0) // No request in progress
        {
            _failureCount = 0;
            _pendingRetryCount = _retryCount;
            try
            {
                for(vector<pair<LookupPrxPtr, LookupReplyPrxPtr> >::const_iterator l = _lookups.begin();
                    l != _lookups.end(); ++l)
                {
#ifdef ICE_CPP11_MAPPING
                    auto self = shared_from_this();
                    l->first->findLocatorAsync(_instanceName, l->second, nullptr, [self](exception_ptr ex)
                    {
                        try
                        {
                            rethrow_exception(ex);
                        }
                        catch(const Ice::LocalException& e)
                        {
                            self->exception(e);
                        }
                    });
#else
                    l->first->begin_findLocator(_instanceName, l->second, Ice::newCallback(new CallbackI(this),
                                                                                           &CallbackI::completed));
#endif
                }
                _timer->schedule(ICE_SHARED_FROM_THIS, _timeout);
            }
            catch(const Ice::LocalException&)
            {
                for(vector<RequestPtr>::const_iterator p = _pendingRequests.begin(); p != _pendingRequests.end(); ++p)
                {
                    (*p)->invoke(_voidLocator);
                }
                _pendingRequests.clear();
                _pendingRetryCount = 0;
            }
        }
    }
}

void
LocatorI::exception(const Ice::LocalException& ex)
{
    Lock sync(*this);
    if(++_failureCount == _lookups.size() && _pendingRetryCount > 0)
    {
        //
        // All the lookup calls failed, cancel the timer and propagate the error to the requests.
        //
        _timer->cancel(ICE_SHARED_FROM_THIS);
        _pendingRetryCount = 0;

        if(_warnOnce)
        {
            Ice::Warning warn(_lookup->ice_getCommunicator()->getLogger());
            warn << "failed to lookup locator with lookup proxy `" << _lookup << "':\n" << ex;
            _warnOnce = false;
        }

        if(_pendingRequests.empty())
        {
            notify();
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
    Lock sync(*this);
    if(--_pendingRetryCount > 0)
    {
        try
        {
            _failureCount = 0;
            for(vector<pair<LookupPrxPtr, LookupReplyPrxPtr> >::const_iterator l = _lookups.begin();
                l != _lookups.end(); ++l)
            {
#ifdef ICE_CPP11_MAPPING
                    auto self = shared_from_this();
                    l->first->findLocatorAsync(_instanceName, l->second, nullptr, [self](exception_ptr ex)
                    {
                        try
                        {
                            rethrow_exception(ex);
                        }
                        catch(const Ice::LocalException& e)
                        {
                            self->exception(e);
                        }
                    });
#else
                    l->first->begin_findLocator(_instanceName, l->second, Ice::newCallback(new CallbackI(this),
                                                                                           &CallbackI::completed));
#endif
            }
            _timer->schedule(ICE_SHARED_FROM_THIS, _timeout);
            return;
        }
        catch(const Ice::LocalException&)
        {
        }
        _pendingRetryCount = 0;
    }

    if(_pendingRequests.empty())
    {
        notify();
    }
    else
    {
        for(vector<RequestPtr>::const_iterator p = _pendingRequests.begin(); p != _pendingRequests.end(); ++p)
        {
            (*p)->invoke(_voidLocator); // Send pending requests on void locator.
        }
       _pendingRequests.clear();
    }
    _nextRetry = IceUtil::Time::now() + _retryDelay; // Only retry when the retry delay expires
}

void
LookupReplyI::foundLocator(ICE_IN(Ice::LocatorPrxPtr) locator, const Ice::Current&)
{
    _locator->foundLocator(locator);
}
