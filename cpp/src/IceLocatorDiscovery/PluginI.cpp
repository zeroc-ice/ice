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

#include <IceLocatorDiscovery/PluginI.h>
#include <IceLocatorDiscovery/IceLocatorDiscovery.h>

using namespace std;
using namespace IceLocatorDiscovery;

#ifndef ICE_LOCATOR_DISCOVERY_API
#   ifdef ICE_LOCATOR_DISCOVERY_API_EXPORTS
#   define ICE_LOCATOR_DISCOVERY_API ICE_DECLSPEC_EXPORT
#   else
#   define ICE_LOCATOR_DISCOVERY_API /**/
#   endif
#endif

//
// Plugin factory function.
//
extern "C" ICE_LOCATOR_DISCOVERY_API Ice::Plugin*
createIceLocatorDiscovery(const Ice::CommunicatorPtr& communicator, const string&, const Ice::StringSeq&)
{
    return new PluginI(communicator);
}

namespace Ice
{

ICE_LOCATOR_DISCOVERY_API void
registerIceLocatorDiscovery(bool loadOnInitialize)
{
    Ice::registerPluginFactory("IceLocatorDiscovery", createIceLocatorDiscovery, loadOnInitialize);
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

namespace
{

class LocatorI; // Forward declaration

class Request :
#ifdef ICE_CPP11_MAPPING
        public std::enable_shared_from_this<Request>
#else
        public virtual IceUtil::Shared
#endif
{
public:

#ifdef ICE_CPP11_MAPPING
    Request(LocatorI* locator,
            const string& operation,
            Ice::OperationMode mode,
            const pair<const Ice::Byte*, const Ice::Byte*>& inParams,
            const Ice::Context& ctx,
            function<void(bool, const pair<const Ice::Byte*, const Ice::Byte*>&)> responseCB,
            function<void(exception_ptr)> exceptionCB) :
        _locator(locator),
        _operation(operation),
        _mode(mode),
        _context(ctx),
        _inParams(inParams.first, inParams.second),
        _responseCB(move(responseCB)),
        _exceptionCB(move(exceptionCB))
#else
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
#endif
    {
    }

    void invoke(const Ice::LocatorPrxPtr&);
#ifdef ICE_CPP11_MAPPING
    void response(const bool, pair<const Ice::Byte*, const Ice::Byte*>);
#else
    void response(const bool, const pair<const Ice::Byte*, const Ice::Byte*>&);
#endif
    void exception(const Ice::Exception&);

protected:

    LocatorI* _locator;
    const string _operation;
    const Ice::OperationMode _mode;
    const Ice::Context _context;
    const Ice::ByteSeq _inParams;
#ifdef ICE_CPP11_MAPPING
    function<void(bool, const pair<const Ice::Byte*, const Ice::Byte*>&)> _responseCB;
    function<void(exception_ptr)> _exceptionCB;
    exception_ptr _exception;
#else
    const Ice::AMD_Object_ice_invokePtr _amdCB;
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

    LocatorI(const LookupPrxPtr&, const Ice::PropertiesPtr&, const string&, const Ice::LocatorPrxPtr&);
    void setLookupReply(const LookupReplyPrxPtr&);

#ifdef ICE_CPP11_MAPPING
    virtual void ice_invokeAsync(pair<const Ice::Byte*, const Ice::Byte*>,
                                 function<void(bool, pair<const Ice::Byte*, const Ice::Byte*>)>,
                                 function<void(exception_ptr)>,
                                 const Ice::Current&);
#else
    virtual void ice_invoke_async(const Ice::AMD_Object_ice_invokePtr&, const pair<const Ice::Byte*, const Ice::Byte*>&,
                                  const Ice::Current&);
#endif

    void foundLocator(const Ice::LocatorPrxPtr&);
    void invoke(const Ice::LocatorPrxPtr&, const RequestPtr&);

private:

    virtual void runTimerTask();

    const LookupPrxPtr _lookup;
    const IceUtil::Time _timeout;
    const int _retryCount;
    const IceUtil::Time _retryDelay;
    const IceUtil::TimerPtr _timer;

    string _instanceName;
    bool _warned;
    LookupReplyPrxPtr _lookupReply;
    Ice::LocatorPrxPtr _locator;
    Ice::LocatorPrxPtr _voidLocator;

    IceUtil::Time _nextRetry;
    int _pendingRetryCount;
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
        properties->setProperty("IceLocatorDiscovery.Locator.AdapterId", Ice::generateUUID()); // Collocated adapter
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

    Ice::ObjectPrxPtr lookupPrx = _communicator->stringToProxy("IceLocatorDiscovery/Lookup -d:" + lookupEndpoints);
    lookupPrx = lookupPrx->ice_collocationOptimized(false); // No collocation optimization for the multicast proxy!
    try
    {
        // Ensure we can establish a connection to the multicast proxy
        // but don't block.
#ifdef ICE_CPP11_MAPPING
        promise<bool> sent;
        promise<void> completed;

        lookupPrx->ice_getConnectionAsync(
            [&](shared_ptr<Ice::Connection>)
            {
                completed.set_value();
            },
            [&](exception_ptr ex)
            {
                completed.set_exception(ex);
            },
            [&](bool sentSynchronously)
            {
                sent.set_value(sentSynchronously);
            });
        if(sent.get_future().get())
        {
            completed.get_future().get();
        }
#else
        Ice::AsyncResultPtr result = lookupPrx->begin_ice_getConnection();
        if(result->sentSynchronously())
        {
            lookupPrx->end_ice_getConnection(result);
        }
#endif
    }
    catch(const Ice::LocalException& ex)
    {
        ostringstream os;
        os << "IceLocatorDiscovery is unable to establish a multicast connection:\n";
        os << "proxy = " << lookupPrx << '\n';
        os << ex;
        throw Ice::PluginInitializationException(__FILE__, __LINE__, os.str());
    }

    Ice::LocatorPrxPtr voidLocator = ICE_UNCHECKED_CAST(Ice::LocatorPrx, _locatorAdapter->addWithUUID(ICE_MAKE_SHARED(VoidLocatorI)));

    string instanceName = properties->getProperty("IceLocatorDiscovery.InstanceName");
    Ice::Identity id;
    id.name = "Locator";
    id.category = !instanceName.empty() ? instanceName : Ice::generateUUID();
    LocatorIPtr locator = ICE_MAKE_SHARED(LocatorI, ICE_UNCHECKED_CAST(LookupPrx, lookupPrx), properties, instanceName, voidLocator);
    _communicator->setDefaultLocator(ICE_UNCHECKED_CAST(Ice::LocatorPrx, _locatorAdapter->add(locator, id)));

    Ice::ObjectPrxPtr lookupReply = _replyAdapter->addWithUUID(ICE_MAKE_SHARED(LookupReplyI, locator))->ice_datagram();
    locator->setLookupReply(ICE_UNCHECKED_CAST(LookupReplyPrx, lookupReply));

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
                                   self->response(ok, move(outPair));
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
        _exceptionCB(_exception);
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

#ifdef ICE_CPP11_MAPPING
void
Request::response(bool ok, pair<const Ice::Byte*, const Ice::Byte*> outParams)
{
    _responseCB(ok, move(outParams));
}
#else
void
Request::response(bool ok, const pair<const Ice::Byte*, const Ice::Byte*>& outParams)
{
    _amdCB->ice_response(ok, outParams);
}
#endif

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
        _exceptionCB(current_exception());
    }
    catch(const Ice::UnknownException&)
    {
        _exceptionCB(current_exception());
    }
    catch(const Ice::NoEndpointException&)
    {
        try
        {
            throw Ice::ObjectNotExistException(__FILE__, __LINE__);
        }
        catch(...)
        {
            _exceptionCB(current_exception());
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
            _exceptionCB(current_exception());
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
            _exceptionCB(current_exception());
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

LocatorI::LocatorI(const LookupPrxPtr& lookup,
                   const Ice::PropertiesPtr& p,
                   const string& instanceName,
                   const Ice::LocatorPrxPtr& voidLocator) :
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
LocatorI::setLookupReply(const LookupReplyPrxPtr& lookupReply)
{
    _lookupReply = lookupReply;
}

#ifdef ICE_CPP11_MAPPING
void
LocatorI::ice_invokeAsync(pair<const Ice::Byte*, const Ice::Byte*> inParams,
                          function<void(bool, pair<const Ice::Byte*, const Ice::Byte*>)> responseCB,
                          function<void(exception_ptr)> exceptionCB,
                          const Ice::Current& current)
{
    invoke(nullptr, make_shared<Request>(this, current.operation, current.mode, inParams, current.ctx,
                                         move(responseCB), move(exceptionCB)));
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
        _timer->cancel(ICE_SHARED_FROM_THIS);
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
LocatorI::invoke(const Ice::LocatorPrxPtr& locator, const RequestPtr& request)
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
#ifdef ICE_CPP11_MAPPING
                _lookup->findLocatorAsync(_instanceName, _lookupReply); // Send multicast request.
#else
                _lookup->begin_findLocator(_instanceName, _lookupReply); // Send multicast request.
#endif
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
LocatorI::runTimerTask()
{
    Lock sync(*this);
    if(--_pendingRetryCount > 0)
    {
        try
        {
#ifdef ICE_CPP11_MAPPING
            _lookup->findLocatorAsync(_instanceName, _lookupReply); // Send multicast request.
#else
            _lookup->begin_findLocator(_instanceName, _lookupReply); // Send multicast request.
#endif
            _timer->schedule(ICE_SHARED_FROM_THIS, _timeout);
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
LookupReplyI::foundLocator(ICE_IN(Ice::LocatorPrxPtr) locator, const Ice::Current&)
{
    _locator->foundLocator(locator);
}
