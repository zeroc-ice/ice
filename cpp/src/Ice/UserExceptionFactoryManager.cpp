// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/UserExceptionFactoryManager.h>
#include <Ice/UserExceptionFactory.h>
#include <Ice/Functional.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(UserExceptionFactoryManager* p) { p->__incRef(); }
void IceInternal::decRef(UserExceptionFactoryManager* p) { p->__decRef(); }

void
IceInternal::UserExceptionFactoryManager::add(const UserExceptionFactoryPtr& factory, const string& id)
{
    IceUtil::Mutex::Lock sync(*this);
    _factoryMapHint = _factoryMap.insert(_factoryMapHint, make_pair(id, factory));
}

void
IceInternal::UserExceptionFactoryManager::remove(const string& id)
{
    IceUtil::Mutex::Lock sync(*this);

    map<string, ::Ice::UserExceptionFactoryPtr>::iterator p = _factoryMap.end();
    
    if (_factoryMapHint != _factoryMap.end())
    {
	if (_factoryMapHint->first == id)
	{
	    p = _factoryMapHint;
	}
    }
    
    if (p == _factoryMap.end())
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

UserExceptionFactoryPtr
IceInternal::UserExceptionFactoryManager::find(const string& id)
{
    IceUtil::Mutex::Lock sync(*this);
    
    map<string, ::Ice::UserExceptionFactoryPtr>::iterator p = _factoryMap.end();
    
    if (_factoryMapHint != _factoryMap.end())
    {
	if (_factoryMapHint->first == id)
	{
	    p = _factoryMapHint;
	}
    }
    
    if (p == _factoryMap.end())
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
