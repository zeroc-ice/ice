//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Reference.h>
#include <Ice/ReferenceFactory.h>
#include <Ice/LocalException.h>
#include <Ice/Instance.h>
#include <Ice/EndpointI.h>
#include <Ice/OpaqueEndpointI.h>
#include <Ice/OutputStream.h>
#include <Ice/RouterInfo.h>
#include <Ice/Router.h>
#include <Ice/LocatorInfo.h>
#include <Ice/Locator.h>
#include <Ice/ConnectionI.h>
#include <Ice/ConnectionFactory.h>
#include <Ice/LoggerUtil.h>
#include <Ice/TraceLevels.h>
#include <Ice/HashUtil.h>
#include "CollocatedRequestHandler.h"
#include "ConnectRequestHandler.h"
#include "FixedRequestHandler.h"
#include "ObjectAdapterFactory.h"
#include <Ice/DefaultsAndOverrides.h>
#include <Ice/Comparable.h>
#include <Ice/StringUtil.h>

#include <IceUtil/Random.h>

#include <functional>
#include <algorithm>

using namespace std;
using namespace Ice;
using namespace IceInternal;

CommunicatorPtr
IceInternal::Reference::getCommunicator() const
{
    return _communicator;
}

ReferencePtr
IceInternal::Reference::changeContext(const Context& newContext) const
{
    ReferencePtr r = clone();
    r->_context = make_shared<SharedContext>(newContext);
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
IceInternal::Reference::changeIdentity(const Identity& newIdentity) const
{
    ReferencePtr r = clone();
    r->_identity = newIdentity;
    return r;
}

ReferencePtr
IceInternal::Reference::changeFacet(const string& newFacet) const
{
    ReferencePtr r = clone();
    r->_facet = newFacet;
    return r;
}

ReferencePtr
IceInternal::Reference::changeInvocationTimeout(int invocationTimeout) const
{
    ReferencePtr r = clone();
    r->_invocationTimeout = invocationTimeout;
    return r;
}

ReferencePtr
IceInternal::Reference::changeEncoding(const Ice::EncodingVersion& encoding) const
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
    r->_overrideCompress = true;
    return r;
}

bool
IceInternal::Reference::getCompressOverride(bool& compress) const
{
    DefaultsAndOverridesPtr defaultsAndOverrides = getInstance()->defaultsAndOverrides();
    if(defaultsAndOverrides->overrideCompress)
    {
        compress = defaultsAndOverrides->overrideCompressValue;
    }
    else if(_overrideCompress)
    {
        compress = _compress;
    }
    else
    {
        return false;
    }
    return true;
}

Int
Reference::hash() const
{
    lock_guard lock(_hashMutex);
    if(!_hashInitialized)
    {
        _hashValue = hashInit();
        _hashInitialized = true;
    }
    return _hashValue;
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
    if(_facet.empty())
    {
        s->write(static_cast<string*>(0), static_cast<string*>(0));
    }
    else
    {
        s->write(&_facet, &_facet + 1);
    }

    s->write(static_cast<Byte>(_mode));

    s->write(_secure);

    if(s->getEncoding() != Ice::Encoding_1_0)
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

    if(id.find_first_of(separators) != string::npos)
    {
        s << '"' << id << '"';
    }
    else
    {
        s << id;
    }

    if(!_facet.empty())
    {
        s << " -f ";

        string fs = escapeString(_facet, "", toStringMode);
        //
        // If the encoded facet string contains characters which
        // the reference parser uses as separators, then we enclose
        // the facet string in quotes.
        //
        if(fs.find_first_of(separators) != string::npos)
        {
            s << '"' << fs << '"';
        }
        else
        {
            s << fs;
        }
    }

    switch(_mode)
    {
        case ModeTwoway:
        {
            s << " -t";
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

    if(_secure)
    {
        s << " -s";
    }

    if(_protocol != Ice::Protocol_1_0)
    {
        //
        // We only print the protocol if it's not 1.0. It's fine as
        // long as we don't add Ice.Default.ProtocolVersion, a
        // stringified proxy will convert back to the same proxy with
        // stringToProxy.
        //
        s << " -p " << _protocol;
    }

    //
    // Always print the encoding version to ensure a stringified proxy
    // will convert back to a proxy with the same encoding with
    // stringToProxy (and won't use Ice.Default.EncodingVersion).
    //
    s << " -e " << _encoding;

    return s.str();

    // Derived class writes the remainder of the string.
}

bool
IceInternal::Reference::operator==(const Reference& r) const
{
    //
    // Note: if(this == &r) test is performed by each non-abstract derived class.
    //

    if(_mode != r._mode)
    {
        return false;
    }

    if(_secure != r._secure)
    {
        return false;
    }

    if(_identity != r._identity)
    {
        return false;
    }

    if(_context->getValue() != r._context->getValue())
    {
        return false;
    }

    if(_facet != r._facet)
    {
        return false;
    }

    if((_overrideCompress != r._overrideCompress) || (_overrideCompress && _compress != r._compress))
    {
        return false;
    }

    if(_protocol != r._protocol)
    {
        return false;
    }

    if(_encoding != r._encoding)
    {
        return false;
    }

    if(_invocationTimeout != r._invocationTimeout)
    {
        return false;
    }
    return true;
}

bool
IceInternal::Reference::operator<(const Reference& r) const
{
    //
    // Note: if(this == &r) test is performed by each non-abstract derived class.
    //

    if(_mode < r._mode)
    {
        return true;
    }
    else if(r._mode < _mode)
    {
        return false;
    }

    if(_identity < r._identity)
    {
        return true;
    }
    else if(r._identity < _identity)
    {
        return false;
    }

    if(_context->getValue() < r._context->getValue())
    {
        return true;
    }
    else if(r._context->getValue() < _context->getValue())
    {
        return false;
    }

    if(_facet < r._facet)
    {
        return true;
    }
    else if(r._facet < _facet)
    {
        return false;
    }

    if(!_overrideCompress && r._overrideCompress)
    {
        return true;
    }
    else if(r._overrideCompress < _overrideCompress)
    {
        return false;
    }
    else if(_overrideCompress)
    {
        if(!_compress && r._compress)
        {
            return true;
        }
        else if(r._compress < _compress)
        {
            return false;
        }
    }

    if(!_secure && r._secure)
    {
        return true;
    }
    else if(r._secure < _secure)
    {
        return false;
    }

    if(_protocol < r._protocol)
    {
        return true;
    }
    else if(r._protocol < _protocol)
    {
        return false;
    }

    if(_encoding < r._encoding)
    {
        return true;
    }
    else if(r._encoding < _encoding)
    {
        return false;
    }

    if(_invocationTimeout < r._invocationTimeout)
    {
        return true;
    }
    else if(r._invocationTimeout < _invocationTimeout)
    {
        return false;
    }

    return false;
}

IceInternal::Reference::Reference(const InstancePtr& instance,
                                  const CommunicatorPtr& communicator,
                                  const Identity& id,
                                  const string& facet,
                                  Mode mode,
                                  bool secure,
                                  const ProtocolVersion& protocol,
                                  const EncodingVersion& encoding,
                                  int invocationTimeout,
                                  const Ice::Context& ctx) :
    _instance(instance),
    _overrideCompress(false),
    _compress(false),
    _communicator(communicator),
    _mode(mode),
    _secure(secure),
    _identity(id),
    _context(make_shared<SharedContext>(ctx)),
    _facet(facet),
    _hashInitialized(false),
    _protocol(protocol),
    _encoding(encoding),
    _invocationTimeout(invocationTimeout)
{
}

IceInternal::Reference::Reference(const Reference& r) :
    enable_shared_from_this<Reference>(),
    _instance(r._instance),
    _overrideCompress(r._overrideCompress),
    _compress(r._compress),
    _communicator(r._communicator),
    _mode(r._mode),
    _secure(r._secure),
    _identity(r._identity),
    _context(r._context),
    _facet(r._facet),
    _hashInitialized(false),
    _protocol(r._protocol),
    _encoding(r._encoding),
    _invocationTimeout(r._invocationTimeout)
{
}

int
IceInternal::Reference::hashInit() const
{
    Int h = 5381;
    hashAdd(h, static_cast<Int>(_mode));
    hashAdd(h, _secure);
    hashAdd(h, _identity.name);
    hashAdd(h, _identity.category);
    hashAdd(h, _context->getValue());
    hashAdd(h, _facet);
    hashAdd(h, _overrideCompress);
    if(_overrideCompress)
    {
        hashAdd(h, _compress);
    }
    hashAdd(h, _protocol.major);
    hashAdd(h, _protocol.minor);
    hashAdd(h, _encoding.major);
    hashAdd(h, _encoding.minor);
    hashAdd(h, _invocationTimeout);
    return h;
}

IceInternal::FixedReference::FixedReference(const InstancePtr& instance,
                                            const CommunicatorPtr& communicator,
                                            const Identity& id,
                                            const string& facet,
                                            Mode mode,
                                            bool secure,
                                            const ProtocolVersion& protocol,
                                            const EncodingVersion& encoding,
                                            const ConnectionIPtr& fixedConnection,
                                            int invocationTimeout,
                                            const Ice::Context& context,
                                            const optional<bool>& compress) :
    Reference(instance, communicator, id, facet, mode, secure, protocol, encoding, invocationTimeout, context),
    _fixedConnection(fixedConnection)
{
    if(compress)
    {
        _overrideCompress = true;
        _compress = *compress;
    }
}

vector<EndpointIPtr>
IceInternal::FixedReference::getEndpoints() const
{
    return vector<EndpointIPtr>();
}

string
IceInternal::FixedReference::getAdapterId() const
{
    return string();
}

bool
IceInternal::FixedReference::getCollocationOptimized() const
{
    return false;
}

bool
IceInternal::FixedReference::getCacheConnection() const
{
    return true;
}

bool
IceInternal::FixedReference::getPreferSecure() const
{
    return false;
}

Ice::EndpointSelectionType
IceInternal::FixedReference::getEndpointSelection() const
{
    return EndpointSelectionType::Random;
}

int
IceInternal::FixedReference::getLocatorCacheTimeout() const
{
    return 0;
}

string
IceInternal::FixedReference::getConnectionId() const
{
    return string();
}

optional<int>
IceInternal::FixedReference::getTimeout() const
{
    return optional<int>();
}

ReferencePtr
IceInternal::FixedReference::changeEndpoints(const vector<EndpointIPtr>& /*newEndpoints*/) const
{
    throw FixedProxyException(__FILE__, __LINE__);
}

ReferencePtr
IceInternal::FixedReference::changeAdapterId(const string& /*newAdapterId*/) const
{
    throw FixedProxyException(__FILE__, __LINE__);
}

ReferencePtr
IceInternal::FixedReference::changeLocator(const LocatorPrxPtr&) const
{
    throw FixedProxyException(__FILE__, __LINE__);
}

ReferencePtr
IceInternal::FixedReference::changeRouter(const RouterPrxPtr&) const
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
IceInternal::FixedReference::changeLocatorCacheTimeout(int) const
{
    throw FixedProxyException(__FILE__, __LINE__);
}

ReferencePtr
IceInternal::FixedReference::changeTimeout(int) const
{
    throw FixedProxyException(__FILE__, __LINE__);
}

ReferencePtr
IceInternal::FixedReference::changeConnectionId(const string&) const
{
    throw FixedProxyException(__FILE__, __LINE__);
}

ReferencePtr
IceInternal::FixedReference::changeConnection(const Ice::ConnectionIPtr& newConnection) const
{
    FixedReferencePtr r = dynamic_pointer_cast<FixedReference>(clone());
    r->_fixedConnection = newConnection;
    return r;
}

bool
IceInternal::FixedReference::isIndirect() const
{
    return false;
}

bool
IceInternal::FixedReference::isWellKnown() const
{
    return false;
}

void
IceInternal::FixedReference::streamWrite(OutputStream*) const
{
    throw FixedProxyException(__FILE__, __LINE__);
}

PropertyDict
IceInternal::FixedReference::toProperty(const string&) const
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
            if(_fixedConnection->endpoint()->datagram())
            {
                throw NoEndpointException(__FILE__, __LINE__, toString());
            }
            break;
        }

        case Reference::ModeDatagram:
        case Reference::ModeBatchDatagram:
        {
            if(!_fixedConnection->endpoint()->datagram())
            {
                throw NoEndpointException(__FILE__, __LINE__, toString());
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
    if(defaultsAndOverrides->overrideSecure)
    {
        secure = defaultsAndOverrides->overrideSecureValue;
    }
    else
    {
        secure = getSecure();
    }
    if(secure && !_fixedConnection->endpoint()->secure())
    {
        throw NoEndpointException(__FILE__, __LINE__, toString());
    }

    _fixedConnection->throwException(); // Throw in case our connection is already destroyed.

    bool compress = false;
    if(defaultsAndOverrides->overrideCompress)
    {
        compress = defaultsAndOverrides->overrideCompressValue;
    }
    else if(_overrideCompress)
    {
        compress = _compress;
    }

    ReferencePtr ref = const_cast<FixedReference*>(this)->shared_from_this();
    return make_shared<FixedRequestHandler>(ref, _fixedConnection, compress);
}

BatchRequestQueuePtr
IceInternal::FixedReference::getBatchRequestQueue() const
{
    return _fixedConnection->getBatchRequestQueue();
}

bool
IceInternal::FixedReference::operator==(const Reference& r) const
{
    if(this == &r)
    {
        return true;
    }
    const FixedReference* rhs = dynamic_cast<const FixedReference*>(&r);
    if(!rhs || !Reference::operator==(r))
    {
        return false;
    }
    return _fixedConnection == rhs->_fixedConnection;
}

bool
IceInternal::FixedReference::operator<(const Reference& r) const
{
    if(this == &r)
    {
        return false;
    }
    if(Reference::operator<(r))
    {
        return true;
    }
    if(!Reference::operator==(r))
    {
        return false;
    }

    const FixedReference* rhs = dynamic_cast<const FixedReference*>(&r);
    if(!rhs)
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

IceInternal::FixedReference::FixedReference(const FixedReference& r) :
    Reference(r),
    _fixedConnection(r._fixedConnection)
{
}

IceInternal::RoutableReference::RoutableReference(const InstancePtr& instance,
                                                  const CommunicatorPtr& communicator,
                                                  const Identity& id,
                                                  const string& facet,
                                                  Mode mode,
                                                  bool secure,
                                                  const ProtocolVersion& protocol,
                                                  const EncodingVersion& encoding,
                                                  const vector<EndpointIPtr>& endpoints,
                                                  const string& adapterId,
                                                  const LocatorInfoPtr& locatorInfo,
                                                  const RouterInfoPtr& routerInfo,
                                                  bool collocationOptimized,
                                                  bool cacheConnection,
                                                  bool preferSecure,
                                                  EndpointSelectionType endpointSelection,
                                                  int locatorCacheTimeout,
                                                  int invocationTimeout,
                                                  const Ice::Context& ctx) :
    Reference(instance, communicator, id, facet, mode, secure, protocol, encoding, invocationTimeout, ctx),
    _endpoints(endpoints),
    _adapterId(adapterId),
    _locatorInfo(locatorInfo),
    _routerInfo(routerInfo),
    _collocationOptimized(collocationOptimized),
    _cacheConnection(cacheConnection),
    _preferSecure(preferSecure),
    _endpointSelection(endpointSelection),
    _locatorCacheTimeout(locatorCacheTimeout),
    _overrideTimeout(false),
    _timeout(-1)
{
    assert(_adapterId.empty() || _endpoints.empty());
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
IceInternal::RoutableReference::getLocatorInfo() const
{
    return _locatorInfo;
}

RouterInfoPtr
IceInternal::RoutableReference::getRouterInfo() const
{
    return _routerInfo;
}

bool
IceInternal::RoutableReference::getCollocationOptimized() const
{
    return _collocationOptimized;
}

bool
IceInternal::RoutableReference::getCacheConnection() const
{
    return _cacheConnection;
}

bool
IceInternal::RoutableReference::getPreferSecure() const
{
    return _preferSecure;
}

Ice::EndpointSelectionType
IceInternal::RoutableReference::getEndpointSelection() const
{
    return _endpointSelection;
}

int
IceInternal::RoutableReference::getLocatorCacheTimeout() const
{
    return _locatorCacheTimeout;
}

string
IceInternal::RoutableReference::getConnectionId() const
{
    return _connectionId;
}

optional<int>
IceInternal::RoutableReference::getTimeout() const
{
    return _overrideTimeout ? optional<int>(_timeout) : nullopt;
}

ReferencePtr
IceInternal::RoutableReference::changeEncoding(const Ice::EncodingVersion& encoding) const
{
    ReferencePtr r = Reference::changeEncoding(encoding);
    if(r.get() != const_cast<RoutableReference*>(this))
    {
        LocatorInfoPtr& locInfo = dynamic_pointer_cast<RoutableReference>(r)->_locatorInfo;
        if(locInfo && locInfo->getLocator()->ice_getEncodingVersion() != encoding)
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
    if(r.get() != const_cast<RoutableReference*>(this) && !_endpoints.empty())
    {
        vector<EndpointIPtr> newEndpoints;
        for(vector<EndpointIPtr>::const_iterator p = _endpoints.begin(); p != _endpoints.end(); ++p)
        {
            newEndpoints.push_back((*p)->compress(newCompress));
        }
        dynamic_pointer_cast<RoutableReference>(r)->_endpoints = newEndpoints;
    }
    return r;
}

ReferencePtr
IceInternal::RoutableReference::changeEndpoints(const vector<EndpointIPtr>& newEndpoints) const
{
    RoutableReferencePtr r = dynamic_pointer_cast<RoutableReference>(clone());
    r->_endpoints = newEndpoints;
    r->applyOverrides(r->_endpoints);
    r->_adapterId.clear();
    return r;
}

ReferencePtr
IceInternal::RoutableReference::changeAdapterId(const string& newAdapterId) const
{
    RoutableReferencePtr r = dynamic_pointer_cast<RoutableReference>(clone());
    r->_adapterId = newAdapterId;
    r->_endpoints.clear();
    return r;
}

ReferencePtr
IceInternal::RoutableReference::changeLocator(const LocatorPrxPtr& newLocator) const
{
    LocatorInfoPtr newLocatorInfo = getInstance()->locatorManager()->get(newLocator);
    RoutableReferencePtr r = dynamic_pointer_cast<RoutableReference>(clone());
    r->_locatorInfo = newLocatorInfo;
    return r;
}

ReferencePtr
IceInternal::RoutableReference::changeRouter(const RouterPrxPtr& newRouter) const
{
    RouterInfoPtr newRouterInfo = getInstance()->routerManager()->get(newRouter);
    RoutableReferencePtr r = dynamic_pointer_cast<RoutableReference>(clone());
    r->_routerInfo = newRouterInfo;
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
IceInternal::RoutableReference::changeLocatorCacheTimeout(int timeout) const
{
    RoutableReferencePtr r = dynamic_pointer_cast<RoutableReference>(clone());
    r->_locatorCacheTimeout = timeout;
    return r;
}

ReferencePtr
IceInternal::RoutableReference::changeTimeout(int newTimeout) const
{
    RoutableReferencePtr r = dynamic_pointer_cast<RoutableReference>(clone());
    r->_timeout = newTimeout;
    r->_overrideTimeout = true;
    if(!_endpoints.empty()) // Also override the timeout on the endpoints.
    {
        vector<EndpointIPtr> newEndpoints;
        for(vector<EndpointIPtr>::const_iterator p = _endpoints.begin(); p != _endpoints.end(); ++p)
        {
            newEndpoints.push_back((*p)->timeout(newTimeout));
        }
        r->_endpoints = newEndpoints;
    }
    return r;
}

ReferencePtr
IceInternal::RoutableReference::changeConnectionId(const string& id) const
{
    RoutableReferencePtr r = dynamic_pointer_cast<RoutableReference>(clone());
    r->_connectionId = id;
    if(!_endpoints.empty()) // Also override the connection id on the endpoints.
    {
        vector<EndpointIPtr> newEndpoints;
        for(vector<EndpointIPtr>::const_iterator p = _endpoints.begin(); p != _endpoints.end(); ++p)
        {
            newEndpoints.push_back((*p)->connectionId(id));
        }
        r->_endpoints = newEndpoints;
    }
    return r;
}

ReferencePtr
IceInternal::RoutableReference::changeConnection(const Ice::ConnectionIPtr& connection) const
{
    return make_shared<FixedReference>(
                              getInstance(),
                              getCommunicator(),
                              getIdentity(),
                              getFacet(),
                              getMode(),
                              getSecure(),
                              getProtocol(),
                              getEncoding(),
                              connection,
                              getInvocationTimeout(),
                              getContext()->getValue(),
                              getCompress());
}

bool
IceInternal::RoutableReference::isIndirect() const
{
    return _endpoints.empty();
}

bool
IceInternal::RoutableReference::isWellKnown() const
{
    return _endpoints.empty() && _adapterId.empty();
}

void
IceInternal::RoutableReference::streamWrite(OutputStream* s) const
{
    Reference::streamWrite(s);

    Int sz = static_cast<Int>(_endpoints.size());
    s->writeSize(sz);
    if(sz)
    {
        assert(_adapterId.empty());
        for(vector<EndpointIPtr>::const_iterator p = _endpoints.begin(); p != _endpoints.end(); ++p)
        {
            s->write((*p)->type());
            (*p)->streamWrite(s);
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

    if(!_endpoints.empty())
    {
        for(vector<EndpointIPtr>::const_iterator p = _endpoints.begin(); p != _endpoints.end(); ++p)
        {
            string endp = (*p)->toString();
            if(!endp.empty())
            {
                result.append(":");
                result.append(endp);
            }
        }
    }
    else if(!_adapterId.empty())
    {
        result.append(" @ ");

        //
        // If the encoded adapter id string contains characters which the
        // reference parser uses as separators, then we enclose the
        // adapter id string in quotes.
        //
        string a = escapeString(_adapterId, "", getInstance()->toStringMode());
        if(a.find_first_of(" :@") != string::npos)
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
IceInternal::RoutableReference::toProperty(const string& prefix) const
{
    Ice::PropertyDict properties;

    properties[prefix] = toString();
    properties[prefix + ".CollocationOptimized"] = _collocationOptimized ? "1" : "0";
    properties[prefix + ".ConnectionCached"] = _cacheConnection ? "1" : "0";
    properties[prefix + ".PreferSecure"] = _preferSecure ? "1" : "0";
    properties[prefix + ".EndpointSelection"] = _endpointSelection == EndpointSelectionType::Random ? "Random" : "Ordered";
    {
        ostringstream s;
        s << _locatorCacheTimeout;
        properties[prefix + ".LocatorCacheTimeout"] = s.str();
    }
    {
        ostringstream s;
        s << getInvocationTimeout();
        properties[prefix + ".InvocationTimeout"] = s.str();
    }
    if(_routerInfo)
    {
        PropertyDict routerProperties = _routerInfo->getRouter()->_getReference()->toProperty(prefix + ".Router");
        for(PropertyDict::const_iterator p = routerProperties.begin(); p != routerProperties.end(); ++p)
        {
            properties[p->first] = p->second;
        }
    }

    if(_locatorInfo)
    {
        PropertyDict locatorProperties = _locatorInfo->getLocator()->_getReference()->toProperty(prefix + ".Locator");
        for(PropertyDict::const_iterator p = locatorProperties.begin(); p != locatorProperties.end(); ++p)
        {
            properties[p->first] = p->second;
        }
    }

    return properties;
}

int
IceInternal::RoutableReference::hashInit() const
{
    int value = Reference::hashInit();
    hashAdd(value, _adapterId);
    return value;
}

bool
IceInternal::RoutableReference::operator==(const Reference& r) const
{
    //
    // Note: if(this == &r) test is performed by each non-abstract derived class.
    //
    if(this == &r)
    {
        return true;
    }

    const RoutableReference* rhs = dynamic_cast<const RoutableReference*>(&r);
    if(!rhs || !Reference::operator==(r))
    {
        return false;
    }
    if(_preferSecure != rhs->_preferSecure)
    {
        return false;
    }
    if(_collocationOptimized != rhs->_collocationOptimized)
    {
        return false;
    }
    if(_cacheConnection != rhs->_cacheConnection)
    {
        return false;
    }
    if(_endpointSelection != rhs->_endpointSelection)
    {
        return false;
    }
    if(_connectionId != rhs->_connectionId)
    {
        return false;
    }
    if((_overrideTimeout != rhs->_overrideTimeout) || (_overrideTimeout && _timeout != rhs->_timeout))
    {
        return false;
    }
    if(!Ice::targetEqualTo(_routerInfo, rhs->_routerInfo))
    {
        return false;
    }
    if(!Ice::targetEqualTo(_locatorInfo, rhs->_locatorInfo))
    {
        return false;
    }

    //
    // TODO: With C++14 we could use the version that receives four iterators and we don't need to explicitly
    // check the sizes are equal.
    //
    if(_endpoints.size() != rhs->_endpoints.size() ||
       !equal(_endpoints.begin(), _endpoints.end(), rhs->_endpoints.begin(), Ice::TargetCompare<shared_ptr<EndpointI>, std::equal_to>()))
    {
        return false;
    }
    if(_adapterId != rhs->_adapterId)
    {
        return false;
    }
    if(_locatorCacheTimeout != rhs->_locatorCacheTimeout)
    {
        return false;
    }
    return true;
}

bool
IceInternal::RoutableReference::operator<(const Reference& r) const
{
    if(this == &r)
    {
        return false;
    }

    if(Reference::operator<(r))
    {
        return true;
    }
    else if(!Reference::operator==(r))
    {
        return false;
    }

    const RoutableReference* rhs = dynamic_cast<const RoutableReference*>(&r);
    if(!rhs)
    {
        assert(dynamic_cast<const FixedReference*>(&r));
        return true; // As a rule, routable references are superior to fixed references.
    }

    if(!_preferSecure && rhs->_preferSecure)
    {
        return true;
    }
    else if(rhs->_preferSecure < _preferSecure)
    {
        return false;
    }
    if(!_collocationOptimized && rhs->_collocationOptimized)
    {
        return true;
    }
    else if(rhs->_collocationOptimized < _collocationOptimized)
    {
        return false;
    }
    if(!_cacheConnection && rhs->_cacheConnection)
    {
        return true;
    }
    else if(rhs->_cacheConnection < _cacheConnection)
    {
        return false;
    }
    if(_endpointSelection < rhs->_endpointSelection)
    {
        return true;
    }
    else if(rhs->_endpointSelection < _endpointSelection)
    {
        return false;
    }
    if(_connectionId < rhs->_connectionId)
    {
        return true;
    }
    else if(rhs->_connectionId < _connectionId)
    {
        return false;
    }
    if(!_overrideTimeout && rhs->_overrideTimeout)
    {
        return true;
    }
    else if(rhs->_overrideTimeout < _overrideTimeout)
    {
        return false;
    }
    else if(_overrideTimeout)
    {
        if(_timeout < rhs->_timeout)
        {
            return true;
        }
        else if(rhs->_timeout < _timeout)
        {
            return false;
        }
    }
    if(Ice::targetLess(_routerInfo, rhs->_routerInfo))
    {
        return true;
    }
    else if(Ice::targetLess(rhs->_routerInfo, _routerInfo))
    {
        return false;
    }
    if(Ice::targetLess(_locatorInfo, rhs->_locatorInfo))
    {
        return true;
    }
    else if(Ice::targetLess(rhs->_locatorInfo, _locatorInfo))
    {
        return false;
    }
    if(_adapterId < rhs->_adapterId)
    {
        return true;
    }
    else if(rhs->_adapterId < _adapterId)
    {
        return false;
    }
    if(lexicographical_compare(_endpoints.begin(), _endpoints.end(), rhs->_endpoints.begin(), rhs->_endpoints.end(),
                               Ice::TargetCompare<shared_ptr<EndpointI>, std::less>()))
    {
        return true;
    }
    else if(rhs->_endpoints < _endpoints)
    {
        return false;
    }
    if(_locatorCacheTimeout < rhs->_locatorCacheTimeout)
    {
        return true;
    }
    else if(rhs->_locatorCacheTimeout < _locatorCacheTimeout)
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
        if(adapter)
        {
            return make_shared<CollocatedRequestHandler>(self, adapter);
        }
    }

    ConnectRequestHandlerPtr handler = make_shared<ConnectRequestHandler>(self);
    getConnectionAsync(
        [handler](Ice::ConnectionIPtr connection, bool compress)
        {
            handler->setConnection(connection, compress);
        },
        [handler](exception_ptr ex)
        {
            handler->setException(ex);
        });

    return handler;
}

BatchRequestQueuePtr
IceInternal::RoutableReference::getBatchRequestQueue() const
{
    return make_shared<BatchRequestQueue>(getInstance(), getMode() == Reference::ModeBatchDatagram);
}

void
IceInternal::RoutableReference::getConnectionAsync(
    function<void(ConnectionIPtr, bool)> response,
    function<void(std::exception_ptr)> exception) const
{
    if(_routerInfo)
    {
        // If we route, we send everything to the router's client proxy endpoints.
        auto self = dynamic_pointer_cast<RoutableReference>(const_cast<RoutableReference*>(this)->shared_from_this());

        _routerInfo->getClientEndpointsAsync(
            [self, response, exception](vector<EndpointIPtr> endpoints)
            {
                if (endpoints.empty())
                {
                    self->getConnectionNoRouterInfoAsync(response, exception);
                }
                else
                {
                    self->applyOverrides(endpoints);
                    self->createConnectionAsync(endpoints, response, exception);
                }
            },
            exception);
        return;
    }

    getConnectionNoRouterInfoAsync(response, exception);
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
                _exception(make_exception_ptr(Ice::NoEndpointException(__FILE__, __LINE__, _reference->toString())));
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
                            if(traceLvls->retry >= 2)
                            {
                                Trace out(reference->getInstance()->initializationData().logger, traceLvls->retryCat);
                                out << "connection to cached endpoints failed\n"
                                    << "removing endpoints from cache and trying again\n" << ex;
                            }
                            reference->getConnectionNoRouterInfoAsync(response, exception); // Retry.
                            return;
                        }
                        exception(exc);
                    }
                });
        }

        void setException(std::exception_ptr ex) final
        {
            _exception(ex);
        }

        Callback(
            const RoutableReferencePtr& reference,
            function<void(ConnectionIPtr, bool)> response,
            function<void(std::exception_ptr)> exception) :
            _reference(reference), _response(std::move(response)), _exception(std::move(exception))
        {
        }

    private:

        const RoutableReferencePtr _reference;
        const function<void(ConnectionIPtr, bool)> _response;
        const function<void(std::exception_ptr)> _exception;
    };

    if(!_endpoints.empty())
    {
        createConnectionAsync(_endpoints, std::move(response), std::move(exception));
        return;
    }

    if(_locatorInfo)
    {
        RoutableReferencePtr self = dynamic_pointer_cast<RoutableReference>(
            const_cast<RoutableReference*>(this)->shared_from_this());
        _locatorInfo->getEndpoints(
            self,
            _locatorCacheTimeout,
            make_shared<Callback>(self, std::move(response), std::move(exception)));
    }
    else
    {
        exception(make_exception_ptr(Ice::NoEndpointException(__FILE__, __LINE__, toString())));
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
        exception(make_exception_ptr(Ice::NoEndpointException(__FILE__, __LINE__, toString())));
        return;
    }

    //
    // Finally, create the connection.
    //
    OutgoingConnectionFactoryPtr factory = getInstance()->outgoingConnectionFactory();
    if(getCacheConnection() || endpoints.size() == 1)
    {
        class CB1 final : public OutgoingConnectionFactory::CreateConnectionCallback
        {
        public:

            void setConnection(const Ice::ConnectionIPtr& connection, bool compress) final
            {
                //
                // If we have a router, set the object adapter for this router
                // (if any) to the new connection, so that callbacks from the
                // router can be received over this new connection.
                //
                if(_routerInfo && _routerInfo->getAdapter())
                {
                    connection->setAdapter(_routerInfo->getAdapter());
                }
                _response(connection, compress);
            }

            void setException(std::exception_ptr ex) final
            {
                _exception(ex);
            }

            CB1(
                const RouterInfoPtr& routerInfo,
                function<void(ConnectionIPtr, bool)> response,
                function<void(std::exception_ptr)> exception) :
                _routerInfo(routerInfo), _response(std::move(response)), _exception(std::move(exception))
            {
            }

        private:

            const RouterInfoPtr _routerInfo;
            const function<void(ConnectionIPtr, bool)> _response;
            const function<void(std::exception_ptr)> _exception;
        };

        //
        // Get an existing connection or create one if there's no
        // existing connection to one of the given endpoints.
        //
        factory->create(endpoints, false, getEndpointSelection(), make_shared<CB1>(_routerInfo, response, exception));
        return;
    }
    else
    {
        class CB2 final :
            public OutgoingConnectionFactory::CreateConnectionCallback,
            public std::enable_shared_from_this<CB2>
        {
        public:

            void setConnection(const Ice::ConnectionIPtr& connection, bool compress) final
            {
                //
                // If we have a router, set the object adapter for this router
                // (if any) to the new connection, so that callbacks from the
                // router can be received over this new connection.
                //
                if(_reference->getRouterInfo() && _reference->getRouterInfo()->getAdapter())
                {
                    connection->setAdapter(_reference->getRouterInfo()->getAdapter());
                }
                _response(connection, compress);
            }

            void setException(std::exception_ptr ex) final
            {
                if(!_exceptionPtr)
                {
                    _exceptionPtr = ex;
                }

                if(++_i == _endpoints.size())
                {
                    _exception(_exceptionPtr);
                    return;
                }

                const bool more = _i != _endpoints.size() - 1;
                vector<EndpointIPtr> endpoint;
                endpoint.push_back(_endpoints[_i]);

                OutgoingConnectionFactoryPtr factory = _reference->getInstance()->outgoingConnectionFactory();
                factory->create(endpoint, more, _reference->getEndpointSelection(), shared_from_this());
            }

            CB2(
                const RoutableReferencePtr& reference,
                const vector<EndpointIPtr>& endpoints,
                function<void(ConnectionIPtr, bool)> response,
                function<void(std::exception_ptr)> exception) :
                _reference(reference),
                _endpoints(endpoints),
                _response(std::move(response)),
                _exception(std::move(exception)),
                _i(0)
            {
            }

        private:

            const RoutableReferencePtr _reference;
            const vector<EndpointIPtr> _endpoints;
            const function<void(ConnectionIPtr, bool)> _response;
            const function<void(std::exception_ptr)> _exception;
            size_t _i;
            exception_ptr _exceptionPtr;
        };

        //
        // Go through the list of endpoints and try to create the
        // connection until it succeeds. This is different from just
        // calling create() with the given endpoints since this might
        // create a new connection even if there's an existing
        // connection for one of the endpoints.
        //

        vector<EndpointIPtr> endpt;
        endpt.push_back(endpoints[0]);
        RoutableReferencePtr self = dynamic_pointer_cast<RoutableReference>(
            const_cast<RoutableReference*>(this)->shared_from_this());
        factory->create(endpt, true, getEndpointSelection(), make_shared<CB2>(self, endpoints, response, exception));
        return;
    }
}

void
IceInternal::RoutableReference::applyOverrides(vector<EndpointIPtr>& endpoints) const
{
    for(vector<EndpointIPtr>::iterator p = endpoints.begin(); p != endpoints.end(); ++p)
    {
        *p = (*p)->connectionId(_connectionId);
        if(_overrideCompress)
        {
            *p = (*p)->compress(_compress);
        }
        if(_overrideTimeout)
        {
            *p = (*p)->timeout(_timeout);
        }
    }
}

IceInternal::RoutableReference::RoutableReference(const RoutableReference& r) :
    Reference(r),
    _endpoints(r._endpoints),
    _adapterId(r._adapterId),
    _locatorInfo(r._locatorInfo),
    _routerInfo(r._routerInfo),
    _collocationOptimized(r._collocationOptimized),
    _cacheConnection(r._cacheConnection),
    _preferSecure(r._preferSecure),
    _endpointSelection(r._endpointSelection),
    _locatorCacheTimeout(r._locatorCacheTimeout),
    _overrideTimeout(r._overrideTimeout),
    _timeout(r._timeout),
    _connectionId(r._connectionId)
{
}

vector<EndpointIPtr>
IceInternal::RoutableReference::filterEndpoints(const vector<EndpointIPtr>& allEndpoints) const
{
    vector<EndpointIPtr> endpoints = allEndpoints;

    // Filter out unknown endpoints.
    endpoints.erase(remove_if(endpoints.begin(), endpoints.end(),
                              [](const EndpointIPtr& p)
                              {
                                  return dynamic_cast<OpaqueEndpointI*>(p.get()) != 0;
                              }),
                    endpoints.end());

    // Filter out endpoints according to the mode of the reference.
    switch(getMode())
    {
        case Reference::ModeTwoway:
        case Reference::ModeOneway:
        case Reference::ModeBatchOneway:
        {
            // Filter out datagram endpoints.
            endpoints.erase(remove_if(endpoints.begin(), endpoints.end(),
                                      [](const EndpointIPtr& p)
                                      {
                                          return p->datagram();
                                      }),
                            endpoints.end());
            break;
        }

        case Reference::ModeDatagram:
        case Reference::ModeBatchDatagram:
        {
            // Filter out non-datagram endpoints.
            endpoints.erase(remove_if(endpoints.begin(), endpoints.end(),
                                      [](const EndpointIPtr& p)
                                      {
                                          return !p->datagram();
                                      }),
                            endpoints.end());
            break;
        }
    }

    //
    // Sort the endpoints according to the endpoint selection type.
    //
    switch(getEndpointSelection())
    {
        case EndpointSelectionType::Random:
        {
            IceUtilInternal::shuffle(endpoints.begin(), endpoints.end());
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
    // make secure endpoints prefered. By default make non-secure
    // endpoints preferred over secure endpoints.
    //
    DefaultsAndOverridesPtr overrides = getInstance()->defaultsAndOverrides();
    if(overrides->overrideSecure ? overrides->overrideSecureValue : getSecure())
    {
        endpoints.erase(remove_if(endpoints.begin(), endpoints.end(),
                                  [](const EndpointIPtr& p)
                                  {
                                      return !p->secure();
                                  }),
                        endpoints.end());
    }
    else if(getPreferSecure())
    {
        //
        // We must use stable_partition() instead of just simply
        // partition(), because otherwise some STL implementations
        // order our now randomized endpoints.
        //
        stable_partition(endpoints.begin(), endpoints.end(),
                         [](const EndpointIPtr& p)
                         {
                             return p->secure();
                         });
    }
    else
    {
        //
        // We must use stable_partition() instead of just simply
        // partition(), because otherwise some STL implementations
        // order our now randomized endpoints.
        //
        stable_partition(endpoints.begin(), endpoints.end(),
                         [](const EndpointIPtr& p)
                         {
                             return !p->secure();
                         });
    }

    return endpoints;
}
