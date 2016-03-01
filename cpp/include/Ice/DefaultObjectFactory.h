// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_DEFAULT_OBJECT_FACTORY_H
#define ICE_DEFAULT_OBJECT_FACTORY_H

#include <Ice/Config.h>
#include <Ice/ObjectFactory.h>

namespace IceInternal
{
template<class O>
class DefaultObjectFactory : public Ice::ObjectFactory
{
public:
    
    DefaultObjectFactory(const ::std::string& typeId) :
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

    virtual void destroy()
    {
    }

private:
    const ::std::string _typeId;
};

}

#endif
