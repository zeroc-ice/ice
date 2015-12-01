
// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/ObjectFactoryManager.h>
#include <Ice/ObjectFactory.h>
#include <Ice/Functional.h>
#include <Ice/LocalException.h>
using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(ObjectFactoryManager* p) { return p; }

void
IceInternal::ObjectFactoryManager::add(const ICE_OBJECT_FACTORY& factory, const string& id)
{
    IceUtil::Mutex::Lock sync(*this);

    if((_factoryMapHint != _factoryMap.end() && _factoryMapHint->first == id)
       || _factoryMap.find(id) != _factoryMap.end())
    {
        AlreadyRegisteredException ex(__FILE__, __LINE__);
        ex.kindOfObject = "object factory";
        ex.id = id;
        throw ex;
    }

    _factoryMapHint = _factoryMap.insert(_factoryMapHint, pair<const string, ICE_OBJECT_FACTORY>(id, factory));
}

void
IceInternal::ObjectFactoryManager::remove(const string& id)
{
    ICE_OBJECT_FACTORY factory = ICE_NULLPTR;
    {
        IceUtil::Mutex::Lock sync(*this);

        FactoryMap::iterator p = _factoryMap.end();
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
                ex.kindOfObject = "object factory";
                ex.id = id;
                throw ex;
            }
        }
        assert(p != _factoryMap.end());

        factory = p->second;

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

    //
    // Destroy outside the lock
    //
    assert(factory != 0);
#ifndef ICE_CPP11_MAPPING
    factory->destroy();
#endif
}

ICE_OBJECT_FACTORY
IceInternal::ObjectFactoryManager::find(const string& id) const
{
    IceUtil::Mutex::Lock sync(*this);

    FactoryMap& factoryMap = const_cast<FactoryMap&>(_factoryMap);

    FactoryMap::iterator p = factoryMap.end();
    if(_factoryMapHint != factoryMap.end())
    {
        if(_factoryMapHint->first == id)
        {
            p = _factoryMapHint;
        }
    }

    if(p == factoryMap.end())
    {
        p = factoryMap.find(id);
    }

    if(p != factoryMap.end())
    {
        _factoryMapHint = p;
        return p->second;
    }
    else
    {
        return ICE_NULLPTR;
    }
}

IceInternal::ObjectFactoryManager::ObjectFactoryManager() :
    _factoryMapHint(_factoryMap.end())
{
}

void
IceInternal::ObjectFactoryManager::destroy()
{
#ifdef ICE_CPP11_MAPPING
    IceUtil::Mutex::Lock sync(*this);
    _factoryMap.clear();
    _factoryMapHint = _factoryMap.end();
#else
    FactoryMap oldMap;
    {
        IceUtil::Mutex::Lock sync(*this);
        oldMap.swap(_factoryMap);
        _factoryMapHint = _factoryMap.end();
    }

    //
    // Destroy all outside lock
    //
    for_each(oldMap.begin(), oldMap.end(),
             Ice::secondVoidMemFun<const string, ObjectFactory>(&ObjectFactory::destroy));
#endif
}
