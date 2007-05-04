// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/Reference.h>
#include <IceE/ReferenceFactory.h>
#include <IceE/LocalException.h>
#include <IceE/Instance.h>
#include <IceE/Endpoint.h>
#include <IceE/BasicStream.h>

#ifdef ICEE_HAS_ROUTER
#   include <IceE/RouterInfo.h>
#   include <IceE/Router.h>
#endif

#ifdef ICEE_HAS_LOCATOR
#   include <IceE/LocatorInfo.h>
#   include <IceE/Locator.h>
#endif

#include <IceE/Connection.h>
#include <IceE/Functional.h>
#include <IceE/OutgoingConnectionFactory.h>
#include <IceE/LoggerUtil.h>
#include <IceE/TraceLevels.h>
#include <IceE/StringUtil.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(IceInternal::Reference* p) { return p; }

class ConnectionIsDatagram : public unary_function<ConnectionPtr, bool>
{
public:

    bool
    operator()(ConnectionPtr p) const
    {
        return p->endpoint()->datagram();
    }
};

class ConnectionIsSecure : public unary_function<ConnectionPtr, bool>
{
public:

    bool
    operator()(ConnectionPtr p) const
    {
        return p->endpoint()->secure();
    }
};

CommunicatorPtr
IceInternal::Reference::getCommunicator() const
{
    return _communicator;
}

ReferencePtr
IceInternal::Reference::changeContext(const Context& newContext) const
{
    ReferencePtr r = _instance->referenceFactory()->copy(this);
    r->_context = newContext;
    return r;
}

ReferencePtr
IceInternal::Reference::changeMode(ReferenceMode newMode) const
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
IceInternal::Reference::changeTimeout(int newTimeout) const
{
    if(_overrideTimeout && newTimeout == _timeout)
    {
	return ReferencePtr(const_cast<Reference*>(this));
    }
    ReferencePtr r = getInstance()->referenceFactory()->copy(this);
    r->_timeout = newTimeout;
    r->_overrideTimeout = true;
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
    string s;

    //  
    // If the encoded identity string contains characters which
    // the reference parser uses as separators, then we enclose
    // the identity string in quotes.
    //
    string id = _instance->identityToString(_identity);
    if(id.find_first_of(" :@") != string::npos)
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
        string fs = _facet;
#ifdef ICEE_HAS_WSTRING
        if(_instance->initializationData().stringConverter)
        {
            UTF8BufferI buffer;
            Byte* last =
                _instance->initializationData().stringConverter->toUTF8(fs.data(), fs.data() + fs.size(), buffer);
            fs = string(reinterpret_cast<const char*>(buffer.getBuffer()), last - buffer.getBuffer());
        }
#endif
	fs = IceUtil::escapeString(fs, "");
	if(fs.find_first_of(" :@") != string::npos)
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
	case ReferenceModeTwoway:
	{
	    s += " -t";
	    break;
	}

	case ReferenceModeOneway:
	{
	    s += " -o";
	    break;
	}

	case ReferenceModeBatchOneway:
	{
	    s += " -O";
	    break;
	}

	case ReferenceModeDatagram:
	{
	    s += " -d";
	    break;
	}

	case ReferenceModeBatchDatagram:
	{
	    s += " -D";
	    break;
	}
    }

    if(getSecure())
    {
	s += " -s";
    }

    return s;

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

    if(_secure != r._secure)
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

    if(_overrideTimeout != r._overrideTimeout || _overrideTimeout && _timeout != r._timeout)
    {
	return false;
    }

    return true;
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
    
    if(!_secure && r._secure)
    {
	return true;
    }
    else if(r._secure < _secure)
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

    if(!_overrideTimeout && r._overrideTimeout)
    {
	return true;
    }
    else if(r._overrideTimeout < _overrideTimeout)
    {
	return false;
    }
    else if(_overrideTimeout)
    {
	if(_timeout < r._timeout)
	{
	    return true;
	}
	else if(r._timeout < _timeout)
	{
	    return false;
	}
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

IceInternal::Reference::Reference(const InstancePtr& inst, const CommunicatorPtr& com, const Identity& ident,
				  const Context& context, const string& fs, ReferenceMode md, bool sec) :
    _hashInitialized(false),
    _instance(inst),
    _communicator(com),
    _mode(md),
    _secure(sec),
    _identity(ident),
    _context(context),
    _facet(fs),
    _overrideTimeout(false),
    _timeout(-1)
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
    _overrideTimeout(r._overrideTimeout),
    _timeout(r._timeout)
{
}

void
IceInternal::Reference::applyOverrides(vector<EndpointPtr>& endpts) const
{
    //
    // Apply the endpoint overrides to each endpoint.
    //
    for(vector<EndpointPtr>::iterator p = endpts.begin(); p != endpts.end(); ++p)
    {
	if(_overrideTimeout)
	{
		*p = (*p)->timeout(_timeout);
	}
    }
}

IceUtil::Shared* IceInternal::upCast(IceInternal::FixedReference* p) { return p; }

IceInternal::FixedReference::FixedReference(const InstancePtr& inst, const CommunicatorPtr& com, const Identity& ident,
					    const Context& context, const string& fs, ReferenceMode md, 
					    const vector<ConnectionPtr>& fixedConns) :
    Reference(inst, com, ident, context, fs, md, false),
    _fixedConnections(fixedConns)
{
}

Reference::Type
IceInternal::FixedReference::getType() const
{
    return TypeFixed;
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
    throw FixedProxyException(__FILE__, __LINE__);
    return 0; // Keep the compiler happy.
}

#endif

#ifdef ICEE_HAS_LOCATOR

string
IceInternal::FixedReference::getAdapterId() const
{
    return string();
}

ReferencePtr
IceInternal::FixedReference::changeAdapterId(const std::string&) const
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

#endif

ReferencePtr
IceInternal::FixedReference::changeTimeout(int) const
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
    return string(); // To keep the compiler from complaining.
}

ConnectionPtr
IceInternal::FixedReference::getConnection() const
{
    vector<ConnectionPtr> connections = _fixedConnections;
    switch(getMode())
    {
        case ReferenceModeTwoway:
        case ReferenceModeOneway:
#ifdef ICEE_HAS_BATCH
        case ReferenceModeBatchOneway:
#endif
        {
            //
            // Filter out datagram connections.
            //
            connections.erase(remove_if(connections.begin(), connections.end(), ConnectionIsDatagram()),
                              connections.end());
            break;
        }
        
        case ReferenceModeDatagram:
#ifdef ICEE_HAS_BATCH
        case ReferenceModeBatchDatagram:
#endif
        {
            //
            // Filter out non-datagram connections.
            //
            connections.erase(remove_if(connections.begin(), connections.end(), not1(ConnectionIsDatagram())),
                              connections.end());
            break;
        }

#ifndef ICEE_HAS_BATCH
        case ReferenceModeBatchDatagram:
        case ReferenceModeBatchOneway:
	{
	    throw FeatureNotSupportedException(__FILE__, __LINE__, "batch proxy mode");
	}
#endif
    }
    
    //
    // Randomize the order of connections.
    //
    random_shuffle(connections.begin(), connections.end());

    //
    // If a secure connection is requested or secure overrides is set,
    // remove all non-secure connections. Otherwise make non-secure
    // connections preferred over secure connections by partitioning
    // the connection vector, so that non-secure connections come
    // first.
    //
    // NOTE: we don't use the stable_partition algorithm from STL to
    // keep the code size down.
    //
    vector<ConnectionPtr>::iterator p = connections.begin();
    vector<ConnectionPtr> secureConnections;
    while(p != connections.end())
    {
	if((*p)->endpoint()->secure())
	{
	    secureConnections.push_back(*p);
	    p = connections.erase(p);
	}
	else
	{
	    ++p;
	}
    }
    if(getSecure())
    {
	connections.swap(secureConnections);
    }
    else
    {
	connections.insert(connections.end(), secureConnections.begin(), secureConnections.end());
    }
    
    if(connections.empty())
    {
	throw NoEndpointException(__FILE__, __LINE__); // No stringified representation for fixed proxies.
    }

    ConnectionPtr connection = connections[0];
    assert(connection);
    connection->throwException(); // Throw in case our connection is already destroyed.

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

#ifdef ICEE_HAS_ROUTER
IceUtil::Shared* IceInternal::upCast(IceInternal::RoutableReference* p) { return p; }

vector<EndpointPtr>
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
    return vector<EndpointPtr>();
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
    return _routerInfo == rhs->_routerInfo;
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
	return _routerInfo < rhs->_routerInfo;
    }
    return false;
}

IceInternal::RoutableReference::RoutableReference(const InstancePtr& inst, const CommunicatorPtr& com,
						  const Identity& ident, const Context& context, const string& fs,
						  ReferenceMode md, bool sec, const RouterInfoPtr& rtrInfo) :
    Reference(inst, com, ident, context, fs, md, sec), _routerInfo(rtrInfo)
{
}

IceInternal::RoutableReference::RoutableReference(const RoutableReference& r) :
    Reference(r), _routerInfo(r._routerInfo)
{
}
#endif

IceUtil::Shared* IceInternal::upCast(IceInternal::DirectReference* p) { return p; }


#ifdef ICEE_HAS_ROUTER
IceInternal::DirectReference::DirectReference(const InstancePtr& inst, const CommunicatorPtr& com,
					      const Identity& ident, const Context& context, const string& fs, ReferenceMode md,
					      bool sec, const vector<EndpointPtr>& endpts,
					      const RouterInfoPtr& rtrInfo) :

    RoutableReference(inst, com, ident, context, fs, md, sec, rtrInfo),
    _endpoints(endpts)
{
}
#else
IceInternal::DirectReference::DirectReference(const InstancePtr& inst, const CommunicatorPtr& com,
					      const Identity& ident, const Context& context, const string& fs, ReferenceMode md,
					      bool sec, const vector<EndpointPtr>& endpts) :
    Reference(inst, com, ident, context, fs, md, sec),
    _endpoints(endpts)
{
}
#endif

Reference::Type
IceInternal::DirectReference::getType() const
{
    return TypeDirect;
}

vector<EndpointPtr>
IceInternal::DirectReference::getEndpoints() const
{
    return _endpoints;
}

#ifdef ICEE_HAS_LOCATOR

string
IceInternal::DirectReference::getAdapterId() const
{
    return string();
}

ReferencePtr
DirectReference::changeAdapterId(const string& newAdapterId) const
{
    if(!newAdapterId.empty())
    {
        LocatorInfoPtr locatorInfo = 
            getInstance()->locatorManager()->get(getInstance()->referenceFactory()->getDefaultLocator());
        return getInstance()->referenceFactory()->create(getIdentity(), *getContext(), getFacet(), getMode(),
                                                         getSecure(), newAdapterId,
#ifdef ICEE_HAS_ROUTER
							 getRouterInfo(),
#endif
                                                         locatorInfo);
    }
    else
    {
        return DirectReferencePtr(const_cast<DirectReference*>(this));
    }
}

ReferencePtr
IceInternal::DirectReference::changeLocator(const LocatorPrx& newLocator) const
{
    return DirectReferencePtr(const_cast<DirectReference*>(this));
}

#endif

ReferencePtr
IceInternal::DirectReference::changeTimeout(int newTimeout) const
{
    DirectReferencePtr r = DirectReferencePtr::dynamicCast(Parent::changeTimeout(newTimeout));
    if(r.get() != this) // Also override the timeout on the endpoints if it was updated.
    {
	vector<EndpointPtr> newEndpoints;
	for(vector<EndpointPtr>::const_iterator p = _endpoints.begin(); p != _endpoints.end(); ++p)
	{
	    newEndpoints.push_back((*p)->timeout(newTimeout));
	}
	r->_endpoints = newEndpoints;
    }
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
    applyOverrides(endpts);

    if(endpts.empty())
    {
	endpts = _endpoints; // Endpoint overrides are already applied on these endpoints.
    }
#else
    vector<EndpointPtr> endpts = _endpoints;
#endif
    vector<EndpointPtr> filteredEndpoints = filterEndpoints(endpts, getMode(), getSecure());
    if(filteredEndpoints.empty())
    {
        throw NoEndpointException(__FILE__, __LINE__, toString());
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
    Parent(r), _endpoints(r._endpoints)
{
}

#ifdef ICEE_HAS_LOCATOR

IceUtil::Shared* IceInternal::upCast(IceInternal::IndirectReference* p) { return p; }

#ifdef ICEE_HAS_ROUTER
IceInternal::IndirectReference::IndirectReference(const InstancePtr& inst, const CommunicatorPtr& com,
						  const Identity& ident, const Context& context, const string& fs,
						  ReferenceMode md, bool sec, const string& adptid,
						  const RouterInfoPtr& rtrInfo, const LocatorInfoPtr& locInfo) :
    RoutableReference(inst, com, ident, context, fs, md, sec, rtrInfo),
    _adapterId(adptid),
    _locatorInfo(locInfo)
{
}
#else
IceInternal::IndirectReference::IndirectReference(const InstancePtr& inst, const CommunicatorPtr& com, 
						  const Identity& ident, const Context& context, const string& fs,
						  ReferenceMode md, bool sec, const string& adptid,
						  const LocatorInfoPtr& locInfo) :
    Reference(inst, com, ident, context, fs, md, sec),
    _adapterId(adptid),
    _locatorInfo(locInfo)
{
}
#endif

Reference::Type
IceInternal::IndirectReference::getType() const
{
    return TypeIndirect;
}

vector<EndpointPtr>
IceInternal::IndirectReference::getEndpoints() const
{
    return vector<EndpointPtr>();
}

string
IceInternal::IndirectReference::getAdapterId() const
{
    return _adapterId;
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
    string a = _adapterId;
#ifdef ICEE_HAS_WSTRING
    if(getInstance()->initializationData().stringConverter)
    {
        UTF8BufferI buffer;
        Byte* last = getInstance()->initializationData().stringConverter->toUTF8(a.data(), a.data() + a.size(), buffer);
        a = string(reinterpret_cast<const char*>(buffer.getBuffer()), last - buffer.getBuffer());
    }
#endif
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

	applyOverrides(endpts);

	vector<EndpointPtr> filteredEndpoints = filterEndpoints(endpts, getMode(), getSecure());
	if(filteredEndpoints.empty())
	{
	    throw NoEndpointException(__FILE__, __LINE__, toString());
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
		    if(traceLevels->retry >= 2)
		    {
		        Trace out(getInstance()->initializationData().logger, traceLevels->retryCat);
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

int
IceInternal::IndirectReference::hash() const
{
    IceUtil::RecMutex::Lock sync(_hashMutex);

    if(_hashInitialized)
    {
        return _hashValue;
    }
#ifdef ICEE_HAS_ROUTER
    RoutableReference::hash(); // Initializes _hashValue.
#else
    Reference::hash(); // Initializes _hashValue.
#endif
    // Add hash of adapter ID to base hash
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
    if(!rhs || !Parent::operator==(r))
    {
        return false;
    }
    return _adapterId == rhs->_adapterId && _locatorInfo == rhs->_locatorInfo;
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
	assert(rhs);
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
    return false;
}

ReferencePtr
IceInternal::IndirectReference::clone() const
{
    return new IndirectReference(*this);
}

IceInternal::IndirectReference::IndirectReference(const IndirectReference& r)
    : Parent(r), 
      _adapterId(r._adapterId), 
      _locatorInfo(r._locatorInfo)
{
}

#endif // ICEE_HAS_LOCATOR

vector<EndpointPtr>
IceInternal::filterEndpoints(const vector<EndpointPtr>& allEndpoints, ReferenceMode m, bool sec)
{
    vector<EndpointPtr> endpoints = allEndpoints;

    //
    // Filter out unknown endpoints.
    //
    endpoints.erase(remove_if(endpoints.begin(), endpoints.end(), Ice::constMemFun(&Endpoint::unknown)),
                    endpoints.end());

    //
    // Filter out endpoints according to the mode of the reference.
    //
    switch(m)
    {
        case ReferenceModeTwoway:
        case ReferenceModeOneway:
#ifdef ICEE_HAS_BATCH
        case ReferenceModeBatchOneway:
#endif
        {
            //
            // Filter out datagram endpoints.
            //
            endpoints.erase(remove_if(endpoints.begin(), endpoints.end(), Ice::constMemFun(&Endpoint::datagram)),
                            endpoints.end());
            break;
        }
        
        case ReferenceModeDatagram:
#ifdef ICEE_HAS_BATCH
        case ReferenceModeBatchDatagram:
#endif
        {
            //
            // Filter out non-datagram endpoints.
            //
	    endpoints.erase(remove_if(endpoints.begin(), endpoints.end(),
				      not1(Ice::constMemFun(&Endpoint::datagram))),
			    endpoints.end());
            break;
        }

#ifndef ICEE_HAS_BATCH
        case ReferenceModeBatchDatagram:
        case ReferenceModeBatchOneway:
	{
	    throw FeatureNotSupportedException(__FILE__, __LINE__, "batch proxy mode");
	}
#endif
    }

    //
    // Randomize the order of endpoints.
    //
    random_shuffle(endpoints.begin(), endpoints.end());
    
    //
    // If a secure connection is requested or secure overrides is set,
    // remove all non-secure endpoints. Otherwise make non-secure
    // endpoints preferred over secure endpoints by partitioning
    // the endpoint vector, so that non-secure endpoints come
    // first.
    //
    // NOTE: we don't use the stable_partition algorithm from STL to
    // keep the code size down.
    //
    vector<EndpointPtr>::iterator p = endpoints.begin();
    vector<EndpointPtr> secureEndpoints;
    while(p != endpoints.end())
    {
	if((*p)->secure())
	{
	    secureEndpoints.push_back(*p);
	    p = endpoints.erase(p);
	}
	else
	{
	    ++p;
	}
    }
    if(sec)
    {
	endpoints.swap(secureEndpoints);
    }
    else
    {
	endpoints.insert(endpoints.end(), secureEndpoints.begin(), secureEndpoints.end());
    }

    return endpoints;
}
