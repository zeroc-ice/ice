//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Proxy.h>
#include <Ice/ProxyFactory.h>
#include <Ice/ReferenceFactory.h>
#include <Ice/Object.h>
#include <Ice/ObjectAdapterFactory.h>
#include <Ice/OutgoingAsync.h>
#include <Ice/Reference.h>
#include <Ice/CollocatedRequestHandler.h>
#include <Ice/EndpointI.h>
#include <Ice/Instance.h>
#include <Ice/RouterInfo.h>
#include <Ice/LocatorInfo.h>
#include <Ice/OutputStream.h>
#include <Ice/InputStream.h>
#include <Ice/LocalException.h>
#include <Ice/ConnectionI.h> // To convert from ConnectionIPtr to ConnectionPtr in ice_getConnection().
#include <Ice/ImplicitContextI.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace Ice
{

const Context noExplicitContext;

}

namespace
{

const string ice_ping_name = "ice_ping";
const string ice_ids_name = "ice_ids";
const string ice_id_name = "ice_id";
const string ice_isA_name = "ice_isA";
const string ice_invoke_name = "ice_invoke";
const string ice_getConnection_name = "ice_getConnection";
const string ice_flushBatchRequests_name = "ice_flushBatchRequests";

}

ProxyFlushBatchAsync::ProxyFlushBatchAsync(const ObjectPrxPtr& proxy) : ProxyOutgoingAsyncBase(proxy)
{
}

AsyncStatus
ProxyFlushBatchAsync::invokeRemote(const ConnectionIPtr& connection, bool compress, bool)
{
    if(_batchRequestNum == 0)
    {
        if(sent())
        {
            return static_cast<AsyncStatus>(AsyncStatusSent | AsyncStatusInvokeSentCallback);
        }
        else
        {
            return AsyncStatusSent;
        }
    }
    _cachedConnection = connection;
    return connection->sendAsyncRequest(ICE_SHARED_FROM_THIS, compress, false, _batchRequestNum);
}

AsyncStatus
ProxyFlushBatchAsync::invokeCollocated(CollocatedRequestHandler* handler)
{
    if(_batchRequestNum == 0)
    {
        if(sent())
        {
            return static_cast<AsyncStatus>(AsyncStatusSent | AsyncStatusInvokeSentCallback);
        }
        else
        {
            return AsyncStatusSent;
        }
    }
    return handler->invokeAsyncRequest(this, _batchRequestNum, false);
}

void
ProxyFlushBatchAsync::invoke(const string& operation)
{
    checkSupportedProtocol(getCompatibleProtocol(_proxy->_getReference()->getProtocol()));
    _observer.attach(_proxy, operation, ::Ice::noExplicitContext);
    bool compress; // Ignore for proxy flushBatchRequests
    _batchRequestNum = _proxy->_getBatchRequestQueue()->swap(&_os, compress);
    invokeImpl(true); // userThread = true
}

ProxyGetConnection::ProxyGetConnection(const ObjectPrxPtr& prx) : ProxyOutgoingAsyncBase(prx)
{
}

AsyncStatus
ProxyGetConnection::invokeRemote(const ConnectionIPtr& connection, bool, bool)
{
    _cachedConnection = connection;
    if(responseImpl(true, true))
    {
        invokeResponseAsync();
    }
    return AsyncStatusSent;
}

AsyncStatus
ProxyGetConnection::invokeCollocated(CollocatedRequestHandler*)
{
    if(responseImpl(true, true))
    {
        invokeResponseAsync();
    }
    return AsyncStatusSent;
}

Ice::ConnectionPtr
ProxyGetConnection::getConnection() const
{
    return _cachedConnection;
}

void
ProxyGetConnection::invoke(const string& operation)
{
    _observer.attach(_proxy, operation, ::Ice::noExplicitContext);
    invokeImpl(true); // userThread = true
}

namespace Ice
{

bool
operator<(const ObjectPrx& lhs, const ObjectPrx& rhs)
{
    return lhs._reference < rhs._reference;
}

bool
operator==(const ObjectPrx& lhs, const ObjectPrx& rhs)
{
    return lhs._reference == rhs._reference;
}

}

void
Ice::ObjectPrx::_iceI_isA(const shared_ptr<IceInternal::OutgoingAsyncT<bool>>& outAsync,
                          const string& typeId,
                          const Context& ctx)
{
    _checkTwowayOnly(ice_isA_name);
    outAsync->invoke(ice_isA_name, OperationMode::Nonmutating, ICE_ENUM(FormatType, DefaultFormat), ctx,
                     [&](Ice::OutputStream* os)
                     {
                         os->write(typeId, false);
                     },
                     nullptr);
}

void
Ice::ObjectPrx::_iceI_ping(const shared_ptr<IceInternal::OutgoingAsyncT<void>>& outAsync, const Context& ctx)
{
    outAsync->invoke(ice_ping_name, OperationMode::Nonmutating, ICE_ENUM(FormatType, DefaultFormat), ctx, nullptr, nullptr);
}

void
Ice::ObjectPrx::_iceI_ids(const shared_ptr<IceInternal::OutgoingAsyncT<vector<string>>>& outAsync, const Context& ctx)
{
    _checkTwowayOnly(ice_ids_name);
    outAsync->invoke(ice_ids_name, OperationMode::Nonmutating, ICE_ENUM(FormatType, DefaultFormat), ctx, nullptr, nullptr,
                     [](Ice::InputStream* stream)
                     {
                         vector<string> v;
                         stream->read(v, false); // no conversion
                         return v;
                     });
}

void
Ice::ObjectPrx::_iceI_id(const shared_ptr<IceInternal::OutgoingAsyncT<string>>& outAsync, const Context& ctx)
{
    _checkTwowayOnly(ice_id_name);
    outAsync->invoke(ice_id_name, OperationMode::Nonmutating, ICE_ENUM(FormatType, DefaultFormat), ctx, nullptr, nullptr,
                     [](Ice::InputStream* stream)
                     {
                         string v;
                         stream->read(v, false); // no conversion
                         return v;
                     });
}

void
Ice::ObjectPrx::_iceI_getConnection(const shared_ptr<IceInternal::ProxyGetConnection>& outAsync)
{
    outAsync->invoke(ice_getConnection_name);
}

void
Ice::ObjectPrx::_iceI_flushBatchRequests(const shared_ptr<IceInternal::ProxyFlushBatchAsync>& outAsync)
{
    outAsync->invoke(ice_flushBatchRequests_name);
}

void
Ice::ObjectPrx::_checkTwowayOnly(const string& name) const
{
    //
    // No mutex lock necessary, there is nothing mutable in this operation.
    //
    if(!ice_isTwoway())
    {
        throw Ice::TwowayOnlyException(__FILE__, __LINE__, name);
    }
}

shared_ptr<ObjectPrx>
Ice::ObjectPrx::_newInstance() const
{
    return createProxy<ObjectPrx>();
}

ostream&
Ice::operator<<(ostream& os, const Ice::ObjectPrx& p)
{
    return os << p.ice_toString();
}

#define ICE_OBJECT_PRX Ice::ObjectPrx
#define CONST_POINTER_CAST_OBJECT_PRX const_pointer_cast<ObjectPrx>(shared_from_this())

Identity
ICE_OBJECT_PRX::ice_getIdentity() const
{
    return _reference->getIdentity();
}

ObjectPrxPtr
ICE_OBJECT_PRX::ice_identity(const Identity& newIdentity) const
{
    if(newIdentity.name.empty())
    {
        throw IllegalIdentityException(__FILE__, __LINE__);
    }
    if(newIdentity == _reference->getIdentity())
    {
        return CONST_POINTER_CAST_OBJECT_PRX;
    }
    else
    {
        auto proxy = createProxy<ObjectPrx>();
        proxy->setup(_reference->changeIdentity(newIdentity));
        return proxy;
    }
}

Context
ICE_OBJECT_PRX::ice_getContext() const
{
    return _reference->getContext()->getValue();
}

ObjectPrxPtr
ICE_OBJECT_PRX::ice_context(const Context& newContext) const
{
    ObjectPrxPtr proxy = _newInstance();
    proxy->setup(_reference->changeContext(newContext));
    return proxy;
}

const string&
ICE_OBJECT_PRX::ice_getFacet() const
{
    return _reference->getFacet();
}

ObjectPrxPtr
ICE_OBJECT_PRX::ice_facet(const string& newFacet) const
{
    if(newFacet == _reference->getFacet())
    {
        return CONST_POINTER_CAST_OBJECT_PRX;
    }
    else
    {
        auto proxy = createProxy<ObjectPrx>();
        proxy->setup(_reference->changeFacet(newFacet));
        return proxy;
    }
}

string
ICE_OBJECT_PRX::ice_getAdapterId() const
{
    return _reference->getAdapterId();
}

ObjectPrxPtr
ICE_OBJECT_PRX::ice_adapterId(const string& newAdapterId) const
{
    if(newAdapterId == _reference->getAdapterId())
    {
        return CONST_POINTER_CAST_OBJECT_PRX;
    }
    else
    {
        ObjectPrxPtr proxy = _newInstance();
        proxy->setup(_reference->changeAdapterId(newAdapterId));
        return proxy;
    }
}

EndpointSeq
ICE_OBJECT_PRX::ice_getEndpoints() const
{
    vector<EndpointIPtr> endpoints = _reference->getEndpoints();
    EndpointSeq retSeq;
    for(vector<EndpointIPtr>::const_iterator p = endpoints.begin(); p != endpoints.end(); ++p)
    {
        retSeq.push_back(ICE_DYNAMIC_CAST(Endpoint, *p));
    }
    return retSeq;
}

ObjectPrxPtr
ICE_OBJECT_PRX::ice_endpoints(const EndpointSeq& newEndpoints) const
{
    vector<EndpointIPtr> endpoints;
    for(EndpointSeq::const_iterator p = newEndpoints.begin(); p != newEndpoints.end(); ++p)
    {
        endpoints.push_back(ICE_DYNAMIC_CAST(EndpointI, *p));
    }

    if(endpoints == _reference->getEndpoints())
    {
        return CONST_POINTER_CAST_OBJECT_PRX;
    }
    else
    {
        ObjectPrxPtr proxy = _newInstance();
        proxy->setup(_reference->changeEndpoints(endpoints));
        return proxy;
    }
}

Int
ICE_OBJECT_PRX::ice_getLocatorCacheTimeout() const
{
    return _reference->getLocatorCacheTimeout();
}

ObjectPrxPtr
ICE_OBJECT_PRX::ice_locatorCacheTimeout(Int newTimeout) const
{
    if(newTimeout < -1)
    {
        ostringstream s;
        s << "invalid value passed to ice_locatorCacheTimeout: " << newTimeout;
        throw invalid_argument(s.str());
    }
    if(newTimeout == _reference->getLocatorCacheTimeout())
    {
        return CONST_POINTER_CAST_OBJECT_PRX;
    }
    else
    {
        ObjectPrxPtr proxy = _newInstance();
        proxy->setup(_reference->changeLocatorCacheTimeout(newTimeout));
        return proxy;
    }
}

bool
ICE_OBJECT_PRX::ice_isConnectionCached() const
{
    return _reference->getCacheConnection();
}

ObjectPrxPtr
ICE_OBJECT_PRX::ice_connectionCached(bool newCache) const
{
    if(newCache == _reference->getCacheConnection())
    {
        return CONST_POINTER_CAST_OBJECT_PRX;
    }
    else
    {
        ObjectPrxPtr proxy = _newInstance();
        proxy->setup(_reference->changeCacheConnection(newCache));
        return proxy;
    }
}

EndpointSelectionType
ICE_OBJECT_PRX::ice_getEndpointSelection() const
{
    return _reference->getEndpointSelection();
}

ObjectPrxPtr
ICE_OBJECT_PRX::ice_endpointSelection(EndpointSelectionType newType) const
{
    if(newType == _reference->getEndpointSelection())
    {
        return CONST_POINTER_CAST_OBJECT_PRX;
    }
    else
    {
        ObjectPrxPtr proxy = _newInstance();
        proxy->setup(_reference->changeEndpointSelection(newType));
        return proxy;
    }
}

bool
ICE_OBJECT_PRX::ice_isSecure() const
{
    return _reference->getSecure();
}

ObjectPrxPtr
ICE_OBJECT_PRX::ice_secure(bool b) const
{
    if(b == _reference->getSecure())
    {
        return CONST_POINTER_CAST_OBJECT_PRX;
    }
    else
    {
        ObjectPrxPtr proxy = _newInstance();
        proxy->setup(_reference->changeSecure(b));
        return proxy;
    }
}

::Ice::EncodingVersion
ICE_OBJECT_PRX::ice_getEncodingVersion() const
{
    return _reference->getEncoding();
}

ObjectPrxPtr
ICE_OBJECT_PRX::ice_encodingVersion(const ::Ice::EncodingVersion& encoding) const
{
    if(encoding == _reference->getEncoding())
    {
        return CONST_POINTER_CAST_OBJECT_PRX;
    }
    else
    {
        ObjectPrxPtr proxy = _newInstance();
        proxy->setup(_reference->changeEncoding(encoding));
        return proxy;
    }
}

bool
ICE_OBJECT_PRX::ice_isPreferSecure() const
{
    return _reference->getPreferSecure();
}

ObjectPrxPtr
ICE_OBJECT_PRX::ice_preferSecure(bool b) const
{
    if(b == _reference->getPreferSecure())
    {
        return CONST_POINTER_CAST_OBJECT_PRX;
    }
    else
    {
        ObjectPrxPtr proxy = _newInstance();
        proxy->setup(_reference->changePreferSecure(b));
        return proxy;
    }
}

RouterPrxPtr
ICE_OBJECT_PRX::ice_getRouter() const
{
    RouterInfoPtr ri = _reference->getRouterInfo();
    return ri ? ri->getRouter() : nullptr;
}

ObjectPrxPtr
ICE_OBJECT_PRX::ice_router(const RouterPrxPtr& router) const
{
    ReferencePtr ref = _reference->changeRouter(router);
    if(ref == _reference)
    {
        return CONST_POINTER_CAST_OBJECT_PRX;
    }
    else
    {
        ObjectPrxPtr proxy = _newInstance();
        proxy->setup(ref);
        return proxy;
    }
}

LocatorPrxPtr
ICE_OBJECT_PRX::ice_getLocator() const
{
    LocatorInfoPtr ri = _reference->getLocatorInfo();
    return ri ? ri->getLocator() : nullptr;
}

ObjectPrxPtr
ICE_OBJECT_PRX::ice_locator(const LocatorPrxPtr& locator) const
{
    ReferencePtr ref = _reference->changeLocator(locator);
    if(ref == _reference)
    {
        return CONST_POINTER_CAST_OBJECT_PRX;
    }
    else
    {
        ObjectPrxPtr proxy = _newInstance();
        proxy->setup(ref);
        return proxy;
    }
}

bool
ICE_OBJECT_PRX::ice_isCollocationOptimized() const
{
    return _reference->getCollocationOptimized();
}

ObjectPrxPtr
ICE_OBJECT_PRX::ice_collocationOptimized(bool b) const
{
    if(b == _reference->getCollocationOptimized())
    {
        return CONST_POINTER_CAST_OBJECT_PRX;
    }
    else
    {
        ObjectPrxPtr proxy = _newInstance();
        proxy->setup(_reference->changeCollocationOptimized(b));
        return proxy;
    }
}

Int
ICE_OBJECT_PRX::ice_getInvocationTimeout() const
{
    return _reference->getInvocationTimeout();
}

ObjectPrxPtr
ICE_OBJECT_PRX::ice_invocationTimeout(Int newTimeout) const
{
    if(newTimeout < 1 && newTimeout != -1 && newTimeout != -2)
    {
        ostringstream s;
        s << "invalid value passed to ice_invocationTimeout: " << newTimeout;
        throw invalid_argument(s.str());
    }
    if(newTimeout == _reference->getInvocationTimeout())
    {
        return CONST_POINTER_CAST_OBJECT_PRX;
    }
    else
    {
        ObjectPrxPtr proxy = _newInstance();
        proxy->setup(_reference->changeInvocationTimeout(newTimeout));
        return proxy;
    }
}

ObjectPrxPtr
ICE_OBJECT_PRX::ice_twoway() const
{
    if(_reference->getMode() == Reference::ModeTwoway)
    {
        return CONST_POINTER_CAST_OBJECT_PRX;
    }
    else
    {
        ObjectPrxPtr proxy = _newInstance();
        proxy->setup(_reference->changeMode(Reference::ModeTwoway));
        return proxy;
    }
}

bool
ICE_OBJECT_PRX::ice_isTwoway() const
{
    return _reference->getMode() == Reference::ModeTwoway;
}

ObjectPrxPtr
ICE_OBJECT_PRX::ice_oneway() const
{
    if(_reference->getMode() == Reference::ModeOneway)
    {
        return CONST_POINTER_CAST_OBJECT_PRX;
    }
    else
    {
        ObjectPrxPtr proxy = _newInstance();
        proxy->setup(_reference->changeMode(Reference::ModeOneway));
        return proxy;
    }
}

bool
ICE_OBJECT_PRX::ice_isOneway() const
{
    return _reference->getMode() == Reference::ModeOneway;
}

ObjectPrxPtr
ICE_OBJECT_PRX::ice_batchOneway() const
{
    if(_reference->getMode() == Reference::ModeBatchOneway)
    {
        return CONST_POINTER_CAST_OBJECT_PRX;
    }
    else
    {
        ObjectPrxPtr proxy = _newInstance();
        proxy->setup(_reference->changeMode(Reference::ModeBatchOneway));
        return proxy;
    }
}

bool
ICE_OBJECT_PRX::ice_isBatchOneway() const
{
    return _reference->getMode() == Reference::ModeBatchOneway;
}

ObjectPrxPtr
ICE_OBJECT_PRX::ice_datagram() const
{
    if(_reference->getMode() == Reference::ModeDatagram)
    {
        return CONST_POINTER_CAST_OBJECT_PRX;
    }
    else
    {
        ObjectPrxPtr proxy = _newInstance();
        proxy->setup(_reference->changeMode(Reference::ModeDatagram));
        return proxy;
    }
}

bool
ICE_OBJECT_PRX::ice_isDatagram() const
{
    return _reference->getMode() == Reference::ModeDatagram;
}

ObjectPrxPtr
ICE_OBJECT_PRX::ice_batchDatagram() const
{
    if(_reference->getMode() == Reference::ModeBatchDatagram)
    {
        return CONST_POINTER_CAST_OBJECT_PRX;
    }
    else
    {
        ObjectPrxPtr proxy = _newInstance();
        proxy->setup(_reference->changeMode(Reference::ModeBatchDatagram));
        return proxy;
    }
}

bool
ICE_OBJECT_PRX::ice_isBatchDatagram() const
{
    return _reference->getMode() == Reference::ModeBatchDatagram;
}

ObjectPrxPtr
ICE_OBJECT_PRX::ice_compress(bool b) const
{
    ReferencePtr ref = _reference->changeCompress(b);
    if(ref == _reference)
    {
        return CONST_POINTER_CAST_OBJECT_PRX;
    }
    else
    {
        ObjectPrxPtr proxy = _newInstance();
        proxy->setup(ref);
        return proxy;
    }
}

IceUtil::Optional<bool>
ICE_OBJECT_PRX::ice_getCompress() const
{
    return _reference->getCompress();
}

ObjectPrxPtr
ICE_OBJECT_PRX::ice_timeout(int t) const
{
    if(t < 1 && t != -1)
    {
        ostringstream s;
        s << "invalid value passed to ice_timeout: " << t;
        throw invalid_argument(s.str());
    }
    ReferencePtr ref = _reference->changeTimeout(t);
    if(ref == _reference)
    {
        return CONST_POINTER_CAST_OBJECT_PRX;
    }
    else
    {
        ObjectPrxPtr proxy = _newInstance();
        proxy->setup(ref);
        return proxy;
    }
}

IceUtil::Optional<int>
ICE_OBJECT_PRX::ice_getTimeout() const
{
    return _reference->getTimeout();
}

ObjectPrxPtr
ICE_OBJECT_PRX::ice_connectionId(const string& id) const
{
    ReferencePtr ref = _reference->changeConnectionId(id);
    if(ref == _reference)
    {
        return CONST_POINTER_CAST_OBJECT_PRX;
    }
    else
    {
        ObjectPrxPtr proxy = _newInstance();
        proxy->setup(ref);
        return proxy;
    }
}

string
ICE_OBJECT_PRX::ice_getConnectionId() const
{
    return _reference->getConnectionId();
}

ObjectPrxPtr
ICE_OBJECT_PRX::ice_fixed(const ::Ice::ConnectionPtr& connection) const
{
    if(!connection)
    {
        throw invalid_argument("invalid null connection passed to ice_fixed");
    }
    ::Ice::ConnectionIPtr impl = ICE_DYNAMIC_CAST(::Ice::ConnectionI, connection);
    if(!impl)
    {
        throw invalid_argument("invalid connection passed to ice_fixed");
    }
    ReferencePtr ref = _reference->changeConnection(impl);
    if(ref == _reference)
    {
        return CONST_POINTER_CAST_OBJECT_PRX;
    }
    else
    {
        ObjectPrxPtr proxy = _newInstance();
        proxy->setup(ref);
        return proxy;
    }
}

bool
ICE_OBJECT_PRX::ice_isFixed() const
{
    return FixedReferencePtr::dynamicCast(_reference);
}

ConnectionPtr
ICE_OBJECT_PRX::ice_getCachedConnection() const
{
    RequestHandlerPtr handler;
    {
        IceUtil::Mutex::Lock sync(_mutex);
        handler =  _requestHandler;
    }

    if(handler)
    {
        try
        {
            return handler->getConnection();
        }
        catch(const LocalException&)
        {
        }
    }
    return 0;
}

void
ICE_OBJECT_PRX::setup(const ReferencePtr& ref)
{
    //
    // No need to synchronize "*this", as this operation is only
    // called upon initialization.
    //

    assert(!_reference);
    assert(!_requestHandler);

    _reference = ref;
}

int
ICE_OBJECT_PRX::_handleException(const Exception& ex,
                                 const RequestHandlerPtr& handler,
                                 OperationMode mode,
                                 bool sent,
                                 int& cnt)
{
    _updateRequestHandler(handler, 0); // Clear the request handler

    //
    // We only retry local exception, system exceptions aren't retried.
    //
    // A CloseConnectionException indicates graceful server shutdown, and is therefore
    // always repeatable without violating "at-most-once". That's because by sending a
    // close connection message, the server guarantees that all outstanding requests
    // can safely be repeated.
    //
    // An ObjectNotExistException can always be retried as well without violating
    // "at-most-once" (see the implementation of the checkRetryAfterException method
    //  of the ProxyFactory class for the reasons why it can be useful).
    //
    // If the request didn't get sent or if it's non-mutating or idempotent it can
    // also always be retried if the retry count isn't reached.
    //
    const LocalException* localEx = dynamic_cast<const LocalException*>(&ex);
    if(localEx && (!sent ||
                   mode == ICE_ENUM(OperationMode, Nonmutating) || mode == ICE_ENUM(OperationMode, Idempotent) ||
                   dynamic_cast<const CloseConnectionException*>(&ex) ||
                   dynamic_cast<const ObjectNotExistException*>(&ex)))
    {
        try
        {
            return _reference->getInstance()->proxyFactory()->checkRetryAfterException(*localEx, _reference, cnt);
        }
        catch(const CommunicatorDestroyedException&)
        {
            //
            // The communicator is already destroyed, so we cannot retry.
            //
            ex.ice_throw();
        }
    }
    else
    {
        ex.ice_throw(); // Retry could break at-most-once semantics, don't retry.
    }
    return 0; // Keep the compiler happy.
}

::IceInternal::RequestHandlerPtr
ICE_OBJECT_PRX::_getRequestHandler()
{
    RequestHandlerPtr handler;
    if(_reference->getCacheConnection())
    {
        IceUtil::Mutex::Lock sync(_mutex);
        if(_requestHandler)
        {
            return _requestHandler;
        }
    }
    return _reference->getRequestHandler(ICE_SHARED_FROM_THIS);
}

IceInternal::BatchRequestQueuePtr
ICE_OBJECT_PRX::_getBatchRequestQueue()
{
    IceUtil::Mutex::Lock sync(_mutex);
    if(!_batchRequestQueue)
    {
        _batchRequestQueue = _reference->getBatchRequestQueue();
    }
    return _batchRequestQueue;
}

::IceInternal::RequestHandlerPtr
ICE_OBJECT_PRX::_setRequestHandler(const ::IceInternal::RequestHandlerPtr& handler)
{
    if(_reference->getCacheConnection())
    {
        IceUtil::Mutex::Lock sync(_mutex);
        if(!_requestHandler)
        {
            _requestHandler = handler;
        }
        return _requestHandler;
    }
    return handler;
}

void
ICE_OBJECT_PRX::_updateRequestHandler(const ::IceInternal::RequestHandlerPtr& previous,
                                      const ::IceInternal::RequestHandlerPtr& handler)
{
    if(_reference->getCacheConnection() && previous)
    {
        IceUtil::Mutex::Lock sync(_mutex);
        if(_requestHandler && _requestHandler.get() != handler.get())
        {
            //
            // Update the request handler only if "previous" is the same
            // as the current request handler. This is called after
            // connection binding by the connect request handler. We only
            // replace the request handler if the current handler is the
            // connect request handler.
            //
            _requestHandler = _requestHandler->update(previous, handler);
        }
    }
}

void
ICE_OBJECT_PRX::_copyFrom(const ObjectPrxPtr& from)
{
    IceUtil::Mutex::Lock sync(from->_mutex);
    _reference = from->_reference;
    _requestHandler = from->_requestHandler;
}

CommunicatorPtr
ICE_OBJECT_PRX::ice_getCommunicator() const
{
    return _reference->getCommunicator();
}

string
ICE_OBJECT_PRX::ice_toString() const
{
    //
    // Returns the stringified proxy. There's no need to convert the
    // string to a native string: a stringified proxy only contains
    // printable ASCII which is a subset of all native character sets.
    //
    return _reference->toString();
}

Int
ICE_OBJECT_PRX::_hash() const
{
    return _reference->hash();
}

void
ICE_OBJECT_PRX::_write(OutputStream& os) const
{
    os.write(_getReference()->getIdentity());
    _getReference()->streamWrite(&os);
}

bool
Ice::proxyIdentityLess(const ObjectPrxPtr& lhs, const ObjectPrxPtr& rhs)
{
    if(!lhs && !rhs)
    {
        return false;
    }
    else if(!lhs && rhs)
    {
        return true;
    }
    else if(lhs && !rhs)
    {
        return false;
    }
    else
    {
        return lhs->ice_getIdentity() < rhs->ice_getIdentity();
    }
}

bool
Ice::proxyIdentityEqual(const ObjectPrxPtr& lhs, const ObjectPrxPtr& rhs)
{
    if(!lhs && !rhs)
    {
        return true;
    }
    else if(!lhs && rhs)
    {
        return false;
    }
    else if(lhs && !rhs)
    {
        return false;
    }
    else
    {
        return lhs->ice_getIdentity() == rhs->ice_getIdentity();
    }
}

bool
Ice::proxyIdentityAndFacetLess(const ObjectPrxPtr& lhs, const ObjectPrxPtr& rhs)
{
    if(!lhs && !rhs)
    {
        return false;
    }
    else if(!lhs && rhs)
    {
        return true;
    }
    else if(lhs && !rhs)
    {
        return false;
    }
    else
    {
        Identity lhsIdentity = lhs->ice_getIdentity();
        Identity rhsIdentity = rhs->ice_getIdentity();

        if(lhsIdentity < rhsIdentity)
        {
            return true;
        }
        else if(rhsIdentity < lhsIdentity)
        {
            return false;
        }

        string lhsFacet = lhs->ice_getFacet();
        string rhsFacet = rhs->ice_getFacet();

        if(lhsFacet < rhsFacet)
        {
            return true;
        }
        else if(rhsFacet < lhsFacet)
        {
            return false;
        }

        return false;
    }
}

bool
Ice::proxyIdentityAndFacetEqual(const ObjectPrxPtr& lhs, const ObjectPrxPtr& rhs)
{
    if(!lhs && !rhs)
    {
        return true;
    }
    else if(!lhs && rhs)
    {
        return false;
    }
    else if(lhs && !rhs)
    {
        return false;
    }
    else
    {
        Identity lhsIdentity = lhs->ice_getIdentity();
        Identity rhsIdentity = rhs->ice_getIdentity();

        if(lhsIdentity == rhsIdentity)
        {
            string lhsFacet = lhs->ice_getFacet();
            string rhsFacet = rhs->ice_getFacet();

            if(lhsFacet == rhsFacet)
            {
                return true;
            }
        }

        return false;
    }
}
