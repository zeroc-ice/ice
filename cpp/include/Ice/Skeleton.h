// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SKELETON_H
#define ICE_SKELETON_H

#include <Ice/SkeletonF.h>
#include <Ice/Shared.h>

namespace __Ice
{

class Incoming;

}

namespace IceServant { namespace Ice
{

class ICE_API ObjectS : virtual public ::__Ice::Shared
{
public:

    ObjectS();
    virtual ~ObjectS();

    virtual bool _implements(const std::string&);
    void ____implements(::__Ice::Incoming&);

    typedef void (ObjectS::*__Op)(::__Ice::Incoming&);
    static __Op __ops[];
    static std::string __names[];
    virtual bool __dispatch(::__Ice::Incoming&, const std::string&);
};

} }

#endif
