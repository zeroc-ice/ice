// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_LOCAL_OBJECT_H
#define ICE_LOCAL_OBJECT_H

#include <IceE/LocalObjectF.h>
#include <IceE/Shared.h>

namespace Ice
{

class ICE_API LocalObject : virtual public ::IceUtil::Shared
{
public:

    virtual bool operator==(const LocalObject&) const;
    virtual bool operator!=(const LocalObject&) const;
    virtual bool operator<(const LocalObject&) const;
    virtual ::Ice::Int ice_hash() const;
};

}

#endif
