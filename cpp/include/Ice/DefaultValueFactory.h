// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_DEFAULT_VALUE_FACTORY_H
#define ICE_DEFAULT_VALUE_FACTORY_H

#include <Ice/Config.h>
#include <Ice/ValueFactory.h>

namespace IceInternal
{

#ifdef ICE_CPP11_MAPPING

template<class V>
::std::shared_ptr<::Ice::Value>
#ifdef NDEBUG
defaultValueFactory(const std::string&)
#else
defaultValueFactory(const std::string& typeId)
#endif
{
    assert(typeId == V::ice_staticId());
    return std::make_shared<V>();
}

#else

template<class V>
class DefaultValueFactory : public Ice::ValueFactory
{
public:

    DefaultValueFactory(const ::std::string& typeId) :
        _typeId(typeId)
    {
    }

#ifndef NDEBUG
    virtual ::Ice::ObjectPtr create(const ::std::string& typeId)
#else
    virtual ::Ice::ObjectPtr create(const ::std::string&)
#endif
    {
        assert(typeId == _typeId);
        return new V;
    }

private:
    const ::std::string _typeId;
};

#endif

}
#endif
