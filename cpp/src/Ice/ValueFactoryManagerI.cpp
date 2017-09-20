// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <Ice/ValueFactoryManagerI.h>
#include <Ice/LocalException.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void
IceInternal::ValueFactoryManagerI::add(ICE_IN(ICE_DELEGATE(ValueFactory)) factory, const string& id)
{
    IceUtil::Mutex::Lock sync(*this);

    if((_factoryMapHint != _factoryMap.end() && _factoryMapHint->first == id)
       || _factoryMap.find(id) != _factoryMap.end())
    {
        throw AlreadyRegisteredException(__FILE__, __LINE__, "value factory", id);
    }

    _factoryMapHint = _factoryMap.insert(_factoryMapHint, pair<const string, ICE_DELEGATE(ValueFactory)>(id, factory));
}

ICE_DELEGATE(ValueFactory)
IceInternal::ValueFactoryManagerI::find(const string& id) const
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

IceInternal::ValueFactoryManagerI::ValueFactoryManagerI() :
    _factoryMapHint(_factoryMap.end())
{
}
