// **********************************************************************
//
// Copyright (c) 2002
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

#ifndef ICE_INCOMING_H
#define ICE_INCOMING_H

#include <Ice/InstanceF.h>
#include <Ice/BasicStream.h>
#include <Ice/Current.h>

namespace IceInternal
{

class ICE_API Incoming : public ::IceUtil::noncopyable
{
public:

    Incoming(const InstancePtr&, const ::Ice::ObjectAdapterPtr&);
    ~Incoming();

    void invoke(bool);

    BasicStream* is();
    BasicStream* os();

private:

    Ice::Current _current;

    BasicStream _is;
    BasicStream _os;
};

}

#endif
