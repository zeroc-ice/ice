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

#include <Ice/CollectorF.h>
#include <Ice/ObjectAdapterF.h>
#include <Ice/Stream.h>

namespace __Ice
{

class ICE_API Incoming
{
public:

    Incoming(const Collector&, const ::Ice::ObjectAdapter&);
    ~Incoming();

    void invoke();

    Stream* is();
    Stream* os();

private:

    Incoming(const Incoming&);
    void operator=(const Incoming&);

    Collector collector_;
    ::Ice::ObjectAdapter adapter_;

    Stream is_;
    Stream os_;
};

}

#endif
