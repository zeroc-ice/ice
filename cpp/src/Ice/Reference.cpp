// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
#include <Ice/DefaultsAndOverrides.h>
#include <IceUtil/StringUtil.h>
#include <IceUtil/Random.h>

#include <functional>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(IceInternal::Reference* p) { return p; }

namespace
{
struct RandomNumberGenerator : public std::unary_function<ptrdiff_t, ptrdiff_t>
{
    ptrdiff_t operator()(ptrdiff_t d)
    {
        return IceUtil::random(static_cast<int>(d));
    }
};

}

CommunicatorPtr
IceInternal::Reference::getCommunicator() const
{
    return _communicator;
}

ReferencePtr
IceInternal::Reference::defaultContext() const
{
    ReferencePtr r = _instance->referenceFactory()->copy(this);
    r->_context = _instance->getDefaultContext();
    return r;
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

Int
Reference::hash() const
{
    IceUtil::RecMutex::Lock sync(_hashMutex);

    if(_hashInitialized)
    {
        return _hashValue;
    }

    string::const_iterator p;
    Context::const_iterator q;

    Int h = static_cast<Int>(_mode);

    for(p = _identity.name.begin(); p != _identity.name.end(); ++p)
    {
        h = 5 * h + *p;
    }

    for(p = _identity.category.begin(); p != _identity.category.end(); ++p)
    {
        h = 5 * h + *p;
    }

    for(q = _context->getValue().begin(); q != _context->getValue().end(); ++q)
    {
        for(p = q->first.begin(); p != q->first.end(); ++p)
        {
            h = 5 * h + *p;
        }
        for(p = q->second.begin(); p != q->second.end(); ++p)
        {
            h = 5 * h + *p;
        }
    }

    for(p = _facet.begin(); p != _facet.end(); ++p)
    {
        h = 5 * h + *p;
    }

    h = 5 * h + static_cast<Int>(getSecure());

    _hashValue = h;
    _hashInitialized = true;

    return h;
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
    
    s->write(getSecure());

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
        string fs = _facet;
        if(_instance->initializationData().stringConverter)
        {
            UTF8BufferI buffer;
            Byte* last = 
                _instance->initializationData().stringConverter->toUTF8(fs.data(), fs.data() + fs.size(), buffer);
            fs = string(reinterpret_cast<const char*>(buffer.getBuffer()), last - buffer.getBuffer());
        }
        fs = IceUtil::escapeString(fs, "");
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

    if(getSecure())
    {
        s << " -s";
    }

    return s.str();

    // Derived class writes the remainder of the string.
}

bool
IceInternal::Reference::operator==(const Reference& r) const
{
    //
    // Note: if(this == &r) test is performed by each non-abstract derived class.
    //
    
    if(getType() != r.getType())
    {
        return false;
    }

    if(_mode != r._mode)
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

    if(getType() < r.getType())
    {
        return true;
    }
    else if(r.getType() < getType())
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

IceInternal::Reference::Reference(const InstancePtr& inst, const CommunicatorPtr& com, const Identity& ident,
                                  const SharedContextPtr& ctx, const string& fs, Mode md) :
    _hashInitialized(false),
    _instance(inst),
    _communicator(com),
    _mode(md),
    _identity(ident),
    _context(ctx),
    _facet(fs)
{
}

IceInternal::Reference::Reference(const Reference& r) :
    _hashInitialized(false),
    _instance(r._instance),
    _communicator(r._communicator),
    _mode(r._mode),
    _identity(r._identity),
    _context(r._context),
    _facet(r._facet)
{
}

IceUtil::Shared* IceInternal::upCast(IceInternal::FixedReference* p) { return p; }

IceInternal::FixedReference::FixedReference(const InstancePtr& inst, const CommunicatorPtr& com, const Identity& ident,
                                            const SharedContextPtr& ctx, const string& fs, Mode md,
                                            const vector<ConnectionIPtr>& fixedConns) :
    Reference(inst, com, ident, ctx, fs, md),
    _fixedConnections(fixedConns)
{
}

Reference::Type
IceInternal::FixedReference::getType() const
{
    return TypeFixed;
}

bool
IceInternal::FixedReference::getSecure() const
{
    return false;
}

bool
IceInternal::FixedReference::getPreferSecure() const
{
    return false;
}

int
IceInternal::FixedReference::getLocatorCacheTimeout() const
{
    return 0;
}

string
IceInternal::FixedReference::getAdapterId() const
{
    return string();
}

vector<EndpointIPtr>
IceInternal::FixedReference::getEndpoints() const
{
    return vector<EndpointIPtr>();
}

bool
IceInternal::FixedReference::getCollocationOptimization() const
{
    return false;
}

bool
IceInternal::FixedReference::getCacheConnection() const
{
    return false;
}

Ice::EndpointSelectionType
IceInternal::FixedReference::getEndpointSelection() const
{
    return Random;
}

bool
IceInternal::FixedReference::getThreadPerConnection() const
{
    return false;
}

ReferencePtr
IceInternal::FixedReference::changeSecure(bool) const
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
IceInternal::FixedReference::changeRouter(const RouterPrx&) const
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
IceInternal::FixedReference::changeCollocationOptimization(bool) const
{
    throw FixedProxyException(__FILE__, __LINE__);
    return 0; // Keep the compiler happy.
}

ReferencePtr
IceInternal::FixedReference::changeCompress(bool) const
{
    // TODO: FixedReferences should probably have a _compress flag,
    // that gets its default from the fixed connection this reference
    // refers to. This should be changable with changeCompress().
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

ReferencePtr
IceInternal::FixedReference::changeLocatorCacheTimeout(int) const
{
    throw FixedProxyException(__FILE__, __LINE__);
    return 0; // Keep the compiler happy.
}

ReferencePtr
IceInternal::FixedReference::changeAdapterId(const string& newAdapterId) const
{
    throw FixedProxyException(__FILE__, __LINE__);
    return 0; // Keep the compiler happy.
}

ReferencePtr
IceInternal::FixedReference::changeEndpoints(const vector<EndpointIPtr>& newEndpoints) const
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
IceInternal::FixedReference::changeEndpointSelection(EndpointSelectionType) const
{
    throw FixedProxyException(__FILE__, __LINE__);
    return 0; // Keep the compiler happy.
}

ReferencePtr
IceInternal::FixedReference::changeThreadPerConnection(bool) const
{
    throw FixedProxyException(__FILE__, __LINE__);
    return 0; // Keep the compiler happy.
}

void
IceInternal::FixedReference::streamWrite(BasicStream* s) const
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

ConnectionIPtr
IceInternal::FixedReference::getConnection(bool& compress) const
{
    vector<ConnectionIPtr> filteredConns = filterConnections(_fixedConnections);
    if(filteredConns.empty())
    {
        NoEndpointException ex(__FILE__, __LINE__);
        ex.proxy = ""; // No stringified representation for fixed proxies
        throw ex;
    }

    ConnectionIPtr connection = filteredConns[0];
    assert(connection);
    connection->throwException(); // Throw in case our connection is already destroyed.
    compress = connection->endpoint()->compress();

    return connection;
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
    return _fixedConnections == rhs->_fixedConnections;
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
    if(Reference::operator==(r))
    {
        const FixedReference* rhs = dynamic_cast<const FixedReference*>(&r);
        assert(rhs);
        return _fixedConnections < rhs->_fixedConnections;
    }
    return false;
}

ReferencePtr
IceInternal::FixedReference::clone() const
{
    return new FixedReference(*this);
}

IceInternal::FixedReference::FixedReference(const FixedReference& r) :
    Reference(r),
    _fixedConnections(r._fixedConnections)
{
}

vector<ConnectionIPtr>
IceInternal::FixedReference::filterConnections(const vector<ConnectionIPtr>& allConnections) const
{
    vector<ConnectionIPtr> connections = allConnections;

    switch(getMode())
    {
        case Reference::ModeTwoway:
        case Reference::ModeOneway:
        case Reference::ModeBatchOneway:
        {
            //
            // Filter out datagram connections.
            //
            connections.erase(remove_if(connections.begin(), connections.end(), ConnectionIsDatagram()),
                              connections.end());
            break;
        }
        
        case Reference::ModeDatagram:
        case Reference::ModeBatchDatagram:
        {
            //
            // Filter out non-datagram connections.
            //
            connections.erase(remove_if(connections.begin(), connections.end(), not1(ConnectionIsDatagram())),
                              connections.end());
            break;
        }
    }
    
    //
    // Randomize the order of connections.
    //
    RandomNumberGenerator rng;
    random_shuffle(connections.begin(), connections.end(), rng);

    //
    // If a secure connection is requested or secure overrides is set,
    // remove all non-secure connections. Otherwise make non-secure
    // connections preferred over secure connections by partitioning
    // the connection vector, so that non-secure connections come
    // first.
    //
    DefaultsAndOverridesPtr overrides = getInstance()->defaultsAndOverrides();
    if(overrides->overrideSecure ? overrides->overrideSecureValue : getSecure())
    {
        connections.erase(remove_if(connections.begin(), connections.end(), not1(ConnectionIsSecure())),
                          connections.end());
    }
    else
    {
        //
        // We must use stable_partition() instead of just simply
        // partition(), because otherwise some STL implementations
        // order our now randomized connections.
        //
        stable_partition(connections.begin(), connections.end(), not1(ConnectionIsSecure()));
    }
    
    return connections;
}

IceUtil::Shared* IceInternal::upCast(IceInternal::RoutableReference* p) { return p; }

vector<EndpointIPtr>
IceInternal::RoutableReference::getRoutedEndpoints() const
{
    if(_routerInfo)
    {
        //
        // If we route, we send everything to the router's client
        // proxy endpoints.
        //
        return _routerInfo->getClientEndpoints();
    }
    return vector<EndpointIPtr>();
}

bool
IceInternal::RoutableReference::getSecure() const
{
    return _secure;
}

bool
IceInternal::RoutableReference::getPreferSecure() const
{
    return _preferSecure;
}

bool
IceInternal::RoutableReference::getCollocationOptimization() const
{
    return _collocationOptimization;
}

bool
IceInternal::RoutableReference::getCacheConnection() const
{
    return _cacheConnection;
}

Ice::EndpointSelectionType
IceInternal::RoutableReference::getEndpointSelection() const
{
    return _endpointSelection;
}

bool
IceInternal::RoutableReference::getThreadPerConnection() const
{
    return _threadPerConnection;
}

ReferencePtr
IceInternal::RoutableReference::changeSecure(bool newSecure) const
{
    if(newSecure == _secure)
    {
        return RoutableReferencePtr(const_cast<RoutableReference*>(this));
    }
    RoutableReferencePtr r = RoutableReferencePtr::dynamicCast(getInstance()->referenceFactory()->copy(this));
    r->_secure = newSecure;
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
IceInternal::RoutableReference::changeCollocationOptimization(bool newCollocationOptimization) const
{
    if(newCollocationOptimization == _collocationOptimization)
    {
        return RoutableReferencePtr(const_cast<RoutableReference*>(this));
    }
    RoutableReferencePtr r = RoutableReferencePtr::dynamicCast(getInstance()->referenceFactory()->copy(this));
    r->_collocationOptimization = newCollocationOptimization;
    return r;
}

ReferencePtr
IceInternal::RoutableReference::changeCompress(bool newCompress) const
{
    if(_overrideCompress && newCompress == _compress)
    {
        return RoutableReferencePtr(const_cast<RoutableReference*>(this));
    }
    RoutableReferencePtr r = RoutableReferencePtr::dynamicCast(getInstance()->referenceFactory()->copy(this));
    r->_compress = newCompress;
    r->_overrideCompress = true;
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
IceInternal::RoutableReference::changeThreadPerConnection(bool newTpc) const
{
    if(newTpc == _threadPerConnection)
    {
        return RoutableReferencePtr(const_cast<RoutableReference*>(this));
    }
    RoutableReferencePtr r = RoutableReferencePtr::dynamicCast(getInstance()->referenceFactory()->copy(this));
    r->_threadPerConnection = newTpc;
    return r;
}

int
IceInternal::RoutableReference::hash() const
{
    return Reference::hash();
}

bool
IceInternal::RoutableReference::operator==(const Reference& r) const
{
    //
    // Note: if(this == &r) test is performed by each non-abstract derived class.
    //

    const RoutableReference* rhs = dynamic_cast<const RoutableReference*>(&r);
    if(!rhs || !Reference::operator==(r))
    {
        return false;
    }
    if(_secure != rhs->_secure)
    {
        return false;
    }
    if(_preferSecure != rhs->_preferSecure)
    {
        return false;
    }
    if(_collocationOptimization != rhs->_collocationOptimization)
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
    if(_overrideCompress != rhs->_overrideCompress || _overrideCompress && _compress != rhs->_compress)
    {
        return false;
    }
    if(_overrideTimeout != rhs->_overrideTimeout || _overrideTimeout && _timeout != rhs->_timeout)
    {
        return false;
    }
    if(_threadPerConnection != rhs->_threadPerConnection)
    {
        return false;
    }
    return _routerInfo == rhs->_routerInfo;
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
    if(Reference::operator==(r))
    {
        const RoutableReference* rhs = dynamic_cast<const RoutableReference*>(&r);
        assert(rhs);
        if(!_secure && rhs->_secure)
        {
            return true;
        }
        else if(rhs->_secure < _secure)
        {
            return false;
        }
        else if(!_preferSecure && rhs->_preferSecure)
        {
            return true;
        }
        else if(rhs->_preferSecure < _preferSecure)
        {
            return false;
        }
        if(!_collocationOptimization && rhs->_collocationOptimization)
        {
            return true;
        }
        else if(rhs->_collocationOptimization < _collocationOptimization)
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
        if(!_overrideCompress && rhs->_overrideCompress)
        {
            return true;
        }
        else if(rhs->_overrideCompress < _overrideCompress)
        {
            return false;
        }
        else if(_overrideCompress)
        {
            if(!_compress && rhs->_compress)
            {
                return true;
            }
            else if(rhs->_compress < _compress)
            {
                return false;
            }
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
        if(!_threadPerConnection && rhs->_threadPerConnection)
        {
            return true;
        }
        else if(rhs->_threadPerConnection < _threadPerConnection)
        {
            return false;
        }
        return _routerInfo < rhs->_routerInfo;
    }
    return false;
}

IceInternal::RoutableReference::RoutableReference(const InstancePtr& inst, const CommunicatorPtr& com,
                                                  const Identity& ident, const SharedContextPtr& ctx, const string& fs,
                                                  Mode md, bool sec, bool prefSec, const RouterInfoPtr& rtrInfo,
                                                  bool collocationOpt, bool cacheConnection,
                                                  EndpointSelectionType endpointSelection, bool threadPerConnection) :
    Reference(inst, com, ident, ctx, fs, md),
    _secure(sec),
    _preferSecure(prefSec),
    _routerInfo(rtrInfo),
    _collocationOptimization(collocationOpt),
    _cacheConnection(cacheConnection),
    _endpointSelection(endpointSelection),
    _overrideCompress(false),
    _compress(false),
    _overrideTimeout(false),
    _timeout(-1),
    _threadPerConnection(threadPerConnection)
{
}

IceInternal::RoutableReference::RoutableReference(const RoutableReference& r) :
    Reference(r),
    _secure(r._secure),
    _preferSecure(r._preferSecure),
    _routerInfo(r._routerInfo),
    _collocationOptimization(r._collocationOptimization),
    _cacheConnection(r._cacheConnection),
    _endpointSelection(r._endpointSelection),
    _connectionId(r._connectionId),
    _overrideCompress(r._overrideCompress),
    _compress(r._compress),
    _overrideTimeout(r._overrideTimeout),
    _timeout(r._timeout),
    _threadPerConnection(r._threadPerConnection)
{
}

ConnectionIPtr
IceInternal::RoutableReference::createConnection(const vector<EndpointIPtr>& allEndpoints, bool& comp) const
{
    vector<EndpointIPtr> endpoints = allEndpoints;

    //
    // Filter out unknown endpoints.
    //
    endpoints.erase(remove_if(endpoints.begin(), endpoints.end(), Ice::constMemFun(&EndpointI::unknown)),
                    endpoints.end());

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

    if(endpoints.empty())
    {
        NoEndpointException ex(__FILE__, __LINE__);
        ex.proxy = toString();
        throw ex;
    }

    //
    // Finally, create the connection.
    //
    OutgoingConnectionFactoryPtr factory = getInstance()->outgoingConnectionFactory();
    if(getCacheConnection() || endpoints.size() == 1)
    {
        //
        // Get an existing connection or create one if there's no
        // existing connection to one of the given endpoints.
        //
        return factory->create(endpoints, false, _threadPerConnection, getEndpointSelection(), comp);
    }
    else
    {
        //
        // Go through the list of endpoints and try to create the
        // connection until it succeeds. This is different from just
        // calling create() with the given endpoints since this might
        // create a new connection even if there's an existing
        // connection for one of the endpoints.
        //

        auto_ptr<LocalException> exception;
        vector<EndpointIPtr> endpoint;
        endpoint.push_back(0);

        for(vector<EndpointIPtr>::const_iterator p = endpoints.begin(); p != endpoints.end(); ++p)
        {
            try
            {
                endpoint.back() = *p;
                return factory->create(endpoint, p + 1 == endpoints.end(), _threadPerConnection, 
                                       getEndpointSelection(), comp);
            }
            catch(const LocalException& ex)
            {
                exception.reset(dynamic_cast<LocalException*>(ex.ice_clone()));
            }
        }

        assert(exception.get());
        exception->ice_throw();
        return 0; // Keeps the compiler happy.
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

IceUtil::Shared* IceInternal::upCast(IceInternal::DirectReference* p) { return p; }

IceInternal::DirectReference::DirectReference(const InstancePtr& inst, const CommunicatorPtr& com,
                                              const Identity& ident, const SharedContextPtr& ctx, const string& fs,
                                              Mode md, bool sec, bool prefSec, const vector<EndpointIPtr>& endpts,
                                              const RouterInfoPtr& rtrInfo, bool collocationOpt, bool cacheConnection,
                                              EndpointSelectionType endpointSelection, bool threadPerConnection) :
    RoutableReference(inst, com, ident, ctx, fs, md, sec, prefSec, rtrInfo, collocationOpt, cacheConnection,
                      endpointSelection, threadPerConnection),
    _endpoints(endpts)
{
}

string
IceInternal::DirectReference::getAdapterId() const
{
    return string();
}

vector<EndpointIPtr>
IceInternal::DirectReference::getEndpoints() const
{
    return _endpoints;
}

Reference::Type
IceInternal::DirectReference::getType() const
{
    return TypeDirect;
}

int
IceInternal::DirectReference::getLocatorCacheTimeout() const
{
    return 0;
}

ReferencePtr
IceInternal::DirectReference::changeLocator(const LocatorPrx& newLocator) const
{
    return DirectReferencePtr(const_cast<DirectReference*>(this));
}

ReferencePtr
IceInternal::DirectReference::changeCompress(bool newCompress) const
{
    DirectReferencePtr r = DirectReferencePtr::dynamicCast(RoutableReference::changeCompress(newCompress));
    if(r.get() != this) // Also override the compress flag on the endpoints if it was updated.
    {
        vector<EndpointIPtr> newEndpoints;
        for(vector<EndpointIPtr>::const_iterator p = _endpoints.begin(); p != _endpoints.end(); ++p)
        {
            newEndpoints.push_back((*p)->compress(newCompress));
        }
        r->_endpoints = newEndpoints;
    }
    return r;
}

ReferencePtr
IceInternal::DirectReference::changeTimeout(int newTimeout) const
{
    DirectReferencePtr r = DirectReferencePtr::dynamicCast(RoutableReference::changeTimeout(newTimeout));
    if(r.get() != this) // Also override the timeout on the endpoints if it was updated.
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
IceInternal::DirectReference::changeConnectionId(const string& newConnectionId) const
{
    DirectReferencePtr r = DirectReferencePtr::dynamicCast(RoutableReference::changeConnectionId(newConnectionId));
    if(r.get() != this) // Also override the connection id on the endpoints if it was updated.
    {
        vector<EndpointIPtr> newEndpoints;
        for(vector<EndpointIPtr>::const_iterator p = _endpoints.begin(); p != _endpoints.end(); ++p)
        {
            newEndpoints.push_back((*p)->connectionId(newConnectionId));
        }
        r->_endpoints = newEndpoints;
    }
    return r;
}

ReferencePtr
IceInternal::DirectReference::changeLocatorCacheTimeout(int) const
{
    return DirectReferencePtr(const_cast<DirectReference*>(this));
}

ReferencePtr
IceInternal::DirectReference::changeAdapterId(const string& newAdapterId) const
{
    if(!newAdapterId.empty())
    {
        LocatorInfoPtr locatorInfo = 
            getInstance()->locatorManager()->get(getInstance()->referenceFactory()->getDefaultLocator());
        return getInstance()->referenceFactory()->create(getIdentity(), getContext(), getFacet(), getMode(),
                                                         getSecure(), getPreferSecure(), newAdapterId, getRouterInfo(),
                                                         locatorInfo, getCollocationOptimization(),
                                                         getCacheConnection(), getEndpointSelection(),
                                                         getThreadPerConnection(), getLocatorCacheTimeout());
    }
    else
    {
        return DirectReferencePtr(const_cast<DirectReference*>(this));
    }
}

ReferencePtr
IceInternal::DirectReference::changeEndpoints(const vector<EndpointIPtr>& newEndpoints) const
{
    if(newEndpoints == _endpoints)
    {
        return DirectReferencePtr(const_cast<DirectReference*>(this));
    }
    DirectReferencePtr r = DirectReferencePtr::dynamicCast(getInstance()->referenceFactory()->copy(this));
    r->_endpoints = newEndpoints;
    r->applyOverrides(r->_endpoints);
    return r;
}

void
IceInternal::DirectReference::streamWrite(BasicStream* s) const
{
    RoutableReference::streamWrite(s);

    Int sz = static_cast<Int>(_endpoints.size());
    s->writeSize(sz);
    if(sz)
    {
        for(vector<EndpointIPtr>::const_iterator p = _endpoints.begin(); p != _endpoints.end(); ++p)
        {
            (*p)->streamWrite(s);
        }
    }
    else
    {
        s->write(string("")); // Adapter id.
    }
}

string
IceInternal::DirectReference::toString() const
{
    //
    // WARNING: Certain features, such as proxy validation in Glacier2,
    // depend on the format of proxy strings. Changes to toString() and
    // methods called to generate parts of the reference string could break
    // these features. Please review for all features that depend on the
    // format of proxyToString() before changing this and related code.
    //
    string result = RoutableReference::toString();

    vector<EndpointIPtr>::const_iterator p;
    for(p = _endpoints.begin(); p != _endpoints.end(); ++p)
    {
        string endp = (*p)->toString();
        if(!endp.empty())
        {
            result.append(":");
            result.append(endp);
        }
    }
    return result;
}

ConnectionIPtr
IceInternal::DirectReference::getConnection(bool& comp) const
{
    vector<EndpointIPtr> endpts = RoutableReference::getRoutedEndpoints();
    applyOverrides(endpts);

    if(endpts.empty())
    {
        endpts = _endpoints; // Endpoint overrides are already applied on these endpoints.
    }

    ConnectionIPtr connection = createConnection(endpts, comp);

    //
    // If we have a router, set the object adapter for this router
    // (if any) to the new connection, so that callbacks from the
    // router can be received over this new connection.
    //
    if(getRouterInfo())
    {
        connection->setAdapter(getRouterInfo()->getAdapter());
    }

    assert(connection);
    return connection;
}

bool
IceInternal::DirectReference::operator==(const Reference& r) const
{
    if(this == &r)
    {
        return true;
    }
    const DirectReference* rhs = dynamic_cast<const DirectReference*>(&r);
    if(!rhs || !RoutableReference::operator==(r))
    {
        return false;
    }
    return _endpoints == rhs->_endpoints;
}

bool
IceInternal::DirectReference::operator!=(const Reference& r) const
{
    return !operator==(r);
}

bool
IceInternal::DirectReference::operator<(const Reference& r) const
{
    if(this == &r)
    {
        return false;
    }
    if(RoutableReference::operator<(r))
    {
        return true;
    }
    if(RoutableReference::operator==(r))
    {
        const DirectReference* rhs = dynamic_cast<const DirectReference*>(&r);
        assert(rhs);
        return _endpoints < rhs->_endpoints;
    }
    return false;
}

ReferencePtr
IceInternal::DirectReference::clone() const
{
    return new DirectReference(*this);
}

IceInternal::DirectReference::DirectReference(const DirectReference& r) :
    RoutableReference(r),
    _endpoints(r._endpoints)
{
}

IceUtil::Shared* IceInternal::upCast(IceInternal::IndirectReference* p) { return p; }

IceInternal::IndirectReference::IndirectReference(const InstancePtr& inst, const CommunicatorPtr& com,
                                                  const Identity& ident, const SharedContextPtr& ctx, const string& fs,
                                                  Mode md, bool sec, bool prefSec, const string& adptid, 
                                                  const RouterInfoPtr& rtrInfo, const LocatorInfoPtr& locInfo,
                                                  bool collocationOpt, bool cacheConnection,
                                                  EndpointSelectionType endpointSelection, bool threadPerConnection,
                                                  int locatorCacheTimeout) :
    RoutableReference(inst, com, ident, ctx, fs, md, sec, prefSec, rtrInfo, collocationOpt, cacheConnection,
                      endpointSelection, threadPerConnection),
    _adapterId(adptid),
    _locatorInfo(locInfo),
    _locatorCacheTimeout(locatorCacheTimeout)
{
}

string
IceInternal::IndirectReference::getAdapterId() const
{
    return _adapterId;
}

vector<EndpointIPtr>
IceInternal::IndirectReference::getEndpoints() const
{
    return vector<EndpointIPtr>();
}

Reference::Type
IceInternal::IndirectReference::getType() const
{
    return TypeIndirect;
}

int
IceInternal::IndirectReference::getLocatorCacheTimeout() const
{
    return _locatorCacheTimeout;
}

ReferencePtr
IceInternal::IndirectReference::changeLocator(const LocatorPrx& newLocator) const
{
    LocatorInfoPtr newLocatorInfo = getInstance()->locatorManager()->get(newLocator);
    if(newLocatorInfo == _locatorInfo)
    {
        return IndirectReferencePtr(const_cast<IndirectReference*>(this));
    }
    IndirectReferencePtr r = IndirectReferencePtr::dynamicCast(getInstance()->referenceFactory()->copy(this));
    r->_locatorInfo = newLocatorInfo;
    return r;
}

ReferencePtr
IceInternal::IndirectReference::changeAdapterId(const string& newAdapterId) const
{
    if(newAdapterId == _adapterId)
    {
        return IndirectReferencePtr(const_cast<IndirectReference*>(this));
    }
    IndirectReferencePtr r = IndirectReferencePtr::dynamicCast(getInstance()->referenceFactory()->copy(this));
    r->_adapterId = newAdapterId;
    return r;
}

ReferencePtr
IceInternal::IndirectReference::changeEndpoints(const vector<EndpointIPtr>& newEndpoints) const
{
    if(!newEndpoints.empty())
    {
        return getInstance()->referenceFactory()->create(getIdentity(), getContext(), getFacet(), getMode(),
                                                         getSecure(), getPreferSecure(), newEndpoints, getRouterInfo(),
                                                         getCollocationOptimization(), getCacheConnection(),
                                                         getEndpointSelection(), getThreadPerConnection());
    }
    else
    {
        return IndirectReferencePtr(const_cast<IndirectReference*>(this));
    }
}

ReferencePtr
IceInternal::IndirectReference::changeLocatorCacheTimeout(int timeout) const
{
    if(timeout == _locatorCacheTimeout)
    {
        return IndirectReferencePtr(const_cast<IndirectReference*>(this));
    }
    IndirectReferencePtr r = IndirectReferencePtr::dynamicCast(getInstance()->referenceFactory()->copy(this));
    r->_locatorCacheTimeout = timeout;
    return r;
}

void
IceInternal::IndirectReference::streamWrite(BasicStream* s) const
{
    RoutableReference::streamWrite(s);

    s->writeSize(0);
    s->write(_adapterId);
}

string
IceInternal::IndirectReference::toString() const
{
    //
    // WARNING: Certain features, such as proxy validation in Glacier2,
    // depend on the format of proxy strings. Changes to toString() and
    // methods called to generate parts of the reference string could break
    // these features. Please review for all features that depend on the
    // format of proxyToString() before changing this and related code.
    //
    string result = RoutableReference::toString();
    if(_adapterId.empty())
    {
        return result;
    }

    result.append(" @ ");

    //
    // If the encoded adapter id string contains characters which the
    // reference parser uses as separators, then we enclose the
    // adapter id string in quotes.
    //
    string a = _adapterId;
    if(getInstance()->initializationData().stringConverter)
    {
        UTF8BufferI buffer;
        Byte* last = getInstance()->initializationData().stringConverter->toUTF8(a.data(), a.data() + a.size(), buffer);
        a = string(reinterpret_cast<const char*>(buffer.getBuffer()), last - buffer.getBuffer());
    }
    a = IceUtil::escapeString(a, "");
    if(a.find_first_of(" ") != string::npos)
    {
        result.append("\"");
        result.append(a);
        result.append("\"");
    }
    else
    {
        result.append(_adapterId);
    }
    return result;
}

ConnectionIPtr
IceInternal::IndirectReference::getConnection(bool& comp) const
{
    ConnectionIPtr connection;

    while(true)
    {
        vector<EndpointIPtr> endpts = RoutableReference::getRoutedEndpoints();
        bool cached = false;
        if(endpts.empty() && _locatorInfo)
        {
            const IndirectReferencePtr self = const_cast<IndirectReference*>(this);
            endpts = _locatorInfo->getEndpoints(self, _locatorCacheTimeout, cached);
        }

        applyOverrides(endpts);

        try
        {
            connection = createConnection(endpts, comp);
            assert(connection);
        }
        catch(const NoEndpointException& ex)
        {
            throw ex; // No need to retry if there's no endpoints.
        }
        catch(const LocalException& ex)
        {
            if(!getRouterInfo())
            {
                assert(_locatorInfo);

                // COMPILERFIX: Braces needed to prevent BCB from causing Reference refCount from
                //              being decremented twice when loop continues.
                {
                    const IndirectReferencePtr self = const_cast<IndirectReference*>(this);
                    _locatorInfo->clearCache(self);
                }

                if(cached)
                {
                    // COMPILERFIX: Braces needed to prevent BCB from causing TraceLevels refCount from
                    //              being decremented twice when loop continues.
                    {
                        TraceLevelsPtr traceLevels = getInstance()->traceLevels();
                        if(traceLevels->retry >= 2)
                        {
                            Trace out(getInstance()->initializationData().logger, traceLevels->retryCat);
                            out << "connection to cached endpoints failed\n"
                                << "removing endpoints from cache and trying one more time\n" << ex;
                        }
                    }
                    continue;
                }
            }

            throw;
        }

        break;
    }

    //
    // If we have a router, set the object adapter for this router
    // (if any) to the new connection, so that callbacks from the
    // router can be received over this new connection.
    //
    if(getRouterInfo())
    {
        connection->setAdapter(getRouterInfo()->getAdapter());
    }

    assert(connection);
    return connection;
}

int
IceInternal::IndirectReference::hash() const
{
    IceUtil::RecMutex::Lock sync(_hashMutex);

    if(_hashInitialized)
    {
        return _hashValue;
    }
    RoutableReference::hash(); // Initializes _hashValue.

    // Add hash of adapter ID to base hash.
    for(string::const_iterator p = _adapterId.begin(); p != _adapterId.end(); ++p)
    {
        _hashValue = 5 * _hashValue + *p;
    }
    return _hashValue;
}

bool
IceInternal::IndirectReference::operator==(const Reference& r) const
{
    if(this == &r)
    {
        return true;
    }
    const IndirectReference* rhs = dynamic_cast<const IndirectReference*>(&r);
    if(!rhs || !RoutableReference::operator==(r))
    {
        return false;
    }

    return _adapterId == rhs->_adapterId && _locatorInfo == rhs->_locatorInfo && 
        _locatorCacheTimeout == rhs->_locatorCacheTimeout;
}

bool
IceInternal::IndirectReference::operator!=(const Reference& r) const
{
    return !operator==(r);
}

bool
IceInternal::IndirectReference::operator<(const Reference& r) const
{
    if(this == &r)
    {
        return false;
    }
    if(RoutableReference::operator<(r))
    {
        return true;
    }
    if(RoutableReference::operator==(r))
    {
        const IndirectReference* rhs = dynamic_cast<const IndirectReference*>(&r);
        assert(rhs);
        if(_adapterId < rhs->_adapterId)
        {
            return true;
        }
        else if(rhs->_adapterId < _adapterId)
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
            
        return _locatorCacheTimeout < rhs->_locatorCacheTimeout;
    }
    return false;
}

ReferencePtr
IceInternal::IndirectReference::clone() const
{
    return new IndirectReference(*this);
}

IceInternal::IndirectReference::IndirectReference(const IndirectReference& r) :
    RoutableReference(r),
    _adapterId(r._adapterId),
    _locatorInfo(r._locatorInfo),
    _locatorCacheTimeout(r._locatorCacheTimeout)
{
}
