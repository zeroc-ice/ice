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

    Request(LocatorI* locator) : _locator(locator)
    {
    }

    virtual void invoke(const Ice::LocatorPrx&) = 0;
    virtual void response(const Ice::ObjectPrx&) = 0;

protected:

    LocatorI* _locator;
    Ice::LocatorPrx _locatorPrx;
};
typedef IceUtil::Handle<Request> RequestPtr;

class LocatorI : public Ice::Locator, private IceUtil::TimerTask, private IceUtil::Monitor<IceUtil::Mutex>
{
public:

    LocatorI(const LookupPrx&, const Ice::PropertiesPtr&);
    void setLookupReply(const LookupReplyPrx&);

    virtual void findObjectById_async(const Ice::AMD_Locator_findObjectByIdPtr&, const Ice::Identity&,
                                      const Ice::Current&) const;

    virtual void findAdapterById_async(const Ice::AMD_Locator_findAdapterByIdPtr&, const string&,
                                       const Ice::Current&) const;

    virtual Ice::LocatorRegistryPrx getRegistry(const Ice::Current&) const;

    void foundLocator(const LocatorPrx&);
    void invoke(const Ice::LocatorPrx&, const RequestPtr&);

private:

    virtual void runTimerTask();
    void queueRequest(const RequestPtr&);

    const LookupPrx _lookup;
    const IceUtil::Time _timeout;
    const int _retryCount;
    const IceUtil::TimerPtr _timer;

    string _instanceName;
    bool _warned;
    LookupReplyPrx _lookupReply;
    Ice::LocatorPrx _locator;

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

class ObjectRequest : public Request
{
public:

    ObjectRequest(LocatorI* locator, const Ice::Identity& id, const Ice::AMD_Locator_findObjectByIdPtr& amdCB) :
        Request(locator), _id(id), _amdCB(amdCB)
    {
    }

    virtual void invoke(const Ice::LocatorPrx&);
    virtual void response(const Ice::ObjectPrx&);

    void
    exception(const Ice::Exception&)
    {
        _locator->invoke(_locatorPrx, this); // Retry with new locator proxy
    }

private:

    const Ice::Identity _id;
    Ice::AMD_Locator_findObjectByIdPtr _amdCB;
};

class AdapterRequest : public Request
{
public:

    AdapterRequest(LocatorI* locator, const string& adapterId, const Ice::AMD_Locator_findAdapterByIdPtr& amdCB) :
        Request(locator), _adapterId(adapterId), _amdCB(amdCB)
    {
    }

    virtual void invoke(const Ice::LocatorPrx&);
    virtual void response(const Ice::ObjectPrx&);

    void
    exception(const Ice::Exception&)
    {
        _locator->invoke(_locatorPrx, this); // Retry with new locator proxy.
    }

private:

    const string _adapterId;
    const Ice::AMD_Locator_findAdapterByIdPtr _amdCB;
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

    LocatorIPtr locator = new LocatorI(LookupPrx::uncheckedCast(lookupPrx), properties);
    _communicator->setDefaultLocator(Ice::LocatorPrx::uncheckedCast(_locatorAdapter->addWithUUID(locator)));

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
AdapterRequest::invoke(const Ice::LocatorPrx& l)
{
    _locatorPrx = l;
    l->begin_findAdapterById(_adapterId, Ice::newCallback_Locator_findAdapterById(this,
                                                                                  &AdapterRequest::response,
                                                                                  &AdapterRequest::exception));
}

void
AdapterRequest::response(const Ice::ObjectPrx& prx)
{
    _amdCB->ice_response(prx);
}

void
ObjectRequest::invoke(const Ice::LocatorPrx& l)
{
    _locatorPrx = l;
    l->begin_findObjectById(_id, Ice::newCallback_Locator_findObjectById(this,
                                                                         &ObjectRequest::response,
                                                                         &ObjectRequest::exception));
}

void
ObjectRequest::response(const Ice::ObjectPrx& prx)
{
    _amdCB->ice_response(prx);
}

LocatorI::LocatorI(const LookupPrx& lookup, const Ice::PropertiesPtr& properties) :
    _lookup(lookup),
    _timeout(IceUtil::Time::milliSeconds(properties->getPropertyAsIntWithDefault("IceGridDiscovery.Timeout", 300))),
    _retryCount(properties->getPropertyAsIntWithDefault("IceGridDiscovery.RetryCount", 3)),
    _timer(IceInternal::getInstanceTimer(lookup->ice_getCommunicator())),
    _instanceName(properties->getProperty("IceGridDiscovery.InstanceName")),
    _warned(false),
    _locator(lookup->ice_getCommunicator()->getDefaultLocator()),
    _pendingRetryCount(0)
{
}

void
LocatorI::setLookupReply(const LookupReplyPrx& lookupReply)
{
    _lookupReply = lookupReply;
}

void
LocatorI::findObjectById_async(const Ice::AMD_Locator_findObjectByIdPtr& amdCB,
                               const Ice::Identity& id,
                               const Ice::Current&) const
{
    const_cast<LocatorI*>(this)->invoke(0, new ObjectRequest(const_cast<LocatorI*>(this), id, amdCB));
}

void
LocatorI::findAdapterById_async(const Ice::AMD_Locator_findAdapterByIdPtr& amdCB,
                                const string& adapterId,
                                const Ice::Current&) const
{
    const_cast<LocatorI*>(this)->invoke(0, new AdapterRequest(const_cast<LocatorI*>(this), adapterId, amdCB));
}

Ice::LocatorRegistryPrx
LocatorI::getRegistry(const Ice::Current&) const
{
    Ice::LocatorPrx locator;
    {
        Lock sync(*this);
        if(!_locator)
        {
            const_cast<LocatorI*>(this)->queueRequest(0); // Search for locator if not already doing so.
            while(_pendingRetryCount > 0)
            {
                wait();
            }
        }
        locator = _locator;
    }
    return locator ? locator->getRegistry() : Ice::LocatorRegistryPrx();
}

void
LocatorI::foundLocator(const LocatorPrx& locator)
{
    Lock sync(*this);
    if(!locator)
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
            _instanceName = _locator->ice_getIdentity().category;
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
    notifyAll();
}

void
LocatorI::invoke(const Ice::LocatorPrx& locator, const RequestPtr& request)
{
    Lock sync(*this);
    if(_locator && _locator != locator)
    {
        request->invoke(_locator);
    }
    else
    {
        _locator = 0;
        queueRequest(request);
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
            (*p)->response(0);
        }
        _pendingRequests.clear();
        notifyAll();
    }
}

void
LocatorI::queueRequest(const RequestPtr& request)
{
    if(request)
    {
        _pendingRequests.push_back(request);
    }

    if(_pendingRetryCount == 0) // No request in progress
    {
        _pendingRetryCount = _retryCount;
        _lookup->begin_findLocator(_instanceName, _lookupReply); // Send multicast request.
        _timer->schedule(this, _timeout);
    }
}

void
LookupReplyI::foundLocator(const LocatorPrx& locator, const Ice::Current&)
{
    _locator->foundLocator(locator);
}
