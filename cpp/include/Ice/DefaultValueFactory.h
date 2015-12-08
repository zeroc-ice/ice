// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_DEFAULT_VALUE_FACTORY_H
#define ICE_DEFAULT_VALUE_FACTORY_H

#include <Ice/Config.h>
#include <Ice/ValueFactory.h>

#ifndef ICE_CPP11_MAPPING
namespace IceInternal
{
template<class O>
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
        return new O;
    }

private:
    const ::std::string _typeId;
};

}
#endif

#endif
