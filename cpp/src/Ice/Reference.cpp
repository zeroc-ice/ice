// Copyright (c) ZeroC, Inc.

#include "Reference.h"
#include "CollocatedRequestHandler.h"
#include "ConnectRequestHandler.h"
#include "ConnectionFactory.h"
#include "ConnectionI.h"
#include "DefaultsAndOverrides.h"
#include "EndpointI.h"
#include "FixedRequestHandler.h"
#include "HashUtil.h"
#include "Ice/LocalExceptions.h"
#include "Ice/Locator.h"
#include "Ice/LoggerUtil.h"
#include "Ice/OutputStream.h"
#include "Ice/Router.h"
#include "Ice/StringUtil.h"
#include "Instance.h"
#include "LocatorInfo.h"
#include "ObjectAdapterFactory.h"
#include "OpaqueEndpointI.h"
#include "Random.h"
#include "ReferenceFactory.h"
#include "RouterInfo.h"
#include "TargetCompare.h"
#include "TraceLevels.h"

#include <algorithm>
#include <functional>

using namespace std;
using namespace Ice;
using namespace IceInternal;

CommunicatorPtr
IceInternal::Reference::getCommunicator() const noexcept
{
    return _communicator;
}

ReferencePtr
IceInternal::Reference::changeContext(Context newContext) const
{
    ReferencePtr r = clone();
    r->_context = make_shared<SharedContext>(std::move(newContext));
    return r;
}

ReferencePtr
IceInternal::Reference::changeMode(Mode newMode) const
{
    ReferencePtr r = clone();
    r->_mode = newMode;
    return r;
}

ReferencePtr
IceInternal::Reference::changeSecure(bool newSecure) const
{
    ReferencePtr r = clone();
    r->_secure = newSecure;
    return r;
}

ReferencePtr
IceInternal::Reference::changeIdentity(Identity newIdentity) const
{
    ReferencePtr r = clone();
    r->_identity = std::move(newIdentity);
    return r;
}

ReferencePtr
IceInternal::Reference::changeFacet(string newFacet) const
{
    ReferencePtr r = clone();
    r->_facet = std::move(newFacet);
    return r;
}

ReferencePtr
IceInternal::Reference::changeInvocationTimeout(chrono::milliseconds invocationTimeout) const
{
    ReferencePtr r = clone();
    r->_invocationTimeout = invocationTimeout;
    return r;
}

ReferencePtr
IceInternal::Reference::changeEncoding(Ice::EncodingVersion encoding) const
{
    ReferencePtr r = clone();
    r->_encoding = encoding;
    return r;
}

ReferencePtr
IceInternal::Reference::changeCompress(bool newCompress) const
{
    ReferencePtr r = clone();
    r->_compress = newCompress;
    return r;
}

optional<bool>
IceInternal::Reference::getCompressOverride() const noexcept
{
    DefaultsAndOverridesPtr defaultsAndOverrides = getInstance()->defaultsAndOverrides();
    optional<bool> compress =
        defaultsAndOverrides->overrideCompress.has_value() ? defaultsAndOverrides->overrideCompress : _compress;
    return compress;
}

size_t
Reference::hash() const noexcept
{
    size_t h = 5381;
    hashAdd(h, static_cast<int32_t>(_mode));
    hashAdd(h, _secure);
    hashAdd(h, _identity.name);
    hashAdd(h, _identity.category);
    hashAdd(h, _context->getValue());
    hashAdd(h, _facet);
    hashAdd(h, _compress);
    // We don't include protocol and encoding in the hash; they are using 1.0 and 1.1, respectively.
    hashAdd(h, _invocationTimeout.count());
    return h;
}

void
IceInternal::Reference::streamWrite(OutputStream* s) const
{
    //
    // Don't write the identity here. Operations calling streamWrite
    // write the identity.
    //

    //
    // For compatibility with the old FacetPath.
    //
    if (_facet.empty())
    {
        s->write(static_cast<string*>(nullptr), static_cast<string*>(nullptr));
    }
    else
    {
        s->write(&_facet, &_facet + 1);
    }

    s->write(static_cast<uint8_t>(_mode));

    s->write(_secure);

    if (s->getEncoding() != Ice::Encoding_1_0)
    {
        s->write(_protocol);
        s->write(_encoding);
    }

    // Derived class writes the remainder of the reference.
}

string
IceInternal::Reference::toString() const
{
    //
    // WARNING: Certain features, such as proxy validation in Glacier2,
    // depend on the format of proxy strings. Changes to toString() and
    // methods called to generate parts of the reference string could break
    // these features. Please review for all features that depend on the
    // format of proxyToString() before changing this and related code.
    //
    ostringstream s;

    ToStringMode toStringMode = _instance->toStringMode();
    const string separators = " :@";

    string id = Ice::identityToString(_identity, toStringMode);

    //
    // If the encoded identity string contains characters which
    // the reference parser uses as separators, then we enclose
    // the identity string in quotes.
    //

    if (id.find_first_of(separators) != string::npos)
    {
        s << '"' << id << '"';
    }
    else
    {
        s << id;
    }

    if (!_facet.empty())
    {
        s << " -f ";

        string fs = escapeString(_facet, "", toStringMode);
        //
        // If the encoded facet string contains characters which
        // the reference parser uses as separators, then we enclose
        // the facet string in quotes.
        //
        if (fs.find_first_of(separators) != string::npos)
        {
            s << '"' << fs << '"';
        }
        else
        {
            s << fs;
        }
    }

    switch (_mode)
    {
        case ModeTwoway:
        {
            // Don't print the default mode.
            break;
        }

        case ModeOneway:
        {
            s << " -o";
            break;
        }

        case ModeBatchOneway:
        {
            s << " -O";
            break;
        }

        case ModeDatagram:
        {
            s << " -d";
            break;
        }

        case ModeBatchDatagram:
        {
            s << " -D";
            break;
        }
    }

    if (_secure)
    {
        s << " -s";
    }

    if (_protocol != Ice::Protocol_1_0)
    {
        // We print the protocol unless it's 1.0.
        s << " -p " << _protocol;
    }

    // We print the encoding if it's not 1.1 or if Ice.Default.EncodingVersion is set to something other than 1.1.
    if (_encoding != Ice::Encoding_1_1 || _instance->defaultsAndOverrides()->defaultEncoding != Ice::Encoding_1_1)
    {
        s << " -e " << _encoding;
    }

    return s.str();

    // Derived class writes the remainder of the string.
}

bool
IceInternal::Reference::operator==(const Reference& r) const noexcept
{
    //
    // Note: if(this == &r) test is performed by each non-abstract derived class.
    //

    if (_mode != r._mode)
    {
        return false;
    }

    if (_secure != r._secure)
    {
        return false;
    }

    if (_identity != r._identity)
    {
        return false;
    }

    if (_context->getValue() != r._context->getValue())
    {
        return false;
    }

    if (_facet != r._facet)
    {
        return false;
    }

    if (_compress != r._compress)
    {
        return false;
    }

    if (_protocol != r._protocol)
    {
        return false;
    }

    if (_encoding != r._encoding)
    {
        return false;
    }

    if (_invocationTimeout != r._invocationTimeout)
    {
        return false;
    }
    return true;
}

bool
IceInternal::Reference::operator<(const Reference& r) const noexcept
{
    //
    // Note: if(this == &r) test is performed by each non-abstract derived class.
    //

    if (_mode < r._mode)
    {
        return true;
    }
    else if (r._mode < _mode)
    {
        return false;
    }

    if (_identity < r._identity)
    {
        return true;
    }
    else if (r._identity < _identity)
    {
        return false;
    }

    if (_context->getValue() < r._context->getValue())
    {
        return true;
    }
    else if (r._context->getValue() < _context->getValue())
    {
        return false;
    }

    if (_facet < r._facet)
    {
        return true;
    }
    else if (r._facet < _facet)
    {
        return false;
    }

    if (_compress < r._compress)
    {
        return true;
    }
    else if (r._compress < _compress)
    {
        return false;
    }

    if (!_secure && r._secure)
    {
        return true;
    }
    else if (r._secure < _secure)
    {
        return false;
    }

    if (_protocol < r._protocol)
    {
        return true;
    }
    else if (r._protocol < _protocol)
    {
        return false;
    }

    if (_encoding < r._encoding)
    {
        return true;
    }
    else if (r._encoding < _encoding)
    {
        return false;
    }

    if (_invocationTimeout < r._invocationTimeout)
    {
        return true;
    }
    else if (r._invocationTimeout < _invocationTimeout)
    {
        return false;
    }

    return false;
}

IceInternal::Reference::Reference(
    InstancePtr instance,
    CommunicatorPtr communicator,
    Identity id,
    string facet,
    Mode mode,
    bool secure,
    std::optional<bool> compress,
    ProtocolVersion protocol,
    EncodingVersion encoding,
    chrono::milliseconds invocationTimeout,
    Context ctx)
    : _instance(std::move(instance)),
      _communicator(std::move(communicator)),
      _mode(mode),
      _secure(secure),
      _compress(compress),
      _identity(std::move(id)),
      _context(make_shared<SharedContext>(std::move(ctx))),
      _facet(std::move(facet)),
      _protocol(protocol),
      _encoding(encoding),
      _invocationTimeout(invocationTimeout)
{
}

IceInternal::Reference::Reference(const Reference& r)
    : enable_shared_from_this<Reference>(),
      _instance(r._instance),
      _communicator(r._communicator),
      _mode(r._mode),
      _secure(r._secure),
      _compress(r._compress),
      _identity(r._identity),
      _context(r._context),
      _facet(r._facet),
      _protocol(r._protocol),
      _encoding(r._encoding),
      _invocationTimeout(r._invocationTimeout)
{
}

IceInternal::FixedReference::FixedReference(
    InstancePtr instance,
    CommunicatorPtr communicator,
    Identity id,
    string facet,
    Mode mode,
    bool secure,
    std::optional<bool> compress,
    ProtocolVersion protocol,
    EncodingVersion encoding,
    ConnectionIPtr fixedConnection,
    chrono::milliseconds invocationTimeout,
    Ice::Context context)
    : Reference(
          std::move(instance),
          std::move(communicator),
          std::move(id),
          std::move(facet),
          mode,
          secure,
          compress,
          protocol,
          encoding,
          invocationTimeout,
          std::move(context)),
      _fixedConnection(std::move(fixedConnection))
{
}

vector<EndpointIPtr>
IceInternal::FixedReference::getEndpoints() const
{
    return {};
}

string
IceInternal::FixedReference::getAdapterId() const
{
    return {};
}

bool
IceInternal::FixedReference::getCollocationOptimized() const noexcept
{
    return false;
}

bool
IceInternal::FixedReference::getCacheConnection() const noexcept
{
    return true;
}

bool
IceInternal::FixedReference::getPreferSecure() const noexcept
{
    return false;
}

Ice::EndpointSelectionType
IceInternal::FixedReference::getEndpointSelection() const noexcept
{
    return EndpointSelectionType::Random;
}

chrono::milliseconds
IceInternal::FixedReference::getLocatorCacheTimeout() const noexcept
{
    return 0ms;
}

string
IceInternal::FixedReference::getConnectionId() const
{
    return {};
}

ReferencePtr
IceInternal::FixedReference::changeEndpoints(vector<EndpointIPtr> /*newEndpoints*/) const
{
    throw FixedProxyException(__FILE__, __LINE__);
}

ReferencePtr
IceInternal::FixedReference::changeAdapterId(string /*newAdapterId*/) const
{
    throw FixedProxyException(__FILE__, __LINE__);
}

ReferencePtr
IceInternal::FixedReference::changeLocator(optional<LocatorPrx>) const
{
    throw FixedProxyException(__FILE__, __LINE__);
}

ReferencePtr
IceInternal::FixedReference::changeRouter(optional<RouterPrx>) const
{
    throw FixedProxyException(__FILE__, __LINE__);
}

ReferencePtr
IceInternal::FixedReference::changeCollocationOptimized(bool) const
{
    throw FixedProxyException(__FILE__, __LINE__);
}

ReferencePtr
IceInternal::FixedReference::changeCacheConnection(bool) const
{
    throw FixedProxyException(__FILE__, __LINE__);
}

ReferencePtr
IceInternal::FixedReference::changePreferSecure(bool) const
{
    throw FixedProxyException(__FILE__, __LINE__);
}

ReferencePtr
IceInternal::FixedReference::changeEndpointSelection(EndpointSelectionType) const
{
    throw FixedProxyException(__FILE__, __LINE__);
}

ReferencePtr
IceInternal::FixedReference::changeLocatorCacheTimeout(chrono::milliseconds) const
{
    throw FixedProxyException(__FILE__, __LINE__);
}

ReferencePtr
IceInternal::FixedReference::changeConnectionId(string) const
{
    throw FixedProxyException(__FILE__, __LINE__);
}

ReferencePtr
IceInternal::FixedReference::changeConnection(Ice::ConnectionIPtr newConnection) const
{
    FixedReferencePtr r = dynamic_pointer_cast<FixedReference>(clone());
    r->_fixedConnection = std::move(newConnection);
    return r;
}

bool
IceInternal::FixedReference::isIndirect() const noexcept
{
    return false;
}

bool
IceInternal::FixedReference::isWellKnown() const noexcept
{
    return false;
}

void
IceInternal::FixedReference::streamWrite(OutputStream*) const
{
    throw FixedProxyException(__FILE__, __LINE__);
}

PropertyDict
IceInternal::FixedReference::toProperty(string) const
{
    throw FixedProxyException(__FILE__, __LINE__);
}

RequestHandlerPtr
IceInternal::FixedReference::getRequestHandler() const
{
    // We need to perform all these checks here and not in the constructor because changeConnection() clones then
    // sets the connection.

    switch (getMode())
    {
        case Reference::ModeTwoway:
        case Reference::ModeOneway:
        case Reference::ModeBatchOneway:
        {
            if (_fixedConnection->endpoint()->datagram())
            {
                throw NoEndpointException(
                    __FILE__,
                    __LINE__,
                    ObjectPrx::_fromReference(const_cast<FixedReference*>(this)->shared_from_this()));
            }
            break;
        }

        case Reference::ModeDatagram:
        case Reference::ModeBatchDatagram:
        {
            if (!_fixedConnection->endpoint()->datagram())
            {
                throw NoEndpointException(
                    __FILE__,
                    __LINE__,
                    ObjectPrx::_fromReference(const_cast<FixedReference*>(this)->shared_from_this()));
            }
            break;
        }
    }

    //
    // If a secure connection is requested or secure overrides is set,
    // check if the connection is secure.
    //
    bool secure;
    DefaultsAndOverridesPtr defaultsAndOverrides = getInstance()->defaultsAndOverrides();
    if (defaultsAndOverrides->overrideSecure.has_value())
    {
        secure = *defaultsAndOverrides->overrideSecure;
    }
    else
    {
        secure = getSecure();
    }
    if (secure && !_fixedConnection->endpoint()->secure())
    {
        throw NoEndpointException(
            __FILE__,
            __LINE__,
            ObjectPrx::_fromReference(const_cast<FixedReference*>(this)->shared_from_this()));
    }

    _fixedConnection->throwException(); // Throw in case our connection is already destroyed.

    bool compress = defaultsAndOverrides->overrideCompress.has_value() ? *defaultsAndOverrides->overrideCompress
                                                                       : getCompress().value_or(false);

    ReferencePtr ref = const_cast<FixedReference*>(this)->shared_from_this();
    return make_shared<FixedRequestHandler>(ref, _fixedConnection, compress);
}

const BatchRequestQueuePtr&
IceInternal::FixedReference::getBatchRequestQueue() const noexcept
{
    return _fixedConnection->getBatchRequestQueue();
}

bool
IceInternal::FixedReference::operator==(const Reference& r) const noexcept
{
    if (this == &r)
    {
        return true;
    }
    const auto* rhs = dynamic_cast<const FixedReference*>(&r);
    if (!rhs || !Reference::operator==(r))
    {
        return false;
    }
    return _fixedConnection == rhs->_fixedConnection;
}

bool
IceInternal::FixedReference::operator<(const Reference& r) const noexcept
{
    if (this == &r)
    {
        return false;
    }
    if (Reference::operator<(r))
    {
        return true;
    }
    if (!Reference::operator==(r))
    {
        return false;
    }

    const auto* rhs = dynamic_cast<const FixedReference*>(&r);
    if (!rhs)
    {
        assert(dynamic_cast<const RoutableReference*>(&r));
        return false; // As a rule, routable references are superior to fixed references.
    }
    return _fixedConnection < rhs->_fixedConnection;
}

ReferencePtr
IceInternal::FixedReference::clone() const
{
    return make_shared<FixedReference>(*this);
}

IceInternal::RoutableReference::RoutableReference(
    InstancePtr instance,
    CommunicatorPtr communicator,
    Identity id,
    string facet,
    Mode mode,
    bool secure,
    optional<bool> compress,
    ProtocolVersion protocol,
    EncodingVersion encoding,
    vector<EndpointIPtr> endpoints,
    string adapterId,
    LocatorInfoPtr locatorInfo,
    RouterInfoPtr routerInfo,
    bool collocationOptimized,
    bool cacheConnection,
    bool preferSecure,
    EndpointSelectionType endpointSelection,
    chrono::milliseconds locatorCacheTimeout,
    chrono::milliseconds invocationTimeout,
    Ice::Context ctx)
    : Reference(
          std::move(instance),
          std::move(communicator),
          std::move(id),
          std::move(facet),
          mode,
          secure,
          compress,
          protocol,
          encoding,
          invocationTimeout,
          std::move(ctx)),
      _endpoints(std::move(endpoints)),
      _adapterId(std::move(adapterId)),
      _locatorInfo(std::move(locatorInfo)),
      _routerInfo(std::move(routerInfo)),
      _collocationOptimized(collocationOptimized),
      _cacheConnection(cacheConnection),
      _preferSecure(preferSecure),
      _endpointSelection(endpointSelection),
      _locatorCacheTimeout(locatorCacheTimeout)
{
    assert(_adapterId.empty() || _endpoints.empty());
    setBatchRequestQueue();
}

vector<EndpointIPtr>
IceInternal::RoutableReference::getEndpoints() const
{
    return _endpoints;
}

string
IceInternal::RoutableReference::getAdapterId() const
{
    return _adapterId;
}

LocatorInfoPtr
IceInternal::RoutableReference::getLocatorInfo() const noexcept
{
    return _locatorInfo;
}

RouterInfoPtr
IceInternal::RoutableReference::getRouterInfo() const noexcept
{
    return _routerInfo;
}

bool
IceInternal::RoutableReference::getCollocationOptimized() const noexcept
{
    return _collocationOptimized;
}

bool
IceInternal::RoutableReference::getCacheConnection() const noexcept
{
    return _cacheConnection;
}

bool
IceInternal::RoutableReference::getPreferSecure() const noexcept
{
    return _preferSecure;
}

Ice::EndpointSelectionType
IceInternal::RoutableReference::getEndpointSelection() const noexcept
{
    return _endpointSelection;
}

chrono::milliseconds
IceInternal::RoutableReference::getLocatorCacheTimeout() const noexcept
{
    return _locatorCacheTimeout;
}

string
IceInternal::RoutableReference::getConnectionId() const
{
    return _connectionId;
}

ReferencePtr
IceInternal::RoutableReference::changeMode(Mode newMode) const
{
    ReferencePtr r = Reference::changeMode(newMode);
    static_pointer_cast<RoutableReference>(r)->setBatchRequestQueue();
    return r;
}

ReferencePtr
IceInternal::RoutableReference::changeEncoding(Ice::EncodingVersion encoding) const
{
    ReferencePtr r = Reference::changeEncoding(encoding);
    if (r.get() != const_cast<RoutableReference*>(this))
    {
        LocatorInfoPtr& locInfo = dynamic_pointer_cast<RoutableReference>(r)->_locatorInfo;
        if (locInfo && locInfo->getLocator()->ice_getEncodingVersion() != encoding)
        {
            locInfo = getInstance()->locatorManager()->get(locInfo->getLocator()->ice_encodingVersion(encoding));
        }
    }
    return r;
}

ReferencePtr
IceInternal::RoutableReference::changeCompress(bool newCompress) const
{
    ReferencePtr r = Reference::changeCompress(newCompress);
    // Also override the compress flag on the endpoints if it was updated.
    if (r.get() != const_cast<RoutableReference*>(this) && !_endpoints.empty())
    {
        vector<EndpointIPtr> newEndpoints;
        newEndpoints.reserve(_endpoints.size());
        for (const auto& endpoint : _endpoints)
        {
            newEndpoints.push_back(endpoint->compress(newCompress));
        }
        dynamic_pointer_cast<RoutableReference>(r)->_endpoints = newEndpoints;
    }
    return r;
}

ReferencePtr
IceInternal::RoutableReference::changeEndpoints(vector<EndpointIPtr> newEndpoints) const
{
    RoutableReferencePtr r = dynamic_pointer_cast<RoutableReference>(clone());
    r->_endpoints = std::move(newEndpoints);
    r->applyOverrides(r->_endpoints);
    r->_adapterId.clear();
    return r;
}

ReferencePtr
IceInternal::RoutableReference::changeAdapterId(string newAdapterId) const
{
    RoutableReferencePtr r = dynamic_pointer_cast<RoutableReference>(clone());
    r->_adapterId = std::move(newAdapterId);
    r->_endpoints.clear();
    return r;
}

ReferencePtr
IceInternal::RoutableReference::changeLocator(optional<LocatorPrx> newLocator) const
{
    LocatorInfoPtr newLocatorInfo = newLocator ? getInstance()->locatorManager()->get(newLocator.value()) : nullptr;
    RoutableReferencePtr r = dynamic_pointer_cast<RoutableReference>(clone());
    r->_locatorInfo = std::move(newLocatorInfo);
    return r;
}

ReferencePtr
IceInternal::RoutableReference::changeRouter(optional<RouterPrx> newRouter) const
{
    RouterInfoPtr newRouterInfo = newRouter ? getInstance()->routerManager()->get(newRouter.value()) : nullptr;
    RoutableReferencePtr r = dynamic_pointer_cast<RoutableReference>(clone());
    r->_routerInfo = std::move(newRouterInfo);
    return r;
}

ReferencePtr
IceInternal::RoutableReference::changeCollocationOptimized(bool newCollocationOptimized) const
{
    RoutableReferencePtr r = dynamic_pointer_cast<RoutableReference>(clone());
    r->_collocationOptimized = newCollocationOptimized;
    return r;
}

ReferencePtr
IceInternal::RoutableReference::changeCacheConnection(bool newCache) const
{
    RoutableReferencePtr r = dynamic_pointer_cast<RoutableReference>(clone());
    r->_cacheConnection = newCache;
    return r;
}

ReferencePtr
IceInternal::RoutableReference::changePreferSecure(bool newPreferSecure) const
{
    RoutableReferencePtr r = dynamic_pointer_cast<RoutableReference>(clone());
    r->_preferSecure = newPreferSecure;
    return r;
}

ReferencePtr
IceInternal::RoutableReference::changeEndpointSelection(EndpointSelectionType newType) const
{
    RoutableReferencePtr r = dynamic_pointer_cast<RoutableReference>(clone());
    r->_endpointSelection = newType;
    return r;
}

ReferencePtr
IceInternal::RoutableReference::changeLocatorCacheTimeout(chrono::milliseconds timeout) const
{
    RoutableReferencePtr r = dynamic_pointer_cast<RoutableReference>(clone());
    r->_locatorCacheTimeout = timeout;
    return r;
}

ReferencePtr
IceInternal::RoutableReference::changeConnectionId(string id) const
{
    RoutableReferencePtr r = dynamic_pointer_cast<RoutableReference>(clone());
    r->_connectionId = id;
    if (!_endpoints.empty()) // Also override the connection id on the endpoints.
    {
        vector<EndpointIPtr> newEndpoints;
        newEndpoints.reserve(_endpoints.size());
        for (const auto& endpoint : _endpoints)
        {
            newEndpoints.push_back(endpoint->connectionId(id));
        }
        r->_endpoints = newEndpoints;
    }
    return r;
}

ReferencePtr
IceInternal::RoutableReference::changeConnection(Ice::ConnectionIPtr connection) const
{
    return make_shared<FixedReference>(
        getInstance(),
        getCommunicator(),
        getIdentity(),
        getFacet(),
        getMode(),
        getSecure(),
        getCompress(),
        getProtocol(),
        getEncoding(),
        std::move(connection),
        getInvocationTimeout(),
        getContext()->getValue());
}

bool
IceInternal::RoutableReference::isIndirect() const noexcept
{
    return _endpoints.empty();
}

bool
IceInternal::RoutableReference::isWellKnown() const noexcept
{
    return _endpoints.empty() && _adapterId.empty();
}

void
IceInternal::RoutableReference::streamWrite(OutputStream* s) const
{
    Reference::streamWrite(s);

    auto sz = static_cast<int32_t>(_endpoints.size());
    s->writeSize(sz);
    if (sz)
    {
        assert(_adapterId.empty());
        for (const auto& endpoint : _endpoints)
        {
            s->write(endpoint->type());
            endpoint->streamWrite(s);
        }
    }
    else
    {
        s->write(_adapterId);
    }
}

string
IceInternal::RoutableReference::toString() const
{
    //
    // WARNING: Certain features, such as proxy validation in Glacier2,
    // depend on the format of proxy strings. Changes to toString() and
    // methods called to generate parts of the reference string could break
    // these features. Please review for all features that depend on the
    // format of proxyToString() before changing this and related code.
    //
    string result = Reference::toString();

    if (!_endpoints.empty())
    {
        for (const auto& endpoint : _endpoints)
        {
            string endp = endpoint->toString();
            if (!endp.empty())
            {
                result.append(":");
                result.append(endp);
            }
        }
    }
    else if (!_adapterId.empty())
    {
        result.append(" @ ");

        //
        // If the encoded adapter id string contains characters which the
        // reference parser uses as separators, then we enclose the
        // adapter id string in quotes.
        //
        string a = escapeString(_adapterId, "", getInstance()->toStringMode());
        if (a.find_first_of(" :@") != string::npos)
        {
            result.append("\"");
            result.append(a);
            result.append("\"");
        }
        else
        {
            result.append(a);
        }
    }
    else
    {
        return result;
    }
    return result;
}

PropertyDict
IceInternal::RoutableReference::toProperty(string prefix) const
{
    Ice::PropertyDict properties;

    properties[prefix + ".CollocationOptimized"] = _collocationOptimized ? "1" : "0";
    properties[prefix + ".ConnectionCached"] = _cacheConnection ? "1" : "0";
    properties[prefix + ".PreferSecure"] = _preferSecure ? "1" : "0";
    properties[prefix + ".EndpointSelection"] =
        _endpointSelection == EndpointSelectionType::Random ? "Random" : "Ordered";
    properties[prefix + ".LocatorCacheTimeout"] =
        to_string(chrono::duration_cast<chrono::seconds>(_locatorCacheTimeout).count());
    properties[prefix + ".InvocationTimeout"] = to_string(getInvocationTimeout().count());

    if (_routerInfo)
    {
        PropertyDict routerProperties = _routerInfo->getRouter()->_getReference()->toProperty(prefix + ".Router");
        for (const auto& routerProp : routerProperties)
        {
            properties[routerProp.first] = routerProp.second;
        }
    }

    if (_locatorInfo)
    {
        PropertyDict locatorProperties = _locatorInfo->getLocator()->_getReference()->toProperty(prefix + ".Locator");
        for (const auto& locatorProp : locatorProperties)
        {
            properties[locatorProp.first] = locatorProp.second;
        }
    }

    properties.emplace(std::move(prefix), toString());

    return properties;
}

size_t
IceInternal::RoutableReference::hash() const noexcept
{
    size_t h = Reference::hash();
    hashAdd(h, _adapterId);
    hashAdd(h, _endpoints);
    return h;
}

bool
IceInternal::RoutableReference::operator==(const Reference& r) const noexcept
{
    //
    // Note: if(this == &r) test is performed by each non-abstract derived class.
    //
    if (this == &r)
    {
        return true;
    }

    const auto* rhs = dynamic_cast<const RoutableReference*>(&r);
    if (!rhs || !Reference::operator==(r))
    {
        return false;
    }
    if (_preferSecure != rhs->_preferSecure)
    {
        return false;
    }
    if (_collocationOptimized != rhs->_collocationOptimized)
    {
        return false;
    }
    if (_cacheConnection != rhs->_cacheConnection)
    {
        return false;
    }
    if (_endpointSelection != rhs->_endpointSelection)
    {
        return false;
    }
    if (_connectionId != rhs->_connectionId)
    {
        return false;
    }
    if (!Ice::targetEqualTo(_routerInfo, rhs->_routerInfo))
    {
        return false;
    }
    if (!Ice::targetEqualTo(_locatorInfo, rhs->_locatorInfo))
    {
        return false;
    }

    // Check if the two endpoint lists are equal.
    if (!equal(
            _endpoints.begin(),
            _endpoints.end(),
            rhs->_endpoints.begin(),
            rhs->_endpoints.end(),
            Ice::TargetCompare<shared_ptr<EndpointI>, std::equal_to>()))
    {
        return false;
    }
    if (_adapterId != rhs->_adapterId)
    {
        return false;
    }
    if (_locatorCacheTimeout != rhs->_locatorCacheTimeout)
    {
        return false;
    }
    return true;
}

bool
IceInternal::RoutableReference::operator<(const Reference& r) const noexcept
{
    if (this == &r)
    {
        return false;
    }

    if (Reference::operator<(r))
    {
        return true;
    }
    else if (!Reference::operator==(r))
    {
        return false;
    }

    const auto* rhs = dynamic_cast<const RoutableReference*>(&r);
    if (!rhs)
    {
        assert(dynamic_cast<const FixedReference*>(&r));
        return true; // As a rule, routable references are superior to fixed references.
    }

    if (!_preferSecure && rhs->_preferSecure)
    {
        return true;
    }
    else if (rhs->_preferSecure < _preferSecure)
    {
        return false;
    }
    if (!_collocationOptimized && rhs->_collocationOptimized)
    {
        return true;
    }
    else if (rhs->_collocationOptimized < _collocationOptimized)
    {
        return false;
    }
    if (!_cacheConnection && rhs->_cacheConnection)
    {
        return true;
    }
    else if (rhs->_cacheConnection < _cacheConnection)
    {
        return false;
    }
    if (_endpointSelection < rhs->_endpointSelection)
    {
        return true;
    }
    else if (rhs->_endpointSelection < _endpointSelection)
    {
        return false;
    }
    if (_connectionId < rhs->_connectionId)
    {
        return true;
    }
    else if (rhs->_connectionId < _connectionId)
    {
        return false;
    }
    if (Ice::targetLess(_routerInfo, rhs->_routerInfo))
    {
        return true;
    }
    else if (Ice::targetLess(rhs->_routerInfo, _routerInfo))
    {
        return false;
    }
    if (Ice::targetLess(_locatorInfo, rhs->_locatorInfo))
    {
        return true;
    }
    else if (Ice::targetLess(rhs->_locatorInfo, _locatorInfo))
    {
        return false;
    }
    if (_adapterId < rhs->_adapterId)
    {
        return true;
    }
    else if (rhs->_adapterId < _adapterId)
    {
        return false;
    }
    if (lexicographical_compare(
            _endpoints.begin(),
            _endpoints.end(),
            rhs->_endpoints.begin(),
            rhs->_endpoints.end(),
            Ice::TargetCompare<shared_ptr<EndpointI>, std::less>()))
    {
        return true;
    }
    else if (rhs->_endpoints < _endpoints)
    {
        return false;
    }
    if (_locatorCacheTimeout < rhs->_locatorCacheTimeout)
    {
        return true;
    }
    else if (rhs->_locatorCacheTimeout < _locatorCacheTimeout)
    {
        return false;
    }
    return false;
}

ReferencePtr
IceInternal::RoutableReference::clone() const
{
    return make_shared<RoutableReference>(*this);
}

RequestHandlerPtr
IceInternal::RoutableReference::getRequestHandler() const
{
    auto self = const_cast<RoutableReference*>(this)->shared_from_this();

    if (_collocationOptimized)
    {
        Ice::ObjectAdapterPtr adapter = _instance->objectAdapterFactory()->findObjectAdapter(self);
        if (adapter)
        {
            return make_shared<CollocatedRequestHandler>(self, adapter);
        }
    }

    ConnectRequestHandlerPtr handler = make_shared<ConnectRequestHandler>(self);
    getConnectionAsync(
        [handler](Ice::ConnectionIPtr connection, bool compress)
        { handler->setConnection(std::move(connection), compress); },
        [handler](exception_ptr ex) { handler->setException(ex); });

    return handler; // NOLINT(clang-analyzer-cplusplus.NewDeleteLeaks)
    // Looks like a clang-tidy bug. See https://github.com/llvm/llvm-project/issues/55219
}

const BatchRequestQueuePtr&
IceInternal::RoutableReference::getBatchRequestQueue() const noexcept
{
    return _batchRequestQueue;
}

void
IceInternal::RoutableReference::getConnectionAsync(
    function<void(ConnectionIPtr, bool)> response,
    function<void(std::exception_ptr)> exception) const
{
    if (_routerInfo)
    {
        // If we route, we send everything to the router's client proxy endpoints.
        auto self = dynamic_pointer_cast<RoutableReference>(const_cast<RoutableReference*>(this)->shared_from_this());

        _routerInfo->getClientEndpointsAsync(
            [self = std::move(self), response = std::move(response), exception](vector<EndpointIPtr> endpoints) mutable
            {
                if (endpoints.empty())
                {
                    self->getConnectionNoRouterInfoAsync(std::move(response), std::move(exception));
                }
                else
                {
                    self->applyOverrides(endpoints);
                    self->createConnectionAsync(endpoints, std::move(response), std::move(exception));
                }
            },
            exception);
    }
    else
    {
        getConnectionNoRouterInfoAsync(std::move(response), std::move(exception));
    }
}

void
IceInternal::RoutableReference::getConnectionNoRouterInfoAsync(
    function<void(ConnectionIPtr, bool)> response,
    function<void(std::exception_ptr)> exception) const
{
    class Callback final : public LocatorInfo::GetEndpointsCallback
    {
    public:
        void setEndpoints(const vector<EndpointIPtr>& endpoints, bool cached) final
        {
            if (endpoints.empty())
            {
                _exception(make_exception_ptr(
                    Ice::NoEndpointException(__FILE__, __LINE__, ObjectPrx::_fromReference(_reference))));
                return;
            }

            vector<EndpointIPtr> endpts = endpoints;
            _reference->applyOverrides(endpts);
            _reference->createConnectionAsync(
                endpts,
                _response,
                [reference = _reference, response = _response, exception = _exception, cached](std::exception_ptr exc)
                {
                    try
                    {
                        rethrow_exception(exc);
                    }
                    catch (const Ice::NoEndpointException&)
                    {
                        exception(exc); // No need to retry if there's no endpoints.
                    }
                    catch (const Ice::LocalException& ex)
                    {
                        LocatorInfoPtr locatorInfo = reference->getLocatorInfo();
                        assert(locatorInfo);
                        locatorInfo->clearCache(reference);
                        if (cached)
                        {
                            TraceLevelsPtr traceLvls = reference->getInstance()->traceLevels();
                            if (traceLvls->retry >= 2)
                            {
                                Trace out(reference->getInstance()->initializationData().logger, traceLvls->retryCat);
                                out << "connection to cached endpoints failed\n"
                                    << "removing endpoints from cache and trying again\n"
                                    << ex;
                            }
                            reference->getConnectionNoRouterInfoAsync(response, exception); // Retry.
                            return;
                        }
                        exception(exc);
                    }
                });
        }

        void setException(std::exception_ptr ex) final { _exception(ex); }

        Callback(
            RoutableReferencePtr reference,
            function<void(ConnectionIPtr, bool)> response,
            function<void(std::exception_ptr)> exception)
            : _reference(std::move(reference)),
              _response(std::move(response)),
              _exception(std::move(exception))
        {
        }

    private:
        const RoutableReferencePtr _reference;
        const function<void(ConnectionIPtr, bool)> _response;
        const function<void(std::exception_ptr)> _exception;
    };

    if (!_endpoints.empty())
    {
        createConnectionAsync(_endpoints, std::move(response), std::move(exception));
        return;
    }

    if (_locatorInfo)
    {
        RoutableReferencePtr self =
            dynamic_pointer_cast<RoutableReference>(const_cast<RoutableReference*>(this)->shared_from_this());
        _locatorInfo->getEndpoints(
            self,
            _locatorCacheTimeout,
            make_shared<Callback>(self, std::move(response), std::move(exception)));
    }
    else
    {
        exception(make_exception_ptr(Ice::NoEndpointException(
            __FILE__,
            __LINE__,
            ObjectPrx::_fromReference(const_cast<RoutableReference*>(this)->shared_from_this()))));
    }
}

void
IceInternal::RoutableReference::createConnectionAsync(
    const vector<EndpointIPtr>& allEndpoints,
    function<void(ConnectionIPtr, bool)> response,
    function<void(std::exception_ptr)> exception) const
{
    vector<EndpointIPtr> endpoints = filterEndpoints(allEndpoints);
    if (endpoints.empty())
    {
        exception(make_exception_ptr(Ice::NoEndpointException(
            __FILE__,
            __LINE__,
            ObjectPrx::_fromReference(const_cast<RoutableReference*>(this)->shared_from_this()))));
        return;
    }

    OutgoingConnectionFactoryPtr factory = getInstance()->outgoingConnectionFactory();
    auto self = static_pointer_cast<RoutableReference>(const_cast<RoutableReference*>(this)->shared_from_this());

    auto createConnectionSucceded =
        [routerInfo = _routerInfo, response = std::move(response)](Ice::ConnectionIPtr connection, bool compress)
    {
        // If we have a router, set the object adapter for this router (if any) to the new connection, so that
        // callbacks from the router can be received over this new connection.
        if (routerInfo && routerInfo->getAdapter())
        {
            connection->setAdapter(routerInfo->getAdapter());
        }
        response(std::move(connection), compress);
    };

    if (getCacheConnection() || endpoints.size() == 1)
    {
        // Get an existing connection or create one if there's no existing connection to one of the given endpoints.
        factory->createAsync(std::move(endpoints), false, std::move(createConnectionSucceded), std::move(exception));
    }
    else
    {
        // Go through the list of endpoints and try to create the connection until it succeeds. This is different from
        // just calling create() with the given endpoints since this might create a new connection even if there's an
        // existing connection for one of the endpoints.

        class CreateConnectionState final : public std::enable_shared_from_this<CreateConnectionState>
        {
        public:
            CreateConnectionState(
                vector<EndpointIPtr> endpoints,
                OutgoingConnectionFactoryPtr factory,
                function<void(Ice::ConnectionIPtr, bool)> createConnectionSucceded,
                function<void(exception_ptr)> exception)
                : _endpoints(std::move(endpoints)),
                  _factory(std::move(factory)),
                  _createConnectionSucceded(std::move(createConnectionSucceded)),
                  _createConnectionFailed(std::move(exception))
            {
            }

            void createAsync()
            {
                _factory->createAsync(
                    {_endpoints[_endpointIndex]},
                    true,
                    _createConnectionSucceded,
                    [self = shared_from_this()](exception_ptr e) { self->handleException(e); });
            }

            void handleException(std::exception_ptr ex)
            {
                if (!_exception)
                {
                    _exception = ex;
                }

                if (++_endpointIndex == _endpoints.size())
                {
                    _createConnectionFailed(_exception);
                    return;
                }

                const bool more = _endpointIndex != _endpoints.size() - 1;
                _factory->createAsync(
                    {_endpoints[_endpointIndex]},
                    more,
                    _createConnectionSucceded,
                    [self = shared_from_this()](exception_ptr e) { self->handleException(e); });
            }

        private:
            exception_ptr _exception = nullptr;
            size_t _endpointIndex = 0;
            vector<EndpointIPtr> _endpoints;
            OutgoingConnectionFactoryPtr _factory;
            std::function<void(Ice::ConnectionIPtr, bool)> _createConnectionSucceded;
            std::function<void(exception_ptr)> _createConnectionFailed;
        };

        auto state = make_shared<CreateConnectionState>(
            std::move(endpoints),
            std::move(factory),
            std::move(createConnectionSucceded),
            std::move(exception));
        state->createAsync();
    }
}

void
IceInternal::RoutableReference::applyOverrides(vector<EndpointIPtr>& endpoints) const
{
    for (auto& endpoint : endpoints)
    {
        endpoint = endpoint->connectionId(_connectionId);
        optional<bool> compress = getCompress();
        if (compress.has_value())
        {
            endpoint = endpoint->compress(*compress);
        }
    }
}

IceInternal::RoutableReference::RoutableReference(const RoutableReference& r)
    : Reference(r),
      _endpoints(r._endpoints),
      _adapterId(r._adapterId),
      _locatorInfo(r._locatorInfo),
      _routerInfo(r._routerInfo),
      _collocationOptimized(r._collocationOptimized),
      _cacheConnection(r._cacheConnection),
      _preferSecure(r._preferSecure),
      _endpointSelection(r._endpointSelection),
      _locatorCacheTimeout(r._locatorCacheTimeout),
      _connectionId(r._connectionId)
{
    setBatchRequestQueue();
}

vector<EndpointIPtr>
IceInternal::RoutableReference::filterEndpoints(const vector<EndpointIPtr>& allEndpoints) const
{
    vector<EndpointIPtr> endpoints = allEndpoints;

    // Filter out unknown endpoints.
    endpoints.erase(
        remove_if(
            endpoints.begin(),
            endpoints.end(),
            [](const EndpointIPtr& p) { return dynamic_pointer_cast<OpaqueEndpointI>(p) != nullptr; }),
        endpoints.end());

    // Filter out endpoints according to the mode of the reference.
    switch (getMode())
    {
        case Reference::ModeTwoway:
        case Reference::ModeOneway:
        case Reference::ModeBatchOneway:
        {
            // Filter out datagram endpoints.
            endpoints.erase(
                remove_if(endpoints.begin(), endpoints.end(), [](const EndpointIPtr& p) { return p->datagram(); }),
                endpoints.end());
            break;
        }

        case Reference::ModeDatagram:
        case Reference::ModeBatchDatagram:
        {
            // Filter out non-datagram endpoints.
            endpoints.erase(
                remove_if(endpoints.begin(), endpoints.end(), [](const EndpointIPtr& p) { return !p->datagram(); }),
                endpoints.end());
            break;
        }
    }

    //
    // Sort the endpoints according to the endpoint selection type.
    //
    switch (getEndpointSelection())
    {
        case EndpointSelectionType::Random:
        {
            IceInternal::shuffle(endpoints.begin(), endpoints.end());
            break;
        }
        case EndpointSelectionType::Ordered:
        {
            // Nothing to do.
            break;
        }
        default:
        {
            assert(false);
            break;
        }
    }

    //
    // If a secure connection is requested or secure overrides is set,
    // remove all non-secure endpoints. Otherwise if preferSecure is set
    // make secure endpoints preferred. By default make non-secure
    // endpoints preferred over secure endpoints.
    //
    DefaultsAndOverridesPtr overrides = getInstance()->defaultsAndOverrides();
    if (overrides->overrideSecure.has_value() ? *overrides->overrideSecure : getSecure())
    {
        endpoints.erase(
            remove_if(endpoints.begin(), endpoints.end(), [](const EndpointIPtr& p) { return !p->secure(); }),
            endpoints.end());
    }
    else if (getPreferSecure())
    {
        //
        // We must use stable_partition() instead of just simply
        // partition(), because otherwise some STL implementations
        // order our now randomized endpoints.
        //
        stable_partition(endpoints.begin(), endpoints.end(), [](const EndpointIPtr& p) { return p->secure(); });
    }
    else
    {
        //
        // We must use stable_partition() instead of just simply
        // partition(), because otherwise some STL implementations
        // order our now randomized endpoints.
        //
        stable_partition(endpoints.begin(), endpoints.end(), [](const EndpointIPtr& p) { return !p->secure(); });
    }

    return endpoints;
}

void
IceInternal::RoutableReference::setBatchRequestQueue()
{
    _batchRequestQueue =
        isBatch() ? make_shared<BatchRequestQueue>(getInstance(), getMode() == Reference::ModeBatchDatagram) : nullptr;
}
