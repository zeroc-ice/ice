// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/ObjectAdapterI.h>
#include <Ice/Instance.h>
#include <Ice/Proxy.h>
#include <Ice/ProxyFactory.h>
#include <Ice/Reference.h>
#include <Ice/Endpoint.h>
#include <Ice/Collector.h>
#include <Ice/LocalException.h>
#include <Ice/Properties.h>
#include <Ice/Functional.h>
#include <sstream>

#ifdef WIN32
#   include <sys/timeb.h>
#else
#   include <sys/time.h>
#endif

using namespace std;
using namespace Ice;
using namespace IceInternal;

string
Ice::ObjectAdapterI::getName()
{
    return _name; // _name is immutable
}

CommunicatorPtr
Ice::ObjectAdapterI::getCommunicator()
{
    return _instance->communicator(); // _instance is immutable
}

void
Ice::ObjectAdapterI::activate()
{
    JTCSyncT<JTCMutex> sync(*this);

    if (_collectorFactories.empty())
    {
	throw ObjectAdapterDeactivatedException(__FILE__, __LINE__);
    }

    for_each(_collectorFactories.begin(), _collectorFactories.end(),
	     ::IceInternal::voidMemFun(&CollectorFactory::activate));
}

void
Ice::ObjectAdapterI::hold()
{
    JTCSyncT<JTCMutex> sync(*this);

    if (_collectorFactories.empty())
    {
	throw ObjectAdapterDeactivatedException(__FILE__, __LINE__);
    }

    for_each(_collectorFactories.begin(), _collectorFactories.end(),
	     ::IceInternal::voidMemFun(& CollectorFactory::hold));
}

void
Ice::ObjectAdapterI::deactivate()
{
    JTCSyncT<JTCMutex> sync(*this);

    if (_collectorFactories.empty())
    {
	throw ObjectAdapterDeactivatedException(__FILE__, __LINE__);
    }

    for_each(_collectorFactories.begin(), _collectorFactories.end(),
	     ::IceInternal::voidMemFun(& CollectorFactory::destroy));
    _collectorFactories.clear();
    _asm.clear();
    _asmHint = _asm.begin();
}

ObjectPrx
Ice::ObjectAdapterI::add(const ObjectPtr& object, const string& ident)
{
    JTCSyncT<JTCMutex> sync(*this);

    if (_collectorFactories.empty())
    {
	throw ObjectAdapterDeactivatedException(__FILE__, __LINE__);
    }

    _asmHint = _asm.insert(_asmHint, make_pair(ident, object));

    return newProxy(ident);
}

ObjectPrx
Ice::ObjectAdapterI::addTemporary(const ObjectPtr& object)
{
    JTCSyncT<JTCMutex> sync(*this);

    if (_collectorFactories.empty())
    {
	throw ObjectAdapterDeactivatedException(__FILE__, __LINE__);
    }

    ostringstream s;

#ifdef WIN32
    struct _timeb tb;
    _ftime(&tb);
    s << hex << '.' << tb.time << '.' << tb.millitm << '.' << rand();
#else
    timeval tv;
    gettimeofday(&tv, 0);
    s << hex << '.' << tv.tv_sec << '.' << tv.tv_usec / 1000 << '.' << rand();
#endif

    _asmHint = _asm.insert(_asmHint, make_pair(s.str(), object));

    return newProxy(s.str());
}

void
Ice::ObjectAdapterI::remove(const string& ident)
{
    JTCSyncT<JTCMutex> sync(*this);

    if (_collectorFactories.empty())
    {
	throw ObjectAdapterDeactivatedException(__FILE__, __LINE__);
    }

    _asm.erase(ident);
    _asmHint = _asm.begin();
}

void
Ice::ObjectAdapterI::setServantLocator(const ServantLocatorPtr& locator)
{
    JTCSyncT<JTCMutex> sync(*this);
    _locator = locator;
}

ServantLocatorPtr
Ice::ObjectAdapterI::getServantLocator()
{
    JTCSyncT<JTCMutex> sync(*this);
    return _locator;
}

ObjectPtr
Ice::ObjectAdapterI::identityToServant(const string& ident)
{
    JTCSyncT<JTCMutex> sync(*this);

    map<string, ObjectPtr>::const_iterator p = _asm.find(ident);
    if (p != _asm.end())
    {
	return p->second;
    }
    else
    {
	return 0;
    }
}

ObjectPtr
Ice::ObjectAdapterI::proxyToServant(const ObjectPrx& proxy)
{
    //
    // We must first check whether at least one endpoint contained in
    // the proxy matches this object adapter.
    //
    ReferencePtr ref = proxy->__reference();
    vector<EndpointPtr>::const_iterator p;
    for (p = ref->endpoints.begin(); p != ref->endpoints.end(); ++p)
    {
	vector<CollectorFactoryPtr>::const_iterator q;
	for (q = _collectorFactories.begin(); q != _collectorFactories.end(); ++q)
	{
	    if ((*q)->equivalent(*p))
	    {
		//
		// OK, endpoints and object adapter match. Let's find
		// the object.
		//
		return identityToServant(ref->identity);
	    }
	}
    }

    throw WrongObjectAdapterException(__FILE__, __LINE__);
}

ObjectPrx
Ice::ObjectAdapterI::createProxy(const string& ident)
{
    JTCSyncT<JTCMutex> sync(*this);
    
    if (_collectorFactories.empty())
    {
	throw ObjectAdapterDeactivatedException(__FILE__, __LINE__);
    }
    
    return newProxy(ident);
}

Ice::ObjectAdapterI::ObjectAdapterI(const InstancePtr& instance, const string& name, const string& endpts) :
    _instance(instance),
    _name(name),
    _asmHint(_asm.begin())
{
    string s(endpts);
    transform(s.begin(), s.end(), s.begin(), tolower);

    string::size_type beg = 0;
    string::size_type end;
    
    //
    // Set the "no delete" flag to true, meaning that this object will
    // not be deleted, even if the reference count drops to zero. This
    // is needed because if the constructor of the CollectorFactory
    // throws an exception, or if the CollectorFactories are destroyed
    // with "deactivate" from within this constructor, all
    // ObjectAdapterPtrs for this object will be destroyed, and thus
    // this object would be deleted if the "no delete" flag is not
    // set.
    //
    __setNoDelete(true);

    try
    {
	while (true)
	{
	    end = s.find(':', beg);
	    if (end == string::npos)
	    {
		end = s.length();
	    }
	    
	    if (end == beg)
	    {
		throw EndpointParseException(__FILE__, __LINE__);
	    }
	    
	    string es = s.substr(beg, end - beg);
	    
	    // Don't store the endpoint in the adapter. The Collector
	    // might change it, for example, to fill in the real port
	    // number if a zero port number is given.
	    EndpointPtr endp = Endpoint::endpointFromString(es);
	    _collectorFactories.push_back(new CollectorFactory(instance, this, endp));
	    
	    if (end == s.length())
	    {
		break;
	    }
	    
	    beg = end + 1;
	}
    }
    catch(...)
    {
	if (!_collectorFactories.empty())
	{
	    deactivate();
	}
	__setNoDelete(false);
	throw;
    }

    __setNoDelete(false);
    
    if (_collectorFactories.empty())
    {
	throw EndpointParseException(__FILE__, __LINE__);
    }

    string value = _instance->properties()->getProperty("Ice.PrintAdapterReady");
    if (atoi(value.c_str()) >= 1)
    {
	cout << _name << " ready" << endl;
    }
}

Ice::ObjectAdapterI::~ObjectAdapterI()
{
    if (!_collectorFactories.empty())
    {
	deactivate();
    }
}

ObjectPrx
Ice::ObjectAdapterI::newProxy(const string& ident)
{
    vector<EndpointPtr> endpoints;
    transform(_collectorFactories.begin(), _collectorFactories.end(), back_inserter(endpoints),
	      ::IceInternal::constMemFun(&CollectorFactory::endpoint));

    ReferencePtr reference = new Reference(_instance, ident, Reference::ModeTwoway, false, endpoints, endpoints);
    return _instance->proxyFactory()->referenceToProxy(reference);
}
