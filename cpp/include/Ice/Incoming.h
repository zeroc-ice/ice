// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_INCOMING_H
#define ICE_INCOMING_H

#include <Ice/ObjectAdapterF.h>
#include <Ice/Stream.h>

namespace __Ice
{

class ICE_API Incoming : noncopyable
{
public:

    Incoming(const ::Ice::ObjectAdapter_ptr&);
    ~Incoming();

    void invoke(Stream&);

    Stream* is();
    Stream* os();

private:

    ::Ice::ObjectAdapter_ptr adapter_;

    Stream is_;
    Stream os_;
};

}

#endif
