// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_INTERFACE_BY_VALUE_H
#define ICE_INTERFACE_BY_VALUE_H

#include <Ice/Value.h>
#include <Ice/OutputStream.h>
#include <Ice/InputStream.h>

#ifdef ICE_CPP11_MAPPING

namespace Ice
{

template<typename T>
class InterfaceByValue : public ValueHelper<InterfaceByValue<T>, Value>
{
public:
    virtual std::string ice_id() const
    {
        return T::ice_staticId();
    }

    static const std::string& ice_staticId()
    {
        return T::ice_staticId();
    }

    std::tuple<> ice_tuple() const
    {
        return std::tie();
    }
};

}

#endif

#endif
