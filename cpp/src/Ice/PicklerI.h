// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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

