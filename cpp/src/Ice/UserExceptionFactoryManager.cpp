// **********************************************************************
//
// Copyright (c) 2002
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

#include <Ice/UserExceptionFactoryManager.h>
#include <Ice/UserExceptionFactory.h>
#include <Ice/Functional.h>
#include <Ice/LocalException.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(UserExceptionFactoryManager* p) { p->__incRef(); }
void IceInternal::decRef(UserExceptionFactoryManager* p) { p->__decRef(); }

const char * const UserExceptionFactoryManager::_kindOfObject = "user exception factory";

void
IceInternal::UserExceptionFactoryManager::add(const UserExceptionFactoryPtr& factory, const string& id)
{
    IceUtil::Mutex::Lock sync(*this);

    if(   (_factoryMapHint != _factoryMap.end() && _factoryMapHint->first == id)
       || _factoryMap.find(id) != _factoryMap.end())
    {
	AlreadyRegisteredException ex(__FILE__, __LINE__);
	ex.kindOfObject = _kindOfObject;
	ex.id = id;
	throw ex;
    }

    _factoryMapHint = _factoryMap.insert(_factoryMapHint, make_pair(id, factory));
}

void
IceInternal::UserExceptionFactoryManager::remove(const string& id)
{
    IceUtil::Mutex::Lock sync(*this);

    map<string, ::Ice::UserExceptionFactoryPtr>::iterator p = _factoryMap.end();
    
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
	if(p == _factoryMap.end())
	{
	    NotRegisteredException ex(__FILE__, __LINE__);
	    ex.kindOfObject = _kindOfObject;
	    ex.id = id;
	    throw ex;
	}
    }
    assert(p != _factoryMap.end());
    
    p->second->destroy();

    if(p == _factoryMapHint)
    {
	_factoryMap.erase(p);
	_factoryMapHint = ++p;
    }
    else
    {
	_factoryMap.erase(p);
    }
}

UserExceptionFactoryPtr
IceInternal::UserExceptionFactoryManager::find(const string& id)
{
    IceUtil::Mutex::Lock sync(*this);
    
    map<string, ::Ice::UserExceptionFactoryPtr>::iterator p = _factoryMap.end();
    
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

IceInternal::UserExceptionFactoryManager::UserExceptionFactoryManager() :
    _factoryMapHint(_factoryMap.end())
{
}

void
IceInternal::UserExceptionFactoryManager::destroy()
{
    IceUtil::Mutex::Lock sync(*this);
    for_each(_factoryMap.begin(), _factoryMap.end(),
	     Ice::secondVoidMemFun<string, UserExceptionFactory>(&UserExceptionFactory::destroy));
    _factoryMap.clear();
    _factoryMapHint = _factoryMap.end();
}
