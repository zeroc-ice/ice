// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
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
#include <Ice/Endpoint.h>
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
#include <IceUtil/StringUtil.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(IceInternal::Reference* p) { p->__incRef(); }
void IceInternal::decRef(IceInternal::Reference* p) { p->__decRef(); }

ReferencePtr
IceInternal::Reference::changeMode(Mode newMode) const
{
    if(newMode == mode)
    {
	return ReferencePtr(const_cast<Reference*>(this));
    }
    ReferencePtr r = instance->referenceFactory()->clone(this);
    r->mode = newMode;
    return r;
}

ReferencePtr
IceInternal::Reference::changeIdentity(const Identity& newIdentity) const
{
    if(newIdentity == identity)
    {
	return ReferencePtr(const_cast<Reference*>(this));
    }
    ReferencePtr r = instance->referenceFactory()->clone(this);
    r->identity = newIdentity;
    return r;
}

ReferencePtr
IceInternal::Reference::changeContext(const Context& newContext) const
{
    if(newContext == context)
    {
	return ReferencePtr(const_cast<Reference*>(this));
    }
    ReferencePtr r = instance->referenceFactory()->clone(this);
    r->context = newContext;
    return r;
}

ReferencePtr
IceInternal::Reference::changeFacet(const string& newFacet) const
{
    if(newFacet == facet)
    {
	return ReferencePtr(const_cast<Reference*>(this));
    }
    ReferencePtr r = instance->referenceFactory()->clone(this);
    r->facet = newFacet;
    return r;
}

ReferencePtr
IceInternal::Reference::changeSecure(bool newSecure) const
{
    if(newSecure == secure)
    {
	return ReferencePtr(const_cast<Reference*>(this));
    }
    ReferencePtr r = instance->referenceFactory()->clone(this);
    r->secure = newSecure;
    return r;
}

ReferencePtr
IceInternal::Reference::changeCollocationOptimization(bool newCollocationOptimization) const
{
    if(newCollocationOptimization == collocationOptimization)
    {
	return ReferencePtr(const_cast<Reference*>(this));
    }
    ReferencePtr r = instance->referenceFactory()->clone(this);
    r->collocationOptimization = newCollocationOptimization;
    return r;
}

ReferencePtr
IceInternal::Reference::changeRouter(const RouterPrx&) const
{
    return ReferencePtr(const_cast<Reference*>(this));
}

ReferencePtr
IceInternal::Reference::changeLocator(const LocatorPrx&) const
{
    return ReferencePtr(const_cast<Reference*>(this));
}

ReferencePtr
IceInternal::Reference::changeDefault() const
{
    return ReferencePtr(const_cast<Reference*>(this));
}

ReferencePtr
IceInternal::Reference::changeCompress(bool) const
{
    return ReferencePtr(const_cast<Reference*>(this));
}

ReferencePtr
IceInternal::Reference::changeTimeout(int) const
{
    return ReferencePtr(const_cast<Reference*>(this));
}

Int
Reference::hash() const
{
    IceUtil::Mutex::Lock sync(hashMutex);

    if(hashInitialized)
    {
        return hashValue;
    }

    string::const_iterator p;
    Context::const_iterator q;

    Int h = static_cast<Int>(mode);

    for(p = identity.name.begin(); p != identity.name.end(); ++p)
    {
        h = 5 * h + *p;
    }

    for(p = identity.category.begin(); p != identity.category.end(); ++p)
    {
        h = 5 * h + *p;
    }

    for(q = context.begin(); q != context.end(); ++q)
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

    for(p = facet.begin(); p != facet.end(); ++p)
    {
	h = 5 * h + *p;
    }

    h = 5 * h + static_cast<Int>(secure);

    hashValue = h;
    hashInitialized = true;

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
    if(facet.empty())
    {
	s->write(vector<string>());
    }
    else
    {
	vector<string> facetPath;
	facetPath.push_back(facet);
	s->write(facetPath);
    }
    
    s->write(static_cast<Byte>(mode));
    
    s->write(secure);

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
    string id = identityToString(identity);
    if(id.find_first_of(" \t\n\r:@") != string::npos)
    {
        s << '"' << id << '"';
    }
    else
    {
        s << id;
    }

    if(!facet.empty())
    {
        s << " -f ";

	//  
	// If the encoded facet string contains characters which
	// the reference parser uses as separators, then we enclose
	// the facet string in quotes.
	//
	string fs = IceUtil::escapeString(facet, "");
	if(fs.find_first_of(" \t\n\r:@") != string::npos)
	{
	    s << '"' << fs << '"';
	}
	else
	{
	    s << fs;
	}
    }

    switch(mode)
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

    if(secure)
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
    
    if(mode != r.mode)
    {
	return false;
    }

    if(identity != r.identity)
    {
	return false;
    }

    if(context != r.context)
    {
	return false;
    }

    if(facet != r.facet)
    {
	return false;
    }

    if(secure != r.secure)
    {
	return false;
    }

    if(collocationOptimization != r.collocationOptimization)
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
    
    if(mode < r.mode)
    {
	return true;
    }
    else if(r.mode < mode)
    {
	return false;
    }
    
    if(identity < r.identity)
    {
	return true;
    }
    else if(r.identity < identity)
    {
	return false;
    }
    
    if(context < r.context)
    {
	return true;
    }
    else if(r.context < context)
    {
	return false;
    }

    if(facet < r.facet)
    {
	return true;
    }
    else if(r.facet < facet)
    {
	return false;
    }

    if(!secure && r.secure)
    {
	return true;
    }
    else if(r.secure < secure)
    {
	return false;
    }
    
    if(!collocationOptimization && r.collocationOptimization)
    {
	return true;
    }
    else if(r.collocationOptimization < collocationOptimization)
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

vector<ConnectionIPtr>
IceInternal::Reference::filterConnections(const vector<ConnectionIPtr>& allConnections) const
{
    vector<ConnectionIPtr> connections = allConnections;

    switch(mode)
    {
	case ModeTwoway:
	case ModeOneway:
	case ModeBatchOneway:
	{
	    //
	    // Filter out datagram connections.
	    //
            connections.erase(remove_if(connections.begin(), connections.end(), ConnectionIsDatagram()),
			      connections.end());
	    break;
	}
	
	case ModeDatagram:
	case ModeBatchDatagram:
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
    random_shuffle(connections.begin(), connections.end());
    
    //
    // If a secure connection is requested, remove all non-secure
    // connections. Otherwise make non-secure connections preferred over
    // secure connections by partitioning the connection vector, so that
    // non-secure connections come first.
    //
    if(secure)
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

IceInternal::Reference::Reference(const InstancePtr& inst, const Ice::Identity& ident, const Ice::Context& ctx,
                                  const std::string& fs, Mode md, bool sec, bool collocationOpt)
    : instance(inst),
      mode(md),
      identity(ident),
      context(ctx),
      facet(fs),
      secure(sec),
      collocationOptimization(collocationOpt),
      hashInitialized(false)
{
}

IceInternal::Reference::Reference(const Reference& r)
{
    instance = r.instance;
    mode = r.mode;
    identity = r.identity;
    context = r.context;
    facet = r.facet;
    secure = r.secure;
    collocationOptimization = r.collocationOptimization;
    hashInitialized = false;
}

void IceInternal::incRef(IceInternal::FixedReference* p) { p->__incRef(); }
void IceInternal::decRef(IceInternal::FixedReference* p) { p->__decRef(); }

IceInternal::FixedReference::FixedReference(const InstancePtr& inst, const Ice::Identity& ident,
						  const Ice::Context& ctx, const std::string& fs, Mode md,
						  bool sec, bool collocationOpt,
						  const vector<Ice::ConnectionIPtr>& fixedConns)
    : Reference(inst, ident, ctx, fs, md, sec, collocationOpt),
      fixedConnections(fixedConns)
{
}

vector<EndpointPtr>
IceInternal::FixedReference::getEndpoints() const
{
    return vector<EndpointPtr>();
}

void
IceInternal::FixedReference::streamWrite(BasicStream* s) const
{
    MarshalException ex(__FILE__, __LINE__);
    ex.reason = "Cannot marshal a fixed reference";
    throw ex;
}

string
IceInternal::FixedReference::toString() const
{
    MarshalException ex(__FILE__, __LINE__);
    ex.reason = "Cannot stringify a fixed reference";
    throw ex;
}

ConnectionIPtr
IceInternal::FixedReference::getConnection(bool& compress) const
{
    vector<ConnectionIPtr> filteredConns = filterConnections(fixedConnections);
    if(filteredConns.empty())
    {
	NoEndpointException ex(__FILE__, __LINE__);
	ex.proxy = toString();
	throw ex;
    }

    ConnectionIPtr connection = filteredConns[0];
    assert(connection);
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
    return fixedConnections == rhs->fixedConnections;
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
    const FixedReference* rhs = dynamic_cast<const FixedReference*>(&r);
    if(!rhs || !Reference::operator<(r))
    {
        return false;
    }
    return fixedConnections < rhs->fixedConnections;
}

ReferencePtr
IceInternal::FixedReference::clone() const
{
    return new FixedReference(*this);
}

IceInternal::FixedReference::FixedReference(const FixedReference& r)
    : Reference(r)
{
    fixedConnections = r.fixedConnections;
}

void IceInternal::incRef(IceInternal::RoutableReference* p) { p->__incRef(); }
void IceInternal::decRef(IceInternal::RoutableReference* p) { p->__decRef(); }

std::vector<EndpointPtr>
IceInternal::RoutableReference::getRoutedEndpoints() const
{
    if(routerInfo)
    {
        //
	// If we route, we send everything to the router's client
	// proxy endpoints.
	//
	ObjectPrx clientProxy = routerInfo->getClientProxy();
	return clientProxy->__reference()->getEndpoints();
    }
    return vector<EndpointPtr>();
}

ReferencePtr
IceInternal::RoutableReference::changeRouter(const RouterPrx& newRouter) const
{
    RouterInfoPtr newRouterInfo = getInstance()->routerManager()->get(newRouter);
    if(newRouterInfo == routerInfo)
    {
	return RoutableReferencePtr(const_cast<RoutableReference*>(this));
    }
    RoutableReferencePtr r = RoutableReferencePtr::dynamicCast(getInstance()->referenceFactory()->clone(this));
    r->routerInfo = newRouterInfo;
    return r;
}

ReferencePtr
IceInternal::RoutableReference::changeDefault() const
{
    RoutableReferencePtr r = RoutableReferencePtr::dynamicCast(getInstance()->referenceFactory()->clone(this));
    r->routerInfo = getInstance()->routerManager()->get(getInstance()->referenceFactory()->getDefaultRouter());
    return r;
}

ReferencePtr
IceInternal::RoutableReference::changeCompress(bool newCompress) const
{
    return RoutableReferencePtr(const_cast<RoutableReference*>(this));
}

ReferencePtr
IceInternal::RoutableReference::changeTimeout(int newTimeout) const
{
    return RoutableReferencePtr(const_cast<RoutableReference*>(this));
}

void
IceInternal::RoutableReference::streamWrite(BasicStream* s) const
{
    Reference::streamWrite(s);
}

string
IceInternal::RoutableReference::toString() const
{
    return Reference::toString();
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
    return routerInfo == rhs->routerInfo;
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
    const RoutableReference* rhs = dynamic_cast<const RoutableReference*>(&r);
    if(!rhs || !Reference::operator<(r))
    {
        return false;
    }
    return routerInfo < rhs->routerInfo;
}

IceInternal::RoutableReference::RoutableReference(const InstancePtr& inst, const Ice::Identity& ident,
						  const Ice::Context& ctx, const std::string& fs, Mode md,
						  bool sec, const RouterInfoPtr& rtrInfo, bool collocationOpt)
    : Reference(inst, ident, ctx, fs, md, sec, collocationOpt),
      routerInfo(rtrInfo)
{
}

IceInternal::RoutableReference::RoutableReference(const RoutableReference& r)
    : Reference(r)
{
    routerInfo = r.routerInfo;
}

void IceInternal::incRef(IceInternal::DirectReference* p) { p->__incRef(); }
void IceInternal::decRef(IceInternal::DirectReference* p) { p->__decRef(); }

IceInternal::DirectReference::DirectReference(const InstancePtr& inst, const Ice::Identity& ident,
					      const Ice::Context& ctx, const std::string& fs, Mode md,
					      bool sec, const std::vector<EndpointPtr>& endpts,
					      const RouterInfoPtr& rtrInfo, bool collocationOpt)
    : RoutableReference(inst, ident, ctx, fs, md, sec, rtrInfo, collocationOpt),
      endpoints(endpts)
{
}

vector<EndpointPtr>
IceInternal::DirectReference::getEndpoints() const
{
    return endpoints;
}

DirectReferencePtr
IceInternal::DirectReference::changeEndpoints(const vector<EndpointPtr>& newEndpoints) const
{
    if(newEndpoints == endpoints)
    {
	return DirectReferencePtr(const_cast<DirectReference*>(this));
    }
    DirectReferencePtr r = DirectReferencePtr::dynamicCast(getInstance()->referenceFactory()->clone(this));
    r->endpoints = newEndpoints;
    return r;
}

ReferencePtr
IceInternal::DirectReference::changeCompress(bool newCompress) const
{
    DirectReferencePtr r = DirectReferencePtr::dynamicCast(RoutableReference::changeCompress(newCompress));
    vector<EndpointPtr> newEndpoints;
    vector<EndpointPtr>::const_iterator p;
    for(p = endpoints.begin(); p != endpoints.end(); ++p)
    {
	newEndpoints.push_back((*p)->compress(newCompress));
    }
    r->endpoints = newEndpoints;
    return r;
}

ReferencePtr
IceInternal::DirectReference::changeTimeout(int newTimeout) const
{
    DirectReferencePtr r = DirectReferencePtr::dynamicCast(RoutableReference::changeTimeout(newTimeout));
    vector<EndpointPtr> newEndpoints;
    vector<EndpointPtr>::const_iterator p;
    for(p = endpoints.begin(); p != endpoints.end(); ++p)
    {
	newEndpoints.push_back((*p)->timeout(newTimeout));
    }
    r->endpoints = newEndpoints;
    return r;
}

void
IceInternal::DirectReference::streamWrite(BasicStream* s) const
{
    RoutableReference::streamWrite(s);

    s->writeSize(Ice::Int(endpoints.size()));
    vector<EndpointPtr>::const_iterator p;
    for(p = endpoints.begin(); p != endpoints.end(); ++p)
    {
	(*p)->streamWrite(s);
    }
}

string
IceInternal::DirectReference::toString() const
{
    string result = RoutableReference::toString();

    vector<EndpointPtr>::const_iterator p;
    for(p = endpoints.begin(); p != endpoints.end(); ++p)
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
    vector<EndpointPtr> endpts = RoutableReference::getRoutedEndpoints();
    if(endpts.empty())
    {
	endpts = endpoints;
    }
    vector<EndpointPtr> filteredEndpoints = filterEndpoints(endpts, getMode(), getSecure());
    if(filteredEndpoints.empty())
    {
        NoEndpointException ex(__FILE__, __LINE__);
	ex.proxy = toString();
	throw ex;
    }

    OutgoingConnectionFactoryPtr factory = getInstance()->outgoingConnectionFactory();
    ConnectionIPtr connection = factory->create(filteredEndpoints, comp);
    assert(connection);

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
    return endpoints == rhs->endpoints;
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
    const DirectReference* rhs = dynamic_cast<const DirectReference*>(&r);
    if(!rhs || !RoutableReference::operator<(r))
    {
        return false;
    }
    return endpoints < rhs->endpoints;
}

ReferencePtr
IceInternal::DirectReference::clone() const
{
    return new DirectReference(*this);
}

IceInternal::DirectReference::DirectReference(const DirectReference& r)
    : RoutableReference(r)
{
    endpoints = r.endpoints;
}


void IceInternal::incRef(IceInternal::IndirectReference* p) { p->__incRef(); }
void IceInternal::decRef(IceInternal::IndirectReference* p) { p->__decRef(); }

IceInternal::IndirectReference::IndirectReference(const InstancePtr& inst, const Ice::Identity& ident,
                                                  const Ice::Context& ctx, const std::string& fs, Mode md,
						  bool sec, const string& adptid, const RouterInfoPtr& rtrInfo,
						  const LocatorInfoPtr& locInfo, bool collocationOpt)
    : RoutableReference(inst, ident, ctx, fs, md, sec, rtrInfo, collocationOpt),
      adapterId(adptid),
      locatorInfo(locInfo)
{
}

vector<EndpointPtr>
IceInternal::IndirectReference::getEndpoints() const
{
    return vector<EndpointPtr>();
}

IndirectReferencePtr
IceInternal::IndirectReference::changeAdapterId(const string& newAdapterId) const
{
    if(newAdapterId == adapterId)
    {
	return IndirectReferencePtr(const_cast<IndirectReference*>(this));
    }
    IndirectReferencePtr r = IndirectReferencePtr::dynamicCast(getInstance()->referenceFactory()->clone(this));
    r->adapterId = adapterId;
    return r;
}

ReferencePtr
IceInternal::IndirectReference::changeLocator(const LocatorPrx& newLocator) const
{
    LocatorInfoPtr newLocatorInfo = getInstance()->locatorManager()->get(newLocator);

    if(newLocatorInfo == locatorInfo)
    {
	return IndirectReferencePtr(const_cast<IndirectReference*>(this));
    }
    IndirectReferencePtr r = IndirectReferencePtr::dynamicCast(getInstance()->referenceFactory()->clone(this));
    r->locatorInfo = newLocatorInfo;
    return r;
}

ReferencePtr
IceInternal::IndirectReference::changeDefault() const
{
    IndirectReferencePtr r = IndirectReferencePtr::dynamicCast(RoutableReference::changeDefault());
    r->locatorInfo = getInstance()->locatorManager()->get(getInstance()->referenceFactory()->getDefaultLocator());
    return r;
}

ReferencePtr
IceInternal::IndirectReference::changeCompress(bool newCompress) const
{
    IndirectReferencePtr r = IndirectReferencePtr::dynamicCast(RoutableReference::changeCompress(newCompress));
    if(locatorInfo)
    {
	LocatorPrx newLocator = LocatorPrx::uncheckedCast(locatorInfo->getLocator()->ice_compress(newCompress));
	r->locatorInfo = getInstance()->locatorManager()->get(newLocator);
    }
    return r;
}

ReferencePtr
IceInternal::IndirectReference::changeTimeout(int newTimeout) const
{
    IndirectReferencePtr r = IndirectReferencePtr::dynamicCast(RoutableReference::changeTimeout(newTimeout));
    LocatorInfoPtr newLocatorInfo;
    if(locatorInfo)
    {
	LocatorPrx newLocator = LocatorPrx::uncheckedCast(locatorInfo->getLocator()->ice_timeout(newTimeout));
	r->locatorInfo = getInstance()->locatorManager()->get(newLocator);
    }
    return r;
}

void
IceInternal::IndirectReference::streamWrite(BasicStream* s) const
{
    RoutableReference::streamWrite(s);

    s->writeSize(0);
    s->write(adapterId);
}

string
IceInternal::IndirectReference::toString() const
{
    string result = RoutableReference::toString();
    if(adapterId.empty())
    {
        return result;
    }

    result.append(" @ ");
    //
    // If the encoded adapter id string contains characters which
    // the reference parser uses as separators, then we enclose
    // the adapter id string in quotes.
    //
    string a = IceUtil::escapeString(adapterId, "");
    if(a.find_first_of(" \t\n\r") != string::npos)
    {
	result.append("\"");
	result.append(a);
	result.append("\"");
    }
    else
    {
	result.append(adapterId);
    }
    return result;
}

ConnectionIPtr
IceInternal::IndirectReference::getConnection(bool& comp) const
{
    ConnectionIPtr connection;

    while(true)
    {
	vector<EndpointPtr> endpts = RoutableReference::getRoutedEndpoints();
	bool cached;
	if(endpts.empty() && locatorInfo)
	{
	    const IndirectReferencePtr self = const_cast<IndirectReference*>(this);
	    endpts = locatorInfo->getEndpoints(self, cached);
	}
	vector<EndpointPtr> filteredEndpoints = filterEndpoints(endpts, getMode(), getSecure());
	if(filteredEndpoints.empty())
	{
	    NoEndpointException ex(__FILE__, __LINE__);
	    ex.proxy = toString();
	    throw ex;
	}

	try
	{
	    OutgoingConnectionFactoryPtr factory = getInstance()->outgoingConnectionFactory();
	    connection = factory->create(filteredEndpoints, comp);
	    assert(connection);
	}
	catch(const LocalException& ex)
	{
	    if(!getRouterInfo())
	    {
	        assert(locatorInfo);
		const IndirectReferencePtr self = const_cast<IndirectReference*>(this);
		locatorInfo->clearCache(self);

		if(cached)
		{
		    TraceLevelsPtr traceLevels = getInstance()->traceLevels();
		    LoggerPtr logger = getInstance()->logger();
		    if(traceLevels->retry >=2)
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
    return adapterId == rhs->adapterId && locatorInfo == rhs->locatorInfo;
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
    const IndirectReference* rhs = dynamic_cast<const IndirectReference*>(&r);
    if(!rhs || !RoutableReference::operator<(r))
    {
        return false;
    }
    return adapterId < rhs->adapterId && locatorInfo < rhs->locatorInfo;
}

ReferencePtr
IceInternal::IndirectReference::clone() const
{
    return new IndirectReference(*this);
}

IceInternal::IndirectReference::IndirectReference(const IndirectReference& r)
    : RoutableReference(r)
{
    adapterId = r.adapterId;
    locatorInfo = r.locatorInfo;
}

vector<EndpointPtr>
IceInternal::filterEndpoints(const vector<EndpointPtr>& allEndpoints, Reference::Mode m, bool sec)
{
    vector<EndpointPtr> endpoints = allEndpoints;

    //
    // Filter out unknown endpoints.
    //
    endpoints.erase(remove_if(endpoints.begin(), endpoints.end(), Ice::constMemFun(&Endpoint::unknown)),
                    endpoints.end());

    switch(m)
    {
	case Reference::ModeTwoway:
	case Reference::ModeOneway:
	case Reference::ModeBatchOneway:
	{
	    //
	    // Filter out datagram endpoints.
	    //
            endpoints.erase(remove_if(endpoints.begin(), endpoints.end(), Ice::constMemFun(&Endpoint::datagram)),
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
                                      not1(Ice::constMemFun(&Endpoint::datagram))),
                            endpoints.end());
	    break;
	}
    }
    
    //
    // Randomize the order of endpoints.
    //
    random_shuffle(endpoints.begin(), endpoints.end());
    
    //
    // If a secure connection is requested, remove all non-secure
    // endpoints. Otherwise make non-secure endpoints preferred over
    // secure endpoints by partitioning the endpoint vector, so that
    // non-secure endpoints come first.
    //
    if(sec)
    {
	endpoints.erase(remove_if(endpoints.begin(), endpoints.end(), not1(Ice::constMemFun(&Endpoint::secure))),
			endpoints.end());
    }
    else
    {
	//
	// We must use stable_partition() instead of just simply
	// partition(), because otherwise some STL implementations
	// order our now randomized endpoints.
	//
	stable_partition(endpoints.begin(), endpoints.end(), not1(Ice::constMemFun(&Endpoint::secure)));
    }
    
    return endpoints;
}

