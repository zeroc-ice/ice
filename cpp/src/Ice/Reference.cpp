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

    h = 5 * h + static_cast<Int>(_secure);

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
    
    s->write(_secure);

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

    if(_secure)
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

    if(_secure != r._secure)
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

    if(!_secure && r._secure)
    {
	return true;
    }
    else if(r._secure < _secure)
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

IceInternal::Reference::Reference(const InstancePtr& inst, const Identity& ident, const Context& ctx,
                                  const string& fs, Mode md, bool sec)
    : _instance(inst),
      _mode(md),
      _identity(ident),
      _hasContext(!ctx.empty()),
      _context(ctx),
      _facet(fs),
      _secure(sec),
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
      _secure(r._secure),
      _hashInitialized(false)
{
}

void IceInternal::incRef(IceInternal::FixedReference* p) { p->__incRef(); }
void IceInternal::decRef(IceInternal::FixedReference* p) { p->__decRef(); }

IceInternal::FixedReference::FixedReference(const InstancePtr& inst, const Identity& ident,
						  const Context& ctx, const string& fs, Mode md,
						  bool sec, const vector<ConnectionIPtr>& fixedConns)
    : Reference(inst, ident, ctx, fs, md, sec),
      _fixedConnections(fixedConns)
{
}

vector<EndpointPtr>
IceInternal::FixedReference::getEndpoints() const
{
    return vector<EndpointPtr>();
}

bool
IceInternal::FixedReference::getCollocationOptimization() const
{
    return false;
}

void
IceInternal::FixedReference::streamWrite(BasicStream* s) const
{
    MarshalException ex(__FILE__, __LINE__);
    ex.reason = "Cannot marshal a fixed reference";
    throw ex;
}

ConnectionIPtr
IceInternal::FixedReference::getConnection(bool& compress) const
{
    vector<ConnectionIPtr> filteredConns = filterConnections(_fixedConnections, getMode(), getSecure());
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
IceInternal::FixedReference::changeDefault() const
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
    return FixedReferencePtr(const_cast<FixedReference*>(this));
}

ReferencePtr
IceInternal::FixedReference::changeTimeout(int) const
{
    return FixedReferencePtr(const_cast<FixedReference*>(this));
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

bool
IceInternal::RoutableReference::getCollocationOptimization() const
{
    return _collocationOptimization;
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
IceInternal::RoutableReference::changeDefault() const
{
    RoutableReferencePtr r = RoutableReferencePtr::dynamicCast(getInstance()->referenceFactory()->copy(this));
    r->_routerInfo = getInstance()->routerManager()->get(getInstance()->referenceFactory()->getDefaultRouter());
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
    if(_collocationOptimization != rhs->_collocationOptimization)
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
	    if(!_collocationOptimization && rhs->_collocationOptimization)
	    {
		return true;
	    }
	    else if(rhs->_collocationOptimization < _collocationOptimization)
	    {
		return false;
	    }
            return _routerInfo < rhs->_routerInfo;
        }
    }
    return false;
}

IceInternal::RoutableReference::RoutableReference(const InstancePtr& inst, const Identity& ident,
						  const Context& ctx, const string& fs, Mode md,
						  bool sec, const RouterInfoPtr& rtrInfo, bool collocationOpt)
    : Reference(inst, ident, ctx, fs, md, sec),
      _routerInfo(rtrInfo),
      _collocationOptimization(collocationOpt)
{
}

IceInternal::RoutableReference::RoutableReference(const RoutableReference& r)
    : Reference(r),
      _routerInfo(r._routerInfo),
      _collocationOptimization(r._collocationOptimization)
{
}

void IceInternal::incRef(IceInternal::DirectReference* p) { p->__incRef(); }
void IceInternal::decRef(IceInternal::DirectReference* p) { p->__decRef(); }

IceInternal::DirectReference::DirectReference(const InstancePtr& inst, const Identity& ident,
					      const Context& ctx, const string& fs, Mode md,
					      bool sec, const vector<EndpointPtr>& endpts,
					      const RouterInfoPtr& rtrInfo, bool collocationOpt)
    : RoutableReference(inst, ident, ctx, fs, md, sec, rtrInfo, collocationOpt),
      _endpoints(endpts)
{
}

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
IceInternal::DirectReference::changeLocator(const LocatorPrx&) const
{
    return DirectReferencePtr(const_cast<DirectReference*>(this));
}

ReferencePtr
IceInternal::DirectReference::changeDefault() const
{
    return DirectReferencePtr(const_cast<DirectReference*>(this));
}

ReferencePtr
IceInternal::DirectReference::changeCompress(bool newCompress) const
{
    DirectReferencePtr r = DirectReferencePtr::dynamicCast(getInstance()->referenceFactory()->copy(this));
    vector<EndpointPtr> newEndpoints;
    vector<EndpointPtr>::const_iterator p;
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
    RoutableReference::streamWrite(s);

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
    string result = RoutableReference::toString();

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

ConnectionIPtr
IceInternal::DirectReference::getConnection(bool& comp) const
{
    vector<EndpointPtr> endpts = RoutableReference::getRoutedEndpoints();
    if(endpts.empty())
    {
	endpts = _endpoints;
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

IceInternal::DirectReference::DirectReference(const DirectReference& r)
    : RoutableReference(r),
      _endpoints(r._endpoints)
{
}


void IceInternal::incRef(IceInternal::IndirectReference* p) { p->__incRef(); }
void IceInternal::decRef(IceInternal::IndirectReference* p) { p->__decRef(); }

IceInternal::IndirectReference::IndirectReference(const InstancePtr& inst, const Identity& ident,
                                                  const Context& ctx, const string& fs, Mode md,
						  bool sec, const string& adptid, const RouterInfoPtr& rtrInfo,
						  const LocatorInfoPtr& locInfo, bool collocationOpt)
    : RoutableReference(inst, ident, ctx, fs, md, sec, rtrInfo, collocationOpt),
      _adapterId(adptid),
      _locatorInfo(locInfo)
{
}

vector<EndpointPtr>
IceInternal::IndirectReference::getEndpoints() const
{
    return vector<EndpointPtr>();
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
IceInternal::IndirectReference::changeDefault() const
{
    IndirectReferencePtr r = IndirectReferencePtr::dynamicCast(RoutableReference::changeDefault());
    r->_locatorInfo = getInstance()->locatorManager()->get(getInstance()->referenceFactory()->getDefaultLocator());
    return r;
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
	vector<EndpointPtr> endpts = RoutableReference::getRoutedEndpoints();
	bool cached = false;
	if(endpts.empty() && _locatorInfo)
	{
	    const IndirectReferencePtr self = const_cast<IndirectReference*>(this);
	    endpts = _locatorInfo->getEndpoints(self, cached);
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
    : RoutableReference(r),
      _adapterId(r._adapterId),
      _locatorInfo(r._locatorInfo)
{
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

vector<ConnectionIPtr>
IceInternal::filterConnections(const vector<ConnectionIPtr>& allConnections, Reference::Mode m, bool secure)
{
    vector<ConnectionIPtr> connections = allConnections;

    switch(m)
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
