// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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
#include <Ice/StringUtil.h>

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

    s->write(facet);
    
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
        ostringstream f;
        FacetPath::const_iterator p = facet.begin();
        while(p != facet.end())
        {
            f << encodeString(*p++, "/");
            if(p != facet.end())
            {
                f << '/';
            }
        }

        //
        // If the encoded facet string contains characters which
        // the reference parser uses as separators, then we enclose
        // the facet string in quotes.
        //
        s << " -f ";
        const string& fs = f.str();
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
	    s << ':' << (*p)->toString();
	}
    }
    else if(!adapterId.empty())
    {
        string a = encodeString(adapterId, "");
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
	return instance->referenceFactory()->create(newIdentity, facet, mode, secure, adapterId,
						    endpoints, routerInfo, locatorInfo, reverseAdapter,
						    collocationOptimization);
    }
}

ReferencePtr
IceInternal::Reference::changeFacet(const FacetPath& newFacet) const
{
    if(newFacet == facet)
    {
	return ReferencePtr(const_cast<Reference*>(this));
    }
    else
    {
	return instance->referenceFactory()->create(identity, newFacet, mode, secure, adapterId,
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

    return instance->referenceFactory()->create(identity, facet, mode, secure, adapterId,
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
	return instance->referenceFactory()->create(identity, facet, newMode, secure, adapterId,
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
	return instance->referenceFactory()->create(identity, facet, mode, newSecure, adapterId,
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

    return instance->referenceFactory()->create(identity, facet, mode, secure, adapterId,
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
	return instance->referenceFactory()->create(identity, facet, mode, secure, newAdapterId,
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
	return instance->referenceFactory()->create(identity, facet, mode, secure, adapterId,
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
	return instance->referenceFactory()->create(identity, facet, mode, secure, adapterId,
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
	return instance->referenceFactory()->create(identity, facet, mode, secure, adapterId,
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
	return instance->referenceFactory()->create(identity, facet, mode, secure, adapterId,
						    endpoints, routerInfo, locatorInfo, reverseAdapter,
						    newCollocationOptimization);
    }
}

ReferencePtr
IceInternal::Reference::changeDefault() const
{
    RouterInfoPtr routerInfo = instance->routerManager()->get(instance->referenceFactory()->getDefaultRouter());
    LocatorInfoPtr locatorInfo = instance->locatorManager()->get(instance->referenceFactory()->getDefaultLocator());

    return instance->referenceFactory()->create(identity, FacetPath(), ModeTwoway, false, adapterId,
						endpoints, routerInfo, locatorInfo, 0, true);
}

IceInternal::Reference::Reference(const InstancePtr& inst,
				  const Identity& ident,
				  const FacetPath& facPath,
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
    facet(facPath),
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
    FacetPath::const_iterator q;

    for(p = identity.name.begin(); p != identity.name.end(); ++p)
    {
	h = 5 * h + *p;
    }

    for(p = identity.category.begin(); p != identity.category.end(); ++p)
    {
	h = 5 * h + *p;
    }

    for(q = facet.begin(); q != facet.end(); ++q)
    {
	for(p = q->begin(); p != q->end(); ++p)
	{
	    h = 5 * h + *p;
	}
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
