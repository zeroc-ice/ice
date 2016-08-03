// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_VIRTUAL_SHARED_H
#define ICE_VIRTUAL_SHARED_H

#include <Ice/Config.h>

namespace Ice
{

#ifdef ICE_CPP11_MAPPING // C++11

class VirtualEnableSharedFromThisBase : public std::enable_shared_from_this<VirtualEnableSharedFromThisBase>
{
public:

     virtual ~VirtualEnableSharedFromThisBase() = default;
};

template<typename T> class EnableSharedFromThis : public virtual VirtualEnableSharedFromThisBase
{
public:

    std::shared_ptr<T> shared_from_this() const
    {
        return std::dynamic_pointer_cast<T>(
            std::const_pointer_cast<VirtualEnableSharedFromThisBase>(
                VirtualEnableSharedFromThisBase::shared_from_this()));
    }

    std::weak_ptr<T> weak_from_this() const
    {
        return shared_from_this();
    }
};

#   define ICE_SHARED Ice::VirtualEnableSharedFromThisBase

#else // C++98

template<typename T> class EnableSharedFromThis : public virtual IceUtil::Shared
{
public:

    T* shared_from_this() const
    {
        return static_cast<T*>(const_cast<EnableSharedFromThis<T>*>(this));
    }
};

#   define ICE_SHARED IceUtil::Shared


#endif

}

#endif
