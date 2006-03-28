// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Reference.h>
#include <Ice/ReferenceFactory.h>
#include <Ice/LocalException.h>
#include <Ice/Instance.h>
#include <Ice/IdentityUtil.h>
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

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(IceInternal::Reference* p) { p->__incRef(); }
void IceInternal::decRef(IceInternal::Reference* p) { p->__decRef(); }

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
    r->_context = newContext;
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

    for(q = _context.begin(); q != _context.end(); ++q)
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
    ostringstream s;

    //  
    // If the encoded identity string contains characters which
    // the reference parser uses as separators, then we enclose
    // the identity string in quotes.
    //
    string id = identityToString(_identity);
    if(id.find_first_of(" \t\n\r:@") != string::npos)
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
	string fs = IceUtil::escapeString(_facet, "");
	if(fs.find_first_of(" \t\n\r:@") != string::npos)
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
    
    if(_mode != r._mode)
    {
	return false;
    }

    if(_identity != r._identity)
    {
	return false;
    }

    if(_context != r._context)
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
    
    if(_context < r._context)
    {
	return true;
    }
    else if(r._context < _context)
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
				  const Context& ctx, const string& fs, Mode md) :
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

void IceInternal::incRef(IceInternal::FixedReference* p) { p->__incRef(); }
void IceInternal::decRef(IceInternal::FixedReference* p) { p->__decRef(); }

IceInternal::FixedReference::FixedReference(const InstancePtr& inst, const CommunicatorPtr& com, const Identity& ident,
					    const Context& ctx, const string& fs, Mode md,
					    const vector<ConnectionIPtr>& fixedConns) :
    Reference(inst, com, ident, ctx, fs, md),
    _fixedConnections(fixedConns)
{
}

const vector<ConnectionIPtr>&
IceInternal::FixedReference::getFixedConnections() const
{
    return _fixedConnections;
}

bool
IceInternal::FixedReference::getSecure() const
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

ReferencePtr
IceInternal::FixedReference::changeSecure(bool) const
{
    return FixedReferencePtr(const_cast<FixedReference*>(this));
}

ReferencePtr
IceInternal::FixedReference::changeRouter(const RouterPrx&) const
{
    return FixedReferencePtr(const_cast<FixedReference*>(this));
}

ReferencePtr
IceInternal::FixedReference::changeLocator(const LocatorPrx&) const
{
    return FixedReferencePtr(const_cast<FixedReference*>(this));
}

ReferencePtr
IceInternal::FixedReference::changeCollocationOptimization(bool) const
{
    return FixedReferencePtr(const_cast<FixedReference*>(this));
}

ReferencePtr
IceInternal::FixedReference::changeCompress(bool) const
{
    // TODO: FixedReferences should probably have a _compress flag,
    // that gets its default from the fixed connection this reference
    // refers to. This should be changable with changeCompress().
    return FixedReferencePtr(const_cast<FixedReference*>(this));
}

ReferencePtr
IceInternal::FixedReference::changeTimeout(int) const
{
    return FixedReferencePtr(const_cast<FixedReference*>(this));
}

ReferencePtr
IceInternal::FixedReference::changeConnectionId(const string&) const
{
    return FixedReferencePtr(const_cast<FixedReference*>(this));
}

ReferencePtr
IceInternal::FixedReference::changeLocatorCacheTimeout(int) const
{
    return FixedReferencePtr(const_cast<FixedReference*>(this));
}

ReferencePtr
IceInternal::FixedReference::changeAdapterId(const string& newAdapterId) const
{
    return FixedReferencePtr(const_cast<FixedReference*>(this));    
}

ReferencePtr
IceInternal::FixedReference::changeEndpoints(const vector<EndpointIPtr>& newEndpoints) const
{
    return FixedReferencePtr(const_cast<FixedReference*>(this));    
}

ReferencePtr
IceInternal::FixedReference::changeCacheConnection(bool) const
{
    return FixedReferencePtr(const_cast<FixedReference*>(this));    
}

ReferencePtr
IceInternal::FixedReference::changeEndpointSelection(EndpointSelectionType) const
{
    return FixedReferencePtr(const_cast<FixedReference*>(this));
}

void
IceInternal::FixedReference::streamWrite(BasicStream* s) const
{
    throw MarshalException(__FILE__, __LINE__, "Cannot marshal a fixed proxy");
}

string
IceInternal::FixedReference::toString() const
{
    throw MarshalException(__FILE__, __LINE__, "Cannot marshal a fixed proxy");

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
        if(rhs)
        {
            return _fixedConnections < rhs->_fixedConnections;
        }
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
    random_shuffle(connections.begin(), connections.end(), ptr_fun(IceUtil::random));
    
    //
    // If a secure connection is requested, remove all non-secure
    // connections. Otherwise make non-secure connections preferred over
    // secure connections by partitioning the connection vector, so that
    // non-secure connections come first.
    //
    if(getSecure())
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

void IceInternal::incRef(IceInternal::RoutableReference* p) { p->__incRef(); }
void IceInternal::decRef(IceInternal::RoutableReference* p) { p->__decRef(); }

vector<EndpointIPtr>
IceInternal::RoutableReference::getRoutedEndpoints() const
{
    if(_routerInfo)
    {
        //
	// If we route, we send everything to the router's client
	// proxy endpoints.
	//
	ObjectPrx clientProxy = _routerInfo->getClientProxy();
	return clientProxy->__reference()->getEndpoints();
    }
    return vector<EndpointIPtr>();
}

bool
IceInternal::RoutableReference::getSecure() const
{
    return _secure;
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
        if(rhs)
        {
	    if(!_secure && rhs->_secure)
	    {
		return true;
	    }
	    else if(rhs->_secure < _secure)
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
            return _routerInfo < rhs->_routerInfo;
        }
    }
    return false;
}

IceInternal::RoutableReference::RoutableReference(const InstancePtr& inst, const CommunicatorPtr& com,
						  const Identity& ident, const Context& ctx, const string& fs, Mode md,
						  bool sec, const RouterInfoPtr& rtrInfo, bool collocationOpt) :
    Reference(inst, com, ident, ctx, fs, md),
    _secure(sec),
    _routerInfo(rtrInfo),
    _collocationOptimization(collocationOpt),
    _cacheConnection(true),
    _endpointSelection(Random)
{
}

IceInternal::RoutableReference::RoutableReference(const RoutableReference& r) :
    Reference(r),
    _secure(r._secure),
    _routerInfo(r._routerInfo),
    _collocationOptimization(r._collocationOptimization),
    _cacheConnection(r._cacheConnection),
    _endpointSelection(r._endpointSelection)
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
	    random_shuffle(endpoints.begin(), endpoints.end(), ptr_fun(IceUtil::random));
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
    // If a secure connection is requested, remove all non-secure
    // endpoints. Otherwise make non-secure endpoints preferred over
    // secure endpoints by partitioning the endpoint vector, so that
    // non-secure endpoints come first.
    //
    if(getSecure())
    {
	endpoints.erase(remove_if(endpoints.begin(), endpoints.end(), not1(Ice::constMemFun(&EndpointI::secure))),
			endpoints.end());
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
	return factory->create(endpoints, false, comp);
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
		return factory->create(endpoint, p + 1 == endpoints.end(), comp);
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

void IceInternal::incRef(IceInternal::DirectReference* p) { p->__incRef(); }
void IceInternal::decRef(IceInternal::DirectReference* p) { p->__decRef(); }

IceInternal::DirectReference::DirectReference(const InstancePtr& inst, const CommunicatorPtr& com,
					      const Identity& ident, const Context& ctx, const string& fs, Mode md,
					      bool sec, const vector<EndpointIPtr>& endpts,
					      const RouterInfoPtr& rtrInfo, bool collocationOpt) :
    RoutableReference(inst, com, ident, ctx, fs, md, sec, rtrInfo, collocationOpt),
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

int
IceInternal::DirectReference::getLocatorCacheTimeout() const
{
    return 0;
}

ReferencePtr
IceInternal::DirectReference::changeLocator(const LocatorPrx& newLocator) const
{
    if(newLocator)
    {
	LocatorInfoPtr newLocatorInfo = getInstance()->locatorManager()->get(newLocator);
	return getInstance()->referenceFactory()->create(getIdentity(), getContext(), getFacet(), getMode(),
							 getSecure(), "", 0, newLocatorInfo,
							 getCollocationOptimization(),
							 getLocatorCacheTimeout());
    }
    else
    {
	return DirectReferencePtr(const_cast<DirectReference*>(this));
    }
}

ReferencePtr
IceInternal::DirectReference::changeCompress(bool newCompress) const
{
    DirectReferencePtr r = DirectReferencePtr::dynamicCast(getInstance()->referenceFactory()->copy(this));
    vector<EndpointIPtr> newEndpoints;
    vector<EndpointIPtr>::const_iterator p;
    for(p = _endpoints.begin(); p != _endpoints.end(); ++p)
    {
	newEndpoints.push_back((*p)->compress(newCompress));
    }
    r->_endpoints = newEndpoints;
    return r;
}

ReferencePtr
IceInternal::DirectReference::changeTimeout(int newTimeout) const
{
    DirectReferencePtr r = DirectReferencePtr::dynamicCast(getInstance()->referenceFactory()->copy(this));
    vector<EndpointIPtr> newEndpoints;
    vector<EndpointIPtr>::const_iterator p;
    for(p = _endpoints.begin(); p != _endpoints.end(); ++p)
    {
	newEndpoints.push_back((*p)->timeout(newTimeout));
    }
    r->_endpoints = newEndpoints;
    return r;
}

ReferencePtr
IceInternal::DirectReference::changeConnectionId(const string& id) const
{
    DirectReferencePtr r = DirectReferencePtr::dynamicCast(getInstance()->referenceFactory()->copy(this));
    vector<EndpointIPtr> newEndpoints;
    vector<EndpointIPtr>::const_iterator p;
    for(p = _endpoints.begin(); p != _endpoints.end(); ++p)
    {
	newEndpoints.push_back((*p)->connectionId(id));
    }
    r->_endpoints = newEndpoints;
    return r;
}

ReferencePtr
IceInternal::DirectReference::changeAdapterId(const string& newAdapterId) const
{
    if(!newAdapterId.empty())
    {
	LocatorInfoPtr locatorInfo = 
	    getInstance()->locatorManager()->get(getInstance()->referenceFactory()->getDefaultLocator());
	return getInstance()->referenceFactory()->create(getIdentity(), getContext(), getFacet(), getMode(),
							 getSecure(), newAdapterId, getRouterInfo(), locatorInfo,
							 getCollocationOptimization(),
							 getLocatorCacheTimeout());
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
    return r;
}

ReferencePtr
IceInternal::DirectReference::changeLocatorCacheTimeout(int) const
{
    return DirectReferencePtr(const_cast<DirectReference*>(this));
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
    if(endpts.empty())
    {
	endpts = _endpoints;
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
        if(rhs)
        {
            return _endpoints < rhs->_endpoints;
        }
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

void IceInternal::incRef(IceInternal::IndirectReference* p) { p->__incRef(); }
void IceInternal::decRef(IceInternal::IndirectReference* p) { p->__decRef(); }

IceInternal::IndirectReference::IndirectReference(const InstancePtr& inst, const CommunicatorPtr& com,
						  const Identity& ident, const Context& ctx, const string& fs, Mode md,
						  bool sec, const string& adptid, const RouterInfoPtr& rtrInfo,
						  const LocatorInfoPtr& locInfo, bool collocationOpt,
						  int locatorCacheTimeout) :
    RoutableReference(inst, com, ident, ctx, fs, md, sec, rtrInfo, collocationOpt),
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

int
IceInternal::IndirectReference::getLocatorCacheTimeout() const
{
    return _locatorCacheTimeout;
}

ReferencePtr
IceInternal::IndirectReference::changeLocator(const LocatorPrx& newLocator) const
{
    //
    // Return a direct reference if a null locator is given.
    //
    if(!newLocator)
    {
	return getInstance()->referenceFactory()->create(getIdentity(), getContext(), getFacet(), getMode(),
							 getSecure(), vector<EndpointIPtr>(), getRouterInfo(),
							 getCollocationOptimization());
    }
    else
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
}

ReferencePtr
IceInternal::IndirectReference::changeCompress(bool newCompress) const
{
    IndirectReferencePtr r = IndirectReferencePtr::dynamicCast(getInstance()->referenceFactory()->copy(this));
    if(_locatorInfo)
    {
	LocatorPrx newLocator = LocatorPrx::uncheckedCast(_locatorInfo->getLocator()->ice_compress(newCompress));
	r->_locatorInfo = getInstance()->locatorManager()->get(newLocator);
    }
    return r;
}

ReferencePtr
IceInternal::IndirectReference::changeTimeout(int newTimeout) const
{
    IndirectReferencePtr r = IndirectReferencePtr::dynamicCast(getInstance()->referenceFactory()->copy(this));
    if(_locatorInfo)
    {
	LocatorPrx newLocator = LocatorPrx::uncheckedCast(_locatorInfo->getLocator()->ice_timeout(newTimeout));
	r->_locatorInfo = getInstance()->locatorManager()->get(newLocator);
    }
    return r;
}

ReferencePtr
IceInternal::IndirectReference::changeConnectionId(const string& id) const
{
    if(id == _connectionId)
    {
	return IndirectReferencePtr(const_cast<IndirectReference*>(this));
    }
    IndirectReferencePtr r = IndirectReferencePtr::dynamicCast(getInstance()->referenceFactory()->copy(this));
    r->_connectionId = id;
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
							 getSecure(), newEndpoints, getRouterInfo(),
							 getCollocationOptimization());
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
    string a = IceUtil::escapeString(_adapterId, "");
    if(a.find_first_of(" \t\n\r") != string::npos)
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

	//
	// Apply the cached connection id to each endpoint.
	//
	for(vector<EndpointIPtr>::iterator p = endpts.begin(); p != endpts.end(); ++p)
	{
	    *p = (*p)->connectionId(_connectionId);
	}

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
		const IndirectReferencePtr self = const_cast<IndirectReference*>(this);
		_locatorInfo->clearCache(self);

		if(cached)
		{
		    TraceLevelsPtr traceLevels = getInstance()->traceLevels();
		    LoggerPtr logger = getInstance()->logger();
		    if(traceLevels->retry >= 2)
		    {
		        Trace out(logger, traceLevels->retryCat);
			out << "connection to cached endpoints failed\n"
			    << "removing endpoints from cache and trying one more time\n" << ex;
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

    return _adapterId == rhs->_adapterId && _connectionId == rhs->_connectionId && _locatorInfo == rhs->_locatorInfo &&
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
        if(rhs)
        {
            if(_adapterId < rhs->_adapterId)
            {
                return true;
            }
            else if(rhs->_adapterId < _adapterId)
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
    _connectionId(r._connectionId),
    _locatorInfo(r._locatorInfo),
    _locatorCacheTimeout(r._locatorCacheTimeout)
{
}
