// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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

    if(compress != r.compress)
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
    
    if(!compress && r.compress)
    {
	return true;
    }
    else if(r.compress < compress)
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

    s->write(compress);

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

    s << identity;

    if(!facet.empty())
    {
	s << " -f " << facet;
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

    if(compress)
    {
	s << " -c";
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
	s << " @ " << adapterId;
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
	return instance->referenceFactory()->create(newIdentity, facet, mode, secure, compress, adapterId,
						    endpoints, routerInfo, locatorInfo, reverseAdapter);
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
	return instance->referenceFactory()->create(identity, newFacet, mode, secure, compress, adapterId,
						    endpoints, routerInfo, locatorInfo, reverseAdapter);
    }
}

ReferencePtr
IceInternal::Reference::changeTimeout(int timeout) const
{
    //
    // We change the timeout settings in all endpoints.
    //
    vector<EndpointPtr>::const_iterator p;

    vector<EndpointPtr> newEndpoints;
    for(p = endpoints.begin(); p != endpoints.end(); ++p)
    {
	newEndpoints.push_back((*p)->timeout(timeout));
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
	    RouterPrx newRouter = RouterPrx::uncheckedCast(routerInfo->getRouter()->ice_timeout(timeout));
	    ObjectPrx newClientProxy = routerInfo->getClientProxy()->ice_timeout(timeout);
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
	LocatorPrx newLocator = LocatorPrx::uncheckedCast(locatorInfo->getLocator()->ice_timeout(timeout));
	newLocatorInfo = instance->locatorManager()->get(newLocator);
    }

    return instance->referenceFactory()->create(identity, facet, mode, secure, compress, adapterId,
						newEndpoints, newRouterInfo, newLocatorInfo, reverseAdapter);
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
	return instance->referenceFactory()->create(identity, facet, newMode, secure, compress, adapterId,
						    endpoints, routerInfo, locatorInfo, reverseAdapter);
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
	return instance->referenceFactory()->create(identity, facet, mode, newSecure, compress, adapterId,
						    endpoints, routerInfo, locatorInfo, reverseAdapter);
    }
}

ReferencePtr
IceInternal::Reference::changeCompress(bool newCompress) const
{
    if(newCompress == compress)
    {
	return ReferencePtr(const_cast<Reference*>(this));
    }
    else
    {
	return instance->referenceFactory()->create(identity, facet, mode, secure, newCompress, adapterId,
						    endpoints, routerInfo, locatorInfo, reverseAdapter);
    }
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
	return instance->referenceFactory()->create(identity, facet, mode, secure, compress, newAdapterId,
						    endpoints, routerInfo, locatorInfo, reverseAdapter);
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
	return instance->referenceFactory()->create(identity, facet, mode, secure, compress, adapterId,
						    newEndpoints, routerInfo, locatorInfo, reverseAdapter);
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
	return instance->referenceFactory()->create(identity, facet, mode, secure, compress, adapterId,
						    endpoints, newRouterInfo, locatorInfo, reverseAdapter);
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
	return instance->referenceFactory()->create(identity, facet, mode, secure, compress, adapterId,
						    endpoints, routerInfo, newLocatorInfo, reverseAdapter);
    }
}

ReferencePtr
IceInternal::Reference::changeDefault() const
{
    return instance->referenceFactory()->create(identity, "", ModeTwoway, false, false, adapterId,
						endpoints, 0, 0, 0);
}

IceInternal::Reference::Reference(const InstancePtr& inst,
				  const Identity& ident,
				  const string& fac,
				  Mode md,
				  bool sec,
				  bool com,
				  const string& adptid,
				  const vector<EndpointPtr>& endpts,
				  const RouterInfoPtr& rtrInfo,
				  const LocatorInfoPtr& locInfo,
				  const ObjectAdapterPtr& rvAdapter) :
    instance(inst),
    identity(ident),
    facet(fac),
    mode(md),
    secure(sec),
    compress(com),
    adapterId(adptid),
    endpoints(endpts),
    routerInfo(rtrInfo),
    locatorInfo(locInfo),
    reverseAdapter(rvAdapter),
    hashValue(0)
{
    //
    // It's either adapter id or endpoints, it can't be both.
    //
    assert(!(!adapterId.empty() && !endpoints.empty()));

    Int h = 0;
	
    string::const_iterator p;

    for(p = identity.name.begin(); p != identity.name.end(); ++p)
    {
	h = 5 * h + *p;
    }

    for(p = identity.category.begin(); p != identity.category.end(); ++p)
    {
	h = 5 * h + *p;
    }

    for(p = facet.begin(); p != facet.end(); ++p)
    {
	h = 5 * h + *p;
    }

    h = 5 * h + static_cast<Int>(mode);

    h = 5 * h + static_cast<Int>(secure);

    h = 5 * h + static_cast<Int>(compress);

    //
    // TODO: Should we also take the endpoints and other stuff into
    // account for hash calculation? Perhaps not, the code above
    // should be good enough for a good hash value.
    //

    const_cast<Int&>(hashValue) = h;
}
