//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/ValueFactoryManagerI.h>
#include <Ice/LocalException.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void
IceInternal::ValueFactoryManagerI::add(ValueFactoryFunc factoryFunc, const string& id)
{
    IceUtil::Mutex::Lock sync(*this);

    if((_factoryFuncMapHint != _factoryFuncMap.end() && _factoryFuncMapHint->first == id)
       || _factoryFuncMap.find(id) != _factoryFuncMap.end())
    {
        throw AlreadyRegisteredException(__FILE__, __LINE__, "value factory", id);
    }

    _factoryFuncMapHint = _factoryFuncMap.insert(_factoryFuncMapHint, make_pair(id, factoryFunc));
}

void
IceInternal::ValueFactoryManagerI::add(const ValueFactoryPtr& factory, const string& id)
{
    IceUtil::Mutex::Lock sync(*this);

    if((_factoryFuncMapHint != _factoryFuncMap.end() && _factoryFuncMapHint->first == id)
       || _factoryFuncMap.find(id) != _factoryFuncMap.end())
    {
        throw AlreadyRegisteredException(__FILE__, __LINE__, "value factory", id);
    }

    ValueFactoryFunc func = [factory](const string& type) -> shared_ptr<Value>
    {
        return factory->create(type);
    };

    _factoryFuncMapHint = _factoryFuncMap.insert(_factoryFuncMapHint, make_pair(id, func));
}

ValueFactoryFunc
IceInternal::ValueFactoryManagerI::find(const string& id) const noexcept
{
    IceUtil::Mutex::Lock sync(*this);

    FactoryFuncMap& factoryFuncMap = const_cast<FactoryFuncMap&>(_factoryFuncMap);

    FactoryFuncMap::iterator p = factoryFuncMap.end();
    if(_factoryFuncMapHint != factoryFuncMap.end())
    {
        if(_factoryFuncMapHint->first == id)
        {
            p = _factoryFuncMapHint;
        }
    }

    if(p == factoryFuncMap.end())
    {
        p = factoryFuncMap.find(id);
    }

    if(p != factoryFuncMap.end())
    {
        _factoryFuncMapHint = p;
        return p->second;
    }
    else
    {
        return nullptr;
    }
}

IceInternal::ValueFactoryManagerI::ValueFactoryManagerI() :
    _factoryFuncMapHint(_factoryFuncMap.end())
{
}
