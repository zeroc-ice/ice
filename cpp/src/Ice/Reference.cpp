// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Reference.h>
#include <Ice/ReferenceFactory.h>
#include <Ice/Instance.h>
#include <Ice/IdentityUtil.h>
#include <Ice/Endpoint.h>
#include <Ice/BasicStream.h>
#include <Ice/RouterInfo.h>
#include <Ice/Router.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(::IceInternal::Reference* p) { p->__incRef(); }
void IceInternal::decRef(::IceInternal::Reference* p) { p->__decRef(); }

bool
IceInternal::Reference::operator==(const Reference& r) const
{
    if (this == &r)
    {
	return true;
    }
    
    if (identity != r.identity)
    {
	return false;
    }

    if (facet != r.facet)
    {
	return false;
    }

    if (mode != r.mode)
    {
	return false;
    }

    if (secure != r.secure)
    {
	return false;
    }

    if (origEndpoints != r.origEndpoints)
    {
	return false;
    }

    if (endpoints != r.endpoints)
    {
	return false;
    }

    if (routerInfo != r.routerInfo)
    {
	return false;
    }

    if (reverseAdapter != r.reverseAdapter)
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
    if (this == &r)
    {
	return false;
    }
    
    if (identity < r.identity)
    {
	return true;
    }
    else if (r.identity < identity)
    {
	return false;
    }

    if (facet < r.facet)
    {
	return true;
    }
    else if (r.facet < facet)
    {
	return false;
    }

    if (mode < r.mode)
    {
	return true;
    }
    else if (r.mode < mode)
    {
	return false;
    }
    
    if (!secure && r.secure)
    {
	return true;
    }
    else if (r.secure < secure)
    {
	return false;
    }
    
    if (origEndpoints < r.origEndpoints)
    {
	return true;
    }
    else if (r.origEndpoints < origEndpoints)
    {
	return false;
    }
    
    if (endpoints < r.endpoints)
    {
	return true;
    }
    else if (r.endpoints < endpoints)
    {
	return false;
    }
    
    if (routerInfo < r.routerInfo)
    {
	return true;
    }
    else if (r.routerInfo < routerInfo)
    {
	return false;
    }
    
    if (reverseAdapter < r.reverseAdapter)
    {
	return true;
    }
    else if (r.reverseAdapter < reverseAdapter)
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
    
    vector<EndpointPtr>::const_iterator p;

    s->write(Ice::Int(origEndpoints.size()));
    for (p = origEndpoints.begin(); p != origEndpoints.end(); ++p)
    {
	(*p)->streamWrite(s);
    }

    if (endpoints == origEndpoints)
    {
	s->write(true);
    }
    else
    {
	s->write(false);
	s->write(Ice::Int(endpoints.size()));
	for (p = endpoints.begin(); p != endpoints.end(); ++p)
	{
	    (*p)->streamWrite(s);
	}
    }
}

string
IceInternal::Reference::toString() const
{
    ostringstream s;

    s << identity;

    if (!facet.empty())
    {
	s << " -f " << facet;
    }

    switch (mode)
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

    if (secure)
    {
	s << " -s";
    }

    vector<EndpointPtr>::const_iterator p;

    for (p = origEndpoints.begin(); p != origEndpoints.end(); ++p)
    {
	s << ':' << (*p)->toString();
    }
    
    if (endpoints != origEndpoints)
    {
	s << ':';
	for (p = endpoints.begin(); p != endpoints.end(); ++p)
	{
	    s << ':' << (*p)->toString();
	}
    }

    return s.str();
}

ReferencePtr
IceInternal::Reference::changeIdentity(const Identity& newIdentity) const
{
    if (newIdentity == identity)
    {
	return ReferencePtr(const_cast<Reference*>(this));
    }
    else
    {
	return instance->referenceFactory()->create(newIdentity, facet, mode, secure,
						    origEndpoints, endpoints,
						    routerInfo, reverseAdapter);
    }
}

ReferencePtr
IceInternal::Reference::changeFacet(const string& newFacet) const
{
    if (newFacet == facet)
    {
	return ReferencePtr(const_cast<Reference*>(this));
    }
    else
    {
	return instance->referenceFactory()->create(identity, newFacet, mode, secure,
						    origEndpoints, endpoints,
						    routerInfo, reverseAdapter);
    }
}

ReferencePtr
IceInternal::Reference::changeTimeout(int timeout) const
{
    //
    // We change the timeout settings in all endpoints.
    //
    vector<EndpointPtr>::const_iterator p;

    vector<EndpointPtr> newOrigEndpoints;
    for (p = origEndpoints.begin(); p != origEndpoints.end(); ++p)
    {
	newOrigEndpoints.push_back((*p)->timeout(timeout));
    }
    
    vector<EndpointPtr> newEndpoints;
    for (p = endpoints.begin(); p != endpoints.end(); ++p)
    {
	newEndpoints.push_back((*p)->timeout(timeout));
    }
    
    //
    // If we have a router, we also change the timeout settings on the
    // router and the router's client proxy.
    //
    RouterInfoPtr newRouterInfo;
    if (routerInfo)
    {
	try
	{
	    RouterPrx newRouter = RouterPrx::uncheckedCast(routerInfo->getRouter()->ice_timeout(timeout));
	    ObjectPrx newClientProxy = routerInfo->getClientProxy()->ice_timeout(timeout);
	    newRouterInfo = instance->routerManager()->get(newRouter);
	    newRouterInfo->setClientProxy(newClientProxy);
	}
	catch (const NoEndpointException&)
	{
	    // Ignore non-existing client proxies.
	}
    }

    return instance->referenceFactory()->create(identity, facet, mode, secure,
						newOrigEndpoints, newEndpoints,
						newRouterInfo, reverseAdapter);
}

ReferencePtr
IceInternal::Reference::changeMode(Mode newMode) const
{
    if (newMode == mode)
    {
	return ReferencePtr(const_cast<Reference*>(this));
    }
    else
    {
	return instance->referenceFactory()->create(identity, facet, newMode, secure,
						    origEndpoints, endpoints,
						    routerInfo, reverseAdapter);
    }
}

ReferencePtr
IceInternal::Reference::changeSecure(bool newSecure) const
{
    if (newSecure == secure)
    {
	return ReferencePtr(const_cast<Reference*>(this));
    }
    else
    {
	return instance->referenceFactory()->create(identity, facet, mode, newSecure,
						    origEndpoints, endpoints,
						    routerInfo, reverseAdapter);
    }
}

ReferencePtr
IceInternal::Reference::changeEndpoints(const vector<EndpointPtr>& newEndpoints) const
{
    if (newEndpoints == endpoints)
    {
	return ReferencePtr(const_cast<Reference*>(this));
    }
    else
    {
	return instance->referenceFactory()->create(identity, facet, mode, secure,
						    origEndpoints, newEndpoints,
						    routerInfo, reverseAdapter);
    }
}

ReferencePtr
IceInternal::Reference::changeRouter(const RouterPrx& newRouter) const
{
    RouterInfoPtr newRouterInfo = instance->routerManager()->get(newRouter);

    if (newRouterInfo == routerInfo)
    {
	return ReferencePtr(const_cast<Reference*>(this));
    }
    else
    {
	return instance->referenceFactory()->create(identity, facet, mode, secure,
						    origEndpoints, endpoints,
						    newRouterInfo, reverseAdapter);
    }
}

ReferencePtr
IceInternal::Reference::changeDefault() const
{
    return instance->referenceFactory()->create(identity, "", ModeTwoway, false,
						origEndpoints, origEndpoints,
						0, 0);
}

IceInternal::Reference::Reference(const InstancePtr& inst,
				  const Identity& ident,
				  const string& fac,
				  Mode md,
				  bool sec,
				  const vector<EndpointPtr>& origEndpts,
				  const vector<EndpointPtr>& endpts,
				  const RouterInfoPtr& rtrInfo,
				  const ObjectAdapterPtr& rvAdapter) :
    instance(inst),
    identity(ident),
    facet(fac),
    mode(md),
    secure(sec),
    origEndpoints(origEndpts),
    endpoints(endpts),
    routerInfo(rtrInfo),
    reverseAdapter(rvAdapter),
    hashValue(0)
{
    calcHashValue();
}

void
IceInternal::Reference::calcHashValue()
{
    Int h = 0;

    string::const_iterator p;

    for (p = identity.name.begin(); p != identity.name.end(); ++p)
    {
	h = 5 * h + *p;
    }

    for (p = identity.category.begin(); p != identity.category.end(); ++p)
    {
	h = 5 * h + *p;
    }

    for (p = facet.begin(); p != facet.end(); ++p)
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
