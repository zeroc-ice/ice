// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
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

    for_each(_table.begin(), _table.end(), Ice::secondVoidMemFun<LocatorPrx, LocatorInfo>(&LocatorInfo::destroy));

    _table.clear();
    _tableHint = _table.end();

    _adapterTables.clear();
}

LocatorInfoPtr
IceInternal::LocatorManager::get(const LocatorPrx& locator)
{
    if(!locator)
    {
	return 0;
    }

    //
    // TODO: reap unused locator info objects?
    //

    IceUtil::Mutex::Lock sync(*this);

    map<LocatorPrx, LocatorInfoPtr>::iterator p = _table.end();
    
    if(_tableHint != _table.end())
    {
	if(_tableHint->first == locator)
	{
	    p = _tableHint;
	}
    }
    
    if(p == _table.end())
    {
	p = _table.find(locator);
    }

    if(p == _table.end())
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
    
    if(p != _adapterEndpointsMap.end())
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
IceInternal::LocatorAdapterTable::clear()
{
     IceUtil::Mutex::Lock sync(*this);

     _adapterEndpointsMap.clear();
}

void
IceInternal::LocatorAdapterTable::add(const string& adapter, const ::std::vector<EndpointPtr>& endpoints)
{
    IceUtil::Mutex::Lock sync(*this);
    
    _adapterEndpointsMap.insert(make_pair(adapter, endpoints));
}

::std::vector<EndpointPtr>
IceInternal::LocatorAdapterTable::remove(const string& adapter)
{
    IceUtil::Mutex::Lock sync(*this);
    
    std::map<std::string, std::vector<EndpointPtr> >::iterator p = _adapterEndpointsMap.find(adapter);
    if(p == _adapterEndpointsMap.end())
    {
	return std::vector<EndpointPtr>();
    }

    std::vector<EndpointPtr> endpoints = p->second;

    _adapterEndpointsMap.erase(p);
    
    return endpoints;
}


IceInternal::LocatorInfo::LocatorInfo(const LocatorPrx& locator, const LocatorAdapterTablePtr& adapterTable) :
    _locator(locator),
    _adapterTable(adapterTable)
{
    assert(_locator);
    assert(_adapterTable);
}

void
IceInternal::LocatorInfo::destroy()
{
    IceUtil::Mutex::Lock sync(*this);

    _locator = 0;
    _locatorRegistry = 0;
    _adapterTable->clear();
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
IceInternal::LocatorInfo::getLocator() const
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
    
    if(!_locatorRegistry) // Lazy initialization.
    {
	_locatorRegistry = _locator->getRegistry();
    }
    
    return _locatorRegistry;
}

std::vector<EndpointPtr>
IceInternal::LocatorInfo::getEndpoints(const ReferencePtr& ref, bool& cached)
{
    ::std::vector<EndpointPtr> endpoints;

    if(!ref->adapterId.empty())
    {
	cached = true;

	if(!_adapterTable->get(ref->adapterId, endpoints))
	{
	    cached = false;

	    //
	    // Search the adapter in the location service if we didn't
	    // find it in the cache.
	    //
	    try
	    {
		ObjectPrx object = _locator->findAdapterByName(ref->adapterId);
		if(object)
		{
		    endpoints = object->__reference()->endpoints;
		}
	    }
	    catch(LocalException& ex)
	    {
		//
		// Ignore. The proxy will most likely get empty
		// endpoints and raise a NoEndpointException().
		//
	    }

	    //
	    // Add to the cache.
	    //
	    if(!endpoints.empty())
	    {
		_adapterTable->add(ref->adapterId, endpoints);
	    }
	}

	if(!endpoints.empty())
	{
	    if(ref->instance->traceLevels()->location >= 1)
	    {
		Trace out(ref->instance->logger(), ref->instance->traceLevels()->locationCat);
		if(cached)
		{
		    out << "found endpoints in locator table\n";
		}
		else
		{
		    out << "retrieved endpoints from locator, adding to locator table\n";
		}
		out << "adapter = " << ref->adapterId << "\n";
		const char* sep = endpoints.size() > 1 ? ":" : "";
		ostringstream o;
		transform(endpoints.begin(), endpoints.end(), ostream_iterator<string>(o, sep),
			  ::Ice::constMemFun(&Endpoint::toString));
		out << "endpoints = " << o.str();
	    }
	}
    }
    else
    {
	cached = false;
    }

    return endpoints;
}

void 
IceInternal::LocatorInfo::clearCache(const ReferencePtr& ref)
{
    if(!ref->adapterId.empty())
    {
	std::vector<EndpointPtr> endpoints = _adapterTable->remove(ref->adapterId);
	if(!endpoints.empty())
	{
	    if(ref->instance->traceLevels()->location >= 1)
	    {
		Trace out(ref->instance->logger(), ref->instance->traceLevels()->locationCat);
		out << "removed endpoints from locator table\n";
		out << "adapter = "  << ref->adapterId;
		const char* sep = endpoints.size() > 1 ? ":" : "";
		ostringstream o;
		transform(endpoints.begin(), endpoints.end(), ostream_iterator<string>(o, sep),
			  ::Ice::constMemFun(&Endpoint::toString));
		out << "endpoints = " << o.str();
	    }
	}
    }
}

