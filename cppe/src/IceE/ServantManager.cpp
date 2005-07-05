// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/ServantManager.h>
#include <IceE/LocalException.h>
#include <IceE/IdentityUtil.h>
#include <IceE/LoggerUtil.h>
#include <IceE/Instance.h>
#include <IceE/StringUtil.h>

using namespace std;
using namespace IceE;
using namespace IceEInternal;

void IceEInternal::incRef(ServantManager* p) { p->__incRef(); }
void IceEInternal::decRef(ServantManager* p) { p->__decRef(); }

void
IceEInternal::ServantManager::addServant(const ObjectPtr& object, const Identity& ident, const string& facet)
{
    IceE::Mutex::Lock sync(*this);
    
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
	    ex.id = identityToString(ident);
	    if(!facet.empty())
	    {
		ex.id += " -f " + IceE::escapeString(facet, "");
	    }
	    throw ex;
	}
    }

    _servantMapMapHint = p;

    p->second.insert(pair<const string, ObjectPtr>(facet, object));
}

ObjectPtr
IceEInternal::ServantManager::removeServant(const Identity& ident, const string& facet)
{
    //
    // We return the removed servant to avoid releasing the last reference count
    // with *this locked. We don't want to run user code, such as the servant
    // destructor, with an internal Ice mutex locked.
    //
    ObjectPtr servant = 0;

    IceE::Mutex::Lock sync(*this);
    
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
	ex.id = identityToString(ident);
	if(!facet.empty())
	{
	    ex.id += " -f " + IceE::escapeString(facet, "");
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
IceEInternal::ServantManager::removeAllFacets(const Identity& ident)
{
    IceE::Mutex::Lock sync(*this);
    
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
	ex.id = identityToString(ident);
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
IceEInternal::ServantManager::findServant(const Identity& ident, const string& facet) const
{
    IceE::Mutex::Lock sync(*this);
    
    assert(_instance); // Must not be called after destruction.

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
IceEInternal::ServantManager::findAllFacets(const Identity& ident) const
{
    IceE::Mutex::Lock sync(*this);
    
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
IceEInternal::ServantManager::hasServant(const Identity& ident) const
{
    IceE::Mutex::Lock sync(*this);
    
    assert(_instance); // Must not be called after destruction.

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

IceEInternal::ServantManager::ServantManager(const InstancePtr& instance, const string& adapterName)
    : _instance(instance),
      _adapterName(adapterName),
      _servantMapMapHint(_servantMapMap.end())
{
}

IceEInternal::ServantManager::~ServantManager()
{
    //
    // Don't check whether destroy() has been called. It might have
    // not been called if the associated object adapter was not
    // properly deactivated.
    //
    //assert(!_instance);
}

void
IceEInternal::ServantManager::destroy()
{
    ServantMapMap servantMapMap;

    {
	IceE::Mutex::Lock sync(*this);
	
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
