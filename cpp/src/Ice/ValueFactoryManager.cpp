
// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <Ice/ValueFactoryManager.h>
#include <Ice/ValueFactory.h>
#include <Ice/ObjectFactory.h>
#include <Ice/Object.h>
#include <Ice/Functional.h>
#include <Ice/LocalException.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(ValueFactoryManager* p) { return p; }

#ifndef ICE_CPP11_MAPPING
IceUtil::Shared* IceInternal::upCast(ValueFactoryWrapper* p) { return p; }

ValueFactoryWrapper::ValueFactoryWrapper(const Ice::ObjectFactoryPtr& factory) : _objectFactory(factory)
{
}

Ice::ValuePtr
ValueFactoryWrapper::create(const string& id)
{
    return _objectFactory->create(id);
}
#endif

void
IceInternal::ValueFactoryManager::add(const ICE_VALUE_FACTORY& factory, const string& id)
{
    IceUtil::Mutex::Lock sync(*this);

    if((_factoryMapHint != _factoryMap.end() && _factoryMapHint->first == id)
       || _factoryMap.find(id) != _factoryMap.end())
    {
        AlreadyRegisteredException ex(__FILE__, __LINE__);
        ex.kindOfObject = "value factory";
        ex.id = id;
        throw ex;
    }

    _factoryMapHint = _factoryMap.insert(_factoryMapHint, pair<const string, ICE_VALUE_FACTORY>(id, factory));
}

void
IceInternal::ValueFactoryManager::add(const Ice::ObjectFactoryPtr& factory, const string& id)
{
    IceUtil::Mutex::Lock sync(*this);

    // Check value factory map since an object factory is added to both object
    // and value factory maps, but not vise versa
    if((_factoryMapHint != _factoryMap.end() && _factoryMapHint->first == id)
       || _factoryMap.find(id) != _factoryMap.end())
    {
        AlreadyRegisteredException ex(__FILE__, __LINE__);
        ex.kindOfObject = "value factory";
        ex.id = id;
        throw ex;
    }

#ifdef ICE_CPP11_MAPPING
    _factoryMapHint = _factoryMap.insert(_factoryMapHint,
                                         pair<const string, ICE_VALUE_FACTORY>(id,
                                            [factory](const string& id)
                                            {
                                                return factory->create(id);
                                            }
                                        ));
#else
    _factoryMapHint = _factoryMap.insert(_factoryMapHint,
                                         pair<const string, ICE_VALUE_FACTORY>(id, new ValueFactoryWrapper(factory)));
#endif

    _objectFactoryMapHint = _objectFactoryMap.insert(_objectFactoryMapHint,
                                                     pair<const string, Ice::ObjectFactoryPtr>(id, factory));
}

ICE_VALUE_FACTORY
IceInternal::ValueFactoryManager::find(const string& id) const
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

Ice::ObjectFactoryPtr
IceInternal::ValueFactoryManager::findObjectFactory(const string& id) const
{
    IceUtil::Mutex::Lock sync(*this);

    ObjectFactoryMap& objectfactoryMap = const_cast<ObjectFactoryMap&>(_objectFactoryMap);

    ObjectFactoryMap::iterator p = objectfactoryMap.end();
    if(_objectFactoryMapHint != objectfactoryMap.end())
    {
        if(_objectFactoryMapHint->first == id)
        {
            p = _objectFactoryMapHint;
        }
    }

    if(p == objectfactoryMap.end())
    {
        p = objectfactoryMap.find(id);
    }

    if(p != objectfactoryMap.end())
    {
        _objectFactoryMapHint = p;
        return p->second;
    }
    else
    {
        return ICE_NULLPTR;
    }
}

IceInternal::ValueFactoryManager::ValueFactoryManager() :
    _factoryMapHint(_factoryMap.end()),
    _objectFactoryMapHint(_objectFactoryMap.end())
{
}

void
IceInternal::ValueFactoryManager::destroy()
{
    ObjectFactoryMap oldMap;
    {
        IceUtil::Mutex::Lock sync(*this);
        _factoryMap.clear();
        _factoryMapHint = _factoryMap.end();
        oldMap.swap(_objectFactoryMap);
        _objectFactoryMapHint = _objectFactoryMap.end();
    }

    //
    // Destroy all outside lock
    //
    for_each(oldMap.begin(), oldMap.end(),
        Ice::secondVoidMemFun<const string, Ice::ObjectFactory>(&Ice::ObjectFactory::destroy));
}
