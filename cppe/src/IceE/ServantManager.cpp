// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/ServantManager.h>
#include <IceE/LocalException.h>
#include <IceE/Instance.h>
#include <IceE/StringUtil.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(ServantManager* p) { return p; }

void
IceInternal::ServantManager::addServant(const ObjectPtr& object, const Identity& ident, const string& facet)
{
    IceUtil::Mutex::Lock sync(*this);
    
    assert(_instance); // Must not be called after destruction.

    ServantMapMap::iterator p = _servantMapMapHint;

    if(p == _servantMapMap.end() || p->first != ident)
    {
	p = _servantMapMap.find(ident);
    }

    if(p == _servantMapMap.end())
    {
	p = _servantMapMap.insert(_servantMapMapHint, pair<const Identity, FacetMap>(ident, FacetMap()));
    }
    else
    {
	if(p->second.find(facet) != p->second.end())
	{
	    AlreadyRegisteredException ex(__FILE__, __LINE__);
	    ex.kindOfObject = "servant";
	    ex.id = _instance->identityToString(ident);
	    if(!facet.empty())
	    {
		ex.id += " -f " + IceUtil::escapeString(facet, "");
	    }
	    throw ex;
	}
    }

    _servantMapMapHint = p;

    p->second.insert(pair<const string, ObjectPtr>(facet, object));
}

ObjectPtr
IceInternal::ServantManager::removeServant(const Identity& ident, const string& facet)
{
    //
    // We return the removed servant to avoid releasing the last reference count
    // with *this locked. We don't want to run user code, such as the servant
    // destructor, with an internal Ice mutex locked.
    //
    ObjectPtr servant = 0;

    IceUtil::Mutex::Lock sync(*this);
    
    assert(_instance); // Must not be called after destruction.

    ServantMapMap::iterator p = _servantMapMapHint;
    FacetMap::iterator q;

    if(p == _servantMapMap.end() || p->first != ident)
    {
	p = _servantMapMap.find(ident);
    }
    
    if(p == _servantMapMap.end() || (q = p->second.find(facet)) == p->second.end())
    {
	NotRegisteredException ex(__FILE__, __LINE__);
	ex.kindOfObject = "servant";
	ex.id = _instance->identityToString(ident);
	if(!facet.empty())
	{
	    ex.id += " -f " + IceUtil::escapeString(facet, "");
	}
	throw ex;
    }

    servant = q->second;
    p->second.erase(q);

    if(p->second.empty())
    {
	if(p == _servantMapMapHint)
	{
	    _servantMapMap.erase(p++);
	    _servantMapMapHint = p;
	}
	else
	{
	    _servantMapMap.erase(p);
	}
    }
    return servant;
}

FacetMap
IceInternal::ServantManager::removeAllFacets(const Identity& ident)
{
    IceUtil::Mutex::Lock sync(*this);
    
    assert(_instance); // Must not be called after destruction.

    ServantMapMap::iterator p = _servantMapMapHint;

    if(p == _servantMapMap.end() || p->first != ident)
    {
	p = _servantMapMap.find(ident);
    }
    
    if(p == _servantMapMap.end())
    {
	NotRegisteredException ex(__FILE__, __LINE__);
	ex.kindOfObject = "servant";
	ex.id = _instance->identityToString(ident);
	throw ex;
    }

    FacetMap result = p->second;

    if(p == _servantMapMapHint)
    {
	_servantMapMap.erase(p++);
	_servantMapMapHint = p;
    }
    else
    {
	_servantMapMap.erase(p);
    }

    return result;
}

ObjectPtr
IceInternal::ServantManager::findServant(const Identity& ident, const string& facet) const
{
    IceUtil::Mutex::Lock sync(*this);
    
    //
    // This assert is not valid if the adapter dispatch incoming
    // requests from bidir connections. This method might be called if
    // requests are received over the bidir connection after the
    // adapter was deactivated.
    //	
    //assert(_instance); // Must not be called after destruction.

    ServantMapMap::iterator p = _servantMapMapHint;
    FacetMap::iterator q;
    
    ServantMapMap& servantMapMap = const_cast<ServantMapMap&>(_servantMapMap);

    if(p == servantMapMap.end() || p->first != ident)
    {
	p = servantMapMap.find(ident);
    }
    
    if(p == servantMapMap.end() || (q = p->second.find(facet)) == p->second.end())
    {
	return 0;
    }
    else
    {
	_servantMapMapHint = p;
	return q->second;
    }
}

FacetMap
IceInternal::ServantManager::findAllFacets(const Identity& ident) const
{
    IceUtil::Mutex::Lock sync(*this);
    
    assert(_instance); // Must not be called after destruction.

    ServantMapMap::iterator p = _servantMapMapHint;
    
    ServantMapMap& servantMapMap = const_cast<ServantMapMap&>(_servantMapMap);

    if(p == servantMapMap.end() || p->first != ident)
    {
	p = servantMapMap.find(ident);
    }
    
    if(p == servantMapMap.end())
    {
	return FacetMap();
    }
    else
    {
	_servantMapMapHint = p;
	return p->second;
    }
}

bool
IceInternal::ServantManager::hasServant(const Identity& ident) const
{
    IceUtil::Mutex::Lock sync(*this);
    
    //
    // This assert is not valid if the adapter dispatch incoming
    // requests from bidir connections. This method might be called if
    // requests are received over the bidir connection after the
    // adapter was deactivated.
    //	
    //assert(_instance); // Must not be called after destruction.

    ServantMapMap::iterator p = _servantMapMapHint;
    ServantMapMap& servantMapMap = const_cast<ServantMapMap&>(_servantMapMap);

    if(p == servantMapMap.end() || p->first != ident)
    {
	p = servantMapMap.find(ident);
    }
    
    if(p == servantMapMap.end())
    {
	return false;
    }
    else
    {
	_servantMapMapHint = p;
	assert(!p->second.empty());
	return true;
    }
}

IceInternal::ServantManager::ServantManager(const InstancePtr& instance, const string& adapterName)
    : _instance(instance),
      _adapterName(adapterName),
      _servantMapMapHint(_servantMapMap.end())
{
}

IceInternal::ServantManager::~ServantManager()
{
    //
    // Don't check whether destroy() has been called. It might have
    // not been called if the associated object adapter was not
    // properly deactivated.
    //
    //assert(!_instance);
}

void
IceInternal::ServantManager::destroy()
{
    ServantMapMap servantMapMap;

    {
	IceUtil::Mutex::Lock sync(*this);
	
	assert(_instance); // Must not be called after destruction.
	
	servantMapMap.swap(_servantMapMap);
	_servantMapMapHint = _servantMapMap.end();
	
	_instance = 0;
    }

    //
    // We clear the maps outside the synchronization as we don't want to
    // hold any internal Ice mutex while running user code (such as servant
    // or servant locator destructors). 
    //
    servantMapMap.clear();
}
