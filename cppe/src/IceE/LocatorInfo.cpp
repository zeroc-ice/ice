// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/Config.h>

#ifdef ICEE_HAS_LOCATOR

#include <IceE/LocatorInfo.h>
#include <IceE/Locator.h>
#include <IceE/LocalException.h>
#include <IceE/Instance.h>
#include <IceE/TraceLevels.h>
#include <IceE/LoggerUtil.h>
#include <IceE/Endpoint.h>
#include <IceE/Reference.h>
#include <IceE/Functional.h>
#include <iterator>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(LocatorManager* p) { return p; }
IceUtil::Shared* IceInternal::upCast(LocatorInfo* p) { return p; }
IceUtil::Shared* IceInternal::upCast(LocatorTable* p) { return p; }

IceInternal::LocatorManager::LocatorManager() :
    _tableHint(_table.end())
{
}

void
IceInternal::LocatorManager::destroy()
{
    IceUtil::Mutex::Lock sync(*this);

    for_each(_table.begin(), _table.end(), Ice::secondVoidMemFun<const LocatorPrx, LocatorInfo>(&LocatorInfo::destroy));

    _table.clear();
    _tableHint = _table.end();

    _locatorTables.clear();
}

LocatorInfoPtr
IceInternal::LocatorManager::get(const LocatorPrx& loc)
{
    if(!loc)
    {
	return 0;
    }

    LocatorPrx locator = LocatorPrx::uncheckedCast(loc->ice_locator(0)); // The locator can't be located.

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
	map<Identity, LocatorTablePtr>::iterator t = _locatorTables.find(locator->ice_getIdentity());
	if(t == _locatorTables.end())
	{
	    t = _locatorTables.insert(_locatorTables.begin(),
				      pair<const Identity, LocatorTablePtr>(locator->ice_getIdentity(),
				      new LocatorTable()));
	}

	_tableHint = _table.insert(_tableHint, 
				pair<const LocatorPrx, LocatorInfoPtr>(locator, new LocatorInfo(locator, t->second)));
    }
    else
    {
	_tableHint = p;
    }

    return _tableHint->second;
}

IceInternal::LocatorTable::LocatorTable()
{
}

void
IceInternal::LocatorTable::clear()
{
     IceUtil::Mutex::Lock sync(*this);

     _adapterEndpointsMap.clear();
     _objectMap.clear();
}

bool
IceInternal::LocatorTable::getAdapterEndpoints(const string& adapter, vector<EndpointPtr>& endpoints) const
{
    IceUtil::Mutex::Lock sync(*this);
    
    map<string, vector<EndpointPtr> >::const_iterator p = _adapterEndpointsMap.find(adapter);
    
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
IceInternal::LocatorTable::addAdapterEndpoints(const string& adapter, const vector<EndpointPtr>& endpoints)
{
    IceUtil::Mutex::Lock sync(*this);
    
    _adapterEndpointsMap.insert(make_pair(adapter, endpoints));
}

vector<EndpointPtr>
IceInternal::LocatorTable::removeAdapterEndpoints(const string& adapter)
{
    IceUtil::Mutex::Lock sync(*this);
    
    map<string, vector<EndpointPtr> >::iterator p = _adapterEndpointsMap.find(adapter);
    if(p == _adapterEndpointsMap.end())
    {
	return vector<EndpointPtr>();
    }

    vector<EndpointPtr> endpoints = p->second;

    _adapterEndpointsMap.erase(p);
    
    return endpoints;
}

bool 
IceInternal::LocatorTable::getProxy(const Identity& id, ObjectPrx& proxy) const
{
    IceUtil::Mutex::Lock sync(*this);
    
    map<Identity, ObjectPrx>::const_iterator p = _objectMap.find(id);
    
    if(p != _objectMap.end())
    {
	proxy = p->second;
	return true;
    }
    else
    {
	return false;
    }
}

void 
IceInternal::LocatorTable::addProxy(const Identity& id, const ObjectPrx& proxy)
{
    IceUtil::Mutex::Lock sync(*this);
    _objectMap.insert(make_pair(id, proxy));
}

ObjectPrx 
IceInternal::LocatorTable::removeProxy(const Identity& id)
{
    IceUtil::Mutex::Lock sync(*this);
    
    map<Identity, ObjectPrx>::iterator p = _objectMap.find(id);
    if(p == _objectMap.end())
    {
	return 0;
    }

    ObjectPrx proxy = p->second;
    _objectMap.erase(p);
    return proxy;
}

IceInternal::LocatorInfo::LocatorInfo(const LocatorPrx& locator, const LocatorTablePtr& table) :
    _locator(locator),
    _table(table)
{
    assert(_locator);
    assert(_table);
}

void
IceInternal::LocatorInfo::destroy()
{
    IceUtil::Mutex::Lock sync(*this);

    _locatorRegistry = 0;
    _table->clear();
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

	//
	// The locator registry can't be located.
	//
	_locatorRegistry = LocatorRegistryPrx::uncheckedCast(_locatorRegistry->ice_locator(0));
    }
    
    return _locatorRegistry;
}

vector<EndpointPtr>
IceInternal::LocatorInfo::getEndpoints(const IndirectReferencePtr& ref, bool& cached)
{
    vector<EndpointPtr> endpoints;
    ObjectPrx object;
    cached = true;    

    try
    {
	if(!ref->getAdapterId().empty())
	{
	    if(!_table->getAdapterEndpoints(ref->getAdapterId(), endpoints))
	    {
		cached = false;

                if(ref->getInstance()->traceLevels()->location >= 1)
                {
                    Trace out(ref->getInstance()->initializationData().logger,
                              ref->getInstance()->traceLevels()->locationCat);
                    out << "searching for adapter by id" << "\n";
                    out << "adapter = " << ref->getAdapterId();
                }
	    
		object = _locator->findAdapterById(ref->getAdapterId());
		if(object)
		{
		    endpoints = object->__reference()->getEndpoints();
		    _table->addAdapterEndpoints(ref->getAdapterId(), endpoints);
		}
	    }
	}
	else
	{
	    bool objectCached = true;
	    if(!_table->getProxy(ref->getIdentity(), object))
	    {

                if(ref->getInstance()->traceLevels()->location >= 1)
                {
                    Trace out(ref->getInstance()->initializationData().logger,
                              ref->getInstance()->traceLevels()->locationCat);
                    out << "searching for object by id" << "\n";
                    out << "object = " << ref->getInstance()->identityToString(ref->getIdentity());
                }

		objectCached = false;
		object = _locator->findObjectById(ref->getIdentity());
	    }

	    bool endpointsCached = true;
	    if(object)
	    {
		DirectReferencePtr odr = DirectReferencePtr::dynamicCast(object->__reference());
		if(odr)
		{
		    endpointsCached = false;
		    endpoints = odr->getEndpoints();
		}
		else
		{
		    IndirectReferencePtr oir = IndirectReferencePtr::dynamicCast(object->__reference());
		    assert(oir);
		    if(!oir->getAdapterId().empty())
		    {
			endpoints = getEndpoints(oir, endpointsCached);
		    }
		}
	    }

	    if(!objectCached && !endpoints.empty())
	    {
		_table->addProxy(ref->getIdentity(), object);
	    }

	    cached = objectCached || endpointsCached;
	}
    }
    catch(const AdapterNotFoundException&)
    {
        if(ref->getInstance()->traceLevels()->location >= 1)
        {
            Trace out(ref->getInstance()->initializationData().logger,
                      ref->getInstance()->traceLevels()->locationCat);
            out << "adapter not found" << "\n";
            out << "adapter = " << ref->getAdapterId();
        }

	NotRegisteredException ex(__FILE__, __LINE__);
	ex.kindOfObject = "object adapter";
	ex.id = ref->getAdapterId();
	throw ex;
    }
    catch(const ObjectNotFoundException&)
    {
        if(ref->getInstance()->traceLevels()->location >= 1)
        {
            Trace out(ref->getInstance()->initializationData().logger,
                      ref->getInstance()->traceLevels()->locationCat);
            out << "object not found" << "\n";
            out << "object = " << ref->getInstance()->identityToString(ref->getIdentity());
        }

	NotRegisteredException ex(__FILE__, __LINE__);
	ex.kindOfObject = "object";
	ex.id = ref->getInstance()->identityToString(ref->getIdentity());
	throw ex;
    }
    catch(const NotRegisteredException&)
    {
	throw;
    }
    catch(const LocalException& ex)
    {
	if(ref->getInstance()->traceLevels()->location >= 1)
	{
	    Trace out(ref->getInstance()->initializationData().logger, ref->getInstance()->traceLevels()->locationCat);
	    out << "couldn't contact the locator to retrieve adapter endpoints\n";
	    if(!ref)
	    {
		out << "object = " << ref->getInstance()->identityToString(ref->getIdentity()) << "\n";
	    }
	    else
	    {
		out << "adapter = " << ref->getAdapterId() << "\n";
	    }
	    out << "reason = " << ex.toString();
	}
	throw;
    }

    if(ref->getInstance()->traceLevels()->location >= 1)
    {
        if(!endpoints.empty())
        {
            if(cached)
            {
                trace("found endpoints in locator table", ref, endpoints);
            }
            else
            {
                trace("retrieved endpoints from locator, adding to locator table", ref, endpoints);
            }
        }
        else
        {
            Trace out(ref->getInstance()->initializationData().logger, ref->getInstance()->traceLevels()->locationCat);
            out << "no endpoints configured for ";
            if(ref->getAdapterId().empty())
            {
                out << "object\n";
                out << "object = " << ref->getInstance()->identityToString(ref->getIdentity());
            }
            else
            {
                out << "adapter\n";
                out << "adapter = " << ref->getAdapterId();
            }
        }
    }

    return endpoints;
}

void
IceInternal::LocatorInfo::clearObjectCache(const IndirectReferencePtr& ref)
{
    if(ref->getAdapterId().empty())
    {
	ObjectPrx object = _table->removeProxy(ref->getIdentity());
	if(object)
	{
	    IndirectReferencePtr oir = IndirectReferencePtr::dynamicCast(object->__reference());
	    if(oir)
	    {
	        if(!oir->getAdapterId().empty())
		{
		    clearCache(oir);
		}
	    }
	    else
	    {
		if(ref->getInstance()->traceLevels()->location >= 2)
		{
		    trace("removed endpoints from locator table", ref, object->__reference()->getEndpoints());
		}
	    }
	}
    }
}

void 
IceInternal::LocatorInfo::clearCache(const IndirectReferencePtr& ref)
{
    if(!ref->getAdapterId().empty())
    {
	vector<EndpointPtr> endpoints = _table->removeAdapterEndpoints(ref->getAdapterId());

	if(!endpoints.empty() && ref->getInstance()->traceLevels()->location >= 2)
	{
	    trace("removed endpoints from locator table", ref, endpoints);
	}
    }
    else
    {
	ObjectPrx object = _table->removeProxy(ref->getIdentity());
	if(object)
	{
	    IndirectReferencePtr oir = IndirectReferencePtr::dynamicCast(object->__reference());
	    if(oir)
	    {
	        if(!oir->getAdapterId().empty())
		{
		    clearCache(oir);
		}
	    }
	    else
	    {
		if(ref->getInstance()->traceLevels()->location >= 2)
		{
		    trace("removed endpoints from locator table", ref, object->__reference()->getEndpoints());
		}
	    }
	}
    }
}

void
IceInternal::LocatorInfo::trace(const string& msg,
	                        const IndirectReferencePtr& ref,
				const vector<EndpointPtr>& endpoints)
{
    Trace out(ref->getInstance()->initializationData().logger, ref->getInstance()->traceLevels()->locationCat);
    out << msg << "\n";
    if(!ref->getAdapterId().empty())
    {
	out << "adapter = "  << ref->getAdapterId() << "\n";
    }
    else
    {
	out << "object = "  << ref->getInstance()->identityToString(ref->getIdentity()) << "\n";
    }

    const char* sep = endpoints.size() > 1 ? ":" : "";
    out << "endpoints = ";
    for(unsigned int i = 0; i < endpoints.size(); ++i)
    {
        out << endpoints[i]->toString() << sep;
    }
}

#endif
