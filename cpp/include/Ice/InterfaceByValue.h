// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_INTERFACE_BY_VALUE_H
#define ICE_INTERFACE_BY_VALUE_H

#include <Ice/Value.h>
#include <Ice/BasicStream.h>

#ifdef ICE_CPP11_MAPPING

namespace Ice
{

template<typename T>
class InterfaceByValue : public Value
{
public:

    virtual void
    __writeImpl(::IceInternal::BasicStream* __os) const
    {
        __os->startWriteSlice(T::ice_staticId(), -1, true);
        __os->endWriteSlice();
    }

    virtual void
    __readImpl(::IceInternal::BasicStream* __is)
    {
        __is->startReadSlice();
        __is->endReadSlice();
    }
    
    virtual const std::string& ice_id() const
    {
        return T::ice_staticId();
    }

    static const std::string& ice_staticId()
    {
        return T::ice_staticId();
    }
};

}
#endif

#endif
