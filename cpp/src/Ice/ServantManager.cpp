// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <Ice/ServantManager.h>
#include <Ice/ServantLocator.h>
#include <Ice/LocalException.h>
#include <Ice/IdentityUtil.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Instance.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(ServantManager* p) { p->__incRef(); }
void IceInternal::decRef(ServantManager* p) { p->__decRef(); }

void
IceInternal::ServantManager::addServant(const ObjectPtr& object, const Identity& ident)
{
    IceUtil::Mutex::Lock sync(*this);
    
    assert(_instance); // Must not be called after destruction.

    if(_servantMap.find(ident) != _servantMap.end())
    {
	AlreadyRegisteredException ex(__FILE__, __LINE__);
	ex.kindOfObject = "servant";
	ex.id = identityToString(ident);
	throw ex;
    }
    
    _servantMapHint = _servantMap.insert(_servantMapHint, ObjectDict::value_type(ident, object));
}

void
IceInternal::ServantManager::removeServant(const Identity& ident)
{
    IceUtil::Mutex::Lock sync(*this);
    
    assert(_instance); // Must not be called after destruction.

    ObjectDict::iterator p = _servantMap.find(ident);
    if(p == _servantMap.end())
    {
	NotRegisteredException ex(__FILE__, __LINE__);
	ex.kindOfObject = "servant";
	ex.id = identityToString(ident);
	throw ex;
    }

    _servantMap.erase(p);
    _servantMapHint = _servantMap.end();
}

ObjectPtr
IceInternal::ServantManager::findServant(const Identity& ident) const
{
    IceUtil::Mutex::Lock sync(*this);

    assert(_instance); // Must not be called after destruction.

    if(_servantMap.end() != _servantMapHint)
    {
	if(_servantMapHint->first == ident)
	{
	    return _servantMapHint->second;
	}
    }
    
    ObjectDict::iterator p = const_cast<Ice::ObjectDict&>(_servantMap).find(ident);
    if(_servantMap.end() != p)
    {
	_servantMapHint = p;
	return p->second;
    }
    else
    {
	return 0;
    }
}

void
IceInternal::ServantManager::addServantLocator(const ServantLocatorPtr& locator, const string& prefix)
{
    IceUtil::Mutex::Lock sync(*this);

    assert(_instance); // Must not be called after destruction.

    if(_locatorMap.find(prefix) != _locatorMap.end())
    {
	AlreadyRegisteredException ex(__FILE__, __LINE__);
	ex.kindOfObject = "servant locator";
	ex.id = prefix;
	throw ex;
    }
    
    _locatorMapHint = _locatorMap.insert(_locatorMapHint, pair<const string, ServantLocatorPtr>(prefix, locator));
}

ServantLocatorPtr
IceInternal::ServantManager::findServantLocator(const string& prefix) const
{
    IceUtil::Mutex::Lock sync(*this);
    
    assert(_instance); // Must not be called after destruction.

    if(_locatorMap.end() != _locatorMapHint)
    {
	if(_locatorMapHint->first == prefix)
	{
	    return _locatorMapHint->second;
	}
    }
    
    map<string, ServantLocatorPtr>::iterator p = const_cast<map<string, ServantLocatorPtr>&>(_locatorMap).find(prefix);
    if(_locatorMap.end() != p)
    {
	_locatorMapHint = p;
	return p->second;
    }
    else
    {
	return 0;
    }

}

IceInternal::ServantManager::ServantManager(const InstancePtr& instance, const string& adapterName)
    : _instance(instance),
      _adapterName(adapterName),
      _servantMapHint(_servantMap.end()),
      _locatorMapHint(_locatorMap.end())
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
    IceUtil::Mutex::Lock sync(*this);
    
    assert(_instance); // Must not be called after destruction.

    _servantMap.clear();
    _servantMapHint = _servantMap.end();

    for(map<string, ServantLocatorPtr>::const_iterator p = _locatorMap.begin(); p != _locatorMap.end(); ++p)
    {
	try
	{
	    p->second->deactivate(p->first);
	}
	catch(const Exception& ex)
	{
	    Error out(_instance->logger());
	    out << "exception during locator deactivation:\n"
		<< "object adapter: `" << _adapterName << "'\n"
		<< "locator prefix: `" << p->first << "'\n"
		<< ex;
	}
	catch(...)
	{
	    Error out(_instance->logger());
	    out << "unknown exception during locator deactivation:\n"
		<< "object adapter: `" << _adapterName << "'\n"
		<< "locator prefix: `" << p->first << "'";
	}
    }

    _locatorMap.clear();
    _locatorMapHint = _locatorMap.end();

    _instance = 0;
}
