// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_OUTGOING_H
#define ICE_OUTGOING_H

#include <Ice/EmitterF.h>
#include <Ice/Stream.h>

namespace __Ice
{

class ICE_API Outgoing
{
public:

    Outgoing(const Emitter&);
    ~Outgoing();

    void invoke();

    Stream* is();
    Stream* os();

private:

    Outgoing(const Outgoing&);
    void operator=(const Outgoing&);

    Emitter emitter_;
    Stream is_;
    Stream os_;
};

}

#endif
