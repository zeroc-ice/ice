// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PICKLER_I_H
#define ICE_PICKLER_I_H

#include <IceUtil/Shared.h>
#include <Ice/Pickler.h>
#include <Ice/InstanceF.h>

namespace Ice
{

class ICE_API PicklerI : public Pickler
{
public:

    void pickle(const ObjectPtr&, std::ostream&);
    ObjectPtr unpickle(const std::string&, std::istream&);

private:

    PicklerI(const ::IceInternal::InstancePtr&);
    friend ::IceInternal::Instance;

    ::IceInternal::InstancePtr _instance;
};

}

#endif

