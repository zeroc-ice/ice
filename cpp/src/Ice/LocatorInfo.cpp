// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/LocatorInfo.h>
#include <Ice/Locator.h>
#include <Ice/LocalException.h>
#include <Ice/Instance.h>
#include <Ice/TraceLevels.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Endpoint.h>
#include <Ice/Reference.h>
#include <Ice/Functional.h>
#include <Ice/IdentityUtil.h>

#include <iterator>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(LocatorManager* p) { p->__incRef(); }
void IceInternal::decRef(LocatorManager* p) { p->__decRef(); }

void IceInternal::incRef(LocatorInfo* p) { p->__incRef(); }
void IceInternal::decRef(LocatorInfo* p) { p->__decRef(); }

void IceInternal::incRef(LocatorAdapterTable* p) { p->__incRef(); }
void IceInternal::decRef(LocatorAdapterTable* p) { p->__decRef(); }

IceInternal::LocatorManager::LocatorManager() :
    _tableHint(_table.end())
{
}

void
IceInternal::LocatorManager::destroy()
{
    IceUtil::Mutex::Lock sync(*this);

    _table.clear();
    _tableHint = _table.end();

    _adapterTables.clear();
}

LocatorInfoPtr
IceInternal::LocatorManager::get(const LocatorPrx& locator)
{
    if (!locator)
    {
	return 0;
    }

    //
    // TODO: reap unused locator info objects?
    //

    IceUtil::Mutex::Lock sync(*this);

    map<LocatorPrx, LocatorInfoPtr>::iterator p = _table.end();
    
    if (_tableHint != _table.end())
    {
	if (_tableHint->first == locator)
	{
	    p = _tableHint;
	}
    }
    
    if (p == _table.end())
    {
	p = _table.find(locator);
    }

    if (p == _table.end())
    {
	//
	// Rely on locator identity for the adapter table. We want to
	// have only one table per locator (not one per locator
	// proxy).
	//
	map<Identity, LocatorAdapterTablePtr>::iterator t = _adapterTables.find(locator->ice_getIdentity());
	if(t == _adapterTables.end())
	{
	    t = _adapterTables.insert(_adapterTables.begin(),
				      make_pair(locator->ice_getIdentity(), new LocatorAdapterTable()));
	}

	_tableHint = _table.insert(_tableHint, make_pair(locator, new LocatorInfo(locator, t->second)));
    }
    else
    {
	_tableHint = p;
    }

    return _tableHint->second;
}

IceInternal::LocatorAdapterTable::LocatorAdapterTable()
{
}

bool
IceInternal::LocatorAdapterTable::get(const string& adapter, ::std::vector<EndpointPtr>& endpoints) const
{
    IceUtil::Mutex::Lock sync(*this);
    
    std::map<std::string, std::vector<EndpointPtr> >::const_iterator p = _adapterEndpointsMap.find(adapter);
    
    if (p != _adapterEndpointsMap.end())
    {
	endpoints = p->second;
	return true;
    }
    else
    {
	return false;
    }
}

void
IceInternal::LocatorAdapterTable::add(const string& adapter, const ::std::vector<EndpointPtr>& endpoints)
{
    IceUtil::Mutex::Lock sync(*this);
    
    _adapterEndpointsMap.insert(make_pair(adapter, endpoints));
}

void
IceInternal::LocatorAdapterTable::remove(const string& adapter)
{
    IceUtil::Mutex::Lock sync(*this);
    
    bool removed = _adapterEndpointsMap.erase(adapter) > 0;
    assert(removed);
}


IceInternal::LocatorInfo::LocatorInfo(const LocatorPrx& locator, const LocatorAdapterTablePtr& adapterTable) :
    _locator(locator),
    _adapterTable(adapterTable)
{
    assert(_locator);
    assert(_adapterTable);
}

bool
IceInternal::LocatorInfo::operator==(const LocatorInfo& rhs) const
{
    return _locator == rhs._locator;
}

bool
IceInternal::LocatorInfo::operator!=(const LocatorInfo& rhs) const
{
    return _locator != rhs._locator;
}

bool
IceInternal::LocatorInfo::operator<(const LocatorInfo& rhs) const
{
    return _locator < rhs._locator;
}

LocatorPrx
IceInternal::LocatorInfo::getLocator()
{
    //
    // No mutex lock necessary, _locator is immutable.
    //
    return _locator;
}

LocatorRegistryPrx
IceInternal::LocatorInfo::getLocatorRegistry()
{
    IceUtil::Mutex::Lock sync(*this);
    
    if (!_locatorRegistry) // Lazy initialization.
    {
	_locatorRegistry = _locator->getRegistry();
    }
    
    return _locatorRegistry;
}

bool 
IceInternal::LocatorInfo::getEndpoints(const ReferencePtr& ref, ::std::vector<EndpointPtr>& endpoints) const
{
    bool cached = false;

    if(!ref->adapterId.empty())
    {
	if(!_adapterTable->get(ref->adapterId, endpoints))
	{
	    //
	    // Search the adapter in the location service if we didn't
	    // find it in the cache.
	    //
	    try
	    {
		ObjectPrx object = _locator->findAdapterByName(ref->adapterId);
		if (object)
		{
		    endpoints = object->__reference()->endpoints;
		}
	    }
	    catch(LocalException&)
	    {
		//
		// Ignore. The proxy will most likely get empty
		// endpoints and raise a NoEndpointException().
		//
	    }
	}
	else
	{
	    cached = true;	
	}

	if(endpoints.size() > 0)
	{
	    if (ref->instance->traceLevels()->location >= 1)
	    {
		Trace out(ref->instance->logger(), ref->instance->traceLevels()->locationCat);
		if(cached)
		    out << "found object in local locator table\n";
		else
		    out << "found object in locator\n";
		out << "identity = " << identityToString(ref->identity) << "\n";
		out << "adapter = " << ref->adapterId << "\n";
		const char* sep = endpoints.size() > 1 ? ":" : "";
		ostringstream o;
		transform(endpoints.begin(), endpoints.end(), ostream_iterator<string>(o,sep),
			  ::Ice::constMemFun(&Endpoint::toString));
		out << "endpoints = " << o.str();
	    }
	}
    }

    return cached;
}

void 
IceInternal::LocatorInfo::addEndpoints(const ReferencePtr& ref, const ::std::vector<EndpointPtr>& endpoints)
{
    assert(!endpoints.empty());

    if (!ref->adapterId.empty())
    {
	_adapterTable->add(ref->adapterId, endpoints);

	if (ref->instance->traceLevels()->location >= 2)
	{
	    Trace out(ref->instance->logger(), ref->instance->traceLevels()->locationCat);
	    out << "added adapter to local locator table\n";
	    out << "adapter = "  << ref->adapterId;
	}
    }
}

void 
IceInternal::LocatorInfo::removeEndpoints(const ReferencePtr& ref)
{
    if (!ref->adapterId.empty())
    {
	_adapterTable->remove(ref->adapterId);

	if (ref->instance->traceLevels()->location >= 2)
	{
	    Trace out(ref->instance->logger(), ref->instance->traceLevels()->locationCat);
	    out << "removed adapter from local locator table\n";
	    out << "adapter = "  << ref->adapterId;
	}    
    }
}
