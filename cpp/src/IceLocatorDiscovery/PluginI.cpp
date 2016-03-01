// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>

#include <IceLocatorDiscovery/PluginI.h>
#include <IceLocatorDiscovery/IceLocatorDiscovery.h>

using namespace std;
using namespace IceLocatorDiscovery;

#ifndef ICE_LOCATOR_DISCOVERY_API
#   ifdef ICE_LOCATOR_DISCOVERY_API_EXPORTS
#       define ICE_LOCATOR_DISCOVERY_API ICE_DECLSPEC_EXPORT
#   else
#       define ICE_LOCATOR_DISCOVERY_API /**/
#   endif
#endif

//
// Plugin factory function.
//
extern "C"
{

ICE_LOCATOR_DISCOVERY_API Ice::Plugin*
createIceLocatorDiscovery(const Ice::CommunicatorPtr& communicator, const string&, const Ice::StringSeq&)
{
    return new PluginI(communicator);
}

}

namespace Ice
{

ICE_LOCATOR_DISCOVERY_API void
registerIceLocatorDiscovery(bool loadOnInitialize)
{
    Ice::registerPluginFactory("IceLocatorDiscovery", createIceLocatorDiscovery, loadOnInitialize);
}

}

namespace
{

class LocatorI; // Forward declaration

class Request : public IceUtil::Shared
{
public:

    Request(LocatorI* locator,
            const string& operation,
            Ice::OperationMode mode,
            const pair<const Ice::Byte*, const Ice::Byte*>& inParams,
            const Ice::Context& ctx,
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
    IceUtil::UniquePtr<Ice::Exception> _exception;
};
typedef IceUtil::Handle<Request> RequestPtr;

class LocatorI : public Ice::BlobjectArrayAsync, private IceUtil::TimerTask, private IceUtil::Monitor<IceUtil::Mutex>
{
public:

    LocatorI(const LookupPrx&, const Ice::PropertiesPtr&, const string&, const Ice::LocatorPrx&);
    void setLookupReply(const LookupReplyPrx&);

    virtual void ice_invoke_async(const Ice::AMD_Object_ice_invokePtr&, const pair<const Ice::Byte*, const Ice::Byte*>&,
                                  const Ice::Current&);

    void foundLocator(const Ice::LocatorPrx&);
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
    Ice::LocatorPrx _voidLocator;

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

    virtual void foundLocator(const Ice::LocatorPrx&, const Ice::Current&);

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
};

}

PluginI::PluginI(const Ice::CommunicatorPtr& communicator) : _communicator(communicator)
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
        address = properties->getPropertyWithDefault("IceLocatorDiscovery.Address", "239.255.0.1");
    }
    else
    {
        address = properties->getPropertyWithDefault("IceLocatorDiscovery.Address", "ff15::1");
    }
    int port = properties->getPropertyAsIntWithDefault("IceLocatorDiscovery.Port", 4061);
    string intf = properties->getProperty("IceLocatorDiscovery.Interface");

    if(properties->getProperty("IceLocatorDiscovery.Reply.Endpoints").empty())
    {
        ostringstream os;
        os << "udp";
        if(!intf.empty())
        {
            os << " -h \"" << intf << "\"";
        }
        properties->setProperty("IceLocatorDiscovery.Reply.Endpoints", os.str());
    }
    if(properties->getProperty("IceLocatorDiscovery.Locator.Endpoints").empty())
    {
        properties->setProperty("IceLocatorDiscovery.Locator.AdapterId", IceUtil::generateUUID()); // Collocated adapter
    }

    _replyAdapter = _communicator->createObjectAdapter("IceLocatorDiscovery.Reply");
    _locatorAdapter = _communicator->createObjectAdapter("IceLocatorDiscovery.Locator");

    // We don't want those adapters to be registered with the locator so clear their locator.
    _replyAdapter->setLocator(0);
    _locatorAdapter->setLocator(0);

    string lookupEndpoints = properties->getProperty("IceLocatorDiscovery.Lookup");
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

    Ice::ObjectPrx lookupPrx = _communicator->stringToProxy("IceLocatorDiscovery/Lookup -d:" + lookupEndpoints);
    lookupPrx = lookupPrx->ice_collocationOptimized(false); // No collocation optimization for the multicast proxy!
    try
    {
        // Ensure we can establish a connection to the multicast proxy
        // but don't block.
        Ice::AsyncResultPtr result = lookupPrx->begin_ice_getConnection();
        if(result->sentSynchronously())
        {
            lookupPrx->end_ice_getConnection(result);
        }
    }
    catch(const Ice::LocalException& ex)
    {
        ostringstream os;
        os << "IceLocatorDiscovery is unable to establish a multicast connection:\n";
        os << "proxy = " << lookupPrx << '\n';
        os << ex;
        throw Ice::PluginInitializationException(__FILE__, __LINE__, os.str());
    }

    Ice::LocatorPrx voidLocator = Ice::LocatorPrx::uncheckedCast(_locatorAdapter->addWithUUID(new VoidLocatorI()));

    string instanceName = properties->getProperty("IceLocatorDiscovery.InstanceName");
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
PluginI::destroy()
{
    _replyAdapter->destroy();
    _locatorAdapter->destroy();
}

void
Request::invoke(const Ice::LocatorPrx& l)
{
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
}

void
Request::response(bool ok, const pair<const Ice::Byte*, const Ice::Byte*>& outParams)
{
    _amdCB->ice_response(ok, outParams);
}

void
Request::exception(const Ice::Exception& ex)
{
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
}

LocatorI::LocatorI(const LookupPrx& lookup,
                   const Ice::PropertiesPtr& p,
                   const string& instanceName,
                   const Ice::LocatorPrx& voidLocator) :
    _lookup(lookup),
    _timeout(IceUtil::Time::milliSeconds(p->getPropertyAsIntWithDefault("IceLocatorDiscovery.Timeout", 300))),
    _retryCount(p->getPropertyAsIntWithDefault("IceLocatorDiscovery.RetryCount", 3)),
    _retryDelay(IceUtil::Time::milliSeconds(p->getPropertyAsIntWithDefault("IceLocatorDiscovery.RetryDelay", 2000))),
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
LocatorI::foundLocator(const Ice::LocatorPrx& locator)
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
            try
            {
                _lookup->begin_findLocator(_instanceName, _lookupReply); // Send multicast request.
                _timer->schedule(this, _timeout);
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
LocatorI::runTimerTask()
{
    Lock sync(*this);
    if(--_pendingRetryCount > 0)
    {
        try
        {
            _lookup->begin_findLocator(_instanceName, _lookupReply); // Send multicast request.
            _timer->schedule(this, _timeout);
            return;
        }
        catch(const Ice::LocalException&)
        {
        }
        _pendingRetryCount = 0;
    }

    for(vector<RequestPtr>::const_iterator p = _pendingRequests.begin(); p != _pendingRequests.end(); ++p)
    {
        (*p)->invoke(_voidLocator); // Send pending requests on void locator.
    }
    _pendingRequests.clear();
    _nextRetry = IceUtil::Time::now() + _retryDelay; // Only retry when the retry delay expires
}

void
LookupReplyI::foundLocator(const Ice::LocatorPrx& locator, const Ice::Current&)
{
    _locator->foundLocator(locator);
}
