// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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

/**
 * Base class for local Slice classes and interfaces.
 * \headerfile Ice/Ice.h
 */
class ICE_API LocalObject : public virtual ::IceUtil::Shared
{
public:

    virtual bool operator==(const LocalObject&) const;
    virtual bool operator<(const LocalObject&) const;
};

}

#endif
#endif
