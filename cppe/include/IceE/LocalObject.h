// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_LOCAL_OBJECT_H
#define ICEE_LOCAL_OBJECT_H

#include <IceE/Shared.h>
#include <IceE/LocalObjectF.h>

namespace IceEInternal
{

class BasicStream;

}

namespace IceE
{

class ICEE_API LocalObject : virtual public ::IceE::Shared
{
public:

    virtual bool operator==(const LocalObject&) const;
    virtual bool operator!=(const LocalObject&) const;
    virtual bool operator<(const LocalObject&) const;
    virtual ::IceE::Int ice_hash() const;
};

}

#endif
