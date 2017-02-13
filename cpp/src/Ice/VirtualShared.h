// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_VIRTUAL_SHARED_H
#define ICE_VIRTUAL_SHARED_H

#ifdef ICE_CPP11_MAPPING

namespace IceInternal
{

class VirtualEnableSharedFromThisBase : public std::enable_shared_from_this<VirtualEnableSharedFromThisBase>
{
public:
    
    virtual ~VirtualEnableSharedFromThisBase() = default;
};

template<typename T> 
class EnableSharedFromThis : public virtual VirtualEnableSharedFromThisBase
{
public:

    std::shared_ptr<T> shared_from_this()
    {
        return std::dynamic_pointer_cast<T>(VirtualEnableSharedFromThisBase::shared_from_this());
    }

    std::shared_ptr<T const> shared_from_this() const
    {
        return std::dynamic_pointer_cast<T const>(VirtualEnableSharedFromThisBase::shared_from_this());
    }
};

}
#endif
#endif
