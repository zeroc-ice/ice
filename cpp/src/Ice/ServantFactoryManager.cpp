// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/ServantFactoryManager.h>
#include <Ice/ServantFactory.h>
#include <Ice/Functional.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(ServantFactoryManager* p) { p->__incRef(); }
void IceInternal::decRef(ServantFactoryManager* p) { p->__decRef(); }

void
IceInternal::ServantFactoryManager::add(const ServantFactoryPtr& factory, const string& id)
{
    JTCSyncT<JTCMutex> sync(*this);
    _factoryMapHint = _factoryMap.insert(_factoryMapHint, make_pair(id, factory));
}

void
IceInternal::ServantFactoryManager::remove(const string& id)
{
    JTCSyncT<JTCMutex> sync(*this);

    map<string, ::Ice::ServantFactoryPtr>::iterator p = _factoryMap.end();
    
    if (_factoryMapHint != _factoryMap.end())
    {
	if (_factoryMapHint->first == id)
	{
	    p = _factoryMapHint;
	}
    }
    
    if (p != _factoryMap.end())
    {
	p = _factoryMap.find(id);
    }
    
    if (p != _factoryMap.end())
    {
	p->second->destroy();
	_factoryMap.erase(p);
	_factoryMapHint = _factoryMap.end();
    }
}

ServantFactoryPtr
IceInternal::ServantFactoryManager::find(const string& id)
{
    JTCSyncT<JTCMutex> sync(*this);
    
    map<string, ::Ice::ServantFactoryPtr>::iterator p = _factoryMap.end();
    
    if (_factoryMapHint != _factoryMap.end())
    {
	if (_factoryMapHint->first == id)
	{
	    p = _factoryMapHint;
	}
    }
    
    if (p != _factoryMap.end())
    {
	p = _factoryMap.find(id);
    }
    
    if (p != _factoryMap.end())
    {
	_factoryMapHint = p;
	return p->second;
    }
    else
    {
	return 0;
    }
}

IceInternal::ServantFactoryManager::ServantFactoryManager() :
    _factoryMapHint(_factoryMap.end())
{
}

void
IceInternal::ServantFactoryManager::destroy()
{
    JTCSyncT<JTCMutex> sync(*this);
    for_each(_factoryMap.begin(), _factoryMap.end(),
	     secondVoidMemFun<string, ServantFactory>(&ServantFactory::destroy));
    _factoryMap.clear();
    _factoryMapHint = _factoryMap.end();
}
