// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PICKLER_H
#define ICE_PICKLER_H

#include <Ice/PicklerF.h>
#include <Ice/InstanceF.h>
#include <Ice/ObjectF.h>
#include <Ice/Shared.h>

namespace Ice
{

class ICE_API Pickler : public ::__Ice::Shared
{
public:

    void pickle(const Object_ptr&, std::ostream&) const;
    Object_ptr unpickle(std::istream&) const;

private:

    Pickler(const ::__Ice::Instance_ptr&);
    friend ::__Ice::Instance;

    ::__Ice::Instance_ptr instance_;
};

}

#endif

