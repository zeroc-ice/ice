// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_LOCAL_OBJECT_H
#define ICE_LOCAL_OBJECT_H

#if !defined(ICE_CPP11_MAPPING) || defined(ICE_BUILDING_SRC)
//
// Part of the C++98 mapping, and "internal" definitions when building Ice
// with the C++11 mapping
//

#include <IceUtil/Shared.h>
#include <Ice/LocalObjectF.h>

namespace Ice
{

class ICE_API LocalObject : public virtual ::IceUtil::Shared
{
public:

    virtual bool operator==(const LocalObject&) const;
    virtual bool operator<(const LocalObject&) const;
};

}

#endif
#endif
