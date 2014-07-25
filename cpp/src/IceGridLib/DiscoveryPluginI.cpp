// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_API_EXPORTS
#   define ICE_GRID_API_EXPORTS
#endif

#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>

#include <IceGridLib/DiscoveryPluginI.h>
#include <IceGrid/Discovery.h>

using namespace std;
using namespace IceGrid;

namespace
{

class LocatorI; // Forward declaration

class Request : public IceUtil::Shared
{
public:

    Request(LocatorI* locator, 
            const string& operation,
            ::Ice::OperationMode mode, 
            const pair<const Ice::Byte*, const Ice::Byte*>& inParams, 
            const ::Ice::Context& ctx,
            const Ice::AMD_Object_ice_invokePtr& amdCB) :
        _locator(locator), 
        _operation(operation),
        _mode(mode),
        _context(ctx),
        _inParams(inParams.first, inParams.second),
        _amdCB(amdCB)
    {
    }

    void invoke(const Ice::LocatorPrx&);
    void response(const bool, const pair<const Ice::Byte*, const Ice::Byte*>&);
    void exception(const Ice::Exception&);

protected:

    LocatorI* _locator;
    const string _operation;
    const Ice::OperationMode _mode;
    const Ice::Context _context;
    const Ice::ByteSeq _inParams;
    const Ice::AMD_Object_ice_invokePtr _amdCB;

    Ice::LocatorPrx _locatorPrx;
};
typedef IceUtil::Handle<Request> RequestPtr;

class LocatorI : public Ice::BlobjectArrayAsync, private IceUtil::TimerTask, private IceUtil::Monitor<IceUtil::Mutex>
{
public:

    LocatorI(const LookupPrx&, const Ice::PropertiesPtr&, const string&, const LocatorPrx&);
    void setLookupReply(const LookupReplyPrx&);

    virtual void ice_invoke_async(const Ice::AMD_Object_ice_invokePtr&, const pair<const Ice::Byte*, const Ice::Byte*>&,
                                  const Ice::Current&);

    void foundLocator(const LocatorPrx&);
    void invoke(const Ice::LocatorPrx&, const RequestPtr&);

private:

    virtual void runTimerTask();

    const LookupPrx _lookup;
    const IceUtil::Time _timeout;
    const int _retryCount;
    const IceUtil::Time _retryDelay;
    const IceUtil::TimerPtr _timer;

    string _instanceName;
    bool _warned;
    LookupReplyPrx _lookupReply;
    Ice::LocatorPrx _locator;
    IceGrid::LocatorPrx _voidLocator;

    IceUtil::Time _nextRetry;
    int _pendingRetryCount;
    vector<RequestPtr> _pendingRequests;
};
typedef IceUtil::Handle<LocatorI> LocatorIPtr;

class LookupReplyI : public LookupReply
{
public:

    LookupReplyI(const LocatorIPtr& locator) : _locator(locator)
    {
    }

    virtual void foundLocator(const LocatorPrx&, const Ice::Current&);

private:

    const LocatorIPtr _locator;
};

//
// The void locator implementation below is used when no locator is found.
//
class VoidLocatorI : public IceGrid::Locator
{
public:
    
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

    virtual Ice::LocatorRegistryPrx 
    getRegistry(const Ice::Current&) const
    {
        return 0;
    }

    virtual IceGrid::RegistryPrx 
    getLocalRegistry(const Ice::Current&) const
    {
        return 0;
    }

    virtual IceGrid::QueryPrx 
    getLocalQuery(const Ice::Current&) const
    {
        return 0;
    }
};

}

//
// Plugin factory function.
//
extern "C"
{

ICE_DECLSPEC_EXPORT Ice::Plugin*
createIceGridDiscovery(const Ice::CommunicatorPtr& communicator, const string&, const Ice::StringSeq&)
{
    return new DiscoveryPluginI(communicator);
}

}

DiscoveryPluginI::DiscoveryPluginI(const Ice::CommunicatorPtr& communicator) : _communicator(communicator)
{
}

void
DiscoveryPluginI::initialize()
{
    Ice::PropertiesPtr properties = _communicator->getProperties();

    bool ipv4 = properties->getPropertyAsIntWithDefault("Ice.IPv4", 1) > 0;
    string address;
    if(ipv4)
    {
        address = properties->getPropertyWithDefault("IceGridDiscovery.Address", "239.255.0.1");
    }
    else
    {
        address = properties->getPropertyWithDefault("IceGridDiscovery.Address", "ff15::1");
    }
    int port = properties->getPropertyAsIntWithDefault("IceGridDiscovery.Port", 4061);
    string intf = properties->getProperty("IceGridDiscovery.Interface");

    if(properties->getProperty("IceGridDiscovery.Reply.Endpoints").empty())
    {
        ostringstream os;
        os << "udp";
        if(!intf.empty())
        {
            os << " -h \"" << intf << "\"";
        }
        properties->setProperty("IceGridDiscovery.Reply.Endpoints", os.str());
    }
    if(properties->getProperty("IceGridDiscovery.Locator.Endpoints").empty())
    {
        properties->setProperty("IceGridDiscovery.Locator.AdapterId", IceUtil::generateUUID()); // Collocated adapter
    }

    _replyAdapter = _communicator->createObjectAdapter("IceGridDiscovery.Reply");
    _locatorAdapter = _communicator->createObjectAdapter("IceGridDiscovery.Locator");

    // We don't want those adapters to be registered with the locator so clear their locator.
    _replyAdapter->setLocator(0);
    _locatorAdapter->setLocator(0);

    string lookupEndpoints = properties->getProperty("IceGridDiscovery.Lookup");
    if(lookupEndpoints.empty())
    {
        ostringstream os;
        os << "udp -h \"" << address << "\" -p " << port;
        if(!intf.empty())
        {
            os << " --interface \"" << intf << "\"";
        }
        lookupEndpoints = os.str();
    }

    Ice::ObjectPrx lookupPrx = _communicator->stringToProxy("IceGridDiscovery/Lookup -d:" + lookupEndpoints);
    lookupPrx = lookupPrx->ice_collocationOptimized(false); // No collocation optimization for the multicast proxy!
    try
    {
        lookupPrx->ice_getConnection(); // Ensure we can establish a connection to the multicast proxy
    }
    catch(const Ice::LocalException& ex)
    {
        ostringstream os;
        os << "unable to establish multicast connection, IceGrid discovery will be disabled:\n";
        os << "proxy = " << lookupPrx << '\n';
        os << ex;
        throw Ice::PluginInitializationException(__FILE__, __LINE__, os.str());
    }

    LocatorPrx voidLocator = LocatorPrx::uncheckedCast(_locatorAdapter->addWithUUID(new VoidLocatorI()));

    string instanceName = properties->getProperty("IceGridDiscovery.InstanceName");
    Ice::Identity id;
    id.name = "Locator";
    id.category = !instanceName.empty() ? instanceName : IceUtil::generateUUID();
    LocatorIPtr locator = new LocatorI(LookupPrx::uncheckedCast(lookupPrx), properties, instanceName, voidLocator);
    _communicator->setDefaultLocator(Ice::LocatorPrx::uncheckedCast(_locatorAdapter->add(locator, id)));

    Ice::ObjectPrx lookupReply = _replyAdapter->addWithUUID(new LookupReplyI(locator))->ice_datagram();
    locator->setLookupReply(LookupReplyPrx::uncheckedCast(lookupReply));

    _replyAdapter->activate();
    _locatorAdapter->activate();
}

void
DiscoveryPluginI::destroy()
{
    _replyAdapter->destroy();
    _locatorAdapter->destroy();
}

void
Request::invoke(const Ice::LocatorPrx& l)
{
    _locatorPrx = l;
    l->begin_ice_invoke(_operation, _mode, _inParams, _context,
                        Ice::newCallback_Object_ice_invoke(this, &Request::response, &Request::exception));
}

void
Request::response(bool ok, const pair<const Ice::Byte*, const Ice::Byte*>& outParams)
{
    _amdCB->ice_response(ok, outParams);
}

void
Request::exception(const Ice::Exception& ex)
{
    _locator->invoke(_locatorPrx, this); // Retry with new locator proxy
}

LocatorI::LocatorI(const LookupPrx& lookup, 
                   const Ice::PropertiesPtr& props, 
                   const string& instanceName,
                   const IceGrid::LocatorPrx& voidLocator) :
    _lookup(lookup),
    _timeout(IceUtil::Time::milliSeconds(props->getPropertyAsIntWithDefault("IceGridDiscovery.Timeout", 300))),
    _retryCount(props->getPropertyAsIntWithDefault("IceGridDiscovery.RetryCount", 3)),
    _retryDelay(IceUtil::Time::milliSeconds(props->getPropertyAsIntWithDefault("IceGridDiscovery.RetryDelay", 2000))),
    _timer(IceInternal::getInstanceTimer(lookup->ice_getCommunicator())),
    _instanceName(instanceName),
    _warned(false),
    _locator(lookup->ice_getCommunicator()->getDefaultLocator()),
    _voidLocator(voidLocator),
    _pendingRetryCount(0)
{
}

void
LocatorI::setLookupReply(const LookupReplyPrx& lookupReply)
{
    _lookupReply = lookupReply;
}

void
LocatorI::ice_invoke_async(const Ice::AMD_Object_ice_invokePtr& amdCB, 
                           const pair<const Ice::Byte*, const Ice::Byte*>& inParams,
                           const Ice::Current& current)
{
    invoke(0, new Request(this, current.operation, current.mode, inParams, current.ctx, amdCB));
}

void
LocatorI::foundLocator(const LocatorPrx& locator)
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
    if(_locator && locator->ice_getIdentity().category != _locator->ice_getIdentity().category)
    {
        if(!_warned)
        {
            _warned = true; // Only warn once.

            Ice::Warning out(locator->ice_getCommunicator()->getLogger());
            out << "received IceGrid locator with different instance name:\n";
            out << "using = `" << _locator->ice_getIdentity().category << "'\n";
            out << "received = `" << locator->ice_getIdentity().category << "'\n";
            out << "This is typically the case if multiple IceGrid registries with different ";
            out << "instance names are deployed and the property `IceGridDiscovery.InstanceName' ";
            out << "is not set.";
        }
        return;
    }

    if(_pendingRetryCount > 0) // No need to retry, we found a locator.
    {
        _timer->cancel(this);
        _pendingRetryCount = 0;
    }

    if(_locator)
    {
        //
        // We found another locator replica, append its endpoints to the
        // current locator proxy endpoints.
        //
        Ice::EndpointSeq newEndpoints = _locator->ice_getEndpoints();
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
        _locator = _locator->ice_endpoints(newEndpoints);
    }
    else
    {
        _locator = locator;
        if(_instanceName.empty())
        {
            _instanceName = _locator->ice_getIdentity().category; // Stick to the first discovered locator.
        }
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

void
LocatorI::invoke(const Ice::LocatorPrx& locator, const RequestPtr& request)
{
    Lock sync(*this);
    if(_locator && _locator != locator)
    {
        request->invoke(_locator);
    }
    else if(IceUtil::Time::now() < _nextRetry)
    {
        request->invoke(_voidLocator); // Don't retry to find a locator before the retry delay expires
    }
    else
    {
        _locator = 0;

        _pendingRequests.push_back(request);

        if(_pendingRetryCount == 0) // No request in progress
        {
            _pendingRetryCount = _retryCount;
            _lookup->begin_findLocator(_instanceName, _lookupReply); // Send multicast request.
            _timer->schedule(this, _timeout);
        }
    }
}

void
LocatorI::runTimerTask()
{
    Lock sync(*this);
    if(--_pendingRetryCount > 0)
    {
        _lookup->begin_findLocator(_instanceName, _lookupReply); // Send multicast request.
        _timer->schedule(this, _timeout);
    }
    else
    {
        assert(!_pendingRequests.empty());
        for(vector<RequestPtr>::const_iterator p = _pendingRequests.begin(); p != _pendingRequests.end(); ++p)
        {
            (*p)->invoke(_voidLocator); // Send pending requests on void locator.
        }
        _pendingRequests.clear();
        _nextRetry = IceUtil::Time::now() + _retryDelay; // Only retry when the retry delay expires
    }
}

void
LookupReplyI::foundLocator(const LocatorPrx& locator, const Ice::Current&)
{
    _locator->foundLocator(locator);
}
