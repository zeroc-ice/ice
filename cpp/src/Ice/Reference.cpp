// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Reference.h>
#include <Ice/ReferenceFactory.h>
#include <Ice/LocalException.h>
#include <Ice/Instance.h>
#include <Ice/EndpointI.h>
#include <Ice/OpaqueEndpointI.h>
#include <Ice/BasicStream.h>
#include <Ice/RouterInfo.h>
#include <Ice/Router.h>
#include <Ice/LocatorInfo.h>
#include <Ice/Locator.h>
#include <Ice/Functional.h>
#include <Ice/ConnectionI.h>
#include <Ice/ConnectionFactory.h>
#include <Ice/LoggerUtil.h>
#include <Ice/TraceLevels.h>
#include <Ice/HashUtil.h>
#include <Ice/RequestHandlerFactory.h>
#include <Ice/ConnectionRequestHandler.h>
#include <Ice/DefaultsAndOverrides.h>
#include <IceUtil/StringUtil.h>
#include <IceUtil/Random.h>
#include <IceUtil/MutexPtrLock.h>

#include <functional>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(IceInternal::Reference* p) { return p; }

namespace
{

IceUtil::Mutex* hashMutex = 0;

class Init
{
public:

    Init()
    {
        hashMutex = new IceUtil::Mutex;
    }

    ~Init()
    {
        delete hashMutex;
        hashMutex = 0;
    }
};

Init init;

struct RandomNumberGenerator : public std::unary_function<ptrdiff_t, ptrdiff_t>
{
    ptrdiff_t operator()(ptrdiff_t d)
    {
        return IceUtilInternal::random(static_cast<int>(d));
    }
};

}

CommunicatorPtr
IceInternal::Reference::getCommunicator() const
{
    return _communicator;
}

ReferencePtr
IceInternal::Reference::changeContext(const Context& newContext) const
{
    ReferencePtr r = _instance->referenceFactory()->copy(this);
    r->_context = new SharedContext(newContext);
    return r;
}

ReferencePtr
IceInternal::Reference::changeMode(Mode newMode) const
{
    if(newMode == _mode)
    {
        return ReferencePtr(const_cast<Reference*>(this));
    }
    ReferencePtr r = _instance->referenceFactory()->copy(this);
    r->_mode = newMode;
    return r;
}

ReferencePtr
IceInternal::Reference::changeSecure(bool newSecure) const
{
    if(newSecure == _secure)
    {
        return ReferencePtr(const_cast<Reference*>(this));
    }
    ReferencePtr r = _instance->referenceFactory()->copy(this);
    r->_secure = newSecure;
    return r;
}

ReferencePtr
IceInternal::Reference::changeIdentity(const Identity& newIdentity) const
{
    if(newIdentity == _identity)
    {
        return ReferencePtr(const_cast<Reference*>(this));
    }
    ReferencePtr r = _instance->referenceFactory()->copy(this);
    r->_identity = newIdentity;
    return r;
}

ReferencePtr
IceInternal::Reference::changeFacet(const string& newFacet) const
{
    if(newFacet == _facet)
    {
        return ReferencePtr(const_cast<Reference*>(this));
    }
    ReferencePtr r = _instance->referenceFactory()->copy(this);
    r->_facet = newFacet;
    return r;
}

ReferencePtr
IceInternal::Reference::changeInvocationTimeout(int invocationTimeout) const
{
    if(_invocationTimeout == invocationTimeout)
    {
        return ReferencePtr(const_cast<Reference*>(this));
    }
    ReferencePtr r = _instance->referenceFactory()->copy(this);
    r->_invocationTimeout = invocationTimeout;
    return r;
}

ReferencePtr
IceInternal::Reference::changeEncoding(const Ice::EncodingVersion& encoding) const
{
    if(_encoding == encoding)
    {
        return ReferencePtr(const_cast<Reference*>(this));
    }
    ReferencePtr r = _instance->referenceFactory()->copy(this);
    r->_encoding = encoding;
    return r;
}

ReferencePtr
IceInternal::Reference::changeCompress(bool newCompress) const
{
    if(_overrideCompress && newCompress == _compress)
    {
        return ReferencePtr(const_cast<Reference*>(this));
    }
    ReferencePtr r = _instance->referenceFactory()->copy(this);
    r->_compress = newCompress;
    r->_overrideCompress = true;
    return r;
}

Int
Reference::hash() const
{
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(hashMutex);
    if(!_hashInitialized)
    {
        _hashValue = hashInit();
        _hashInitialized = true;
    }
    return _hashValue;
}

void
IceInternal::Reference::streamWrite(BasicStream* s) const
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

    if(s->getWriteEncoding() != Ice::Encoding_1_0)
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

    //
    // If the encoded identity string contains characters which
    // the reference parser uses as separators, then we enclose
    // the identity string in quotes.
    //
    string id = _instance->identityToString(_identity);
    if(id.find_first_of(" :@") != string::npos)
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

        //
        // If the encoded facet string contains characters which
        // the reference parser uses as separators, then we enclose
        // the facet string in quotes.
        //
        string fs = nativeToUTF8(_facet, _instance->getStringConverter());
        fs = IceUtilInternal::escapeString(fs, "");
        if(fs.find_first_of(" :@") != string::npos)
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
IceInternal::Reference::operator!=(const Reference& r) const
{
    return !operator==(r);
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

class ConnectionIsDatagram : public unary_function<ConnectionIPtr, bool>
{
public:

    bool
    operator()(ConnectionIPtr p) const
    {
        return p->endpoint()->datagram();
    }
};

class ConnectionIsSecure : public unary_function<ConnectionIPtr, bool>
{
public:

    bool
    operator()(ConnectionIPtr p) const
    {
        return p->endpoint()->secure();
    }
};

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
    _hashInitialized(false),
    _instance(instance),
    _communicator(communicator),
    _mode(mode),
    _secure(secure),
    _identity(id),
    _context(new SharedContext(ctx)),
    _facet(facet),
    _protocol(protocol),
    _encoding(encoding),
    _invocationTimeout(invocationTimeout),
    _overrideCompress(false),
    _compress(false)
{
}

IceInternal::Reference::Reference(const Reference& r) :
    _hashInitialized(false),
    _instance(r._instance),
    _communicator(r._communicator),
    _mode(r._mode),
    _secure(r._secure),
    _identity(r._identity),
    _context(r._context),
    _facet(r._facet),
    _protocol(r._protocol),
    _encoding(r._encoding),
    _invocationTimeout(r._invocationTimeout),
    _overrideCompress(r._overrideCompress),
    _compress(r._compress)
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

IceUtil::Shared* IceInternal::upCast(IceInternal::FixedReference* p) { return p; }

IceInternal::FixedReference::FixedReference(const InstancePtr& instance,
                                            const CommunicatorPtr& communicator,
                                            const Identity& id,
                                            const string& facet,
                                            Mode mode,
                                            bool secure,
                                            const EncodingVersion& encoding,
                                            const ConnectionIPtr& fixedConnection) :
    Reference(instance, communicator, id, facet, mode, secure, Ice::Protocol_1_0, encoding, -1, Ice::Context()),
    _fixedConnection(fixedConnection)
{
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
    return Random;
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

ReferencePtr
IceInternal::FixedReference::changeEndpoints(const vector<EndpointIPtr>& /*newEndpoints*/) const
{
    throw FixedProxyException(__FILE__, __LINE__);
    return 0; // Keep the compiler happy.
}

ReferencePtr
IceInternal::FixedReference::changeAdapterId(const string& /*newAdapterId*/) const
{
    throw FixedProxyException(__FILE__, __LINE__);
    return 0; // Keep the compiler happy.
}

ReferencePtr
IceInternal::FixedReference::changeLocator(const LocatorPrx&) const
{
    throw FixedProxyException(__FILE__, __LINE__);
    return 0; // Keep the compiler happy.
}

ReferencePtr
IceInternal::FixedReference::changeRouter(const RouterPrx&) const
{
    throw FixedProxyException(__FILE__, __LINE__);
    return 0; // Keep the compiler happy.
}

ReferencePtr
IceInternal::FixedReference::changeCollocationOptimized(bool) const
{
    throw FixedProxyException(__FILE__, __LINE__);
    return 0; // Keep the compiler happy.
}

ReferencePtr
IceInternal::FixedReference::changeCacheConnection(bool) const
{
    throw FixedProxyException(__FILE__, __LINE__);
    return 0; // Keep the compiler happy.
}

ReferencePtr
IceInternal::FixedReference::changePreferSecure(bool) const
{
    throw FixedProxyException(__FILE__, __LINE__);
    return 0; // Keep the compiler happy.
}

ReferencePtr
IceInternal::FixedReference::changeEndpointSelection(EndpointSelectionType) const
{
    throw FixedProxyException(__FILE__, __LINE__);
    return 0; // Keep the compiler happy.
}

ReferencePtr
IceInternal::FixedReference::changeLocatorCacheTimeout(int) const
{
    throw FixedProxyException(__FILE__, __LINE__);
    return 0; // Keep the compiler happy.
}

ReferencePtr
IceInternal::FixedReference::changeTimeout(int) const
{
    throw FixedProxyException(__FILE__, __LINE__);
    return 0; // Keep the compiler happy.
}

ReferencePtr
IceInternal::FixedReference::changeConnectionId(const string&) const
{
    throw FixedProxyException(__FILE__, __LINE__);
    return 0; // Keep the compiler happy.
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
IceInternal::FixedReference::streamWrite(BasicStream*) const
{
    throw FixedProxyException(__FILE__, __LINE__);
}

string
IceInternal::FixedReference::toString() const
{
    throw FixedProxyException(__FILE__, __LINE__);

    assert(false);   // Cannot be reached.
    return string(); // To keep the compiler from complaining.
}

PropertyDict
IceInternal::FixedReference::toProperty(const string&) const
{
    throw FixedProxyException(__FILE__, __LINE__);

    assert(false);   // Cannot be reached.
    return PropertyDict(); // To keep the compiler from complaining.
}

RequestHandlerPtr
IceInternal::FixedReference::getRequestHandler(const Ice::ObjectPrx& proxy) const
{
    switch(getMode())
    {
    case Reference::ModeTwoway:
    case Reference::ModeOneway:
    case Reference::ModeBatchOneway:
    {
        if(_fixedConnection->endpoint()->datagram())
        {
            throw NoEndpointException(__FILE__, __LINE__, "");
        }
        break;
    }

    case Reference::ModeDatagram:
    case Reference::ModeBatchDatagram:
    {
        if(!_fixedConnection->endpoint()->datagram())
        {
            throw NoEndpointException(__FILE__, __LINE__, "");
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
        throw NoEndpointException(__FILE__, __LINE__, "");
    }

    _fixedConnection->throwException(); // Throw in case our connection is already destroyed.

    bool compress;
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
        compress = _fixedConnection->endpoint()->compress();
    }

    ReferencePtr ref = const_cast<FixedReference*>(this);
    return proxy->__setRequestHandler(new ConnectionRequestHandler(ref, _fixedConnection, compress));
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
IceInternal::FixedReference::operator!=(const Reference& r) const
{
    return !operator==(r);
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
    return new FixedReference(*this);
}

IceInternal::FixedReference::FixedReference(const FixedReference& r) :
    Reference(r),
    _fixedConnection(r._fixedConnection)
{
}

IceUtil::Shared* IceInternal::upCast(IceInternal::RoutableReference* p) { return p; }

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

ReferencePtr
IceInternal::RoutableReference::changeEncoding(const Ice::EncodingVersion& encoding) const
{
    ReferencePtr r = Reference::changeEncoding(encoding);
    if(r.get() != const_cast<RoutableReference*>(this))
    {
        LocatorInfoPtr& locInfo = RoutableReferencePtr::dynamicCast(r)->_locatorInfo;
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
        RoutableReferencePtr::dynamicCast(r)->_endpoints = newEndpoints;
    }
    return r;
}

ReferencePtr
IceInternal::RoutableReference::changeEndpoints(const vector<EndpointIPtr>& newEndpoints) const
{
    if(newEndpoints == _endpoints)
    {
        return RoutableReferencePtr(const_cast<RoutableReference*>(this));
    }
    RoutableReferencePtr r = RoutableReferencePtr::dynamicCast(getInstance()->referenceFactory()->copy(this));
    r->_endpoints = newEndpoints;
    r->applyOverrides(r->_endpoints);
    r->_adapterId.clear();
    return r;
}

ReferencePtr
IceInternal::RoutableReference::changeAdapterId(const string& newAdapterId) const
{
    if(newAdapterId == _adapterId)
    {
        return RoutableReferencePtr(const_cast<RoutableReference*>(this));
    }
    RoutableReferencePtr r = RoutableReferencePtr::dynamicCast(getInstance()->referenceFactory()->copy(this));
    r->_adapterId = newAdapterId;
    r->_endpoints.clear();
    return r;
}

ReferencePtr
IceInternal::RoutableReference::changeLocator(const LocatorPrx& newLocator) const
{
    LocatorInfoPtr newLocatorInfo = getInstance()->locatorManager()->get(newLocator);
    if(newLocatorInfo == _locatorInfo)
    {
        return RoutableReferencePtr(const_cast<RoutableReference*>(this));
    }
    RoutableReferencePtr r = RoutableReferencePtr::dynamicCast(getInstance()->referenceFactory()->copy(this));
    r->_locatorInfo = newLocatorInfo;
    return r;
}

ReferencePtr
IceInternal::RoutableReference::changeRouter(const RouterPrx& newRouter) const
{
    RouterInfoPtr newRouterInfo = getInstance()->routerManager()->get(newRouter);
    if(newRouterInfo == _routerInfo)
    {
        return RoutableReferencePtr(const_cast<RoutableReference*>(this));
    }
    RoutableReferencePtr r = RoutableReferencePtr::dynamicCast(getInstance()->referenceFactory()->copy(this));
    r->_routerInfo = newRouterInfo;
    return r;
}

ReferencePtr
IceInternal::RoutableReference::changeCollocationOptimized(bool newCollocationOptimized) const
{
    if(newCollocationOptimized == _collocationOptimized)
    {
        return RoutableReferencePtr(const_cast<RoutableReference*>(this));
    }
    RoutableReferencePtr r = RoutableReferencePtr::dynamicCast(getInstance()->referenceFactory()->copy(this));
    r->_collocationOptimized = newCollocationOptimized;
    return r;
}

ReferencePtr
IceInternal::RoutableReference::changeCacheConnection(bool newCache) const
{
    if(newCache == _cacheConnection)
    {
        return RoutableReferencePtr(const_cast<RoutableReference*>(this));
    }
    RoutableReferencePtr r = RoutableReferencePtr::dynamicCast(getInstance()->referenceFactory()->copy(this));
    r->_cacheConnection = newCache;
    return r;
}

ReferencePtr
IceInternal::RoutableReference::changePreferSecure(bool newPreferSecure) const
{
    if(newPreferSecure == _preferSecure)
    {
        return RoutableReferencePtr(const_cast<RoutableReference*>(this));
    }
    RoutableReferencePtr r = RoutableReferencePtr::dynamicCast(getInstance()->referenceFactory()->copy(this));
    r->_preferSecure = newPreferSecure;
    return r;
}

ReferencePtr
IceInternal::RoutableReference::changeEndpointSelection(EndpointSelectionType newType) const
{
    if(newType == _endpointSelection)
    {
        return RoutableReferencePtr(const_cast<RoutableReference*>(this));
    }
    RoutableReferencePtr r = RoutableReferencePtr::dynamicCast(getInstance()->referenceFactory()->copy(this));
    r->_endpointSelection = newType;
    return r;
}

ReferencePtr
IceInternal::RoutableReference::changeLocatorCacheTimeout(int timeout) const
{
    if(timeout == _locatorCacheTimeout)
    {
        return RoutableReferencePtr(const_cast<RoutableReference*>(this));
    }
    RoutableReferencePtr r = RoutableReferencePtr::dynamicCast(getInstance()->referenceFactory()->copy(this));
    r->_locatorCacheTimeout = timeout;
    return r;
}

ReferencePtr
IceInternal::RoutableReference::changeTimeout(int newTimeout) const
{
    if(_overrideTimeout && newTimeout == _timeout)
    {
        return RoutableReferencePtr(const_cast<RoutableReference*>(this));
    }
    RoutableReferencePtr r = RoutableReferencePtr::dynamicCast(getInstance()->referenceFactory()->copy(this));
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
    if(id == _connectionId)
    {
        return RoutableReferencePtr(const_cast<RoutableReference*>(this));
    }
    RoutableReferencePtr r = RoutableReferencePtr::dynamicCast(getInstance()->referenceFactory()->copy(this));
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
IceInternal::RoutableReference::streamWrite(BasicStream* s) const
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
        string a = nativeToUTF8(_adapterId, getInstance()->getStringConverter());
        a = IceUtilInternal::escapeString(a, "");
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
    properties[prefix + ".EndpointSelection"] = _endpointSelection == Random ? "Random" : "Ordered";
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
        PropertyDict routerProperties = _routerInfo->getRouter()->__reference()->toProperty(prefix + ".Router");
        for(PropertyDict::const_iterator p = routerProperties.begin(); p != routerProperties.end(); ++p)
        {
            properties[p->first] = p->second;
        }
    }

    if(_locatorInfo)
    {
        PropertyDict locatorProperties = _locatorInfo->getLocator()->__reference()->toProperty(prefix + ".Locator");
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
    if(_routerInfo != rhs->_routerInfo)
    {
        return false;
    }
    if(_locatorInfo != rhs->_locatorInfo)
    {
        return false;
    }
    if(_endpoints != rhs->_endpoints)
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
IceInternal::RoutableReference::operator!=(const Reference& r) const
{
    return !operator==(r);
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
    if(_routerInfo < rhs->_routerInfo)
    {
        return true;
    }
    else if(rhs->_routerInfo < _routerInfo)
    {
        return false;
    }
    if(_locatorInfo < rhs->_locatorInfo)
    {
        return true;
    }
    else if(rhs->_locatorInfo < _locatorInfo)
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
    if(_endpoints < rhs->_endpoints)
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
    return new RoutableReference(*this);
}

RequestHandlerPtr
IceInternal::RoutableReference::getRequestHandler(const Ice::ObjectPrx& proxy) const
{
    return getInstance()->requestHandlerFactory()->getRequestHandler(const_cast<RoutableReference*>(this), proxy);
}

BatchRequestQueuePtr
IceInternal::RoutableReference::getBatchRequestQueue() const
{
    return new BatchRequestQueue(getInstance(), getMode() == Reference::ModeBatchDatagram);
}

void
IceInternal::RoutableReference::getConnection(const GetConnectionCallbackPtr& callback) const
{
    class Callback : public RouterInfo::GetClientEndpointsCallback
    {
    public:

        virtual void
        setEndpoints(const vector<EndpointIPtr>& endpoints)
        {
            vector<EndpointIPtr> endpts = endpoints;
            if(!endpts.empty())
            {
                _reference->applyOverrides(endpts);
                _reference->createConnection(endpts, _callback);
                return;
            }

            _reference->getConnectionNoRouterInfo(_callback);
        }

        virtual void
        setException(const Ice::LocalException& ex)
        {
            _callback->setException(ex);
        }

        Callback(const RoutableReferencePtr& reference, const GetConnectionCallbackPtr& callback) :
            _reference(reference), _callback(callback)
        {
        }

    private:

        const RoutableReferencePtr _reference;
        const GetConnectionCallbackPtr _callback;
    };

    if(_routerInfo)
    {
        //
        // If we route, we send everything to the router's client
        // proxy endpoints.
        //
        _routerInfo->getClientEndpoints(new Callback(const_cast<RoutableReference*>(this), callback));
        return;
    }

    getConnectionNoRouterInfo(callback);
}

void
IceInternal::RoutableReference::getConnectionNoRouterInfo(const GetConnectionCallbackPtr& callback) const
{
    class Callback : public LocatorInfo::GetEndpointsCallback
    {
    public:

        class Callback2 : public Reference::GetConnectionCallback
        {
        public:

            virtual void
            setConnection(const Ice::ConnectionIPtr& connection, bool compress)
            {
                _callback->setConnection(connection, compress);
            }

            virtual void
            setException(const Ice::LocalException& exc)
            {
                try
                {
                    exc.ice_throw();
                }
                catch(const Ice::NoEndpointException& ex)
                {
                    _callback->setException(ex); // No need to retry if there's no endpoints.
                }
                catch(const Ice::LocalException& ex)
                {
                    LocatorInfoPtr locatorInfo = _reference->getLocatorInfo();
                    assert(locatorInfo);
                    locatorInfo->clearCache(_reference);
                    if(_cached)
                    {
                        TraceLevelsPtr traceLvls = _reference->getInstance()->traceLevels();
                        if(traceLvls->retry >= 2)
                            {
                            Trace out(_reference->getInstance()->initializationData().logger, traceLvls->retryCat);
                            out << "connection to cached endpoints failed\n"
                                << "removing endpoints from cache and trying one more time\n" << ex;
                        }
                        _reference->getConnectionNoRouterInfo(_callback); // Retry.
                        return;
                    }
                    _callback->setException(ex);
                }
            }

            Callback2(const RoutableReferencePtr& reference, const GetConnectionCallbackPtr& cb, bool cached) :
                _reference(reference), _callback(cb), _cached(cached)
            {
            }

        private:

            const RoutableReferencePtr _reference;
            const GetConnectionCallbackPtr _callback;
            const bool _cached;
        };


        virtual void
        setEndpoints(const vector<EndpointIPtr>& endpoints, bool cached)
        {
            if(endpoints.empty())
            {
                _callback->setException(Ice::NoEndpointException(__FILE__, __LINE__, _reference->toString()));
                return;
            }

            vector<EndpointIPtr> endpts = endpoints;
            _reference->applyOverrides(endpts);
            _reference->createConnection(endpts, new Callback2(_reference, _callback, cached));
        }

        virtual void
        setException(const Ice::LocalException& ex)
        {
            _callback->setException(ex);
        }

        Callback(const RoutableReferencePtr& reference, const GetConnectionCallbackPtr& callback) :
            _reference(reference), _callback(callback)
        {
        }

    private:

        const RoutableReferencePtr _reference;
        const GetConnectionCallbackPtr _callback;
    };

    if(!_endpoints.empty())
    {
        createConnection(_endpoints, callback);
        return;
    }

    if(_locatorInfo)
    {
        RoutableReference* self = const_cast<RoutableReference*>(this);
        _locatorInfo->getEndpointsWithCallback(self, _locatorCacheTimeout, new Callback(self, callback));
    }
    else
    {
        callback->setException(Ice::NoEndpointException(__FILE__, __LINE__, toString()));
    }
}

void
IceInternal::RoutableReference::createConnection(const vector<EndpointIPtr>& allEndpoints,
                                                 const GetConnectionCallbackPtr& callback) const
{
    vector<EndpointIPtr> endpoints = filterEndpoints(allEndpoints);
    if(endpoints.empty())
    {
        callback->setException(Ice::NoEndpointException(__FILE__, __LINE__, toString()));
        return;
    }

    //
    // Finally, create the connection.
    //
    OutgoingConnectionFactoryPtr factory = getInstance()->outgoingConnectionFactory();
    if(getCacheConnection() || endpoints.size() == 1)
    {
        class CB1 : public OutgoingConnectionFactory::CreateConnectionCallback
        {
        public:

            virtual void
            setConnection(const Ice::ConnectionIPtr& connection, bool compress)
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
                _callback->setConnection(connection, compress);
            }

            virtual void
            setException(const Ice::LocalException& ex)
            {
                _callback->setException(ex);
            }

            CB1(const RouterInfoPtr& routerInfo, const GetConnectionCallbackPtr& callback) :
                _routerInfo(routerInfo), _callback(callback)
            {
            }

        private:

            const RouterInfoPtr _routerInfo;
            const GetConnectionCallbackPtr _callback;
        };

        //
        // Get an existing connection or create one if there's no
        // existing connection to one of the given endpoints.
        //
        factory->create(endpoints, false, getEndpointSelection(), new CB1(_routerInfo, callback));
        return;
    }
    else
    {
        class CB2 : public OutgoingConnectionFactory::CreateConnectionCallback
        {
        public:

            virtual void
            setConnection(const Ice::ConnectionIPtr& connection, bool compress)
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
                _callback->setConnection(connection, compress);
            }

            virtual void
            setException(const Ice::LocalException& ex)
            {
                if(!_exception.get())
                {
                    _exception.reset(ex.ice_clone());
                }

                if(++_i == _endpoints.size())
                {
                    _callback->setException(*_exception.get());
                    return;
                }

                const bool more = _i != _endpoints.size() - 1;
                vector<EndpointIPtr> endpoint;
                endpoint.push_back(_endpoints[_i]);

                OutgoingConnectionFactoryPtr factory = _reference->getInstance()->outgoingConnectionFactory();
                factory->create(endpoint, more, _reference->getEndpointSelection(), this);
            }

            CB2(const RoutableReferencePtr& reference, const vector<EndpointIPtr>& endpoints,
                const GetConnectionCallbackPtr& callback) :
                _reference(reference),
                _endpoints(endpoints),
                _callback(callback),
                _i(0)
            {
            }

        private:

            const RoutableReferencePtr _reference;
            const vector<EndpointIPtr> _endpoints;
            const GetConnectionCallbackPtr _callback;
            size_t _i;
            IceUtil::UniquePtr<Ice::LocalException> _exception;
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
        RoutableReference* self = const_cast<RoutableReference*>(this);
        factory->create(endpt, true, getEndpointSelection(), new CB2(self, endpoints, callback));
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

namespace
{

struct EndpointIsOpaque : public unary_function<EndpointIPtr, bool>
{
public:

    bool
    operator()(EndpointIPtr p) const
    {
        return dynamic_cast<OpaqueEndpointI*>(p.get()) != 0;
    }
};

}

vector<EndpointIPtr>
IceInternal::RoutableReference::filterEndpoints(const vector<EndpointIPtr>& allEndpoints) const
{
    vector<EndpointIPtr> endpoints = allEndpoints;

    //
    // Filter out unknown endpoints.
    //
    endpoints.erase(remove_if(endpoints.begin(), endpoints.end(), EndpointIsOpaque()), endpoints.end());

    //
    // Filter out endpoints according to the mode of the reference.
    //
    switch(getMode())
    {
        case Reference::ModeTwoway:
        case Reference::ModeOneway:
        case Reference::ModeBatchOneway:
        {
            //
            // Filter out datagram endpoints.
            //
            endpoints.erase(remove_if(endpoints.begin(), endpoints.end(), Ice::constMemFun(&EndpointI::datagram)),
                            endpoints.end());
            break;
        }

        case Reference::ModeDatagram:
        case Reference::ModeBatchDatagram:
        {
            //
            // Filter out non-datagram endpoints.
            //
            endpoints.erase(remove_if(endpoints.begin(), endpoints.end(),
                                      not1(Ice::constMemFun(&EndpointI::datagram))),
                            endpoints.end());
            break;
        }
    }

    //
    // Sort the endpoints according to the endpoint selection type.
    //
    switch(getEndpointSelection())
    {
        case Random:
        {
            RandomNumberGenerator rng;
            random_shuffle(endpoints.begin(), endpoints.end(), rng);
            break;
        }
        case Ordered:
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
        endpoints.erase(remove_if(endpoints.begin(), endpoints.end(), not1(Ice::constMemFun(&EndpointI::secure))),
                        endpoints.end());
    }
    else if(getPreferSecure())
    {
        //
        // We must use stable_partition() instead of just simply
        // partition(), because otherwise some STL implementations
        // order our now randomized endpoints.
        //
         stable_partition(endpoints.begin(), endpoints.end(), Ice::constMemFun(&EndpointI::secure));
    }
    else
    {
        //
        // We must use stable_partition() instead of just simply
        // partition(), because otherwise some STL implementations
        // order our now randomized endpoints.
        //
        stable_partition(endpoints.begin(), endpoints.end(), not1(Ice::constMemFun(&EndpointI::secure)));
    }

    return endpoints;
}
