// Copyright (c) ZeroC, Inc.

#include "Ice/Proxy.h"
#include "CheckIdentity.h"
#include "ConnectionI.h"
#include "EndpointI.h"
#include "Ice/Initialize.h"
#include "Ice/LocalExceptions.h"
#include "Instance.h"
#include "LocatorInfo.h"
#include "Reference.h"
#include "ReferenceFactory.h"
#include "RequestHandlerCache.h"
#include "RouterInfo.h"
#include "TargetCompare.h"

#include <stdexcept>

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace Ice
{
    const Context noExplicitContext;
}

namespace
{
    inline ReferencePtr createReference(const CommunicatorPtr& communicator, string_view proxyString)
    {
        if (!communicator)
        {
            throw std::invalid_argument("communicator cannot be null");
        }

        ReferencePtr ref = getInstance(communicator)->referenceFactory()->create(proxyString, "");
        if (!ref)
        {
            throw std::invalid_argument("invalid proxy string");
        }
        return ref;
    }
}

Ice::ObjectPrx::ObjectPrx(const CommunicatorPtr& communicator, string_view proxyString)
    : ObjectPrx(createReference(communicator, proxyString))
{
}

Ice::ObjectPrx::ObjectPrx(ReferencePtr&& ref)
    : _reference(std::move(ref)),
      _requestHandlerCache(make_shared<RequestHandlerCache>(_reference))
{
}

Ice::ObjectPrx::~ObjectPrx() = default; // avoid weak vtable

void
Ice::ObjectPrx::_checkTwowayOnly(string_view name) const
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

const Identity&
Ice::ObjectPrx::ice_getIdentity() const noexcept
{
    return _reference->getIdentity();
}

Context
Ice::ObjectPrx::ice_getContext() const
{
    return _reference->getContext()->getValue();
}

const string&
Ice::ObjectPrx::ice_getFacet() const noexcept
{
    return _reference->getFacet();
}

string
Ice::ObjectPrx::ice_getAdapterId() const
{
    return _reference->getAdapterId();
}

EndpointSeq
Ice::ObjectPrx::ice_getEndpoints() const
{
    EndpointSeq retSeq;
    for (const auto& p : _reference->getEndpoints())
    {
        retSeq.emplace_back(p);
    }
    return retSeq;
}

chrono::milliseconds
Ice::ObjectPrx::ice_getLocatorCacheTimeout() const noexcept
{
    return _reference->getLocatorCacheTimeout();
}

bool
Ice::ObjectPrx::ice_isConnectionCached() const noexcept
{
    return _reference->getCacheConnection();
}

EndpointSelectionType
Ice::ObjectPrx::ice_getEndpointSelection() const noexcept
{
    return _reference->getEndpointSelection();
}

bool
Ice::ObjectPrx::ice_isSecure() const noexcept
{
    return _reference->getSecure();
}

Ice::EncodingVersion
Ice::ObjectPrx::ice_getEncodingVersion() const noexcept
{
    return _reference->getEncoding();
}

bool
Ice::ObjectPrx::ice_isPreferSecure() const noexcept
{
    return _reference->getPreferSecure();
}

optional<RouterPrx>
Ice::ObjectPrx::ice_getRouter() const noexcept
{
    RouterInfoPtr routerInfo = _reference->getRouterInfo();
    return routerInfo ? make_optional(routerInfo->getRouter()) : nullopt;
}

optional<LocatorPrx>
Ice::ObjectPrx::ice_getLocator() const noexcept
{
    LocatorInfoPtr locatorInfo = _reference->getLocatorInfo();
    return locatorInfo ? make_optional(locatorInfo->getLocator()) : nullopt;
}

bool
Ice::ObjectPrx::ice_isCollocationOptimized() const noexcept
{
    return _reference->getCollocationOptimized();
}

chrono::milliseconds
Ice::ObjectPrx::ice_getInvocationTimeout() const noexcept
{
    return _reference->getInvocationTimeout();
}

bool
Ice::ObjectPrx::ice_isTwoway() const noexcept
{
    return _reference->getMode() == Reference::ModeTwoway;
}

bool
Ice::ObjectPrx::ice_isOneway() const noexcept
{
    return _reference->getMode() == Reference::ModeOneway;
}

bool
Ice::ObjectPrx::ice_isBatchOneway() const noexcept
{
    return _reference->getMode() == Reference::ModeBatchOneway;
}

bool
Ice::ObjectPrx::ice_isDatagram() const noexcept
{
    return _reference->getMode() == Reference::ModeDatagram;
}

bool
Ice::ObjectPrx::ice_isBatchDatagram() const noexcept
{
    return _reference->getMode() == Reference::ModeBatchDatagram;
}

optional<bool>
Ice::ObjectPrx::ice_getCompress() const noexcept
{
    return _reference->getCompress();
}

string
Ice::ObjectPrx::ice_getConnectionId() const
{
    return _reference->getConnectionId();
}

bool
Ice::ObjectPrx::ice_isFixed() const noexcept
{
    return dynamic_pointer_cast<FixedReference>(_reference) != nullptr;
}

ConnectionPtr
Ice::ObjectPrx::ice_getCachedConnection() const noexcept
{
    return _requestHandlerCache->getCachedConnection();
}

CommunicatorPtr
Ice::ObjectPrx::ice_getCommunicator() const noexcept
{
    return _reference->getCommunicator();
}

const char*
Ice::ObjectPrx::ice_staticId() noexcept
{
    return "::Ice::Object";
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

size_t
Ice::ObjectPrx::_hash() const noexcept
{
    return hash<Reference>{}(*_reference);
}

void
Ice::ObjectPrx::_write(OutputStream& os) const
{
    os.write(_reference->getIdentity());
    _reference->streamWrite(&os);
}

ReferencePtr
Ice::ObjectPrx::_adapterId(string newAdapterId) const
{
    if (newAdapterId == _reference->getAdapterId())
    {
        return _reference;
    }
    else
    {
        return _reference->changeAdapterId(std::move(newAdapterId));
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
Ice::ObjectPrx::_connectionId(string id) const
{
    ReferencePtr ref = _reference->changeConnectionId(std::move(id));
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
Ice::ObjectPrx::_context(Context newContext) const
{
    return _reference->changeContext(std::move(newContext));
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
Ice::ObjectPrx::_encodingVersion(EncodingVersion encoding) const
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
Ice::ObjectPrx::_endpoints(EndpointSeq newEndpoints) const // NOLINT(performance-unnecessary-value-param)
{
    vector<EndpointIPtr> endpoints;
    for (const auto& p : newEndpoints)
    {
        endpoints.emplace_back(dynamic_pointer_cast<EndpointI>(p));
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
        return _reference->changeEndpoints(std::move(endpoints));
    }
}

ReferencePtr
Ice::ObjectPrx::_identity(Identity newIdentity) const
{
    checkIdentity(newIdentity, __FILE__, __LINE__);
    if (newIdentity == _reference->getIdentity())
    {
        return _reference;
    }
    else
    {
        return _reference->changeIdentity(std::move(newIdentity));
    }
}

ReferencePtr
Ice::ObjectPrx::_facet(string newFacet) const
{
    if (newFacet == _reference->getFacet())
    {
        return _reference;
    }
    else
    {
        return _reference->changeFacet(std::move(newFacet));
    }
}

ReferencePtr
Ice::ObjectPrx::_fixed(ConnectionPtr connection) const // NOLINT(performance-unnecessary-value-param)
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
    ReferencePtr ref = _reference->changeConnection(std::move(impl));
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
Ice::ObjectPrx::_invocationTimeout(chrono::milliseconds newTimeout) const
{
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
Ice::ObjectPrx::_locator(const std::optional<LocatorPrx>& locator) const
{
    ReferencePtr ref = _reference->changeLocator(locator);
    if (*ref == *_reference)
    {
        return _reference;
    }
    else
    {
        return ref;
    }
}

ReferencePtr
Ice::ObjectPrx::_locatorCacheTimeout(chrono::milliseconds newTimeout) const
{
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
Ice::ObjectPrx::_router(const std::optional<RouterPrx>& router) const
{
    ReferencePtr ref = _reference->changeRouter(router);
    if (*ref == *_reference)
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
