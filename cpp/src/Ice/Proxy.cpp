//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Ice/Proxy.h"
#include "Ice/Comparable.h"
#include "Reference.h"
#include "EndpointI.h"
#include "RouterInfo.h"
#include "LocatorInfo.h"
#include "Ice/LocalException.h"
#include "RequestHandlerCache.h"
#include "ConnectionI.h"

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace Ice
{

const Context noExplicitContext;

}

namespace Ice
{

bool
operator<(const ObjectPrx& lhs, const ObjectPrx& rhs)
{
    return targetLess(lhs._getReference(), rhs._getReference());
}

bool
operator==(const ObjectPrx& lhs, const ObjectPrx& rhs)
{
    return targetEqualTo(lhs._getReference(), rhs._getReference());
}

}

Ice::ObjectPrx::ObjectPrx(const ReferencePtr& ref) noexcept :
    _reference(ref),
    _requestHandlerCache(make_shared<RequestHandlerCache>(ref)),
    _batchRequestQueue(ref->isBatch() ? ref->getBatchRequestQueue() : nullptr)
{
}

Ice::ObjectPrx::ObjectPrx(const ObjectPrx& other) noexcept :
    std::enable_shared_from_this<ObjectPrx>(),
    _reference(other._reference),
    _requestHandlerCache(other._requestHandlerCache),
    _batchRequestQueue(_reference->isBatch() ? _reference->getBatchRequestQueue() : nullptr)
{
}

void
Ice::ObjectPrx::_checkTwowayOnly(const string& name) const
{
    if (!ice_isTwoway())
    {
        throw Ice::TwowayOnlyException(__FILE__, __LINE__, name);
    }
}

ostream&
Ice::operator<<(ostream& os, const Ice::ObjectPrx& p)
{
    return os << p.ice_toString();
}

Identity
Ice::ObjectPrx::ice_getIdentity() const
{
    return _reference->getIdentity();
}

ObjectPrxPtr
Ice::ObjectPrx::ice_identity(const Identity& newIdentity) const
{
    if(newIdentity.name.empty())
    {
        throw IllegalIdentityException(__FILE__, __LINE__);
    }
    if(newIdentity == _reference->getIdentity())
    {
        return const_pointer_cast<ObjectPrx>(shared_from_this());
    }
    else
    {
        return make_shared<ObjectPrx>(_reference->changeIdentity(newIdentity));
    }
}

Context
Ice::ObjectPrx::ice_getContext() const
{
    return _reference->getContext()->getValue();
}

const string&
Ice::ObjectPrx::ice_getFacet() const
{
    return _reference->getFacet();
}

ObjectPrxPtr
Ice::ObjectPrx::ice_facet(const string& newFacet) const
{
    if(newFacet == _reference->getFacet())
    {
        return const_pointer_cast<ObjectPrx>(shared_from_this());
    }
    else
    {
        return make_shared<ObjectPrx>(_reference->changeFacet(newFacet));
    }
}

string
Ice::ObjectPrx::ice_getAdapterId() const
{
    return _reference->getAdapterId();
}

EndpointSeq
Ice::ObjectPrx::ice_getEndpoints() const
{
    vector<EndpointIPtr> endpoints = _reference->getEndpoints();
    EndpointSeq retSeq;
    for(vector<EndpointIPtr>::const_iterator p = endpoints.begin(); p != endpoints.end(); ++p)
    {
        retSeq.push_back(dynamic_pointer_cast<Endpoint>(*p));
    }
    return retSeq;
}

Int
Ice::ObjectPrx::ice_getLocatorCacheTimeout() const
{
    return _reference->getLocatorCacheTimeout();
}

bool
Ice::ObjectPrx::ice_isConnectionCached() const
{
    return _reference->getCacheConnection();
}

EndpointSelectionType
Ice::ObjectPrx::ice_getEndpointSelection() const
{
    return _reference->getEndpointSelection();
}

bool
Ice::ObjectPrx::ice_isSecure() const
{
    return _reference->getSecure();
}

::Ice::EncodingVersion
Ice::ObjectPrx::ice_getEncodingVersion() const
{
    return _reference->getEncoding();
}

bool
Ice::ObjectPrx::ice_isPreferSecure() const
{
    return _reference->getPreferSecure();
}

RouterPrxPtr
Ice::ObjectPrx::ice_getRouter() const
{
    RouterInfoPtr ri = _reference->getRouterInfo();
    return ri ? ri->getRouter() : nullptr;
}

LocatorPrxPtr
Ice::ObjectPrx::ice_getLocator() const
{
    LocatorInfoPtr ri = _reference->getLocatorInfo();
    return ri ? ri->getLocator() : nullptr;
}

bool
Ice::ObjectPrx::ice_isCollocationOptimized() const
{
    return _reference->getCollocationOptimized();
}

Int
Ice::ObjectPrx::ice_getInvocationTimeout() const
{
    return _reference->getInvocationTimeout();
}

bool
Ice::ObjectPrx::ice_isTwoway() const
{
    return _reference->getMode() == Reference::ModeTwoway;
}

bool
Ice::ObjectPrx::ice_isOneway() const
{
    return _reference->getMode() == Reference::ModeOneway;
}

bool
Ice::ObjectPrx::ice_isBatchOneway() const
{
    return _reference->getMode() == Reference::ModeBatchOneway;
}

bool
Ice::ObjectPrx::ice_isDatagram() const
{
    return _reference->getMode() == Reference::ModeDatagram;
}

bool
Ice::ObjectPrx::ice_isBatchDatagram() const
{
    return _reference->getMode() == Reference::ModeBatchDatagram;
}

optional<bool>
Ice::ObjectPrx::ice_getCompress() const
{
    return _reference->getCompress();
}

optional<int>
Ice::ObjectPrx::ice_getTimeout() const
{
    return _reference->getTimeout();
}

string
Ice::ObjectPrx::ice_getConnectionId() const
{
    return _reference->getConnectionId();
}

bool
Ice::ObjectPrx::ice_isFixed() const
{
    return dynamic_pointer_cast<FixedReference>(_reference) != nullptr;
}

ConnectionPtr
Ice::ObjectPrx::ice_getCachedConnection() const
{
    return _requestHandlerCache->getCachedConnection();
}

CommunicatorPtr
Ice::ObjectPrx::ice_getCommunicator() const
{
    return _reference->getCommunicator();
}

string
Ice::ObjectPrx::ice_toString() const
{
    //
    // Returns the stringified proxy. There's no need to convert the
    // string to a native string: a stringified proxy only contains
    // printable ASCII which is a subset of all native character sets.
    //
    return _reference->toString();
}

Int
Ice::ObjectPrx::_hash() const
{
    return _reference->hash();
}

void
Ice::ObjectPrx::_write(OutputStream& os) const
{
    os.write(_reference->getIdentity());
    _reference->streamWrite(&os);
}

ReferencePtr
Ice::ObjectPrx::_adapterId(const string& newAdapterId) const
{
    if (newAdapterId == _reference->getAdapterId())
    {
        return _reference;
    }
    else
    {
        return _reference->changeAdapterId(newAdapterId);
    }
}

ReferencePtr
Ice::ObjectPrx::_batchDatagram() const
{
    if (_reference->getMode() == Reference::ModeBatchDatagram)
    {
        return _reference;
    }
    else
    {
       return _reference->changeMode(Reference::ModeBatchDatagram);
    }
}

ReferencePtr
Ice::ObjectPrx::_batchOneway() const
{
    if (_reference->getMode() == Reference::ModeBatchOneway)
    {
        return _reference;
    }
    else
    {
        return _reference->changeMode(Reference::ModeBatchOneway);
    }
}

ReferencePtr
Ice::ObjectPrx::_collocationOptimized(bool b) const
{
    if (b == _reference->getCollocationOptimized())
    {
        return _reference;
    }
    else
    {
        return _reference->changeCollocationOptimized(b);
    }
}

ReferencePtr
Ice::ObjectPrx::_compress(bool b) const
{
    ReferencePtr ref = _reference->changeCompress(b);
    if (targetEqualTo(ref, _reference))
    {
        return _reference;
    }
    else
    {
       return ref;
    }
}

ReferencePtr
Ice::ObjectPrx::_connectionCached(bool newCache) const
{
    if (newCache == _reference->getCacheConnection())
    {
        return _reference;
    }
    else
    {
        return _reference->changeCacheConnection(newCache);
    }
}

ReferencePtr
Ice::ObjectPrx::_connectionId(const string& id) const
{
    ReferencePtr ref = _reference->changeConnectionId(id);
    if (targetEqualTo(ref, _reference))
    {
        return _reference;
    }
    else
    {
        return ref;
    }
}

ReferencePtr
Ice::ObjectPrx::_context(const Context& newContext) const
{
    return _reference->changeContext(newContext);
}

ReferencePtr
Ice::ObjectPrx::_datagram() const
{
    if (_reference->getMode() == Reference::ModeDatagram)
    {
        return _reference;
    }
    else
    {
        return _reference->changeMode(Reference::ModeDatagram);
    }
}

ReferencePtr
Ice::ObjectPrx::_encodingVersion(const ::Ice::EncodingVersion& encoding) const
{
    if (encoding == _reference->getEncoding())
    {
        return _reference;
    }
    else
    {
        return _reference->changeEncoding(encoding);
    }
}

ReferencePtr
Ice::ObjectPrx::_endpointSelection(EndpointSelectionType newType) const
{
    if (newType == _reference->getEndpointSelection())
    {
        return _reference;
    }
    else
    {
        return _reference->changeEndpointSelection(newType);
    }
}

ReferencePtr
Ice::ObjectPrx::_endpoints(const EndpointSeq& newEndpoints) const
{
    vector<EndpointIPtr> endpoints;
    for (EndpointSeq::const_iterator p = newEndpoints.begin(); p != newEndpoints.end(); ++p)
    {
        endpoints.push_back(dynamic_pointer_cast<EndpointI>(*p));
    }

    auto currentEndpoints = _reference->getEndpoints();

    if (equal(
            endpoints.begin(),
            endpoints.end(),
            currentEndpoints.begin(),
            currentEndpoints.end(),
            targetEqualTo<EndpointIPtr, EndpointIPtr>))
    {
        return _reference;
    }
    else
    {
        return _reference->changeEndpoints(endpoints);
    }
}

ReferencePtr
Ice::ObjectPrx::_fixed(const ::Ice::ConnectionPtr& connection) const
{
    if (!connection)
    {
        throw invalid_argument("invalid null connection passed to ice_fixed");
    }
    auto impl = dynamic_pointer_cast<Ice::ConnectionI>(connection);
    if (!impl)
    {
        throw invalid_argument("invalid connection passed to ice_fixed");
    }
    ReferencePtr ref = _reference->changeConnection(impl);
    if (targetEqualTo(ref, _reference))
    {
        return _reference;
    }
    else
    {
        return ref;
    }
}

ReferencePtr
Ice::ObjectPrx::_invocationTimeout(Int newTimeout) const
{
    if (newTimeout < 1 && newTimeout != -1 && newTimeout != -2)
    {
        ostringstream s;
        s << "invalid value passed to ice_invocationTimeout: " << newTimeout;
        throw invalid_argument(s.str());
    }
    if (newTimeout == _reference->getInvocationTimeout())
    {
        return _reference;
    }
    else
    {
        return _reference->changeInvocationTimeout(newTimeout);
    }
}

ReferencePtr
Ice::ObjectPrx::_locator(const LocatorPrxPtr& locator) const
{
    ReferencePtr ref = _reference->changeLocator(locator);
    if (targetEqualTo(ref, _reference))
    {
        return _reference;
    }
    else
    {
        return ref;
    }
}

ReferencePtr
Ice::ObjectPrx::_locatorCacheTimeout(Int newTimeout) const
{
    if (newTimeout < -1)
    {
        ostringstream s;
        s << "invalid value passed to ice_locatorCacheTimeout: " << newTimeout;
        throw invalid_argument(s.str());
    }
    if (newTimeout == _reference->getLocatorCacheTimeout())
    {
        return _reference;
    }
    else
    {
        return _reference->changeLocatorCacheTimeout(newTimeout);
    }
}

ReferencePtr
Ice::ObjectPrx::_oneway() const
{
    if (_reference->getMode() == Reference::ModeOneway)
    {
        return _reference;
    }
    else
    {
        return _reference->changeMode(Reference::ModeOneway);
    }
}

ReferencePtr
Ice::ObjectPrx::_preferSecure(bool b) const
{
    if (b == _reference->getPreferSecure())
    {
        return _reference;
    }
    else
    {
       return _reference->changePreferSecure(b);
    }
}

ReferencePtr
Ice::ObjectPrx::_router(const RouterPrxPtr& router) const
{
    ReferencePtr ref = _reference->changeRouter(router);
    if (targetEqualTo(ref, _reference))
    {
        return _reference;
    }
    else
    {
        return ref;
    }
}

ReferencePtr
Ice::ObjectPrx::_secure(bool b) const
{
    if (b == _reference->getSecure())
    {
        return _reference;
    }
    else
    {
        return _reference->changeSecure(b);
    }
}

ReferencePtr
Ice::ObjectPrx::_timeout(int t) const
{
    if (t < 1 && t != -1)
    {
        ostringstream s;
        s << "invalid value passed to ice_timeout: " << t;
        throw invalid_argument(s.str());
    }
    ReferencePtr ref = _reference->changeTimeout(t);
    if (targetEqualTo(ref, _reference))
    {
        return _reference;
    }
    else
    {
        return ref;
    }
}

ReferencePtr
Ice::ObjectPrx::_twoway() const
{
    if (_reference->getMode() == Reference::ModeTwoway)
    {
        return _reference;
    }
    else
    {
        return _reference->changeMode(Reference::ModeTwoway);
    }
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
