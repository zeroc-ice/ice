// **********************************************************************
//
// Copyright (c) 2001
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

#include <Ice/ObjectFactoryManager.h>
#include <Ice/ObjectFactory.h>
#include <Ice/Functional.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(ObjectFactoryManager* p) { p->__incRef(); }
void IceInternal::decRef(ObjectFactoryManager* p) { p->__decRef(); }

void
IceInternal::ObjectFactoryManager::add(const ObjectFactoryPtr& factory, const string& id)
{
    IceUtil::Mutex::Lock sync(*this);
    _factoryMapHint = _factoryMap.insert(_factoryMapHint, make_pair(id, factory));
}

void
IceInternal::ObjectFactoryManager::remove(const string& id)
{
    IceUtil::Mutex::Lock sync(*this);

    map<string, ::Ice::ObjectFactoryPtr>::iterator p = _factoryMap.end();
    
    if(_factoryMapHint != _factoryMap.end())
    {
	if(_factoryMapHint->first == id)
	{
	    p = _factoryMapHint;
	}
    }
    
    if(p == _factoryMap.end())
    {
	p = _factoryMap.find(id);
    }
    
    if(p != _factoryMap.end())
    {
	p->second->destroy();

	if(p == _factoryMapHint)
	{
	    _factoryMap.erase(p++);
	    _factoryMapHint = p;
	}
	else
	{
	    _factoryMap.erase(p);
	}
    }
}

ObjectFactoryPtr
IceInternal::ObjectFactoryManager::find(const string& id)
{
    IceUtil::Mutex::Lock sync(*this);
    
    map<string, ::Ice::ObjectFactoryPtr>::iterator p = _factoryMap.end();
    
    if(_factoryMapHint != _factoryMap.end())
    {
	if(_factoryMapHint->first == id)
	{
	    p = _factoryMapHint;
	}
    }
    
    if(p == _factoryMap.end())
    {
	p = _factoryMap.find(id);
    }
    
    if(p != _factoryMap.end())
    {
	_factoryMapHint = p;
	return p->second;
    }
    else
    {
	return 0;
    }
}

IceInternal::ObjectFactoryManager::ObjectFactoryManager() :
    _factoryMapHint(_factoryMap.end())
{
}

void
IceInternal::ObjectFactoryManager::destroy()
{
    IceUtil::Mutex::Lock sync(*this);
    for_each(_factoryMap.begin(), _factoryMap.end(),
	     Ice::secondVoidMemFun<string, ObjectFactory>(&ObjectFactory::destroy));
    _factoryMap.clear();
    _factoryMapHint = _factoryMap.end();
}
