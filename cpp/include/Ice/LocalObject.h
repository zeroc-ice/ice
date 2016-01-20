// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_LOCAL_OBJECT_H
#define ICE_LOCAL_OBJECT_H

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
