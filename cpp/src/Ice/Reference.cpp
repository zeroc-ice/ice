// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
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
#include <Ice/ObjectAdapterI.h> // For getIncomingConnections().
#include <Ice/Connection.h>
#include <Ice/ConnectionFactory.h>
#include <Ice/LoggerUtil.h>
#include <Ice/TraceLevels.h>
#include <IceUtil/StringUtil.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(::IceInternal::Reference* p) { p->__incRef(); }
void IceInternal::decRef(::IceInternal::Reference* p) { p->__decRef(); }

bool
IceInternal::Reference::operator==(const Reference& r) const
{
    if(this == &r)
    {
	return true;
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

    if(mode != r.mode)
    {
	return false;
    }

    if(secure != r.secure)
    {
	return false;
    }

    if(adapterId != r.adapterId)
    {
	return false;
    }

    if(endpoints != r.endpoints)
    {
	return false;
    }

    if(routerInfo != r.routerInfo)
    {
	return false;
    }

    if(locatorInfo != r.locatorInfo)
    {
	return false;
    }

    if(reverseAdapter != r.reverseAdapter)
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
    if(this == &r)
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

    if(mode < r.mode)
    {
	return true;
    }
    else if(r.mode < mode)
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
    
    if(adapterId < r.adapterId)
    {
	return true;
    }
    else if(r.adapterId < adapterId)
    {
	return false;
    }
    
    if(endpoints < r.endpoints)
    {
	return true;
    }
    else if(r.endpoints < endpoints)
    {
	return false;
    }
    
    if(routerInfo < r.routerInfo)
    {
	return true;
    }
    else if(r.routerInfo < routerInfo)
    {
	return false;
    }
    
    if(locatorInfo < r.locatorInfo)
    {
	return true;
    }
    else if(r.locatorInfo < locatorInfo)
    {
	return false;
    }
    
    if(reverseAdapter < r.reverseAdapter)
    {
	return true;
    }
    else if(r.reverseAdapter < reverseAdapter)
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

    s->writeSize(Ice::Int(endpoints.size()));

    if(!endpoints.empty())
    {
	assert(adapterId.empty());

	vector<EndpointPtr>::const_iterator p;

	for(p = endpoints.begin(); p != endpoints.end(); ++p)
	{
	    (*p)->streamWrite(s);
	}
    }
    else
    {
	s->write(adapterId);
    }
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

    if(!endpoints.empty())
    {
	assert(adapterId.empty());

	vector<EndpointPtr>::const_iterator p;
	for(p = endpoints.begin(); p != endpoints.end(); ++p)
	{
	    string endp = (*p)->toString();
	    if(!endp.empty())
	    {
		s << ':' << endp;
	    }
	}
    }
    else if(!adapterId.empty())
    {
        string a = IceUtil::escapeString(adapterId, "");
        //
        // If the encoded adapter id string contains characters which
        // the reference parser uses as separators, then we enclose
        // the adapter id string in quotes.
        //
        s << " @ ";
        if(a.find_first_of(" \t\n\r") != string::npos)
        {
            s << '"' << a << '"';
        }
        else
        {
            s << adapterId;
        }
    }
    
    return s.str();
}

ReferencePtr
IceInternal::Reference::changeIdentity(const Identity& newIdentity) const
{
    if(newIdentity == identity)
    {
	return ReferencePtr(const_cast<Reference*>(this));
    }
    else
    {
	return instance->referenceFactory()->create(newIdentity, context, facet, mode, secure, adapterId,
						    endpoints, routerInfo, locatorInfo, reverseAdapter,
						    collocationOptimization);
    }
}

ReferencePtr
IceInternal::Reference::changeContext(const Context& newContext) const
{
    if(newContext == context)
    {
	return ReferencePtr(const_cast<Reference*>(this));
    }
    else
    {
	return instance->referenceFactory()->create(identity, newContext, facet, mode, secure, adapterId,
						    endpoints, routerInfo, locatorInfo, reverseAdapter,
						    collocationOptimization);
    }
}

ReferencePtr
IceInternal::Reference::changeFacet(const string& newFacet) const
{
    if(newFacet == facet)
    {
	return ReferencePtr(const_cast<Reference*>(this));
    }
    else
    {
	return instance->referenceFactory()->create(identity, context, newFacet, mode, secure, adapterId,
						    endpoints, routerInfo, locatorInfo, reverseAdapter,
						    collocationOptimization);
    }
}

ReferencePtr
IceInternal::Reference::changeTimeout(int newTimeout) const
{
    //
    // We change the timeout settings in all endpoints.
    //
    vector<EndpointPtr>::const_iterator p;

    vector<EndpointPtr> newEndpoints;
    for(p = endpoints.begin(); p != endpoints.end(); ++p)
    {
	newEndpoints.push_back((*p)->timeout(newTimeout));
    }
    
    //
    // If we have a router, we also change the timeout settings on the
    // router and the router's client proxy.
    //
    RouterInfoPtr newRouterInfo;
    if(routerInfo)
    {
	try
	{
	    RouterPrx newRouter = RouterPrx::uncheckedCast(routerInfo->getRouter()->ice_timeout(newTimeout));
	    ObjectPrx newClientProxy = routerInfo->getClientProxy()->ice_timeout(newTimeout);
	    newRouterInfo = instance->routerManager()->get(newRouter);
	    newRouterInfo->setClientProxy(newClientProxy);
	}
	catch(const NoEndpointException&)
	{
	    // Ignore non-existing client proxies.
	}
    }

    //
    // If we have a locator, we also change the timeout settings on the
    // locator.
    //
    LocatorInfoPtr newLocatorInfo;
    if(locatorInfo)
    {
	LocatorPrx newLocator = LocatorPrx::uncheckedCast(locatorInfo->getLocator()->ice_timeout(newTimeout));
	newLocatorInfo = instance->locatorManager()->get(newLocator);
    }

    return instance->referenceFactory()->create(identity, context, facet, mode, secure, adapterId,
						newEndpoints, newRouterInfo, newLocatorInfo, reverseAdapter,
						collocationOptimization);
}

ReferencePtr
IceInternal::Reference::changeMode(Mode newMode) const
{
    if(newMode == mode)
    {
	return ReferencePtr(const_cast<Reference*>(this));
    }
    else
    {
	return instance->referenceFactory()->create(identity, context, facet, newMode, secure, adapterId,
						    endpoints, routerInfo, locatorInfo, reverseAdapter,
						    collocationOptimization);
    }
}

ReferencePtr
IceInternal::Reference::changeSecure(bool newSecure) const
{
    if(newSecure == secure)
    {
	return ReferencePtr(const_cast<Reference*>(this));
    }
    else
    {
	return instance->referenceFactory()->create(identity, context, facet, mode, newSecure, adapterId,
						    endpoints, routerInfo, locatorInfo, reverseAdapter,
						    collocationOptimization);
    }
}

ReferencePtr
IceInternal::Reference::changeCompress(bool newCompress) const
{
    //
    // We change the compress settings in all endpoints.
    //
    vector<EndpointPtr>::const_iterator p;

    vector<EndpointPtr> newEndpoints;
    for(p = endpoints.begin(); p != endpoints.end(); ++p)
    {
	newEndpoints.push_back((*p)->compress(newCompress));
    }
    
    //
    // If we have a router, we also change the compress settings on the
    // router and the router's client proxy.
    //
    RouterInfoPtr newRouterInfo;
    if(routerInfo)
    {
	try
	{
	    RouterPrx newRouter = RouterPrx::uncheckedCast(routerInfo->getRouter()->ice_compress(newCompress));
	    ObjectPrx newClientProxy = routerInfo->getClientProxy()->ice_compress(newCompress);
	    newRouterInfo = instance->routerManager()->get(newRouter);
	    newRouterInfo->setClientProxy(newClientProxy);
	}
	catch(const NoEndpointException&)
	{
	    // Ignore non-existing client proxies.
	}
    }

    //
    // If we have a locator, we also change the compress settings on the
    // locator.
    //
    LocatorInfoPtr newLocatorInfo;
    if(locatorInfo)
    {
	LocatorPrx newLocator = LocatorPrx::uncheckedCast(locatorInfo->getLocator()->ice_compress(newCompress));
	newLocatorInfo = instance->locatorManager()->get(newLocator);
    }

    return instance->referenceFactory()->create(identity, context, facet, mode, secure, adapterId,
						newEndpoints, newRouterInfo, newLocatorInfo, reverseAdapter,
						collocationOptimization);
}

ReferencePtr
IceInternal::Reference::changeAdapterId(const string& newAdapterId) const
{
    if(newAdapterId == adapterId)
    {
	return ReferencePtr(const_cast<Reference*>(this));
    }
    else
    {
	return instance->referenceFactory()->create(identity, context, facet, mode, secure, newAdapterId,
						    endpoints, routerInfo, locatorInfo, reverseAdapter,
						    collocationOptimization);
    }
}

ReferencePtr
IceInternal::Reference::changeEndpoints(const vector<EndpointPtr>& newEndpoints) const
{
    if(newEndpoints == endpoints)
    {
	return ReferencePtr(const_cast<Reference*>(this));
    }
    else
    {
	return instance->referenceFactory()->create(identity, context, facet, mode, secure, adapterId,
						    newEndpoints, routerInfo, locatorInfo, reverseAdapter,
						    collocationOptimization);
    }
}

ReferencePtr
IceInternal::Reference::changeRouter(const RouterPrx& newRouter) const
{
    RouterInfoPtr newRouterInfo = instance->routerManager()->get(newRouter);

    if(newRouterInfo == routerInfo)
    {
	return ReferencePtr(const_cast<Reference*>(this));
    }
    else
    {
	return instance->referenceFactory()->create(identity, context, facet, mode, secure, adapterId,
						    endpoints, newRouterInfo, locatorInfo, reverseAdapter,
						    collocationOptimization);
    }
}

ReferencePtr
IceInternal::Reference::changeLocator(const LocatorPrx& newLocator) const
{
    LocatorInfoPtr newLocatorInfo = instance->locatorManager()->get(newLocator);

    if(newLocatorInfo == locatorInfo)
    {
	return ReferencePtr(const_cast<Reference*>(this));
    }
    else
    {
	return instance->referenceFactory()->create(identity, context, facet, mode, secure, adapterId,
						    endpoints, routerInfo, newLocatorInfo, reverseAdapter,
						    collocationOptimization);
    }
}

ReferencePtr
IceInternal::Reference::changeCollocationOptimization(bool newCollocationOptimization) const
{
    if(newCollocationOptimization == collocationOptimization)
    {
	return ReferencePtr(const_cast<Reference*>(this));
    }
    else
    {
	return instance->referenceFactory()->create(identity, context, facet, mode, secure, adapterId,
						    endpoints, routerInfo, locatorInfo, reverseAdapter,
						    newCollocationOptimization);
    }
}

ReferencePtr
IceInternal::Reference::changeDefault() const
{
    RouterInfoPtr defaultRouterInfo = instance->routerManager()->
	get(instance->referenceFactory()->getDefaultRouter());
    LocatorInfoPtr defaultLocatorInfo = instance->locatorManager()->
	get(instance->referenceFactory()->getDefaultLocator());

    return instance->referenceFactory()->create(identity, context, "", ModeTwoway, false, adapterId,
						endpoints, defaultRouterInfo, defaultLocatorInfo, 0, true);
}

ConnectionPtr
IceInternal::Reference::getConnection() const
{
    ConnectionPtr connection;

    if(reverseAdapter)
    {
	//
	// If we have a reverse object adapter, we use the incoming
	// connections from such object adapter.
	//
	ObjectAdapterIPtr adapter = ObjectAdapterIPtr::dynamicCast(reverseAdapter);
	assert(adapter);
	list<ConnectionPtr> connections = adapter->getIncomingConnections();

	vector<EndpointPtr> endpts;
	endpts.reserve(connections.size());
	transform(connections.begin(), connections.end(), back_inserter(endpts),
		  ::Ice::constMemFun(&Connection::endpoint));
	endpts = filterEndpoints(endpts);
	
	if(endpts.empty())
	{
	    NoEndpointException ex(__FILE__, __LINE__);
	    ex.proxy = toString();
	    throw ex;
	}

	list<ConnectionPtr>::iterator p;
	for(p = connections.begin(); p != connections.end(); ++p)
	{
	    if((*p)->endpoint() == endpts.front())
	    {
		break;
	    }
	}
	assert(p != connections.end());
	connection = *p;
    }
    else
    {	
	while(true)
	{
	    bool cached;
	    vector<EndpointPtr> endpts;

	    if(routerInfo)
	    {
		//
		// If we route, we send everything to the router's client
		// proxy endpoints.
		//
		ObjectPrx clientProxy = routerInfo->getClientProxy();
		endpts = clientProxy->__reference()->endpoints;
	    }
	    else if(!endpoints.empty())
	    {
		endpts = endpoints;
	    }
	    else if(locatorInfo)
	    {
		ReferencePtr self = const_cast<Reference*>(this);
		endpts = locatorInfo->getEndpoints(self, cached);
	    }

	    vector<EndpointPtr> filteredEndpts = filterEndpoints(endpts);
	    if(filteredEndpts.empty())
	    {
		NoEndpointException ex(__FILE__, __LINE__);
		ex.proxy = toString();
		throw ex;
	    }

	    try
	    {
		OutgoingConnectionFactoryPtr factory = instance->outgoingConnectionFactory();
		connection = factory->create(filteredEndpts);
		assert(connection);
	    }
	    catch(const LocalException& ex)
	    {
		if(!routerInfo && endpoints.empty())
		{	
		    assert(locatorInfo);
		    ReferencePtr self = const_cast<Reference*>(this);
		    locatorInfo->clearCache(self);
		    
		    if(cached)
		    {
			TraceLevelsPtr traceLevels = instance->traceLevels();
			LoggerPtr logger = instance->logger();
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
	if(routerInfo)
	{
	    connection->setAdapter(routerInfo->getAdapter());
	}
    }

    assert(connection);
    return connection;
}

vector<EndpointPtr>
IceInternal::Reference::filterEndpoints(const vector<EndpointPtr>& allEndpoints) const
{
    vector<EndpointPtr> endpoints = allEndpoints;

    //
    // Filter out unknown endpoints.
    //
    endpoints.erase(remove_if(endpoints.begin(), endpoints.end(), ::Ice::constMemFun(&Endpoint::unknown)),
                    endpoints.end());

    switch(mode)
    {
	case ModeTwoway:
	case ModeOneway:
	case ModeBatchOneway:
	{
	    //
	    // Filter out datagram endpoints.
	    //
            endpoints.erase(remove_if(endpoints.begin(), endpoints.end(), ::Ice::constMemFun(&Endpoint::datagram)),
                            endpoints.end());
	    break;
	}
	
	case ModeDatagram:
	case ModeBatchDatagram:
	{
	    //
	    // Filter out non-datagram endpoints.
	    //
            endpoints.erase(remove_if(endpoints.begin(), endpoints.end(),
                                      not1(::Ice::constMemFun(&Endpoint::datagram))),
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
    if(secure)
    {
	endpoints.erase(remove_if(endpoints.begin(), endpoints.end(), not1(::Ice::constMemFun(&Endpoint::secure))),
			endpoints.end());
    }
    else
    {
	//
	// We must use stable_partition() instead of just simply
	// partition(), because otherwise some STL implementations
	// order our now randomized endpoints.
	//
	stable_partition(endpoints.begin(), endpoints.end(), not1(::Ice::constMemFun(&Endpoint::secure)));
    }
    
    return endpoints;
}


IceInternal::Reference::Reference(const InstancePtr& inst,
				  const Identity& ident,
				  const Context& ctx,
				  const string& fs,
				  Mode md,
				  bool sec,
				  const string& adptid,
				  const vector<EndpointPtr>& endpts,
				  const RouterInfoPtr& rtrInfo,
				  const LocatorInfoPtr& locInfo,
				  const ObjectAdapterPtr& rvAdapter,
				  bool collocationOptimization) :
    instance(inst),
    identity(ident),
    context(ctx),
    facet(fs),
    mode(md),
    secure(sec),
    adapterId(adptid),
    endpoints(endpts),
    routerInfo(rtrInfo),
    locatorInfo(locInfo),
    reverseAdapter(rvAdapter),
    collocationOptimization(collocationOptimization),
    hashValue(0)
{
    //
    // It's either adapter id or endpoints, it can't be both.
    //
    assert(!(!adapterId.empty() && !endpoints.empty()));

    Int h = 0;
	
    string::const_iterator p;
    Context::const_iterator q;

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

    h = 5 * h + static_cast<Int>(mode);

    h = 5 * h + static_cast<Int>(secure);

    //
    // TODO: Should we also take the endpoints and other stuff into
    // account for hash calculation? Perhaps not, the code above
    // should be good enough for a good hash value.
    //

    const_cast<Int&>(hashValue) = h;
}
