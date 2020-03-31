//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_LOCAL_OBJECT_H
#define ICE_LOCAL_OBJECT_H

#ifdef ICE_BUILDING_SRC

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
