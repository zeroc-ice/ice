// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_INCOMING_H
#define ICE_INCOMING_H

#include <Ice/ObjectAdapterF.h>
#include <Ice/InstanceF.h>
#include <Ice/BasicStream.h>

namespace IceInternal
{

class ICE_API Incoming : public ::IceUtil::noncopyable
{
public:

    Incoming(const InstancePtr&, const ::Ice::ObjectAdapterPtr&);

    void invoke(bool);

    BasicStream* is();
    BasicStream* os();

private:

    //
    // Optimization. The adapter may not be deleted while a
    // stack-allocated Incoming still holds it.
    //
    const ::Ice::ObjectAdapterPtr& _adapter;

    BasicStream _is;
    BasicStream _os;
};

}

#endif
