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

namespace _Ice
{

class ICE_API Outgoing
{
public:

    Outgoing(const Emitter&, const char*);
    ~Outgoing();

    void invoke();

    Stream* os() { return &os_; }
    Stream* is() { return &is_; }
    const char* operation() const { return operation_; }

private:

    Outgoing(const Outgoing&);
    void operator=(const Outgoing&);

    const Emitter& emitter_;
    const char* operation_;
    Stream os_;
    Stream is_;
};

}

#endif
