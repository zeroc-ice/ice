// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/Reference.h>
#include <IceE/ReferenceFactory.h>
#include <IceE/LocalException.h>
#include <IceE/Instance.h>
#include <IceE/IdentityUtil.h>
#include <IceE/Endpoint.h>
#include <IceE/BasicStream.h>
#ifdef ICEE_HAS_ROUTER
#    include <IceE/RouterInfo.h>
#    include <IceE/Router.h>
#    include <IceE/Connection.h>
#endif
#ifdef ICEE_HAS_LOCATOR
#    include <IceE/LocatorInfo.h>
#    include <IceE/Locator.h>
#endif
#include <IceE/Functional.h>
#include <IceE/OutgoingConnectionFactory.h>
#include <IceE/LoggerUtil.h>
#include <IceE/TraceLevels.h>
#include <IceE/StringUtil.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(IceInternal::Reference* p) { p->__incRef(); }
void IceInternal::decRef(IceInternal::Reference* p) { p->__decRef(); }

const Context&
IceInternal::Reference::getContext() const
{
    return _hasContext ? _context : _instance->getDefaultContext();
}

ReferencePtr
IceInternal::Reference::defaultContext() const
{
    if(!_hasContext)
    {
	return ReferencePtr(const_cast<Reference*>(this));
    }
    ReferencePtr r = _instance->referenceFactory()->copy(this);
    r->_hasContext = false;
    r->_context.clear();
    return r;
}

CommunicatorPtr
IceInternal::Reference::getCommunicator() const
{
    return _instance->communicator();
}

ReferencePtr
IceInternal::Reference::changeContext(const Context& newContext) const
{
    if(_hasContext && newContext == _context)
    {
	return ReferencePtr(const_cast<Reference*>(this));
    }
    ReferencePtr r = _instance->referenceFactory()->copy(this);
    r->_hasContext = true;
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

ReferencePtr
IceInternal::Reference::changeDefault() const
{
    ReferencePtr r = _instance->referenceFactory()->copy(this);
    r->_mode = ModeTwoway;
    r->_hasContext = false;
    r->_context.clear();
    r->_facet = "";
    return r;
}

Int
Reference::hash() const
{
    IceUtil::Mutex::Lock sync(_hashMutex);

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

    if(_hasContext)
    {
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
    }

    for(p = _facet.begin(); p != _facet.end(); ++p)
    {
	h = 5 * h + *p;
    }

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
	s->write(vector<string>());
    }
    else
    {
	vector<string> facetPath;
	facetPath.push_back(_facet);
	s->write(facetPath);
    }
    
    s->write(static_cast<Byte>(_mode));

    s->write(false); // Secure
    
    // Derived class writes the remainder of the reference.
}

string
IceInternal::Reference::toString() const
{
    string s;

    //  
    // If the encoded identity string contains characters which
    // the reference parser uses as separators, then we enclose
    // the identity string in quotes.
    //
    string id = identityToString(_identity);
    if(id.find_first_of(" \t\n\r:@") != string::npos)
    {
        s += "\"";
	s += id;
        s += "\"";
    }
    else
    {
        s += id;
    }

    if(!_facet.empty())
    {
        s += " -f ";

	//  
	// If the encoded facet string contains characters which
	// the reference parser uses as separators, then we enclose
	// the facet string in quotes.
	//
	string fs = IceUtil::escapeString(_facet, "");
	if(fs.find_first_of(" \t\n\r:@") != string::npos)
	{
            s += "\"";
	    s += fs;
            s += "\"";
	}
	else
	{
	    s += fs;
	}
    }

    switch(_mode)
    {
	case ModeTwoway:
	{
	    s += " -t";
	    break;
	}

	case ModeOneway:
	{
	    s += " -o";
	    break;
	}

#ifdef ICEE_HAS_BATCH
	case ModeBatchOneway:
	{
	    s += " -O";
	    break;
	}
#endif
	default:
	{
	    //
	    // TODO: Should this assert?
	    //
	    break;
	}
    }

    return s;
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

    if(_hasContext != r._hasContext)
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
    
    if(_hasContext < r._hasContext)
    {
        return true;
    }
    else if(r._hasContext < _hasContext)
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

IceInternal::Reference::Reference(const InstancePtr& inst, const Identity& ident, const Context& ctx,
                                  const string& fs, Mode md)
    : _instance(inst),
      _mode(md),
      _identity(ident),
      _hasContext(!ctx.empty()),
      _context(ctx),
      _facet(fs),
      _hashInitialized(false)
{
}

IceInternal::Reference::Reference(const Reference& r)
    : _instance(r._instance),
      _mode(r._mode),
      _identity(r._identity),
      _hasContext(r._hasContext),
      _context(r._context),
      _facet(r._facet),
      _hashInitialized(false)
{
}

void IceInternal::incRef(IceInternal::FixedReference* p) { p->__incRef(); }
void IceInternal::decRef(IceInternal::FixedReference* p) { p->__decRef(); }

IceInternal::FixedReference::FixedReference(const InstancePtr& inst, const Identity& ident,
					    const Context& ctx, const string& fs, Mode md,
					    const vector<ConnectionPtr>& fixedConns)
    : Reference(inst, ident, ctx, fs, md),
      _fixedConnections(fixedConns)
{
}

const vector<ConnectionPtr>&
IceInternal::FixedReference::getFixedConnections() const
{
    return _fixedConnections;
}

vector<EndpointPtr>
IceInternal::FixedReference::getEndpoints() const
{
    return vector<EndpointPtr>();
}

#ifdef ICEE_HAS_ROUTER

ReferencePtr
IceInternal::FixedReference::changeRouter(const RouterPrx&) const
{
    return FixedReferencePtr(const_cast<FixedReference*>(this));
}

#endif

#ifdef ICEE_HAS_LOCATOR

ReferencePtr
IceInternal::FixedReference::changeLocator(const LocatorPrx&) const
{
    return FixedReferencePtr(const_cast<FixedReference*>(this));
}

#endif

ReferencePtr
IceInternal::FixedReference::changeTimeout(int) const
{
    return FixedReferencePtr(const_cast<FixedReference*>(this));
}

void
IceInternal::FixedReference::streamWrite(BasicStream* s) const
{
    throw MarshalException(__FILE__, __LINE__, "Cannot marshal a fixed reference");
}

string
IceInternal::FixedReference::toString() const
{
    throw MarshalException(__FILE__, __LINE__, "Cannot marshal a fixed reference");
}

ConnectionPtr
IceInternal::FixedReference::getConnection() const
{
    //
    // Randomize the order of connections.
    //
    if(_fixedConnections.empty())
    {
	NoEndpointException ex(__FILE__, __LINE__);
	ex.proxy = toString();
	throw ex;
    }

    vector<ConnectionPtr> randomCons = _fixedConnections;
    random_shuffle(randomCons.begin(), randomCons.end());

    ConnectionPtr connection = randomCons[0];
    assert(connection);

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

IceInternal::FixedReference::FixedReference(const FixedReference& r)
    : Reference(r),
      _fixedConnections(r._fixedConnections)
{
}

#ifdef ICEE_HAS_ROUTER
void IceInternal::incRef(IceInternal::RoutableReference* p) { p->__incRef(); }
void IceInternal::decRef(IceInternal::RoutableReference* p) { p->__decRef(); }

vector<EndpointPtr>
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
    return vector<EndpointPtr>();
}

ReferencePtr
IceInternal::RoutableReference::changeDefault() const
{
    RoutableReferencePtr r = RoutableReferencePtr::dynamicCast(Reference::changeDefault());
    r->_routerInfo = getInstance()->routerManager()->get(getInstance()->referenceFactory()->getDefaultRouter());
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
            return _routerInfo < rhs->_routerInfo;
        }
    }
    return false;
}

IceInternal::RoutableReference::RoutableReference(const InstancePtr& inst, const Identity& ident,
						  const Context& ctx, const string& fs, Mode md,
						  const RouterInfoPtr& rtrInfo)
    : Reference(inst, ident, ctx, fs, md), _routerInfo(rtrInfo)
{
}

IceInternal::RoutableReference::RoutableReference(const RoutableReference& r)
    : Reference(r), _routerInfo(r._routerInfo)
{
}
#endif

void IceInternal::incRef(IceInternal::DirectReference* p) { p->__incRef(); }
void IceInternal::decRef(IceInternal::DirectReference* p) { p->__decRef(); }


#ifdef ICEE_HAS_ROUTER
IceInternal::DirectReference::DirectReference(const InstancePtr& inst, const Identity& ident,
					      const Context& ctx, const string& fs, Mode md,
					      const vector<EndpointPtr>& endpts, const RouterInfoPtr& rtrInfo) :

    RoutableReference(inst, ident, ctx, fs, md, rtrInfo),
    _endpoints(endpts)
{
}
#else
IceInternal::DirectReference::DirectReference(const InstancePtr& inst, const Identity& ident,
					      const Context& ctx, const string& fs, Mode md,
					      const vector<EndpointPtr>& endpts) :
    Reference(inst, ident, ctx, fs, md),
    _endpoints(endpts)
{
}
#endif

vector<EndpointPtr>
IceInternal::DirectReference::getEndpoints() const
{
    return _endpoints;
}

DirectReferencePtr
IceInternal::DirectReference::changeEndpoints(const vector<EndpointPtr>& newEndpoints) const
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
IceInternal::DirectReference::changeDefault() const
{
    //
    // Return an indirect reference if a default locator is set.
    //
#ifdef ICEE_HAS_LOCATOR
    LocatorPrx loc = getInstance()->referenceFactory()->getDefaultLocator();
    if(loc)
    {
	LocatorInfoPtr newLocatorInfo = getInstance()->locatorManager()->get(loc);
	return getInstance()->referenceFactory()->create(getIdentity(), Context(), "", ModeTwoway,
							 ""
#ifdef ICEE_HAS_ROUTER
							 , 0
#endif // ICEE_HAS_ROUTER
							 , newLocatorInfo);
    }
    else
#endif // ICEE_HAS_LOCATOR
    {
	return Parent::changeDefault();
    }
}

#ifdef ICEE_HAS_LOCATOR

ReferencePtr
IceInternal::DirectReference::changeLocator(const LocatorPrx& newLocator) const
{
    if(newLocator)
    {
	LocatorInfoPtr newLocatorInfo = getInstance()->locatorManager()->get(newLocator);
	return getInstance()->referenceFactory()->create(getIdentity(), getContext(), getFacet(), getMode(),
							 ""
#ifdef ICEE_HAS_ROUTER
							 , 0
#endif
							 , newLocatorInfo);
    }
    else
    {
	return DirectReferencePtr(const_cast<DirectReference*>(this));
    }
}

#endif

ReferencePtr
IceInternal::DirectReference::changeTimeout(int newTimeout) const
{
    DirectReferencePtr r = DirectReferencePtr::dynamicCast(getInstance()->referenceFactory()->copy(this));
    vector<EndpointPtr> newEndpoints;
    vector<EndpointPtr>::const_iterator p;
    for(p = _endpoints.begin(); p != _endpoints.end(); ++p)
    {
	newEndpoints.push_back((*p)->timeout(newTimeout));
    }
    r->_endpoints = newEndpoints;
    return r;
}

void
IceInternal::DirectReference::streamWrite(BasicStream* s) const
{
    Parent::streamWrite(s);

    Int sz = static_cast<Int>(_endpoints.size());
    s->writeSize(sz);
    if(sz)
    {
	for(vector<EndpointPtr>::const_iterator p = _endpoints.begin(); p != _endpoints.end(); ++p)
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
    string result = Parent::toString();

    vector<EndpointPtr>::const_iterator p;
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

ConnectionPtr
IceInternal::DirectReference::getConnection() const
{
#ifdef ICEE_HAS_ROUTER
    vector<EndpointPtr> endpts = Parent::getRoutedEndpoints();
    if(endpts.empty())
    {
	endpts = _endpoints;
    }
#else
    vector<EndpointPtr> endpts = _endpoints;
#endif
    vector<EndpointPtr> filteredEndpoints = filterEndpoints(endpts);
    if(filteredEndpoints.empty())
    {
        NoEndpointException ex(__FILE__, __LINE__);
	ex.proxy = toString();
	throw ex;
    }

    OutgoingConnectionFactoryPtr factory = getInstance()->outgoingConnectionFactory();
    ConnectionPtr connection = factory->create(filteredEndpoints);
    assert(connection);

#if defined(ICEE_HAS_ROUTER) && !defined(ICEE_PURE_CLIENT)
    //
    // If we have a router, set the object adapter for this router
    // (if any) to the new connection, so that callbacks from the
    // router can be received over this new connection.
    //
    if(getRouterInfo())
    {
        connection->setAdapter(getRouterInfo()->getAdapter());
    }
#endif

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
    if(!rhs || !Parent::operator==(r))
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
    if(Parent::operator<(r))
    {
        return true;
    }
    if(Parent::operator==(r))
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

IceInternal::DirectReference::DirectReference(const DirectReference& r)
    : Parent(r), _endpoints(r._endpoints)
{
}

#ifdef ICEE_HAS_LOCATOR

void IceInternal::incRef(IceInternal::IndirectReference* p) { p->__incRef(); }
void IceInternal::decRef(IceInternal::IndirectReference* p) { p->__decRef(); }

#ifdef ICEE_HAS_ROUTER
IceInternal::IndirectReference::IndirectReference(const InstancePtr& inst, const Identity& ident,
                                                  const Context& ctx, const string& fs, Mode md,
						  const string& adptid, const RouterInfoPtr& rtrInfo,
						  const LocatorInfoPtr& locInfo)
    : RoutableReference(inst, ident, ctx, fs, md, rtrInfo),
      _adapterId(adptid),
      _locatorInfo(locInfo)
{
}
#else
IceInternal::IndirectReference::IndirectReference(const InstancePtr& inst, const Identity& ident,
                                                  const Context& ctx, const string& fs, Mode md,
						  const string& adptid
						  , const LocatorInfoPtr& locInfo)
    : Reference(inst, ident, ctx, fs, md),
      _adapterId(adptid),
      _locatorInfo(locInfo)
{
}
#endif

vector<EndpointPtr>
IceInternal::IndirectReference::getEndpoints() const
{
    return vector<EndpointPtr>();
}

ReferencePtr
IceInternal::IndirectReference::changeDefault() const
{
    //
    // Return a direct reference if no default locator is defined.
    //
    LocatorPrx loc = getInstance()->referenceFactory()->getDefaultLocator();
    if(!loc)
    {
	return getInstance()->referenceFactory()->create(getIdentity(), Context(), "", ModeTwoway,
							 vector<EndpointPtr>()
#ifdef ICEE_HAS_ROUTER
							 , getRouterInfo()
#endif
							 );
    }
    else
    {
	IndirectReferencePtr r = IndirectReferencePtr::dynamicCast(Parent::changeDefault());
	r->_locatorInfo = getInstance()->locatorManager()->get(loc);
	return r;
    }
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
							 vector<EndpointPtr>()
#ifdef ICEE_HAS_ROUTER
							 , getRouterInfo()
#endif
							 );
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

void
IceInternal::IndirectReference::streamWrite(BasicStream* s) const
{
    Parent::streamWrite(s);

    s->writeSize(0);
    s->write(_adapterId);
}

string
IceInternal::IndirectReference::toString() const
{
    string result = Parent::toString();
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

ConnectionPtr
IceInternal::IndirectReference::getConnection() const
{
    ConnectionPtr connection;

    while(true)
    {
#ifdef ICEE_HAS_ROUTER
	vector<EndpointPtr> endpts = Parent::getRoutedEndpoints();
#else
	vector<EndpointPtr> endpts;
#endif
	bool cached = false;
	if(endpts.empty() && _locatorInfo)
	{
	    const IndirectReferencePtr self = const_cast<IndirectReference*>(this);
	    endpts = _locatorInfo->getEndpoints(self, cached);
	}
	vector<EndpointPtr> filteredEndpoints = filterEndpoints(endpts);
	if(filteredEndpoints.empty())
	{
	    NoEndpointException ex(__FILE__, __LINE__);
	    ex.proxy = toString();
	    throw ex;
	}

	try
	{
	    OutgoingConnectionFactoryPtr factory = getInstance()->outgoingConnectionFactory();
	    connection = factory->create(filteredEndpoints);
	    assert(connection);
	}
	catch(const LocalException& ex)
	{
#ifdef ICEE_HAS_ROUTER
	    if(!getRouterInfo())
#endif
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
			    << "removing endpoints from cache and trying one more time\n" << ex.toString();
		    }
		    continue;
		}
	    }

	    throw;
	}

	break;
    }

#if defined(ICEE_HAS_ROUTER) && !defined(ICEE_PURE_CLIENT)
    //
    // If we have a router, set the object adapter for this router
    // (if any) to the new connection, so that callbacks from the
    // router can be received over this new connection.
    //
    if(getRouterInfo())
    {
        connection->setAdapter(getRouterInfo()->getAdapter());
    }
#endif

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
    if(!rhs || !Parent::operator==(r))
    {
        return false;
    }
    return _adapterId == rhs->_adapterId && _locatorInfo == rhs->_locatorInfo;
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
    if(Parent::operator<(r))
    {
        return true;
    }
    if(Parent::operator==(r))
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
            return _locatorInfo < rhs->_locatorInfo;
        }
    }
    return false;
}

ReferencePtr
IceInternal::IndirectReference::clone() const
{
    return new IndirectReference(*this);
}

IceInternal::IndirectReference::IndirectReference(const IndirectReference& r)
    : Parent(r), _adapterId(r._adapterId), _locatorInfo(r._locatorInfo)
{
}

#endif // ICEE_HAS_LOCATOR

vector<EndpointPtr>
IceInternal::filterEndpoints(const vector<EndpointPtr>& allEndpoints)
{
    vector<EndpointPtr> endpoints = allEndpoints;

    //
    // Filter out unknown endpoints.
    //
    endpoints.erase(remove_if(endpoints.begin(), endpoints.end(), Ice::constMemFun(&Endpoint::unknown)),
                    endpoints.end());
    
    //
    // Randomize the order of endpoints.
    //
    random_shuffle(endpoints.begin(), endpoints.end());
    
    return endpoints;
}
